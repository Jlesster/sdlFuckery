#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_video.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

SDL_Window* window;
SDL_GPUDevice* device;
SDL_GPUBuffer* vBuffer;
SDL_GPUTransferBuffer* transferBuffer;
SDL_GPUGraphicsPipeline* graphicsPipeline;

const int wWidth = 720;
const int wHeight = 480;

struct vertex {
  float x, y, z;
  float r, g, b, a;
};

static vertex vertices[]
{
    //Triangle
    {0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f}, //Top vertex
    {-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}, //Bottom left vertex
    {0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f}, //Bottom right vertex

    //Square
    {-0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},    // top  left vertex
    {-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f},   // bottom left vertex
    {0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f},     // top right vertex
    //Square pt 2
    {0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f},     // top right
    {-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f},   // bottom left vertex
    {0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}     // bottom right vertex
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  window = SDL_CreateWindow("Base", wWidth, wHeight, SDL_WINDOW_RESIZABLE);
  if(!window) {
    SDL_Log("Failed to init window");
    return SDL_APP_FAILURE;
  }

  device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
  if(!device) {
    SDL_Log("Failed to init device");
    return SDL_APP_FAILURE;
  }

  if(!SDL_ClaimWindowForGPUDevice(device, window)) {
    SDL_Log("Failed to claim window");
    return SDL_APP_FAILURE;
  }

  SDL_GPUBufferCreateInfo bufferInfo{};
  bufferInfo.size = sizeof(vertices);
  bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
  vBuffer = SDL_CreateGPUBuffer(device, &bufferInfo);

  SDL_GPUCommandBuffer* initCmd = SDL_AcquireGPUCommandBuffer(device);
  SDL_GPUCopyPass* cPass = SDL_BeginGPUCopyPass(initCmd);

  SDL_GPUTransferBufferCreateInfo transferInfo{};
  transferInfo.size = sizeof(vertices);
  transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
  transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);

  vertex* data = (vertex*)SDL_MapGPUTransferBuffer(device, transferBuffer, false);
  SDL_memcpy(data, vertices, sizeof(vertices));
  SDL_UnmapGPUTransferBuffer(device, transferBuffer);

  data[0] = vertices[0];
  data[1] = vertices[1];
  data[2] = vertices[2];

  SDL_GPUTransferBufferLocation location{};
  location.transfer_buffer = transferBuffer;
  location.offset = 0;

  SDL_GPUBufferRegion region{};
  region.buffer = vBuffer;
  region.size = sizeof(vertices);
  region.offset = 0;

  SDL_UploadToGPUBuffer(cPass, &location, &region, true);
  SDL_EndGPUCopyPass(cPass);
  SDL_SubmitGPUCommandBuffer(initCmd);

  size_t vertexCodeSize;
  void* vertexCode = SDL_LoadFile("shaders/vertex.spv", &vertexCodeSize);
  if(!vertexCode) {
    SDL_Log("Failed to load vertex shader");
    return SDL_APP_FAILURE;
  }

  SDL_GPUShaderCreateInfo vertexInfo{};
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

  if(!vertexShader) {
    SDL_Log("Failed initialising vertex shader");
    return SDL_APP_FAILURE;
  }

  size_t fragmentCodeSize;
  void* fragmentCode = SDL_LoadFile("shaders/fragment.spv", &fragmentCodeSize);
  if(!fragmentCode) {
    SDL_Log("Failed to load fragment shader");
  }

  SDL_GPUShaderCreateInfo fragmentInfo{};
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

  if(!fragmentCode) {
    SDL_Log("Failed to initialise fragment shader");
    return SDL_APP_FAILURE;
  }

  SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.vertex_shader = vertexShader;
  pipelineInfo.fragment_shader = fragmentShader;
  pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

  SDL_GPUVertexBufferDescription vertexBufferDescriptions[1];
  vertexBufferDescriptions[0].slot = 0;
  vertexBufferDescriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
  vertexBufferDescriptions[0].instance_step_rate = 0;
  vertexBufferDescriptions[0].pitch = sizeof(vertex);

  SDL_GPUVertexAttribute vertexAttributes[2];
  vertexAttributes[0].buffer_slot = 0;
  vertexAttributes[0].location = 0;
  vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
  vertexAttributes[0].offset = 0;

  vertexAttributes[1].buffer_slot = 0;
  vertexAttributes[1].location = 1;
  vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
  vertexAttributes[1].offset = sizeof(float) * 3;

  pipelineInfo.vertex_input_state.num_vertex_buffers = 1;
  pipelineInfo.vertex_input_state.vertex_buffer_descriptions = vertexBufferDescriptions;
  pipelineInfo.vertex_input_state.num_vertex_attributes = 2;
  pipelineInfo.vertex_input_state.vertex_attributes = vertexAttributes;

  SDL_GPUColorTargetDescription cTDs[1];
  cTDs[0] = {};
  cTDs[0].blend_state.enable_blend = false;
  cTDs[0].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
  cTDs[0].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
  cTDs[0].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
  cTDs[0].blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
  cTDs[0].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
  cTDs[0].blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
  cTDs[0].format = SDL_GetGPUSwapchainTextureFormat(device, window);

  pipelineInfo.target_info.num_color_targets = 1;
  pipelineInfo.target_info.color_target_descriptions = cTDs;

  graphicsPipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineInfo);

  SDL_ReleaseGPUShader(device, vertexShader);
  SDL_ReleaseGPUShader(device,fragmentShader);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);

  SDL_GPUTexture* sTexture;
  Uint32 width, height;

  // SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &sTexture, &width, &height);
  if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &sTexture, &width, &height)) {
    SDL_SubmitGPUCommandBuffer(cmd);
    return SDL_APP_CONTINUE;  // Skip this frame
  }

  const double tick = (double)(SDL_GetTicks()) / 1000;
  const float red = (float)(0.4 + 0.4 * SDL_sin(tick));
  const float blue = (float)(0.4 + 0.4 * SDL_sin(tick + SDL_PI_F * 2 / 3));
  const float green = (float)(0.4 + 0.4 * SDL_sin(tick + SDL_PI_F * 4 / 3));

  SDL_GPUColorTargetInfo cTI{};
  cTI.clear_color = {red, blue, green, 255/255.0f};
  cTI.load_op = SDL_GPU_LOADOP_CLEAR;
  cTI.store_op = SDL_GPU_STOREOP_STORE;
  cTI.texture = sTexture;

  SDL_GPURenderPass* rPass = SDL_BeginGPURenderPass(cmd, &cTI, 1, NULL);
  SDL_BindGPUGraphicsPipeline(rPass, graphicsPipeline);
  SDL_GPUBufferBinding bufferBindings[1];
  bufferBindings[0].buffer = vBuffer;
  bufferBindings[0].offset = 0;
  SDL_BindGPUVertexBuffers(rPass, 0, bufferBindings, 1);
  SDL_DrawGPUPrimitives(rPass, 6, 1, 0, 0);

  SDL_EndGPURenderPass(rPass);
  SDL_SubmitGPUCommandBuffer(cmd);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  if(event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
    return SDL_APP_SUCCESS;
  }

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  SDL_ReleaseGPUBuffer(device, vBuffer);
  SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
  SDL_ReleaseGPUGraphicsPipeline(device, graphicsPipeline);

  SDL_DestroyGPUDevice(device);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
