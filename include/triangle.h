#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "canvas.h"

// One triangle instance
typedef struct {
    float cx, cy;    // center in canvas coords
    float size;      // half-height
    Color color;     // r,g,b
    float angle;     // current rotation (radians)
    float speed;     // radians/sec
} Triangle;

// Draw a triangle wireframe outline to the canvas
void drawTriangle(Canvas* canvas, const Triangle* t);

// Draw a line between two points
void drawLine(Canvas* canvas, int x0, int y0, int x1, int y1, Color color);

#endif // TRIANGLE_H
