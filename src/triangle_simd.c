#include "../include/triangle_simd.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

// Memory alignment for SIMD operations
#define ALIGN_SIZE 32

// Helper to allocate aligned memory for SIMD operations
static void* aligned_malloc(size_t size) {
#if defined(__AVX2__) || defined(__SSE2__)
    return _mm_malloc(size, ALIGN_SIZE);
#else
    return malloc(size);
#endif
}

// Helper to free aligned memory
static void aligned_free(void* ptr) {
#if defined(__AVX2__) || defined(__SSE2__)
    _mm_free(ptr);
#else
    free(ptr);
#endif
}

// Initialize the SIMD triangle data structure
void triangleDataSIMD_init(TriangleDataSIMD* data, int capacity) {
    // Round up capacity to nearest SIMD_LANES multiple
    int alignedCapacity = ((capacity + SIMD_LANES - 1) / SIMD_LANES) * SIMD_LANES;
    
    data->capacity = alignedCapacity;
    data->count = 0;
    
    // Allocate aligned memory for all arrays
    data->cx = (float*)aligned_malloc(alignedCapacity * sizeof(float));
    data->cy = (float*)aligned_malloc(alignedCapacity * sizeof(float));
    data->size = (float*)aligned_malloc(alignedCapacity * sizeof(float));
    data->angle = (float*)aligned_malloc(alignedCapacity * sizeof(float));
    data->speed = (float*)aligned_malloc(alignedCapacity * sizeof(float));
    data->color = (Color*)aligned_malloc(alignedCapacity * sizeof(Color));
    data->visible = (bool*)aligned_malloc(alignedCapacity * sizeof(bool));
    
    // Zero out the memory
    memset(data->cx, 0, alignedCapacity * sizeof(float));
    memset(data->cy, 0, alignedCapacity * sizeof(float));
    memset(data->size, 0, alignedCapacity * sizeof(float));
    memset(data->angle, 0, alignedCapacity * sizeof(float));
    memset(data->speed, 0, alignedCapacity * sizeof(float));
    memset(data->color, 0, alignedCapacity * sizeof(Color));
    memset(data->visible, 0, alignedCapacity * sizeof(bool));
}

// Free the SIMD triangle data
void triangleDataSIMD_free(TriangleDataSIMD* data) {
    aligned_free(data->cx);
    aligned_free(data->cy);
    aligned_free(data->size);
    aligned_free(data->angle);
    aligned_free(data->speed);
    aligned_free(data->color);
    aligned_free(data->visible);
    
    data->cx = NULL;
    data->cy = NULL;
    data->size = NULL;
    data->angle = NULL;
    data->speed = NULL;
    data->color = NULL;
    data->visible = NULL;
    data->capacity = 0;
    data->count = 0;
}

// Convert AoS triangle array to SoA format for SIMD processing
void triangleDataSIMD_fromTriangles(TriangleDataSIMD* data, const Triangle* triangles, int count) {
    if (count > data->capacity) {
        fprintf(stderr, "Error: Triangle count exceeds SIMD data capacity\n");
        return;
    }
    
    // Copy triangle data into SoA format
    for (int i = 0; i < count; i++) {
        data->cx[i] = triangles[i].cx;
        data->cy[i] = triangles[i].cy;
        data->size[i] = triangles[i].size;
        data->angle[i] = triangles[i].angle;
        data->speed[i] = triangles[i].speed;
        data->color[i] = triangles[i].color;
        data->visible[i] = true; // Initially all visible
    }
    
    // Zero out any remaining elements in the arrays
    if (count < data->capacity) {
        int remainingElements = data->capacity - count;
        memset(&data->cx[count], 0, remainingElements * sizeof(float));
        memset(&data->cy[count], 0, remainingElements * sizeof(float));
        memset(&data->size[count], 0, remainingElements * sizeof(float));
        memset(&data->angle[count], 0, remainingElements * sizeof(float));
        memset(&data->speed[count], 0, remainingElements * sizeof(float));
        memset(&data->color[count], 0, remainingElements * sizeof(Color));
        memset(&data->visible[count], 0, remainingElements * sizeof(bool));
    }
    
    data->count = count;
}

