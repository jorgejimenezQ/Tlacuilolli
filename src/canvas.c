#include "../include/canvas.h"
#include <stdlib.h>

bool Canvas_Init(Canvas* canvas, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;
    canvas->width  = width;
    canvas->height = height;
    canvas->window = SDL_CreateWindow(
        "Batched Triangles",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height, SDL_WINDOW_SHOWN
    );
    if (!canvas->window) return false;

    canvas->renderer = SDL_CreateRenderer(
        canvas->window, -1, SDL_RENDERER_ACCELERATED
    );
    if (!canvas->renderer) return false;

    canvas->texture = SDL_CreateTexture(
        canvas->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        width, height
    );
    if (!canvas->texture) return false;

    // Allocate front buffer
    canvas->pixels = calloc((size_t)width * height, sizeof(uint32_t));
    if (!canvas->pixels) return false;
    
    // Allocate back buffer
    canvas->backBuffer = calloc((size_t)width * height, sizeof(uint32_t));
    if (!canvas->backBuffer) {
        free(canvas->pixels);
        return false;
    }
    
    return true;
}

void Canvas_Destroy(Canvas* canvas) {
    free(canvas->pixels);
    free(canvas->backBuffer);
    SDL_DestroyTexture(canvas->texture);
    SDL_DestroyRenderer(canvas->renderer);
    SDL_DestroyWindow(canvas->window);
    SDL_Quit();
}

void Canvas_PutPixel(Canvas* canvas, int cx, int cy, Color color) {
    int sx = canvas->width/2 + cx;
    int sy = canvas->height/2 - cy;
    if (sx < 0 || sy < 0 || sx >= canvas->width || sy >= canvas->height)
        return;
    uint32_t pixel = (0xFF << 24)
                   | (color.r << 16)
                   | (color.g <<  8)
                   | (color.b <<  0);
    canvas->backBuffer[sy * canvas->width + sx] = pixel;
}

void Canvas_Update(Canvas* canvas) {
    // Upload back buffer to texture
    SDL_UpdateTexture(
        canvas->texture, NULL,
        canvas->backBuffer,
        canvas->width * sizeof(uint32_t)
    );
    
    // Render the texture to the screen
    SDL_RenderClear(canvas->renderer);
    SDL_RenderCopy(canvas->renderer, canvas->texture, NULL, NULL);
    SDL_RenderPresent(canvas->renderer);
    
    // Swap buffers
    uint32_t* tmp = canvas->pixels;
    canvas->pixels = canvas->backBuffer;
    canvas->backBuffer = tmp;
}
