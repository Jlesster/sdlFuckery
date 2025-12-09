1iinclude <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_video.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

SDL_Window* window;
SDL_GPUCommandBuffer* cmd;
SDL_GPUDevice* device;
SDL_GPURenderPass* rPass;
SDL_GPUCopyPass* cPass;
SDL_GPUTransferBuffer* transferBuffer;
SDL_GPUTexture* sTexture;
SDL_GPUBuffer* vBuffer;
Uint32 width, height;

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
  cPass = SDL_BeginGPUCopyPass(cmd);

  SDL_GPUBufferCreateInfo bufferInfo{};
  bufferInfo.size = sizeof(vertices);
  bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
  vBuffer = SDL_CreateGPUBuffer(device, &bufferInfo);

  SDL_GPUTransferBufferCreateInfo transferInfo{};
  transferInfo.size = sizeof(vertices);
  transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
  transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);

  SDL_ClaimWindowForGPUDevice(device, window);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  cmd = SDL_AcquireGPUCommandBuffer(device);
  SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &sTexture, &width, &height);

  if(window == NULL) {
    SDL_SubmitGPUCommandBuffer(cmd);
    return SDL_APP_SUCCESS;
  }

  const double tick = (double)(SDL_GetTicks()) / 1000;
  const float red = (float)(0.9 + 0.9 * SDL_sin(tick));
  const float blue = (float)(0.9 + 0.9 * SDL_sin(tick + SDL_PI_F * 2 / 3));
  const float green = (float)(0.9 + 0.9 * SDL_sin(tick + SDL_PI_F * 4 / 3));

  SDL_GPUColorTargetInfo cTI{};
  cTI.clear_color = {red, blue, green, 255/255.0f};
  cTI.load_op = SDL_GPU_LOADOP_CLEAR;
  cTI.store_op = SDL_GPU_STOREOP_STORE;
  cTI.texture = sTexture;

  rPass = SDL_BeginGPURenderPass(cmd, &cTI, 1, NULL);
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
  SDL_DestroyWindow(window);
  SDL_DestroyGPUDevice(device);
  SDL_ReleaseGPUBuffer(device, vBuffer);
  SDL_Quit();
}