#if defined(__AVX2__)
// AVX2 implementation (8 floats at once)
void updateAndCullSIMD(TriangleDataSIMD* data, float dt, int canvasWidth, int canvasHeight) {
    // Create smaller frustum for visible culling effect (80% of canvas)
    float frustum_width = canvasWidth * 0.8f;
    float frustum_height = canvasHeight * 0.8f;
    
    // Constants for SIMD processing
    __m256 dt_vec = _mm256_set1_ps(dt);
    __m256 frustum_w_half = _mm256_set1_ps(frustum_width / 2.0f);
    __m256 frustum_h_half = _mm256_set1_ps(frustum_height / 2.0f);
    __m256 zero = _mm256_setzero_ps();
    __m256 margin = _mm256_set1_ps(1.5f);  // Extra margin for rotation
    
    // Process triangles in groups of 8 (AVX2 width)
    for (int i = 0; i < data->count; i += 8) {
        // Load 8 angles and 8 speeds
        __m256 angle_vec = _mm256_load_ps(&data->angle[i]);
        __m256 speed_vec = _mm256_load_ps(&data->speed[i]);
        __m256 cx_vec = _mm256_load_ps(&data->cx[i]);
        __m256 cy_vec = _mm256_load_ps(&data->cy[i]);
        __m256 size_vec = _mm256_load_ps(&data->size[i]);
        
        // Update angles: angle += speed * dt
        angle_vec = _mm256_add_ps(angle_vec, _mm256_mul_ps(speed_vec, dt_vec));
        _mm256_store_ps(&data->angle[i], angle_vec);
        
        // Calculate max extent for each triangle (size * margin)
        __m256 extent_vec = _mm256_mul_ps(size_vec, margin);
        
        // Calculate bounds for culling
        __m256 min_x = _mm256_sub_ps(cx_vec, extent_vec);
        __m256 max_x = _mm256_add_ps(cx_vec, extent_vec);
        __m256 min_y = _mm256_sub_ps(cy_vec, extent_vec);
        __m256 max_y = _mm256_add_ps(cy_vec, extent_vec);
        
        // Check if triangles are outside the frustum
        __m256 cmp1 = _mm256_cmp_ps(max_x, _mm256_sub_ps(zero, frustum_w_half), _CMP_LT_OQ); // max_x < -frustum_w_half
        __m256 cmp2 = _mm256_cmp_ps(min_x, frustum_w_half, _CMP_GT_OQ);                     // min_x > frustum_w_half
        __m256 cmp3 = _mm256_cmp_ps(max_y, _mm256_sub_ps(zero, frustum_h_half), _CMP_LT_OQ); // max_y < -frustum_h_half
        __m256 cmp4 = _mm256_cmp_ps(min_y, frustum_h_half, _CMP_GT_OQ);                     // min_y > frustum_h_half
        
        // Combine all conditions with OR
        __m256 outside = _mm256_or_ps(_mm256_or_ps(cmp1, cmp2), _mm256_or_ps(cmp3, cmp4));
        
        // Convert the mask to integers (0xFFFFFFFF for true, 0 for false)
        int outside_mask = _mm256_movemask_ps(outside);
        
        // Set visibility flags (invert mask since outside=1 means invisible)
        for (int j = 0; j < 8 && i + j < data->count; j++) {
            data->visible[i + j] = !((outside_mask >> j) & 1);
        }
    }
}

