/**
 * @file hello_world_demo.c
 * @brief Implementation of the hello world demo with bouncing shapes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "../include/engine.h"
#include "../include/input.h"
#include "../include/triangle.h"
#include "../include/text.h"
#include "hello_world_demo.h"

/**
 * @brief Number of triangles to display in the demo
 */
#define NUM_TRIANGLES 5

/**
 * @brief Number of circles to display in the demo
 */
#define NUM_CIRCLES 5

/**
 * @brief Structure for circle objects
 */
typedef struct {
    float cx, cy;         /**< Center position */
    float vx, vy;         /**< Velocity */
    float radius;         /**< Circle radius */
    Color color;          /**< Circle color */
} Circle;

/* Global state for the demo */
static int WINDOW_WIDTH = 800;            /**< Window width in pixels (default) */
static int WINDOW_HEIGHT = 600;           /**< Window height in pixels (default) */
static Triangle triangles[NUM_TRIANGLES]; /**< Array of triangles */
static Circle circles[NUM_CIRCLES];       /**< Array of circles */
static bool darkBackground = true;        /**< Background color toggle */

/* Layer declarations */
static void bgUpdate(float dt);
static void bgRender(void);
static Layer background = { "Background", bgUpdate, bgRender, true };

static void fgUpdate(float dt);
static void fgRender(void);
static Layer foreground = { "Foreground", fgUpdate, fgRender, true };

/**
 * @brief Generate a random color
 * 
 * Creates a color with random RGB values (0-255).
 * 
 * @return Randomly generated Color
 */
static Color randomColor(void) {
    Color c;
    c.r = rand() % 256;
    c.g = rand() % 256;
    c.b = rand() % 256;
    return c;
}

/**
 * @brief Draw a circle at the specified position
 * 
 * Implements a filled circle using Bresenham's circle algorithm.
 * 
 * @param canvas Pointer to the Canvas to draw on
 * @param cx X-coordinate of circle center
 * @param cy Y-coordinate of circle center
 * @param radius Radius of the circle in pixels
 * @param color Color to use for drawing the circle
 */
