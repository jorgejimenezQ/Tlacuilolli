#include "../include/triangle_demo.h"
#include "../include/triangle.h"
#include "../include/triangle_simd.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <sys/time.h>

// Ensure we have the declaration for the triangle drawing function
extern void drawTriangle(Canvas* canvas, const Triangle* t);

// Static array of triangles for the demo
static Triangle* triangles = NULL;

// SIMD data structure for optimized rendering
static TriangleDataSIMD simdData;

// Performance timing variables
static struct timeval lastFrameTime;
static double lastFrameDuration = 0.0;
static int frameCounter = 0;
static double totalFrameTime = 0.0;

// Get current time in seconds
static double getCurrentTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

void initRandomTriangles(int w, int h) {
    // Initialize both the traditional triangle array and the SIMD data structure
    triangles = malloc(sizeof(*triangles) * (size_t)TRIANGLE_COUNT);
    triangleDataSIMD_init(&simdData, TRIANGLE_COUNT);
    
    srand((unsigned)time(NULL));
    for (int i = 0; i < TRIANGLE_COUNT; i++) {
        triangles[i].cx    = ((float)rand()/RAND_MAX)*w  - w/2.0f;
        triangles[i].cy    = ((float)rand()/RAND_MAX)*h  - h/2.0f;
        triangles[i].size  = 1 + ((float)rand()/RAND_MAX)*10;
        triangles[i].color = (Color){
            (uint8_t)(rand() % 256),
            (uint8_t)(rand() % 256),
            (uint8_t)(rand() % 256)
        };
        triangles[i].angle = ((float)rand()/RAND_MAX)*2*M_PI;
        triangles[i].speed = ((float)rand()/RAND_MAX)*2.0f - 1.0f;
    }
    
    // Convert the triangle array to SIMD-friendly SoA format
    triangleDataSIMD_fromTriangles(&simdData, triangles, TRIANGLE_COUNT);
    
    // Initialize timing
    gettimeofday(&lastFrameTime, NULL);
    frameCounter = 0;
    totalFrameTime = 0.0;
}

void renderRandomTriangles(Canvas* canvas, float dt) {
    // Start timing this frame
    double frameStart = getCurrentTime();
    
#if defined(USE_AVX2) || defined(USE_SSE2)
    // SIMD optimized path
    
    // Update all triangles and perform frustum culling with SIMD
    updateAndCullSIMD(&simdData, dt, canvas->width, canvas->height);
    
    // Render all visible triangles
    renderTrianglesSIMD(canvas, &simdData);
    
    // Update the original triangle array (just angles for mouse interaction)
    for (int i = 0; i < TRIANGLE_COUNT; i++) {
        triangles[i].angle = simdData.angle[i];
    }
#else
    // Scalar fallback path
    
    // Canvas dimensions for culling check
    int canvas_width = canvas->width;
    int canvas_height = canvas->height;
    
    // Create a smaller frustum boundary to make culling visible at the edges
    // Using 80% of the canvas size to create a visible border effect
    float frustum_width = canvas_width * 0.8f;
    float frustum_height = canvas_height * 0.8f;
    
    for (int i = 0; i < TRIANGLE_COUNT; i++) {
        triangles[i].angle += triangles[i].speed * dt;
        
        // Simple frustum culling - check if triangle is entirely outside our reduced frustum
        // Consider the triangle's center position and maximum possible extent (size)
        float max_extent = triangles[i].size * 1.5f; // Adding a small margin for rotation
        
        // If the triangle's bounding box is completely outside the reduced frustum, skip it
        if (triangles[i].cx + max_extent < -frustum_width/2.0f || 
            triangles[i].cx - max_extent > frustum_width/2.0f ||
            triangles[i].cy + max_extent < -frustum_height/2.0f ||
            triangles[i].cy - max_extent > frustum_height/2.0f) {
            continue; // Skip this triangle - it's outside the reduced view
        }
        
        // Draw the triangle since it's at least partially visible
        drawTriangle(canvas, &triangles[i]);
    }
#endif
    
    // Collect timing data
    double frameEnd = getCurrentTime();
    double frameDuration = frameEnd - frameStart;
    
    // Update performance metrics
    lastFrameDuration = frameDuration;
    totalFrameTime += frameDuration;
    frameCounter++;
    
    // Print performance metrics every 60 frames
    if (frameCounter % 60 == 0) {
        double avgFrameTime = totalFrameTime / frameCounter;
        double fps = 1.0 / avgFrameTime;
        double trianglesPerSec = TRIANGLE_COUNT * fps;
        
#if defined(USE_AVX2)
        // printf("AVX2 - ");
#elif defined(USE_SSE2)
        // printf("SSE2 - ");
#else
        // printf("Scalar - ");
#endif
        printf("FPS: %.1f, Triangles/sec: %.1fM, Frame time: %.3f ms\n", 
               fps, trianglesPerSec / 1000000.0, avgFrameTime * 1000.0);
        
        // Reset counters for the next sample
        if (frameCounter >= 120) {
            frameCounter = 0;
            totalFrameTime = 0.0;
        }
    }
}

void updateTrianglesWithMouse(int mouseX, int mouseY, int canvasWidth, int canvasHeight, int isPressed) {
    // Convert mouse coordinates to canvas coordinate system (centered at origin)
    // Note: Flip the Y coordinate because screen Y increases downward but our canvas Y increases upward
    float canvasMouseX = mouseX - canvasWidth/2.0f;
    float canvasMouseY = canvasHeight/2.0f - mouseY; // Flipped Y axis
    
    // Interaction strength multiplier (stronger when mouse is pressed)
    float strengthMultiplier = isPressed ? 2.5f : 1.0f;
    
    // Calculate distance and apply force to each triangle
    for (int i = 0; i < TRIANGLE_COUNT; i++) {
        // Calculate distance from mouse to triangle
        float dx = triangles[i].cx - canvasMouseX;
        float dy = triangles[i].cy - canvasMouseY;
        float distSquared = dx*dx + dy*dy;
        
        // Skip triangles too far from mouse cursor
        if (distSquared > MOUSE_INFLUENCE_RADIUS * MOUSE_INFLUENCE_RADIUS) {
            continue;
        }
        
        // Calculate force based on distance (closer = stronger)
        float distance = sqrtf(distSquared);
        if (distance < 1.0f) distance = 1.0f; // Avoid division by zero
        
        // Direction vector from mouse to triangle (normalized)
        float dirX = dx / distance;
        float dirY = dy / distance;
        
        // Force decreases with square of distance
        float force = (MOUSE_FORCE_FACTOR * strengthMultiplier) / (distance * 0.5f);
        
        // Apply force to triangle position
        triangles[i].cx += dirX * force;
        triangles[i].cy += dirY * force;
        
        // Add a slight rotation effect based on mouse movement
        triangles[i].angle += (dirX + dirY) * 0.01f * strengthMultiplier;
        
#if defined(USE_AVX2) || defined(USE_SSE2)
        // Update SIMD data structure as well
        simdData.cx[i] = triangles[i].cx;
        simdData.cy[i] = triangles[i].cy;
        simdData.angle[i] = triangles[i].angle;
#endif
    }
}