#elif defined(__SSE2__)
// SSE2 implementation (4 floats at once)
void updateAndCullSIMD(TriangleDataSIMD* data, float dt, int canvasWidth, int canvasHeight) {
    // Create smaller frustum for visible culling effect (80% of canvas)
    float frustum_width = canvasWidth * 0.8f;
    float frustum_height = canvasHeight * 0.8f;
    
    // Constants for SIMD processing
    __m128 dt_vec = _mm_set1_ps(dt);
    __m128 frustum_w_half = _mm_set1_ps(frustum_width / 2.0f);
    __m128 frustum_h_half = _mm_set1_ps(frustum_height / 2.0f);
    __m128 zero = _mm_setzero_ps();
    __m128 margin = _mm_set1_ps(1.5f);  // Extra margin for rotation
    
    // Process triangles in groups of 4 (SSE2 width)
    for (int i = 0; i < data->count; i += 4) {
        // Load 4 angles and 4 speeds
        __m128 angle_vec = _mm_load_ps(&data->angle[i]);
        __m128 speed_vec = _mm_load_ps(&data->speed[i]);
        __m128 cx_vec = _mm_load_ps(&data->cx[i]);
        __m128 cy_vec = _mm_load_ps(&data->cy[i]);
        __m128 size_vec = _mm_load_ps(&data->size[i]);
        
        // Update angles: angle += speed * dt
        angle_vec = _mm_add_ps(angle_vec, _mm_mul_ps(speed_vec, dt_vec));
        _mm_store_ps(&data->angle[i], angle_vec);
        
        // Calculate max extent for each triangle (size * margin)
        __m128 extent_vec = _mm_mul_ps(size_vec, margin);
        
        // Calculate bounds for culling
        __m128 min_x = _mm_sub_ps(cx_vec, extent_vec);
        __m128 max_x = _mm_add_ps(cx_vec, extent_vec);
        __m128 min_y = _mm_sub_ps(cy_vec, extent_vec);
        __m128 max_y = _mm_add_ps(cy_vec, extent_vec);
        
        // Check if triangles are outside the frustum
        __m128 cmp1 = _mm_cmplt_ps(max_x, _mm_sub_ps(zero, frustum_w_half)); // max_x < -frustum_w_half
        __m128 cmp2 = _mm_cmpgt_ps(min_x, frustum_w_half);                  // min_x > frustum_w_half
        __m128 cmp3 = _mm_cmplt_ps(max_y, _mm_sub_ps(zero, frustum_h_half)); // max_y < -frustum_h_half
        __m128 cmp4 = _mm_cmpgt_ps(min_y, frustum_h_half);                  // min_y > frustum_h_half
        
        // Combine all conditions with OR
        __m128 outside = _mm_or_ps(_mm_or_ps(cmp1, cmp2), _mm_or_ps(cmp3, cmp4));
        
        // Convert the mask to integers (0xFFFFFFFF for true, 0 for false)
        int outside_mask = _mm_movemask_ps(outside);
        
        // Set visibility flags (invert mask since outside=1 means invisible)
        for (int j = 0; j < 4 && i + j < data->count; j++) {
            data->visible[i + j] = !((outside_mask >> j) & 1);
        }
    }
}

#else
// Scalar fallback implementation
void updateAndCullSIMD(TriangleDataSIMD* data, float dt, int canvasWidth, int canvasHeight) {
    // Create smaller frustum for visible culling effect (80% of canvas)
    float frustum_width = canvasWidth * 0.8f;
    float frustum_height = canvasHeight * 0.8f;
    float frustum_w_half = frustum_width / 2.0f;
    float frustum_h_half = frustum_height / 2.0f;
    
    for (int i = 0; i < data->count; i++) {
        // Update angle
        data->angle[i] += data->speed[i] * dt;
        
        // Calculate max extent
        float max_extent = data->size[i] * 1.5f;
        
        // Check if triangle is outside the frustum
        if (data->cx[i] + max_extent < -frustum_w_half || 
            data->cx[i] - max_extent > frustum_w_half ||
            data->cy[i] + max_extent < -frustum_h_half ||
            data->cy[i] - max_extent > frustum_h_half) {
            data->visible[i] = false;
        } else {
            data->visible[i] = true;
        }
    }
}
#endif