static void drawCircle(Canvas* canvas, float cx, float cy, float radius, Color color) {
    // Draw a circle using Bresenham's circle algorithm
    int x = 0;
    int y = (int)radius;
    int d = 3 - 2 * (int)radius;
    
    while (y >= x) {
        // Draw the eight octants
        Canvas_PutPixel(canvas, (int)cx + x, (int)cy + y, color);
        Canvas_PutPixel(canvas, (int)cx + y, (int)cy + x, color);
        Canvas_PutPixel(canvas, (int)cx - x, (int)cy + y, color);
        Canvas_PutPixel(canvas, (int)cx - y, (int)cy + x, color);
        Canvas_PutPixel(canvas, (int)cx + x, (int)cy - y, color);
        Canvas_PutPixel(canvas, (int)cx + y, (int)cy - x, color);
        Canvas_PutPixel(canvas, (int)cx - x, (int)cy - y, color);
        Canvas_PutPixel(canvas, (int)cx - y, (int)cy - x, color);
        
        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
    
    // Fill the circle
    for (int r = 0; r < radius; r++) {
        int x = 0;
        int y = r;
        int d = 3 - 2 * r;
        
        while (y >= x) {
            // Draw horizontal lines between the points
            for (int i = (int)cx - x; i <= (int)cx + x; i++) {
                Canvas_PutPixel(canvas, i, (int)cy + y, color);
                Canvas_PutPixel(canvas, i, (int)cy - y, color);
            }
            
            for (int i = (int)cx - y; i <= (int)cx + y; i++) {
                Canvas_PutPixel(canvas, i, (int)cy + x, color);
                Canvas_PutPixel(canvas, i, (int)cy - x, color);
            }
            
            if (d < 0) {
                d = d + 4 * x + 6;
            } else {
                d = d + 4 * (x - y) + 10;
                y--;
            }
            x++;
        }
    }
}

/**
 * @brief Set the window dimensions for the hello world demo
 * 
 * @param width Window width in pixels
 * @param height Window height in pixels
 */
void helloWorldDemo_SetDimensions(int width, int height) {
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
}

/**
 * @brief Setup function called once at startup
 * 
 * Initializes all the elements needed for the demo:
 * - Random triangles with size, position, color
 * - Random circles with velocity, size, position, color
 * - Registers the layers to the engine
 * - Initializes the text rendering system
 */
void helloWorldDemo_Setup(void) {
    // Seed the random number generator
    srand((unsigned int)time(NULL));
    
    // Initialize triangles with random properties
    for (int i = 0; i < NUM_TRIANGLES; i++) {
        // Random position within canvas bounds
        triangles[i].cx = (float)(rand() % WINDOW_WIDTH - (WINDOW_WIDTH/2.0f));
        triangles[i].cy = (float)(rand() % WINDOW_HEIGHT - (WINDOW_HEIGHT/2.0f));
        
        // Random size between 20 and 40
        triangles[i].size = 20.0f + (float)(rand() % 21);
        
        // Random angle
        triangles[i].angle = (float)(rand() % 628) / 100.0f;
        
        // Random color
        triangles[i].color = randomColor();
        
        // Random rotation speed
        triangles[i].speed = ((float)(rand() % 200) / 100.0f) - 1.0f;
    }
    
    // Initialize circles with random properties
    for (int i = 0; i < NUM_CIRCLES; i++) {
        // Random position within canvas bounds
        circles[i].cx = (float)(rand() % WINDOW_WIDTH - (WINDOW_WIDTH/2.0f));
        circles[i].cy = (float)(rand() % WINDOW_HEIGHT - (WINDOW_HEIGHT/2.0f));
        
        // Random velocity (-100 to 100 pixels per second)
        circles[i].vx = ((float)(rand() % 200) - 100.0f);
        circles[i].vy = ((float)(rand() % 200) - 100.0f);
        
        // Random radius between 15 and 30
        circles[i].radius = 15.0f + (float)(rand() % 16);
        
        // Random color
        circles[i].color = randomColor();
    }
    
    // Register layers
    registerLayer(&background);
    registerLayer(&foreground);
    
    // Initialize text rendering with font from assets/fonts directory
    if (!textInit("Ribeye-Regular.ttf", 24)) {
        fprintf(stderr, "Error: Failed to initialize text subsystem\n");
        // Continue anyway - we might see warnings in console
    }
    
    // Print instructions
    printf("Hello World Demo with Bouncing Shapes\n");
    printf("Controls:\n");
    printf("  SPACE: Toggle background color\n");
    printf("  ESC: Exit\n");
}

/**
 * @brief Background layer update function
 * 
 * Handles space key to toggle background color.
 * 
 * @param dt Delta time in seconds since last update
 */
static void bgUpdate(float dt) {
    // Note: inputUpdate is now called in fgUpdate to avoid duplicate calls
    // We still handle the space key here as it's background-related
    if (wasKeyJustPressed(SDL_SCANCODE_SPACE)) {
        darkBackground = !darkBackground;
        printf("Background color toggled: %s\n", darkBackground ? "Dark" : "Dark Blue");
    }
    
    (void)dt; // Avoid unused parameter warning
}

/**
 * @brief Background layer render function
 * 
 * Draws the background color (either black or dark blue).
 */
static void bgRender(void) {
    Canvas* canvas = getCanvas();
    
    // If darkBackground is false, fill with dark blue
    if (!darkBackground) {
        for (int y = 0; y < WINDOW_HEIGHT; y++) {
            for (int x = 0; x < WINDOW_WIDTH; x++) {
                Color darkBlue = {20, 20, 50};
                Canvas_PutPixel(canvas, x - WINDOW_WIDTH/2, WINDOW_HEIGHT/2 - y, darkBlue);
            }
        }
    }
}

/**
 * @brief Foreground layer update function
 * 
 * Updates all triangles and circles positions, handles bouncing off edges.
 * Also processes input for all layers (single point of input handling).
 * 
 * @param dt Delta time in seconds since last update
 */
static void fgUpdate(float dt) {
    // Process input first - this is the main input update for all layers
    inputUpdate();
    
    // Update triangles (rotate them)
    for (int i = 0; i < NUM_TRIANGLES; i++) {
        // Rotate triangle based on its speed
        triangles[i].angle += triangles[i].speed * dt;
        
        // Move triangles
        float moveSpeed = 50.0f; // pixels per second
        float angle = (float)(rand() % 628) / 100.0f;
        
        triangles[i].cx += cosf(angle) * moveSpeed * dt;
        triangles[i].cy += sinf(angle) * moveSpeed * dt;
        
        // Bounce off boundaries
        float bound = triangles[i].size * 1.5f;
        if (triangles[i].cx < -WINDOW_WIDTH/2.0f + bound) {
            triangles[i].cx = -WINDOW_WIDTH/2.0f + bound;
            triangles[i].speed = -triangles[i].speed;
        }
        if (triangles[i].cx > WINDOW_WIDTH/2.0f - bound) {
            triangles[i].cx = WINDOW_WIDTH/2.0f - bound;
            triangles[i].speed = -triangles[i].speed;
        }
        if (triangles[i].cy < -WINDOW_HEIGHT/2.0f + bound) {
            triangles[i].cy = -WINDOW_HEIGHT/2.0f + bound;
            triangles[i].speed = -triangles[i].speed;
        }
        if (triangles[i].cy > WINDOW_HEIGHT/2.0f - bound) {
            triangles[i].cy = WINDOW_HEIGHT/2.0f - bound;
            triangles[i].speed = -triangles[i].speed;
        }
    }
    
    // Update circles (move them)
    for (int i = 0; i < NUM_CIRCLES; i++) {
        // Update position based on velocity
        circles[i].cx += circles[i].vx * dt;
        circles[i].cy += circles[i].vy * dt;
        
        // Bounce off the boundaries
        if (circles[i].cx - circles[i].radius < -WINDOW_WIDTH/2.0f) {
            circles[i].cx = -WINDOW_WIDTH/2.0f + circles[i].radius;
            circles[i].vx = fabsf(circles[i].vx);
        }
        if (circles[i].cx + circles[i].radius > WINDOW_WIDTH/2.0f) {
            circles[i].cx = WINDOW_WIDTH/2.0f - circles[i].radius;
            circles[i].vx = -fabsf(circles[i].vx);
        }
        if (circles[i].cy - circles[i].radius < -WINDOW_HEIGHT/2.0f) {
            circles[i].cy = -WINDOW_HEIGHT/2.0f + circles[i].radius;
            circles[i].vy = fabsf(circles[i].vy);
        }
        if (circles[i].cy + circles[i].radius > WINDOW_HEIGHT/2.0f) {
            circles[i].cy = WINDOW_HEIGHT/2.0f - circles[i].radius;
            circles[i].vy = -fabsf(circles[i].vy);
        }
    }
}

/**
 * @brief Foreground layer render function
 * 
 * Draws all triangles, circles, and the hello world text.
 */
static void fgRender(void) {
    Canvas* canvas = getCanvas();
    
    // Draw hello world text
    Color white = {255, 255, 255};
    textDraw(canvas, 0, WINDOW_HEIGHT/3.0f, "Hello World!", white);
    
    // Draw all triangles
    for (int i = 0; i < NUM_TRIANGLES; i++) {
        drawTriangle(canvas, &triangles[i]);
    }
    
    // Draw all circles
    for (int i = 0; i < NUM_CIRCLES; i++) {
        drawCircle(canvas, circles[i].cx, circles[i].cy, circles[i].radius, circles[i].color);
    }
}

/* End of the hello world demo implementation */
