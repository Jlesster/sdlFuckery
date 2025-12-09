// #include <SDL3/SDL_events.h>
// #include <SDL3/SDL_gpu.h>
// #define SDL_MAIN_USE_CALLBACKS
// #include <SDL3/SDL.h>
// #include <SDL3/SDL_main.h>
//
// SDL_Window* window;
// SDL_GPUDevice* device;
// SDL_GPUCommandBuffer* cmd;
// SDL_GPURenderPass* rPass;
// SDL_GPUTexture* sTexture;
//
// Uint32 width, height;
// const int wWidth = 720;
// const int wHeight = 480;
//
// SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv){
//   window = SDL_CreateWindow("Basic window", wWidth, wHeight, SDL_WINDOW_MOUSE_GRABBED | SDL_WINDOW_RESIZABLE);
//   device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
//   SDL_ClaimWindowForGPUDevice(device, window);
//
//   return SDL_APP_CONTINUE;
// }
//
// SDL_AppResult SDL_AppIterate(void *appstate) {
//   cmd = SDL_AcquireGPUCommandBuffer(device);
//   SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &sTexture, &width, &height);
//
//   if(sTexture == NULL) {
//     SDL_SubmitGPUCommandBuffer(cmd);
//     return SDL_APP_CONTINUE;
//   }
//   const double tick = ((double) SDL_GetTicks()) / 1000.0;
//   const float red = (float) (0.9 + 0.4 * SDL_sin(tick));
//   const float green = (float) (0.9 + 0.4 * SDL_sin(tick + SDL_PI_F * 2 / 3));
//   const float blue = (float) (0.9 + 0.4 * SDL_sin(tick + SDL_PI_F * 4 / 3));
//
//   SDL_GPUColorTargetInfo targetInfo{};
//   targetInfo.clear_color = {red, blue, green, 255/255.0f};
//   targetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
//   targetInfo.store_op = SDL_GPU_STOREOP_STORE;
//   targetInfo.texture = sTexture;
//
//   rPass = SDL_BeginGPURenderPass(cmd, &targetInfo, 1, NULL);
//
//   SDL_EndGPURenderPass(rPass);
//
//   SDL_SubmitGPUCommandBuffer(cmd);
//
//   return SDL_APP_CONTINUE;
// }
//
// SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
//   if(event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
//     return SDL_APP_SUCCESS;
//   }
//
//   return SDL_APP_CONTINUE;
// }
//
// void SDL_AppQuit(void *appstate, SDL_AppResult result) {
//   SDL_DestroyWindow(window);
//   SDL_DestroyGPUDevice(device);
//   SDL_Quit();
// }
