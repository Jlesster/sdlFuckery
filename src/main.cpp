#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_video.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <stdio.h>

SDL_Window* window;
SDL_GPUDevice* device;
SDL_GPUTransferBuffer* transferBuffer;
SDL_GPUGraphicsPipeline* graphicsPipeline;
SDL_GPUBuffer* vBuffer;

const int wWidth = 720;
const int wHeight = 480;

struct vertex {
  float x, y, z;
  float r, g, b, a;
};

static vertex vertices[] = {
    {0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},     // top - red
    {-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},   // bottom left - green
    {0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f}     // bottom right - blue
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  // Create window
  window = SDL_CreateWindow("SDL3 Triangle", wWidth, wHeight, SDL_WINDOW_RESIZABLE);
  if (!window) {
    SDL_Log("Failed to create window: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Create GPU device
  device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
  if (!device) {
    SDL_Log("Failed to create GPU device: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Claim window for GPU
  if (!SDL_ClaimWindowForGPUDevice(device, window)) {
    SDL_Log("Failed to claim window: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Create vertex buffer
  SDL_GPUBufferCreateInfo bufferInfo = {};
  bufferInfo.size = sizeof(vertices);
  bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
  vBuffer = SDL_CreateGPUBuffer(device, &bufferInfo);
  if (!vBuffer) {
    SDL_Log("Failed to create vertex buffer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Upload vertex data
  SDL_GPUCommandBuffer* initCmd = SDL_AcquireGPUCommandBuffer(device);
  SDL_GPUCopyPass* cPass = SDL_BeginGPUCopyPass(initCmd);

  SDL_GPUTransferBufferCreateInfo transferInfo = {};
  transferInfo.size = sizeof(vertices);
  transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
  transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);

  vertex* data = (vertex*)SDL_MapGPUTransferBuffer(device, transferBuffer, false);
  SDL_memcpy(data, vertices, sizeof(vertices));
  SDL_UnmapGPUTransferBuffer(device, transferBuffer);

  SDL_GPUTransferBufferLocation location = {};
  location.transfer_buffer = transferBuffer;
  location.offset = 0;

  SDL_GPUBufferRegion region = {};
  region.buffer = vBuffer;
  region.size = sizeof(vertices);
  region.offset = 0;

  SDL_UploadToGPUBuffer(cPass, &location, &region, false);
  SDL_EndGPUCopyPass(cPass);
  SDL_SubmitGPUCommandBuffer(initCmd);

  // Load shaders
  size_t vertexCodeSize;
  void* vertexCode = SDL_LoadFile("shaders/vertex.spv", &vertexCodeSize);
  if (!vertexCode) {
    SDL_Log("Failed to load vertex shader: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_GPUShaderCreateInfo vertexInfo = {};
  vertexInfo.code = (Uint8*)vertexCode;
  vertexInfo.code_size = vertexCodeSize;
  vertexInfo.entrypoint = "main";
  vertexInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
  vertexInfo.stage = SDL_GPU_SHADERSTAGE_VERTEX;
  vertexInfo.num_samplers = 0;
  vertexInfo.num_storage_buffers = 0;
  vertexInfo.num_storage_textures = 0;
  vertexInfo.num_uniform_buffers = 0;

  SDL_GPUShader* vertexShader = SDL_CreateGPUShader(device, &vertexInfo);
  SDL_free(vertexCode);

  if (!vertexShader) {
    SDL_Log("Failed to create vertex shader: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  size_t fragmentCodeSize;
  void* fragmentCode = SDL_LoadFile("shaders/fragment.spv", &fragmentCodeSize);
  if (!fragmentCode) {
    SDL_Log("Failed to load fragment shader: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_GPUShaderCreateInfo fragmentInfo = {};
  fragmentInfo.code = (Uint8*)fragmentCode;
  fragmentInfo.code_size = fragmentCodeSize;
  fragmentInfo.entrypoint = "main";
  fragmentInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
  fragmentInfo.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
  fragmentInfo.num_samplers = 0;
  fragmentInfo.num_storage_buffers = 0;
  fragmentInfo.num_storage_textures = 0;
  fragmentInfo.num_uniform_buffers = 0;

  SDL_GPUShader* fragmentShader = SDL_CreateGPUShader(device, &fragmentInfo);
  SDL_free(fragmentCode);

  if (!fragmentShader) {
    SDL_Log("Failed to create fragment shader: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Create graphics pipeline
  SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {};

  // Shaders
  pipelineInfo.vertex_shader = vertexShader;
  pipelineInfo.fragment_shader = fragmentShader;

  // Vertex input
  SDL_GPUVertexBufferDescription vertexBufferDesc = {};
  vertexBufferDesc.slot = 0;
  vertexBufferDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
  vertexBufferDesc.instance_step_rate = 0;
  vertexBufferDesc.pitch = sizeof(vertex);

  SDL_GPUVertexAttribute vertexAttributes[2] = {};
  // Position
  vertexAttributes[0].buffer_slot = 0;
  vertexAttributes[0].location = 0;
  vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
  vertexAttributes[0].offset = 0;
  // Color
  vertexAttributes[1].buffer_slot = 0;
  vertexAttributes[1].location = 1;
  vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
  vertexAttributes[1].offset = sizeof(float) * 3;

  pipelineInfo.vertex_input_state.num_vertex_buffers = 1;
  pipelineInfo.vertex_input_state.vertex_buffer_descriptions = &vertexBufferDesc;
  pipelineInfo.vertex_input_state.num_vertex_attributes = 2;
  pipelineInfo.vertex_input_state.vertex_attributes = vertexAttributes;

  // Primitive assembly
  pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

  // Rasterizer state
  pipelineInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
  pipelineInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
  pipelineInfo.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

  // Multisample state
  pipelineInfo.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;
  pipelineInfo.multisample_state.sample_mask = 0xFFFFFFFF;

  // Color target
  SDL_GPUColorTargetDescription colorTarget = {};
  colorTarget.format = SDL_GetGPUSwapchainTextureFormat(device, window);
  colorTarget.blend_state.enable_blend = false;
  colorTarget.blend_state.color_write_mask = 0xF; // RGBA

  pipelineInfo.target_info.num_color_targets = 1;
  pipelineInfo.target_info.color_target_descriptions = &colorTarget;
  pipelineInfo.target_info.has_depth_stencil_target = false;

  graphicsPipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineInfo);

  SDL_ReleaseGPUShader(device, vertexShader);
  SDL_ReleaseGPUShader(device, fragmentShader);

  if (!graphicsPipeline) {
    SDL_Log("Failed to create graphics pipeline: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_Log("Initialization successful!");
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
  if (!cmd) {
    SDL_Log("Failed to acquire command buffer");
    return SDL_APP_CONTINUE;
  }

  SDL_GPUTexture* sTexture;
  Uint32 width, height;

  if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &sTexture, &width, &height)) {
    SDL_SubmitGPUCommandBuffer(cmd);
    return SDL_APP_CONTINUE;
  }

  if (!sTexture) {
    SDL_SubmitGPUCommandBuffer(cmd);
    return SDL_APP_CONTINUE;
  }

  // Animated background color
  const double tick = (double)(SDL_GetTicks()) / 1000;
  const float red = (float)(0.5 + 0.5 * SDL_sin(tick));
  const float green = (float)(0.5 + 0.5 * SDL_sin(tick + SDL_PI_F * 2.0f / 3.0f));
  const float blue = (float)(0.5 + 0.5 * SDL_sin(tick + SDL_PI_F * 4.0f / 3.0f));

  // Begin render pass
  SDL_GPUColorTargetInfo colorTarget = {};
  colorTarget.texture = sTexture;
  colorTarget.clear_color.r = red * 0.3f;  // Darken background so triangle is visible
  colorTarget.clear_color.g = green * 0.3f;
  colorTarget.clear_color.b = blue * 0.3f;
  colorTarget.clear_color.a = 1.0f;
  colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
  colorTarget.store_op = SDL_GPU_STOREOP_STORE;

  SDL_GPURenderPass* rPass = SDL_BeginGPURenderPass(cmd, &colorTarget, 1, NULL);

  // Bind pipeline and draw
  SDL_BindGPUGraphicsPipeline(rPass, graphicsPipeline);

  SDL_GPUBufferBinding bufferBinding = {};
  bufferBinding.buffer = vBuffer;
  bufferBinding.offset = 0;

  SDL_BindGPUVertexBuffers(rPass, 0, &bufferBinding, 1);
  SDL_DrawGPUPrimitives(rPass, 3, 1, 0, 0);

  SDL_EndGPURenderPass(rPass);
  SDL_SubmitGPUCommandBuffer(cmd);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
    return SDL_APP_SUCCESS;
  }
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  if (graphicsPipeline) SDL_ReleaseGPUGraphicsPipeline(device, graphicsPipeline);
  if (vBuffer) SDL_ReleaseGPUBuffer(device, vBuffer);
  if (transferBuffer) SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
  if (device) SDL_DestroyGPUDevice(device);
  if (window) SDL_DestroyWindow(window);
}