// drawLine is now included from triangle.h

// Helper function to calculate vertices for a single triangle
static void calcTriangleVertices(float cx, float cy, float size, float angle, int vx[3], int vy[3]) {
    // Local base-triangle pointing up
    float bx[3] = { 0, size, -size };
    float by[3] = { -size, size, size };
    float c = cosf(angle);
    float s = sinf(angle);
    
    for (int i = 0; i < 3; i++) {
        float rx = bx[i]*c - by[i]*s;
        float ry = bx[i]*s + by[i]*c;
        vx[i] = (int)(cx + rx);
        vy[i] = (int)(cy + ry);
    }
}

// Render all visible triangles using SIMD processing
void renderTrianglesSIMD(Canvas* canvas, TriangleDataSIMD* data) {
    for (int i = 0; i < data->count; i++) {
        if (data->visible[i]) {
            int vx[3], vy[3];
            calcTriangleVertices(data->cx[i], data->cy[i], data->size[i], data->angle[i], vx, vy);
            
            // Draw the three edges
            drawLine(canvas, vx[0], vy[0], vx[1], vy[1], data->color[i]);
            drawLine(canvas, vx[1], vy[1], vx[2], vy[2], data->color[i]);
            drawLine(canvas, vx[2], vy[2], vx[0], vy[0], data->color[i]);
        }
    }
}

