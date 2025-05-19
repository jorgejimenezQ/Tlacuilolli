#ifndef CANVAS_H
#define CANVAS_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdbool.h>

// Simple RGB color
typedef struct {
    uint8_t r, g, b;
} Color;

// Canvas that wraps an SDL window/renderer/texture and two pixel buffers (double-buffering)
typedef struct {
    SDL_Window*   window;
    SDL_Renderer* renderer;
    SDL_Texture*  texture;
    uint32_t*     pixels;     // Front buffer (currently uploaded to texture)
    uint32_t*     backBuffer; // Back buffer (current drawing target)
    int           width;
    int           height;
} Canvas;

// Initialize SDL, create window & renderer & streaming texture, allocate pixels[]
bool Canvas_Init(Canvas* canvas, int width, int height);

// Free pixel buffer and SDL objects
void Canvas_Destroy(Canvas* canvas);

// Put a single pixel at center-origin coords (cx, cy) with an r8g8b8 color
void Canvas_PutPixel(Canvas* canvas, int cx, int cy, Color color);

// Upload backBuffer to texture, swap buffers, and render to the screen
void Canvas_Update(Canvas* canvas);

#endif // CANVAS_H
