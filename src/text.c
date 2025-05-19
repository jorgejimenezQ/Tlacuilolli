#include "../include/text.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>  // For getcwd

// Font handle
static TTF_Font* font = NULL;

bool textInit(const char* fontFilename, int fontSize) {
    // Check if TTF was initialized
    if (!TTF_WasInit()) {
        fprintf(stderr, "SDL_ttf not initialized before textInit()\n");
        return false;
    }
    
    // Construct the full path to the font file
    // First try with relative path from current directory
    char fontPath[256];
    snprintf(fontPath, sizeof(fontPath), "assets/fonts/%s", fontFilename);
    
    // Try to load the font
    font = TTF_OpenFont(fontPath, fontSize);
    
    // If that fails, try with full path
    if (!font) {
        fprintf(stderr, "Failed to load font from '%s': %s\n", fontPath, TTF_GetError());
        // Try with absolute path (assuming executable is in project root)
        char cwd[256];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {  // getcwd returns NULL on error
            snprintf(fontPath, sizeof(fontPath), "%s/assets/fonts/%s", cwd, fontFilename);
            fprintf(stderr, "Trying absolute path: %s\n", fontPath);
            font = TTF_OpenFont(fontPath, fontSize);
        }
    }
    
    // Check if font loaded
    if (!font) {
        fprintf(stderr, "Failed to load font '%s': %s\n", fontPath, TTF_GetError());
        return false;
    }
    
    fprintf(stderr, "Font loaded successfully: %s\n", fontPath);
    return true;
}

void textDraw(Canvas* canvas, int cx, int cy, const char* text, Color color) {
    // Safety checks
    if (!font) {
        fprintf(stderr, "Error: Font not loaded in textDraw\n");
        return;
    }
    if (!text || text[0] == '\0') {
        fprintf(stderr, "Error: Empty text passed to textDraw\n");
        return;
    }
    if (!canvas) {
        fprintf(stderr, "Error: Null canvas passed to textDraw\n");
        return;
    }
    
    // Use blended rendering for nice antialiased text
    SDL_Color sdlColor = {color.r, color.g, color.b, 255};
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, sdlColor);
    
    if (!surface) {
        fprintf(stderr, "Failed to render text: '%s'\n", TTF_GetError());
        return;
    }
    
    // Calculate text dimensions and position
    int textWidth = surface->w;
    int textHeight = surface->h;
    
    // Convert from centered coordinates to top-left origin
    // Our canvas origin is in the center, SDL's surface origin is top-left
    int x = canvas->width / 2 + cx - textWidth / 2;   // Center text horizontally around cx
    int y = canvas->height / 2 - cy - textHeight / 2; // And vertically around cy
    
    // Check if completely off-screen
    if (x + textWidth < 0 || y + textHeight < 0 || 
        x >= canvas->width || y >= canvas->height) {
        SDL_FreeSurface(surface);
        return;
    }
    
    // Get format information
    SDL_PixelFormat* format = surface->format;
    
    // Only proceed if surface is in a recognized format
    if (format->BytesPerPixel != 4) {
        fprintf(stderr, "Unsupported surface format: %d bytes per pixel\n", format->BytesPerPixel);
        SDL_FreeSurface(surface);
        return;
    }
    
    // Access pixel data
    Uint32* src = (Uint32*)surface->pixels;
    
    // Copy pixels to canvas, converting from SDL format to our format
    for (int sy = 0; sy < textHeight; sy++) {
        for (int sx = 0; sx < textWidth; sx++) {
            // Get source pixel in SDL format
            int srcIndex = sy * (surface->pitch / 4) + sx;
            Uint32 pixel = src[srcIndex];
            
            // Extract RGBA components
            Uint8 r, g, b, a;
            SDL_GetRGBA(pixel, format, &r, &g, &b, &a);
            
            // Skip fully transparent pixels
            if (a < 10) continue;
            
            // Create a blended color based on alpha
            // Adjust the alpha scaling to taste
            Color pixelColor;
            if (a >= 250) {
                // Fully opaque - use the original color
                pixelColor = color;
            } else {
                // Alpha-scale the original color
                float alphaFactor = a / 255.0f;
                pixelColor.r = (uint8_t)(color.r * alphaFactor);
                pixelColor.g = (uint8_t)(color.g * alphaFactor);
                pixelColor.b = (uint8_t)(color.b * alphaFactor);
            }
            
            // Calculate position in canvas coordinates
            // Convert from top-left origin to centered origin
            int canvasX = x + sx - canvas->width / 2;
            int canvasY = canvas->height / 2 - (y + sy);
            
            // Put the pixel on the canvas
            Canvas_PutPixel(canvas, canvasX, canvasY, pixelColor);
        }
    }
    
    // Clean up
    SDL_FreeSurface(surface);
}

void textShutdown(void) {
    if (font) {
        TTF_CloseFont(font);
        font = NULL;
    }
}