#if defined(__AVX2__)
// AVX2 implementation of batch triangle rendering
void drawTrianglesBatchSIMD(Canvas* canvas, const float* cx, const float* cy, 
                          const float* size, const float* angle, const Color* color,
                          int batchSize) {
    // Ensure batchSize <= 8
    if (batchSize > 8) batchSize = 8;
    
    // For simplicity, we're calculating vertices with SIMD but still drawing with scalar code
    // A full implementation would vectorize the line drawing as well
    
    // Base triangle template (common for all triangles)
    __m256 base_x0 = _mm256_set1_ps(0.0f);
    __m256 base_x1 = _mm256_set1_ps(1.0f);
    __m256 base_x2 = _mm256_set1_ps(-1.0f);
    __m256 base_y0 = _mm256_set1_ps(-1.0f);
    __m256 base_y1 = _mm256_set1_ps(1.0f);
    __m256 base_y2 = _mm256_set1_ps(1.0f);
    
    // Load 8 positions, sizes and angles
    __m256 cx_vec = _mm256_loadu_ps(cx);
    __m256 cy_vec = _mm256_loadu_ps(cy);
    __m256 size_vec = _mm256_loadu_ps(size);
    
    // Calculate sin/cos for each angle
    // Note: In a production system, you'd use a fast SIMD sin/cos approximation
    // Here we'll compute them separately for simplicity
    float c_vals[8], s_vals[8];
    for (int i = 0; i < batchSize; i++) {
        c_vals[i] = cosf(angle[i]);
        s_vals[i] = sinf(angle[i]);
    }
    __m256 c_vec = _mm256_loadu_ps(c_vals);
    __m256 s_vec = _mm256_loadu_ps(s_vals);
    
    // Scale the base triangle by size
    __m256 bx0 = _mm256_mul_ps(base_x0, size_vec);
    __m256 by0 = _mm256_mul_ps(base_y0, size_vec);
    __m256 bx1 = _mm256_mul_ps(base_x1, size_vec);
    __m256 by1 = _mm256_mul_ps(base_y1, size_vec);
    __m256 bx2 = _mm256_mul_ps(base_x2, size_vec);
    __m256 by2 = _mm256_mul_ps(base_y2, size_vec);
    
    // Rotate and translate all vertices for vertex 0
    __m256 rx0 = _mm256_sub_ps(_mm256_mul_ps(bx0, c_vec), _mm256_mul_ps(by0, s_vec));
    __m256 ry0 = _mm256_add_ps(_mm256_mul_ps(bx0, s_vec), _mm256_mul_ps(by0, c_vec));
    __m256 vx0 = _mm256_add_ps(cx_vec, rx0);
    __m256 vy0 = _mm256_add_ps(cy_vec, ry0);
    
    // Rotate and translate all vertices for vertex 1
    __m256 rx1 = _mm256_sub_ps(_mm256_mul_ps(bx1, c_vec), _mm256_mul_ps(by1, s_vec));
    __m256 ry1 = _mm256_add_ps(_mm256_mul_ps(bx1, s_vec), _mm256_mul_ps(by1, c_vec));
    __m256 vx1 = _mm256_add_ps(cx_vec, rx1);
    __m256 vy1 = _mm256_add_ps(cy_vec, ry1);
    
    // Rotate and translate all vertices for vertex 2
    __m256 rx2 = _mm256_sub_ps(_mm256_mul_ps(bx2, c_vec), _mm256_mul_ps(by2, s_vec));
    __m256 ry2 = _mm256_add_ps(_mm256_mul_ps(bx2, s_vec), _mm256_mul_ps(by2, c_vec));
    __m256 vx2 = _mm256_add_ps(cx_vec, rx2);
    __m256 vy2 = _mm256_add_ps(cy_vec, ry2);
    
    // Store results
    float vx0_arr[8], vy0_arr[8], vx1_arr[8], vy1_arr[8], vx2_arr[8], vy2_arr[8];
    _mm256_storeu_ps(vx0_arr, vx0);
    _mm256_storeu_ps(vy0_arr, vy0);
    _mm256_storeu_ps(vx1_arr, vx1);
    _mm256_storeu_ps(vy1_arr, vy1);
    _mm256_storeu_ps(vx2_arr, vx2);
    _mm256_storeu_ps(vy2_arr, vy2);
    
    // Draw all triangles using the calculated vertices
    for (int i = 0; i < batchSize; i++) {
        drawLine(canvas, (int)vx0_arr[i], (int)vy0_arr[i], (int)vx1_arr[i], (int)vy1_arr[i], color[i]);
        drawLine(canvas, (int)vx1_arr[i], (int)vy1_arr[i], (int)vx2_arr[i], (int)vy2_arr[i], color[i]);
        drawLine(canvas, (int)vx2_arr[i], (int)vy2_arr[i], (int)vx0_arr[i], (int)vy0_arr[i], color[i]);
    }
}

