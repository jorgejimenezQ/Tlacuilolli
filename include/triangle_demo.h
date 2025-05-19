#ifndef TRIANGLE_DEMO_H
#define TRIANGLE_DEMO_H

#include "canvas.h"

// Number of triangles to generate and render
#define TRIANGLE_COUNT 100000

// Mouse interaction parameters
#define MOUSE_INFLUENCE_RADIUS 100.0f  // How far the mouse affects triangles
#define MOUSE_FORCE_FACTOR    20.0f    // How strongly the mouse pushes triangles

// Initialize the triangle demo with random triangles
void initRandomTriangles(int canvasW, int canvasH);

// Render all triangles in the demo, with optional frustum culling
void renderRandomTriangles(Canvas* canvas, float dt);

// Update triangles based on mouse position
// mouseX, mouseY: Mouse coordinates in window space
// canvasWidth, canvasHeight: Dimensions of the canvas
// isPressed: Whether mouse button is being pressed
void updateTrianglesWithMouse(int mouseX, int mouseY, int canvasWidth, int canvasHeight, int isPressed);

#endif // TRIANGLE_DEMO_H
