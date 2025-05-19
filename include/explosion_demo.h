#ifndef EXPLOSION_DEMO_H
#define EXPLOSION_DEMO_H

#include "canvas.h"

// Maximum number of particles that can be active at once
#define MAX_PARTICLES 1000

// Default number of particles to emit per explosion
#define PARTICLES_PER_EXPLOSION 150

// Particle appearance and behavior parameters
#define PARTICLE_MIN_SIZE 1.0f
#define PARTICLE_MAX_SIZE 3.0f
#define PARTICLE_MIN_SPEED 50.0f
#define PARTICLE_MAX_SPEED 200.0f
#define PARTICLE_MIN_LIFETIME 0.5f
#define PARTICLE_MAX_LIFETIME 2.0f

// Particle structure for explosion effect
typedef struct {
    float cx, cy;         // Position in canvas coordinates
    float dx, dy;         // Velocity components
    float size;           // Size of the triangle
    Color color;          // RGB color
    float angle;          // Current rotation angle
    float rotation_speed; // Speed of rotation in radians/second
    float age;            // Current age in seconds
    float max_age;        // Maximum lifetime in seconds
    bool active;          // Whether the particle is currently active
} Particle;

// Initialize the explosion demo
void initExplosionDemo(int canvasW, int canvasH);

// Cleanup resources used by the explosion demo
void cleanupExplosionDemo(void);

// Create a new explosion at the specified canvas coordinates
void handleClickExplosion(float canvasX, float canvasY);

// Update all active particles
void updateExplosion(float dt);

// Render all active particles
void renderExplosion(Canvas* canvas);

#endif // EXPLOSION_DEMO_H