#elif defined(__SSE2__)
// SSE2 implementation of batch triangle rendering
void drawTrianglesBatchSIMD(Canvas* canvas, const float* cx, const float* cy, 
                          const float* size, const float* angle, const Color* color,
                          int batchSize) {
    // Ensure batchSize <= 4
    if (batchSize > 4) batchSize = 4;
    
    // Base triangle template (common for all triangles)
    __m128 base_x0 = _mm_set1_ps(0.0f);
    __m128 base_x1 = _mm_set1_ps(1.0f);
    __m128 base_x2 = _mm_set1_ps(-1.0f);
    __m128 base_y0 = _mm_set1_ps(-1.0f);
    __m128 base_y1 = _mm_set1_ps(1.0f);
    __m128 base_y2 = _mm_set1_ps(1.0f);
    
    // Load 4 positions, sizes and angles
    __m128 cx_vec = _mm_loadu_ps(cx);
    __m128 cy_vec = _mm_loadu_ps(cy);
    __m128 size_vec = _mm_loadu_ps(size);
    
    // Calculate sin/cos for each angle
    float c_vals[4], s_vals[4];
    for (int i = 0; i < batchSize; i++) {
        c_vals[i] = cosf(angle[i]);
        s_vals[i] = sinf(angle[i]);
    }
    __m128 c_vec = _mm_loadu_ps(c_vals);
    __m128 s_vec = _mm_loadu_ps(s_vals);
    
    // Scale the base triangle by size
    __m128 bx0 = _mm_mul_ps(base_x0, size_vec);
    __m128 by0 = _mm_mul_ps(base_y0, size_vec);
    __m128 bx1 = _mm_mul_ps(base_x1, size_vec);
    __m128 by1 = _mm_mul_ps(base_y1, size_vec);
    __m128 bx2 = _mm_mul_ps(base_x2, size_vec);
    __m128 by2 = _mm_mul_ps(base_y2, size_vec);
    
    // Rotate and translate all vertices for vertex 0
    __m128 rx0 = _mm_sub_ps(_mm_mul_ps(bx0, c_vec), _mm_mul_ps(by0, s_vec));
    __m128 ry0 = _mm_add_ps(_mm_mul_ps(bx0, s_vec), _mm_mul_ps(by0, c_vec));
    __m128 vx0 = _mm_add_ps(cx_vec, rx0);
    __m128 vy0 = _mm_add_ps(cy_vec, ry0);
    
    // Rotate and translate all vertices for vertex 1
    __m128 rx1 = _mm_sub_ps(_mm_mul_ps(bx1, c_vec), _mm_mul_ps(by1, s_vec));
    __m128 ry1 = _mm_add_ps(_mm_mul_ps(bx1, s_vec), _mm_mul_ps(by1, c_vec));
    __m128 vx1 = _mm_add_ps(cx_vec, rx1);
    __m128 vy1 = _mm_add_ps(cy_vec, ry1);
    
    // Rotate and translate all vertices for vertex 2
    __m128 rx2 = _mm_sub_ps(_mm_mul_ps(bx2, c_vec), _mm_mul_ps(by2, s_vec));
    __m128 ry2 = _mm_add_ps(_mm_mul_ps(bx2, s_vec), _mm_mul_ps(by2, c_vec));
    __m128 vx2 = _mm_add_ps(cx_vec, rx2);
    __m128 vy2 = _mm_add_ps(cy_vec, ry2);
    
    // Store results
    float vx0_arr[4], vy0_arr[4], vx1_arr[4], vy1_arr[4], vx2_arr[4], vy2_arr[4];
    _mm_storeu_ps(vx0_arr, vx0);
    _mm_storeu_ps(vy0_arr, vy0);
    _mm_storeu_ps(vx1_arr, vx1);
    _mm_storeu_ps(vy1_arr, vy1);
    _mm_storeu_ps(vx2_arr, vx2);
    _mm_storeu_ps(vy2_arr, vy2);
    
    // Draw all triangles using the calculated vertices
    for (int i = 0; i < batchSize; i++) {
        drawLine(canvas, (int)vx0_arr[i], (int)vy0_arr[i], (int)vx1_arr[i], (int)vy1_arr[i], color[i]);
        drawLine(canvas, (int)vx1_arr[i], (int)vy1_arr[i], (int)vx2_arr[i], (int)vy2_arr[i], color[i]);
        drawLine(canvas, (int)vx2_arr[i], (int)vy2_arr[i], (int)vx0_arr[i], (int)vy0_arr[i], color[i]);
    }
}

#else
// Scalar fallback implementation
void drawTrianglesBatchSIMD(Canvas* canvas, const float* cx, const float* cy, 
                          const float* size, const float* angle, const Color* color,
                          int batchSize) {
    for (int i = 0; i < batchSize; i++) {
        int vx[3], vy[3];
        calcTriangleVertices(cx[i], cy[i], size[i], angle[i], vx, vy);
        
        drawLine(canvas, vx[0], vy[0], vx[1], vy[1], color[i]);
        drawLine(canvas, vx[1], vy[1], vx[2], vy[2], color[i]);
        drawLine(canvas, vx[2], vy[2], vx[0], vy[0], color[i]);
    }
}
#endif
