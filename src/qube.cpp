#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
#include <cmath>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

SDL_Window* window;
SDL_GLContext glContext;

float wWidth = 800;
float wHeight = 600;

float angle = 0.0f;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  //video init error check
  if(!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // FIX: Set OpenGL attributes BEFORE creating window
  // These ensure we get a compatibility profile that supports legacy OpenGL
  // SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  // SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  // FIX: Request COMPATIBILITY profile (not CORE) for legacy functions like glBegin/glEnd
  // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

  //init sdl window with opengl support
  window = SDL_CreateWindow(
    "Cube Trials",
    wWidth, wHeight,
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
  );
  //window error check
  if(!window) {
    SDL_Log("Failed to init window: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  glContext = SDL_GL_CreateContext(window);
  if(!glContext) {
    SDL_Log("Failed to create GL context: %s", SDL_GetError()); // FIX: Fixed typo "LG" -> "GL"
    return SDL_APP_FAILURE;
  }
  SDL_GL_MakeCurrent(window, glContext);

  // DEBUG: Print OpenGL info to verify we have the right context
  SDL_Log("OpenGL Version: %s", glGetString(GL_VERSION));
  SDL_Log("OpenGL Vendor: %s", glGetString(GL_VENDOR));

  int profile;
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profile);
  SDL_Log("OpenGL Profile: %d (0=default, 1=core, 2=compat, 4=ES)", profile);

  //enabling depth test for 3d
  glEnable(GL_DEPTH_TEST);

  // DEBUG: Check for OpenGL errors
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    SDL_Log("OpenGL Error during init: %d", err);
  }

  return SDL_APP_CONTINUE;
}

void perspective(float fovY, float aspect, float zNear, float zFar, float* matrix) {
  float f = 1.0f / tanf(fovY * 0.5f * SDL_PI_F / 180.0f);

  // Column-major order for OpenGL
  matrix[0] = f / aspect;
  matrix[1] = 0;
  matrix[2] = 0;
  matrix[3] = 0;

  matrix[4] = 0;
  matrix[5] = f;
  matrix[6] = 0;
  matrix[7] = 0;

  matrix[8] = 0;
  matrix[9] = 0;
  matrix[10] = (zFar + zNear) / (zNear - zFar);
  matrix[11] = -1;

  matrix[12] = 0;
  matrix[13] = 0;
  // FIX: Use 2.0f instead of 2 to ensure floating-point precision
  matrix[14] = (2.0f * zFar * zNear) / (zNear - zFar);
  matrix[15] = 0;
}

void drawCube() {
  glBegin(GL_QUADS);

  //Front face (red)
  glColor3f(1.0f,  0.0f,  0.0f);
  glVertex3f(-1.0f, -1.0f,  1.0f);
  glVertex3f( 1.0f, -1.0f,  1.0f);
  glVertex3f( 1.0f,  1.0f,  1.0f);
  glVertex3f(-1.0f,  1.0f,  1.0f);

  //Back face (green)
  glColor3f( 0.0f,  1.0f,  0.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f,  1.0f, -1.0f);
  glVertex3f( 1.0f,  1.0f, -1.0f);
  glVertex3f( 1.0f, -1.0f, -1.0f);

  //Top face (blue)
  glColor3f( 0.0f,  0.0f,  1.0f);
  glVertex3f(-1.0f,  1.0f, -1.0f);
  glVertex3f(-1.0f,  1.0f,  1.0f);
  glVertex3f( 1.0f,  1.0f,  1.0f);
  glVertex3f( 1.0f,  1.0f, -1.0f);

  //Bottom face (yellow)
  glColor3f( 1.0f,  1.0f,  0.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glVertex3f( 1.0f, -1.0f, -1.0f);
  glVertex3f( 1.0f, -1.0f,  1.0f);
  glVertex3f(-1.0f, -1.0f,  1.0f);

  //Right face (magenta)
  glColor3f( 1.0f,  0.0f,  1.0f);
  glVertex3f( 1.0f, -1.0f, -1.0f);
  glVertex3f( 1.0f,  1.0f, -1.0f);
  glVertex3f( 1.0f,  1.0f,  1.0f);
  glVertex3f( 1.0f, -1.0f,  1.0f);

  //Left face (cyan)
  glColor3f( 0.0f,  1.0f,  1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f,  1.0f);
  glVertex3f(-1.0f,  1.0f,  1.0f);
  glVertex3f(-1.0f,  1.0f, -1.0f);

  glEnd();
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  // DEBUG: Print once to verify this function is being called
  static bool printed = false;
  if (!printed) {
    SDL_Log("AppIterate called - rendering should start");
    printed = true;
  }

  //Clear screen
  // FIX: Cast to int to avoid potential warnings
  glViewport(0, 0, (int)wWidth, (int)wHeight);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f); //Background color
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //set up proj-matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  float proj[16];
  perspective(60.0f, wWidth / wHeight, 1.0f, 100.0f, proj);
  glLoadMatrixf(proj);

  //set up viewmodel matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.0f, 0.0f, -5.0f);
  glRotatef(angle, 1.0f, 1.0f, 0.0f);

  //draw cube
  drawCube();

  // DEBUG: Check for OpenGL errors after drawing
  GLenum err = glGetError();
  if (err != GL_NO_ERROR && !printed) {
    SDL_Log("OpenGL Error after draw: %d", err);
  }

  //update rotation angle
  angle += 0.5f;
  if(angle >= 360.0f) {
    angle -= 360.0f;
  }

  //present to screen
  SDL_GL_SwapWindow(window);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  if(event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  }
  if(event->type == SDL_EVENT_WINDOW_RESIZED) {
    wWidth = event->window.data1;
    wHeight = event->window.data2;
  }
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  if(glContext) {
    SDL_GL_DestroyContext(glContext);
  }
  if(window) {
    SDL_DestroyWindow(window);
  }
  SDL_Quit();
}
