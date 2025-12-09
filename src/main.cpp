#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>

SDL_Window *window;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  window = SDL_CreateWindow("Hello maybe triangle", 720, 480, 0);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void **appstate, int argc, char **argv) {

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
}
