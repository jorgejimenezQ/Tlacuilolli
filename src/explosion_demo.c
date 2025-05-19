#include "../include/explosion_demo.h"
#include "../include/triangle.h"
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

// Ensure we have the declaration for the triangle drawing function
extern void drawTriangle(Canvas* canvas, const Triangle* t);

// Array of particles for the explosion effect
static Particle particles[MAX_PARTICLES];
static int canvasWidth, canvasHeight;

// Initialize all particles as inactive
void initExplosionDemo(int canvasW, int canvasH) {
    // Store canvas dimensions for later use
    canvasWidth = canvasW;
    canvasHeight = canvasH;
    
    // Seed random number generator if not done already
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned)time(NULL));
        seeded = true;
    }
    
    // Initialize all particles as inactive
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
    }
}

// Free any resources allocated by the explosion demo
void cleanupExplosionDemo(void) {
    // Currently no dynamic resources to clean up
}

// Helper function to get a random float between min and max
static float randomRange(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

// Helper function to get a random color
static Color randomColor(void) {
    // Prefer brighter colors for better visibility
    return (Color){
        (uint8_t)(128 + rand() % 128),
        (uint8_t)(128 + rand() % 128),
        (uint8_t)(128 + rand() % 128)
    };
}

// Find an available particle slot or reuse the oldest one if none available
static int findAvailableParticleSlot(void) {
    // First try to find an inactive particle
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
            return i;
        }
    }
    
    // If all particles are active, find the oldest one to reuse
    int oldestIndex = 0;
    float oldestAge = 0.0f;
    
    for (int i = 0; i < MAX_PARTICLES; i++) {
        float ageRatio = particles[i].age / particles[i].max_age;
        if (ageRatio > oldestAge) {
            oldestAge = ageRatio;
            oldestIndex = i;
        }
    }
    
    return oldestIndex;
}

// Create a new explosion at the specified canvas coordinates
void handleClickExplosion(float canvasX, float canvasY) {
    // Create PARTICLES_PER_EXPLOSION new particles
    for (int i = 0; i < PARTICLES_PER_EXPLOSION; i++) {
        // Find an available particle slot
        int index = findAvailableParticleSlot();
        
        // Initialize the particle
        particles[index].cx = canvasX;
        particles[index].cy = canvasY;
        
        // Random direction (in radians)
        float angle = randomRange(0, 2.0f * M_PI);
        float speed = randomRange(PARTICLE_MIN_SPEED, PARTICLE_MAX_SPEED);
        
        // Convert angle and speed to velocity components
        particles[index].dx = cosf(angle) * speed;
        particles[index].dy = sinf(angle) * speed;
        
        // Random size, color, rotation
        particles[index].size = randomRange(PARTICLE_MIN_SIZE, PARTICLE_MAX_SIZE);
        particles[index].color = randomColor();
        particles[index].angle = randomRange(0, 2.0f * M_PI);
        particles[index].rotation_speed = randomRange(-10.0f, 10.0f);
        
        // Lifetime
        particles[index].age = 0.0f;
        particles[index].max_age = randomRange(PARTICLE_MIN_LIFETIME, PARTICLE_MAX_LIFETIME);
        particles[index].active = true;
    }
}

// Update all active particles
void updateExplosion(float dt) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        
        // Update age and check if particle has expired
        particles[i].age += dt;
        if (particles[i].age >= particles[i].max_age) {
            particles[i].active = false;
            continue;
        }
        
        // Update position based on velocity
        particles[i].cx += particles[i].dx * dt;
        particles[i].cy += particles[i].dy * dt;
        
        // Update rotation
        particles[i].angle += particles[i].rotation_speed * dt;
        
        // Add a simple gravity effect
        particles[i].dy -= 50.0f * dt;
        
        // Apply some drag to slow particles over time
        float drag = 0.95f;
        particles[i].dx *= powf(drag, dt * 10.0f);
        particles[i].dy *= powf(drag, dt * 10.0f);
        
        // Fade out the particle as it gets older
        // (Ratio used later for color fading during rendering)
        
        // Check if particle is outside the canvas bounds with a margin
        float margin = 50.0f; // Allow particles to go slightly off-screen
        if (particles[i].cx < -canvasWidth/2.0f - margin || 
            particles[i].cx > canvasWidth/2.0f + margin ||
            particles[i].cy < -canvasHeight/2.0f - margin || 
            particles[i].cy > canvasHeight/2.0f + margin) {
            particles[i].active = false;
        }
    }
}

// Render all active particles
void renderExplosion(Canvas* canvas) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        
        // Create a temporary triangle with the particle properties
        Triangle t;
        t.cx = particles[i].cx;
        t.cy = particles[i].cy;
        t.size = particles[i].size;
        t.angle = particles[i].angle;
        
        // Fade out color as the particle ages
        float fadeRatio = 1.0f - (particles[i].age / particles[i].max_age);
        t.color.r = (uint8_t)(particles[i].color.r * fadeRatio);
        t.color.g = (uint8_t)(particles[i].color.g * fadeRatio);
        t.color.b = (uint8_t)(particles[i].color.b * fadeRatio);
        
        // Draw the particle as a small triangle
        drawTriangle(canvas, &t);
    }
}
