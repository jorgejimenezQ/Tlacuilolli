#ifndef ENGINE_H
#define ENGINE_H

#include "canvas.h"
#include "text.h"
#include <stdbool.h>

// Called once at startup
void setup(void);

// Called every frame with elapsed time (seconds)
void update(float dt);

// Starts the SDL2 loop: window title, size, and target FPS
int runEngine(const char* title, int width, int height, int fps);

// Get access to the canvas for drawing
Canvas* getCanvas(void);

typedef struct Layer {
  const char* name;
  void (*update)(float dt);
  void (*render)(void);
  bool enabled;
} Layer;

// Layer management
void registerLayer(Layer* layer);
void unregisterLayer(const char* name);
void setLayerEnabled(const char* name, bool enabled);

#endif // ENGINE_H
