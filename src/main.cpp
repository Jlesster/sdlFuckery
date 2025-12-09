#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>

//init window and gpu
Uint32 width, height;
SDL_Window* window;
SDL_GPUDevice* device;
SDL_GPUTexture* swapTexture;
SDL_GPURenderPass* renderPass;
SDL_GPUCommandBuffer* cmd;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  //creating window
  window = SDL_CreateWindow("Hello maybe triangle", 720, 480, 0);

  //creating device
  device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
  SDL_ClaimWindowForGPUDevice(device, window);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
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

  //get ticks and convert from ms to s
  const double tick = ((double)SDL_GetTicks()) / 1000.0;
  //make sine wave between each color using tick as basis
  const float red = (float) (0.7 + 0.7 * SDL_sin(tick));
  const float blue = (float) (0.7 + 0.7 * SDL_sin(tick + SDL_PI_D * 2 / 3));
  const float green = (float) (0.7 + 0.7 * SDL_sin(tick + SDL_PI_D * 4 / 3));

  //create the color target                                     1
  SDL_GPUColorTargetInfo colorTargetInfo{};
  colorTargetInfo.clear_color = {red, blue, green, 255/255.0f};
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

