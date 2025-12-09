#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_video.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

SDL_Window* window;
SDL_GPUDevice* device;
SDL_GPUTransferBuffer* transferBuffer;
SDL_GPUBuffer* vBuffer;

const int wWidth = 720;
const int wHeight = 480;

struct vertex {
  float x, y, z;
  float r, g, b, a;
};

static vertex vertices[]
{
    {0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},     // top vertex
    {-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f},   // bottom left vertex
    {0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f}     // bottom right vertex
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  window = SDL_CreateWindow("Base", wWidth, wHeight, SDL_WINDOW_RESIZABLE);
  device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
  SDL_GPUCommandBuffer* initCmd = SDL_AcquireGPUCommandBuffer(device);
  SDL_GPUCopyPass* cPass = SDL_BeginGPUCopyPass(initCmd);

  SDL_GPUBufferCreateInfo bufferInfo{};
  bufferInfo.size = sizeof(vertices);
  bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
  vBuffer = SDL_CreateGPUBuffer(device, &bufferInfo);

  SDL_GPUTransferBufferCreateInfo transferInfo{};
  transferInfo.size = sizeof(vertices);
  transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
  transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);

  vertex* data = (vertex*)SDL_MapGPUTransferBuffer(device, transferBuffer, false);

  data[0] = vertices[0];
  data[1] = vertices[1];
  data[2] = vertices[2];

  SDL_UnmapGPUTransferBuffer(device, transferBuffer);

  SDL_GPUTransferBufferLocation location{};
  location.transfer_buffer = transferBuffer;
  location.offset = 0;

  SDL_GPUBufferRegion region{};
  region.buffer = vBuffer;
  region.size = sizeof(vertices);
  region.offset = 0;

  size_t vertexCodeSize;
  void* vertexCode = SDL_LoadFile("shaders/vertex.spv", &vertexCodeSize);

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

  size_t fragmentCodeSize;
  void* fragmentCode = SDL_LoadFile("shaders/fragment.spv", &fragmentCodeSize);

  SDL_GPUShaderCreateInfo fragmentInfo{};
  fragmentInfo.code = (Uint8*)fragmentCode;
  fragmentInfo.code_size = fragmentCodeSize;
  fragmentInfo.entrypoint = "main";
  fragmentInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
  fragmentInfo.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
  fragmentInfo.num_storage_buffers = 0;
  fragmentInfo.num_storage_textures = 0;
  fragmentInfo.num_uniform_buffers = 0;
  SDL_GPUShader* fragmentShader = SDL_CreateGPUShader(device, &fragmentInfo);
  SDL_free(fragmentCode);

  SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.vertex_shader = vertexShader;
  pipelineInfo.fragment_shader = fragmentShader;
  pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

  SDL_GPUVertexBufferDescription vertexBufferDescriptions[1];
  vertexBufferDescriptions[0].slot = 0;
  vertexBufferDescriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
  vertexBufferDescriptions[0].instance_step_rate = 0;
  vertexBufferDescriptions[0].pitch = sizeof(vertex);

  pipelineInfo.vertex_input_state.num_vertex_buffers = 1;
  pipelineInfo.vertex_input_state.vertex_buffer_descriptions = vertexBufferDescriptions;

  SDL_GPUVertexAttribute vertexAttributes[2];
  vertexAttributes[0].buffer_slot = 0;
  vertexAttributes[0].location = 0;
  vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
  vertexAttributes[0].offset = 0;

  vertexAttributes[1].buffer_slot = 0;
  vertexAttributes[1].location = 1;
  vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
  vertexAttributes[1].offset = sizeof(float) * 3;

  pipelineInfo.vertex_input_state.num_vertex_attributes = 3;
  pipelineInfo.vertex_input_state.vertex_attributes = vertexAttributes;

  SDL_ReleaseGPUShader(device, vertexShader);
  SDL_ReleaseGPUShader(device,fragmentShader);

  SDL_UploadToGPUBuffer(cPass, &location, &region, true);
  SDL_EndGPUCopyPass(cPass);
  SDL_ClaimWindowForGPUDevice(device, window);
  SDL_SubmitGPUCommandBuffer(initCmd);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);

  SDL_GPUTexture* sTexture;
  Uint32 width, height;

  SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &sTexture, &width, &height);

  const double tick = (double)(SDL_GetTicks()) / 1000;
  const float red = (float)(0.9 + 0.9 * SDL_sin(tick));
  const float blue = (float)(0.9 + 0.9 * SDL_sin(tick + SDL_PI_F * 2 / 3));
  const float green = (float)(0.9 + 0.9 * SDL_sin(tick + SDL_PI_F * 4 / 3));

  SDL_GPUColorTargetInfo cTI{};
  cTI.clear_color = {red, blue, green, 255/255.0f};
  cTI.load_op = SDL_GPU_LOADOP_CLEAR;
  cTI.store_op = SDL_GPU_STOREOP_STORE;
  cTI.texture = sTexture;

  SDL_GPURenderPass* rPass = SDL_BeginGPURenderPass(cmd, &cTI, 1, NULL);
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

  SDL_DestroyGPUDevice(device);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
