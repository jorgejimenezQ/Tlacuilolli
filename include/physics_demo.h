#ifndef PHYSICS_DEMO_H
#define PHYSICS_DEMO_H

#include "canvas.h"
#include <stdbool.h>

// Number of physics objects to simulate
#define PHYSICS_COUNT 2000

// Number of square obstacles in the scene
#define OBSTACLE_COUNT 5

// Constants for physics simulation
#define GRAVITY_ACCELERATION 500.0f

// Default gravity scale (1.0 = normal gravity)
extern float gravityScale;
#define RESTITUTION 0.6f         // Bounciness factor
#define FRICTION 0.8f            // Friction factor on collision
#define PROJECTILE_SPEED 300.0f  // Initial speed of projectiles
#define PROJECTILE_SIZE 6.0f     // Size of projectile triangles
#define OBJECT_MIN_SIZE 2.0f     // Minimum size of physics objects
#define OBJECT_MAX_SIZE 8.0f     // Maximum size of physics objects

// Physics object structure
typedef struct {
    float cx, cy;              // Position
    float vx, vy;              // Velocity 
    float size;                // Size of the triangle
    float angle;               // Current rotation angle
    float angularVelocity;     // Rotation speed
    Color color;               // Object color
    bool active;               // Whether the object is currently active
} PhysicsObject;

// Square obstacle that physics objects can collide with
typedef struct {
    float cx, cy;         // Center position
    float width, height;   // Dimensions of the square
    float angle;          // Rotation angle (in radians)
    Color color;          // Color of the square
    bool active;          // Whether the obstacle is active
} Obstacle;

// Initialize the physics demo
void initPhysicsDemo(int canvasW, int canvasH);

// Clean up resources used by the physics demo
void cleanupPhysicsDemo(void);

// Spawn a projectile from a position aimed at a target position
void spawnProjectile(float x, float y, float targetX, float targetY);

// Update all physics objects
void updatePhysics(float dt);

// Render all physics objects
void renderPhysics(Canvas* canvas);

// Set the gravity scale factor (0.0 = no gravity, 1.0 = normal gravity)
void setGravityScale(float scale);

// Make all active physics objects jump with the specified impulse
void jumpAllObjects(float impulse);

#endif // PHYSICS_DEMO_H
