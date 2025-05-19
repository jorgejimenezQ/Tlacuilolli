#include "../include/engine.h"
#include "../include/input.h"
#include "../include/canvas.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// Layer management
static Layer* layers[32];
static int layerCount = 0;

// Global canvas that the game uses for drawing
static Canvas canvas;
static int targetFPS = 60;
static bool isRunning = true;

// Internal function to process SDL events and handle window close
static void processEngineEvents(void) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) isRunning = false;
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) isRunning = false;
    }
}

int runEngine(const char* title, int width, int height, int fps) {
    // Initialize SDL and create canvas
    if (!Canvas_Init(&canvas, width, height)) {
        fprintf(stderr, "Failed to initialize canvas\n");
        return 1;
    }
    
    // Initialize SDL_ttf
    if (TTF_Init() != 0) {
        fprintf(stderr, "Failed to initialize SDL_ttf: %s\n", TTF_GetError());
        Canvas_Destroy(&canvas);
        return 1;
    }
    
    // Change window title
    SDL_SetWindowTitle(canvas.window, title);
    
    // Set target FPS
    targetFPS = fps;
    Uint32 frameTargetTime = 1000 / targetFPS;
    
    // Initialize input system
    inputInit(width, height);
    
    // Call user's setup function
    setup();
    
    // Main game loop variables
    Uint32 frameStart, frameTime;
    Uint32 previousTime = SDL_GetTicks();
    
    // Main game loop
    while (isRunning) {
        frameStart = SDL_GetTicks();
        
        // Process SDL events
        processEngineEvents();
        
        // Update input state
        inputUpdate();
        
        // Compute delta time
        Uint32 currentTime = SDL_GetTicks();
        Uint32 deltaTime = currentTime - previousTime;
        previousTime = currentTime;
        float dt = deltaTime / 1000.0f;
        
        // Clear back buffer once at the beginning of frame
        memset(canvas.backBuffer, 0, canvas.width * canvas.height * sizeof(uint32_t));
        
        // Call update function for each enabled layer
        for (int i = 0; i < layerCount; i++) {
            if (layers[i]->enabled) layers[i]->update(dt);
        }
        
        // Call render function for each enabled layer in order (background to foreground)
        // This ensures layers render on top of each other correctly
        for (int i = 0; i < layerCount; i++) {
            if (layers[i]->enabled) layers[i]->render();
        }
        
        // Present the frame
        Canvas_Update(&canvas);
        
        // Cap the frame rate
        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < frameTargetTime) {
            SDL_Delay(frameTargetTime - frameTime);
        }
    }
    
    // Clean up resources
    textShutdown();
    TTF_Quit();
    Canvas_Destroy(&canvas);
    return 0;
}

// Function to get the canvas for drawing
Canvas* getCanvas(void) {
    return &canvas;
}

// Register a new layer
void registerLayer(Layer* layer) {
    if (layerCount >= 32) {
        fprintf(stderr, "Error: Maximum number of layers (32) exceeded\n");
        return;
    }
    
    // Check if layer with same name already exists
    for (int i = 0; i < layerCount; i++) {
        if (strcmp(layers[i]->name, layer->name) == 0) {
            fprintf(stderr, "Error: Layer with name '%s' already exists\n", layer->name);
            return;
        }
    }
    
    // Add the layer to the array
    layers[layerCount++] = layer;
}

// Unregister a layer by name
void unregisterLayer(const char* name) {
    for (int i = 0; i < layerCount; i++) {
        if (strcmp(layers[i]->name, name) == 0) {
            // Remove layer by shifting all subsequent layers down
            for (int j = i; j < layerCount - 1; j++) {
                layers[j] = layers[j+1];
            }
            layerCount--;
            return;
        }
    }
    
    fprintf(stderr, "Warning: Attempted to unregister nonexistent layer '%s'\n", name);
}

// Enable or disable a layer by name
void setLayerEnabled(const char* name, bool enabled) {
    for (int i = 0; i < layerCount; i++) {
        if (strcmp(layers[i]->name, name) == 0) {
            layers[i]->enabled = enabled;
            return;
        }
    }
    
    fprintf(stderr, "Warning: Attempted to set enabled state for nonexistent layer '%s'\n", name);
}
