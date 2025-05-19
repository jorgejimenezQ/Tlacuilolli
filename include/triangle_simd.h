#ifndef TRIANGLE_SIMD_H
#define TRIANGLE_SIMD_H

#include "canvas.h"
#include "triangle.h"
#include <stdbool.h>

// SIMD capabilities detection
#if defined(__AVX2__)
  #define USE_AVX2 1
  #include <immintrin.h>
  #define SIMD_LANES 8
  typedef __m256 simd_float;
  typedef __m256i simd_int;
#elif defined(__SSE2__)
  #define USE_SSE2 1
  #include <emmintrin.h>
  #define SIMD_LANES 4
  typedef __m128 simd_float;
  typedef __m128i simd_int;
#else
  #define SIMD_LANES 1
#endif

// Structure of Arrays (SoA) for SIMD-friendly triangle data
typedef struct {
    // Aligned arrays for SIMD processing
    float* cx;         // x center coordinates
    float* cy;         // y center coordinates
    float* size;       // sizes
    float* angle;      // current angles
    float* speed;      // rotation speeds
    Color* color;      // colors
    bool* visible;     // culling result
    int capacity;      // allocated size
    int count;         // actual count
} TriangleDataSIMD;

// Initialize the SIMD triangle data structure
void triangleDataSIMD_init(TriangleDataSIMD* data, int capacity);

// Free the SIMD triangle data
void triangleDataSIMD_free(TriangleDataSIMD* data);

// Convert AoS triangle array to SoA format for SIMD processing
void triangleDataSIMD_fromTriangles(TriangleDataSIMD* data, const Triangle* triangles, int count);

// Update triangle angles and perform culling using SIMD
void updateAndCullSIMD(TriangleDataSIMD* data, float dt, int canvasWidth, int canvasHeight);

// Render all visible triangles using SIMD-accelerated processing
void renderTrianglesSIMD(Canvas* canvas, TriangleDataSIMD* data);

// Draw a batch of 4 or 8 triangles (depending on SSE or AVX)
void drawTrianglesBatchSIMD(Canvas* canvas, const float* cx, const float* cy, 
                          const float* size, const float* angle, const Color* color,
                          int batchSize);

#endif // TRIANGLE_SIMD_H
