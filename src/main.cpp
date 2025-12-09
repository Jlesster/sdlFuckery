#include <SDL3/SDL_gpu.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>

//init window and gpu
SDL_Window* window;
SDL_GPUDevice* device;
SDL_GPUCommandBuffer* cmd;
SDL_GPUTexture* swapTexture;
SDL_GPURenderPass* renderPass;

Uint32 width, height;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  //creating window
  window = SDL_CreateWindow("Hello maybe triangle", 720, 480, 0);

  //creating device
  device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
  SDL_ClaimWindowForGPUDevice(device, window);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void **appstate, int argc, char **argv) {
  //creating command buffer (gpu command array)
  cmd = SDL_AcquireGPUCommandBuffer(device);

  //get the swapchain texture
  SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &swapTexture, &width, &height);

  //end fram early if no swapchain texture
  if(swapTexture == NULL) {
    //always submint the command buffer after new instructions
    SDL_SubmitGPUCommandBuffer(cmd);
    return SDL_APP_CONTINUE;
  }

  //create the color target
  SDL_GPUColorTargetInfo colorTargetInfo{};
  colorTargetInfo.clear_color = {240/255.0f, 240/255.0f, 240/255.0f, 255/255.0f};
  colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
  colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
  colorTargetInfo.texture = swapTexture;

  //begin render pass
  renderPass = SDL_BeginGPURenderPass(cmd, &colorTargetInfo, 1, NULL);

  //drawing

  //end render pass
  SDL_EndGPURenderPass(renderPass);

  //sending command buffer to gpu
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
  //destroying device and window
  SDL_DestroyGPUDevice(device);
  SDL_DestroyWindow(window);
}
