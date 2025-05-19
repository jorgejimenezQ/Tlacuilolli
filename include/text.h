#ifndef TEXT_H
#define TEXT_H

#include "canvas.h"
#include <stdbool.h>

// Initialize text subsystem with the given font filename (from assets/fonts) and point size
bool textInit(const char* fontFilename, int fontSize);

// Draw a UTF-8 string centered at (cx, cy) on the canvas
void textDraw(Canvas* canvas, int cx, int cy, const char* text, Color color);

// Shutdown and free text resources
void textShutdown(void);

#endif // TEXT_H
