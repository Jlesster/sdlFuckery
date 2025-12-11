// #include <SDL3/SDL_init.h>
// #include <SDL3/SDL_opengl.h>
// #include <SDL3/SDL_video.h>
// #include <SDL3/SDL_events.h>
// #include <cmath>
// #include <iostream>
//
// float angle = 0;
// bool running = true;
//
// void perspective(float fovY, float aspect, float zNear, float zFar, float* matrix) {
//     float f = 1.0f / tanf(fovY * 0.5f * (3.14159265358979323846f / 180.0f));
//
//     // Column-major order for OpenGL
//     matrix[0]  = f / aspect;
//     matrix[1]  = 0;
//     matrix[2]  = 0;
//     matrix[3]  = 0;
//
//     matrix[4]  = 0;
//     matrix[5]  = f;
//     matrix[6]  = 0;
//     matrix[7]  = 0;
//
//     matrix[8]  = 0;
//     matrix[9]  = 0;
//     matrix[10] = (zFar + zNear) / (zNear - zFar);
//     matrix[11] = -1;
//
//     matrix[12] = 0;
//     matrix[13] = 0;
//     matrix[14] = (2.0f * zFar * zNear) / (zNear - zFar);
//     matrix[15] = 0;
// }
//
// void drawCube() {
//     glBegin(GL_QUADS);
//     // Front
//     glColor3f(1, 0, 0);
//     glVertex3f(-1, -1,  1);
//     glVertex3f( 1, -1,  1);
//     glVertex3f( 1,  1,  1);
//     glVertex3f(-1,  1,  1);
//     // Back
//     glColor3f(0, 1, 0);
//     glVertex3f(-1, -1, -1);
//     glVertex3f(-1,  1, -1);
//     glVertex3f( 1,  1, -1);
//     glVertex3f( 1, -1, -1);
//     // Top
//     glColor3f(0, 0, 1);
//     glVertex3f(-1, 1, -1);
//     glVertex3f(-1, 1,  1);
//     glVertex3f( 1, 1,  1);
//     glVertex3f( 1, 1, -1);
//     // Bottom
//     glColor3f(1, 1, 0);
//     glVertex3f(-1, -1, -1);
//     glVertex3f( 1, -1, -1);
//     glVertex3f( 1, -1,  1);
//     glVertex3f(-1, -1,  1);
//     // Right
//     glColor3f(1, 0, 1);
//     glVertex3f(1, -1, -1);
//     glVertex3f(1,  1, -1);
//     glVertex3f(1,  1,  1);
//     glVertex3f(1, -1,  1);
//     // Left
//     glColor3f(0, 1, 1);
//     glVertex3f(-1, -1, -1);
//     glVertex3f(-1, -1,  1);
//     glVertex3f(-1,  1,  1);
//     glVertex3f(-1,  1, -1);
//     glEnd();
// }
//
// int main() {
//     if (!SDL_Init(SDL_INIT_VIDEO)) {
//         std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
//         return 1;
//     }
//
//     // Set OpenGL attributes for compatibility profile BEFORE creating window
//     // Try requesting OpenGL 2.1 which should give compatibility by default
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
//     SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
//     SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
//
//     SDL_Window* window = SDL_CreateWindow("SDL3 OpenGL Cube",
//         800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
//     if (!window) {
//         std::cerr << "CreateWindow Error: " << SDL_GetError() << "\n";
//         SDL_Quit();
//         return 1;
//     }
//
//     SDL_GLContext glContext = SDL_GL_CreateContext(window);
//     if (!glContext) {
//         std::cerr << "GL Context Error: " << SDL_GetError() << "\n";
//         SDL_DestroyWindow(window);
//         SDL_Quit();
//         return 1;
//     }
//
//     SDL_GL_MakeCurrent(window, glContext);
//
//     // Debug: Check OpenGL version and features
//     std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";
//     std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << "\n";
//     std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << "\n";
//
//     // Verify we got compatibility profile
//     int profile;
//     SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profile);
//     std::cout << "OpenGL Profile: " << profile << " (0=default, 1=core, 2=compat, 4=ES)\n";
//
//     int major, minor;
//     SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
//     SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
//     std::cout << "OpenGL Context Version: " << major << "." << minor << "\n";
//
//     // Enable depth test AFTER context is current
//     glEnable(GL_DEPTH_TEST);
//     glDepthFunc(GL_LESS);
//
//     // Check for OpenGL errors
//     GLenum err = glGetError();
//     if (err != GL_NO_ERROR) {
//         std::cerr << "OpenGL Error during setup: " << err << "\n";
//     }
//
//     // Declare the event variable
//     SDL_Event event;
//
//     int wWidth = 800;
//     int wHeight = 600;
//
//     // Main loop
//     while (running) {
//         // Handle events
//         while (SDL_PollEvent(&event)) {
//             if (event.type == SDL_EVENT_QUIT) {
//                 running = false;
//             }
//             if (event.type == SDL_EVENT_WINDOW_RESIZED) {
//                 wWidth = event.window.data1;
//                 wHeight = event.window.data2;
//             }
//         }
//
//         // Clear screen
//         glViewport(0, 0, wWidth, wHeight);
//         glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
//         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//         // Set up projection matrix
//         glMatrixMode(GL_PROJECTION);
//         glLoadIdentity();
//         float proj[16];
//         perspective(60.0f, (float)wWidth / (float)wHeight, 1.0f, 100.0f, proj);
//         glLoadMatrixf(proj);
//
//         // Set up modelview matrix
//         glMatrixMode(GL_MODELVIEW);
//         glLoadIdentity();
//         glTranslatef(0.0f, 0.0f, -5.0f);
//         glRotatef(angle, 1.0f, 1.0f, 0.0f);
//
//         // Draw the cube
//         drawCube();
//
//         // Check for OpenGL errors after drawing
//         GLenum err = glGetError();
//         if (err != GL_NO_ERROR) {
//             std::cerr << "OpenGL Error after draw: " << err << "\n";
//         };
//
//         // Update rotation
//         angle += 0.5f;
//         if (angle >= 360.0f) {
//             angle -= 360.0f;
//         }
//
//         // Swap buffers
//         SDL_GL_SwapWindow(window);
//     }
//
//     // Cleanup
//     SDL_GL_DestroyContext(glContext);
//     SDL_DestroyWindow(window);
//     SDL_Quit();
//
//     return 0;
// }
