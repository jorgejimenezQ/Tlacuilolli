#include "../include/physics_demo.h"
#include "../include/triangle.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

// Global gravity scale variable
float gravityScale = 1.0f;

// Canvas dimensions for collision detection
static int canvasWidth, canvasHeight;

// Array of physics objects
static PhysicsObject objects[PHYSICS_COUNT];

// Array of square obstacles
static Obstacle obstacles[OBSTACLE_COUNT];

// Ensure we have the declaration for the triangle drawing function
extern void drawTriangle(Canvas* canvas, const Triangle* t);

// Helper function to get a random float between min and max
static float randomRange(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

// Helper function to get a random color
static Color randomColor(void) {
    return (Color){
        (uint8_t)(128 + rand() % 128),
        (uint8_t)(128 + rand() % 128),
        (uint8_t)(128 + rand() % 128)
    };
}

// Helper function to create a square with specific properties
static void createObstacle(int index, float x, float y, float width, float height, float angle, Color color) {
    obstacles[index].cx = x;
    obstacles[index].cy = y;
    obstacles[index].width = width;
    obstacles[index].height = height;
    obstacles[index].angle = angle;
    obstacles[index].color = color;
    obstacles[index].active = true;
}

// Function to draw a rotated rectangle
static void drawRotatedRectangle(Canvas* canvas, const Obstacle* obstacle) {
    // Calculate the four corners of the rectangle
    float halfWidth = obstacle->width / 2.0f;
    float halfHeight = obstacle->height / 2.0f;
    
    // Pre-calculate sine and cosine of the angle
    float cosa = cosf(obstacle->angle);
    float sina = sinf(obstacle->angle);
    
    // Calculate corner positions (rotated around center)
    float corners[4][2] = {
        { -halfWidth, -halfHeight }, // Top-left
        {  halfWidth, -halfHeight }, // Top-right
        {  halfWidth,  halfHeight }, // Bottom-right
        { -halfWidth,  halfHeight }  // Bottom-left
    };
    
    int xPoints[4], yPoints[4];
    
    // Rotate and translate each corner
    for (int i = 0; i < 4; i++) {
        // Rotate point around origin
        float rx = corners[i][0] * cosa - corners[i][1] * sina;
        float ry = corners[i][0] * sina + corners[i][1] * cosa;
        
        // Just directly add the rotated point to the obstacle center
        // Our physics is working correctly, and the triangles are interacting properly
        // So we'll keep this simple coordinate transformation
        xPoints[i] = (int)(obstacle->cx + rx);
        yPoints[i] = (int)(obstacle->cy + ry);
    }
    
    // Draw the four lines of the rectangle
    for (int i = 0; i < 4; i++) {
        int next = (i + 1) % 4;
        drawLine(canvas, xPoints[i], yPoints[i], xPoints[next], yPoints[next], obstacle->color);
    }
}

// Initialize the physics demo
void initPhysicsDemo(int canvasW, int canvasH) {
    // Store canvas dimensions for collision detection
    canvasWidth = canvasW;
    canvasHeight = canvasH;
    
    // Seed random number generator if not already done
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }
    
    // Initialize physics objects with random properties
    for (int i = 0; i < PHYSICS_COUNT; i++) {
        objects[i].cx = randomRange(-canvasW / 3.0f, canvasW / 3.0f);
        objects[i].cy = randomRange(0, canvasH / 2.0f); // Start from upper half
        objects[i].vx = randomRange(-50.0f, 50.0f);
        objects[i].vy = randomRange(-20.0f, 50.0f);
        objects[i].size = randomRange(OBJECT_MIN_SIZE, OBJECT_MAX_SIZE);
        objects[i].angle = randomRange(0, 2.0f * M_PI);
        objects[i].angularVelocity = randomRange(-2.0f, 2.0f);
        objects[i].color = randomColor();
        objects[i].active = true;
    }
    
    // Leave some objects inactive initially
    for (int i = 0; i < PHYSICS_COUNT / 5; i++) {
        objects[rand() % PHYSICS_COUNT].active = false;
    }
    
    // Initialize obstacle squares with different positions and sizes
    float halfW = canvasW / 2.0f;
    float halfH = canvasH / 2.0f;
    
    // 1. Left platform
    createObstacle(0, -halfW * 0.6f, -halfH * 0.2f, 120.0f, 30.0f, 0.2f, 
                   (Color){ 50, 200, 50 });
                   
    // 2. Right platform
    createObstacle(1, halfW * 0.6f, -halfH * 0.3f, 120.0f, 30.0f, -0.2f, 
                   (Color){ 50, 50, 200 });
                   
    // 3. Center obstacle
    createObstacle(2, 0.0f, 0.0f, 100.0f, 100.0f, M_PI / 4.0f, 
                   (Color){ 200, 50, 50 });
                   
    // 4. Top-left obstacle
    createObstacle(3, -halfW * 0.5f, halfH * 0.5f, 70.0f, 70.0f, 0.0f, 
                   (Color){ 200, 200, 50 });
                   
    // 5. Bottom-center small obstacle
    createObstacle(4, 0.0f, -halfH * 0.7f, 50.0f, 50.0f, M_PI / 6.0f, 
                   (Color){ 200, 50, 200 });
    
    printf("Physics demo initialized with %d active objects and %d obstacles\n", 
           PHYSICS_COUNT, OBSTACLE_COUNT);
}

// Clean up resources used by the physics demo
void cleanupPhysicsDemo(void) {
    // Currently no dynamic resources to clean up
}

// Find an inactive object to reuse, or overwrite the oldest one
static int findAvailableObjectSlot(void) {
    // First check for inactive objects
    for (int i = 0; i < PHYSICS_COUNT; i++) {
        if (!objects[i].active) {
            return i;
        }
    }
    
    // If all active, select a random one
    return rand() % PHYSICS_COUNT;
}

// Spawn a projectile from a position aimed at a target position
void spawnProjectile(float x, float y, float targetX, float targetY) {
    // Find an available slot
    int index = findAvailableObjectSlot();
    
    // Calculate direction vector
    float dx = targetX - x;
    float dy = targetY - y;
    
    // Normalize direction vector
    float length = sqrtf(dx*dx + dy*dy);
    if (length < 0.0001f) {
        // Avoid division by zero, shoot upward if target is same as origin
        dx = 0;
        dy = 1;
    } else {
        dx /= length;
        dy /= length;
    }
    
    // Initialize the projectile
    objects[index].cx = x;
    objects[index].cy = y;
    objects[index].vx = dx * PROJECTILE_SPEED;
    objects[index].vy = dy * PROJECTILE_SPEED;
    objects[index].size = PROJECTILE_SIZE;
    objects[index].angle = atan2f(dy, dx);
    objects[index].angularVelocity = randomRange(-3.0f, 3.0f);
    
    // Make projectile a bright color to stand out
    objects[index].color = (Color){
        (uint8_t)(180 + rand() % 75),
        (uint8_t)(180 + rand() % 75),
        (uint8_t)(180 + rand() % 75)
    };
    
    objects[index].active = true;
}

// Set the gravity scale factor
void setGravityScale(float scale) {
    gravityScale = scale;
}

// Make all active physics objects jump with the specified impulse
void jumpAllObjects(float impulse) {
    // Apply an upward velocity impulse to all active objects
    for (int i = 0; i < PHYSICS_COUNT; i++) {
        if (objects[i].active) {
            // Add the impulse to the y velocity (upward)
            objects[i].vy += impulse;
            
            // Add some random horizontal movement for variety
            objects[i].vx += randomRange(-20.0f, 20.0f);
            
            // Add some random spin
            objects[i].angularVelocity += randomRange(-2.0f, 2.0f);
        }
    }
}

// Check if a point is inside a rotated rectangle
static bool pointInRotatedRect(float px, float py, const Obstacle* rect) {
    // Translate point to origin
    float tx = px - rect->cx;
    float ty = py - rect->cy;
    
    // Rotate point in the opposite direction of rectangle
    float cosa = cosf(-rect->angle);
    float sina = sinf(-rect->angle);
    float rx = tx * cosa - ty * sina;
    float ry = tx * sina + ty * cosa;
    
    // Now check if point is within the axis-aligned rectangle
    float halfW = rect->width / 2.0f;
    float halfH = rect->height / 2.0f;
    
    return (rx >= -halfW && rx <= halfW && ry >= -halfH && ry <= halfH);
}

// Calculate the closest point on a line segment to a given point
static void closestPointOnLine(float px, float py, float x1, float y1, float x2, float y2, float* outX, float* outY) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len2 = dx*dx + dy*dy;
    
    if (len2 < 0.0001f) {
        // The line segment is just a point
        *outX = x1;
        *outY = y1;
        return;
    }
    
    // Calculate projection of point onto line
    float t = ((px - x1) * dx + (py - y1) * dy) / len2;
    
    // Clamp t to [0,1] to stay within the line segment
    t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
    
    *outX = x1 + t * dx;
    *outY = y1 + t * dy;
}

// Find closest point on a rotated rectangle to a given point
static void closestPointOnRect(float px, float py, const Obstacle* rect, float* outX, float* outY) {
    // First check if the point is inside the rectangle
    if (pointInRotatedRect(px, py, rect)) {
        *outX = px;
        *outY = py;
        return;
    }
    
    // Calculate the four corners of the rectangle
    float halfW = rect->width / 2.0f;
    float halfH = rect->height / 2.0f;
    float cosa = cosf(rect->angle);
    float sina = sinf(rect->angle);
    
    // Calculate corner positions (rotated around center)
    float corners[4][2] = {
        { rect->cx + (-halfW * cosa - (-halfH) * sina), rect->cy + (-halfW * sina + (-halfH) * cosa) }, // Top-left
        { rect->cx + (halfW * cosa - (-halfH) * sina), rect->cy + (halfW * sina + (-halfH) * cosa) },   // Top-right
        { rect->cx + (halfW * cosa - halfH * sina), rect->cy + (halfW * sina + halfH * cosa) },          // Bottom-right
        { rect->cx + (-halfW * cosa - halfH * sina), rect->cy + (-halfW * sina + halfH * cosa) }         // Bottom-left
    };
    
    // Find the closest point on each edge
    float closestDist = 1e10f;
    float cx = 0.0f, cy = 0.0f;
    
    for (int i = 0; i < 4; i++) {
        int next = (i + 1) % 4;
        float tmpX, tmpY;
        
        closestPointOnLine(px, py, corners[i][0], corners[i][1], corners[next][0], corners[next][1], &tmpX, &tmpY);
        
        float dist = (tmpX - px) * (tmpX - px) + (tmpY - py) * (tmpY - py);
        if (dist < closestDist) {
            closestDist = dist;
            cx = tmpX;
            cy = tmpY;
        }
    }
    
    *outX = cx;
    *outY = cy;
}

// Handle collision between a physics object and an obstacle
static void handleObstacleCollision(PhysicsObject* obj, Obstacle* obstacle) {
    // First predict where the object would be after the current velocity is applied
    // This helps catch tunneling for fast-moving objects
    float predictedX = obj->cx + obj->vx * 0.016f; // Predict one frame ahead at 60fps
    float predictedY = obj->cy + obj->vy * 0.016f;
    
    // Find the closest point on the obstacle to both current and predicted positions
    float closestX, closestY;
    float closestPredX, closestPredY;
    closestPointOnRect(obj->cx, obj->cy, obstacle, &closestX, &closestY);
    closestPointOnRect(predictedX, predictedY, obstacle, &closestPredX, &closestPredY);
    
    // Calculate distance between object center and closest point
    float dx = obj->cx - closestX;
    float dy = obj->cy - closestY;
    float distSquared = dx*dx + dy*dy;
    
    // Also check predicted position
    float predDx = predictedX - closestPredX;
    float predDy = predictedY - closestPredY;
    float predDistSquared = predDx*predDx + predDy*predDy;
    
    // Set effective radius with a small safety margin to prevent clipping
    float effectiveRadius = obj->size * 1.1f;
    
    // Check if either current or predicted position has a collision
    if (distSquared < effectiveRadius * effectiveRadius || 
        predDistSquared < effectiveRadius * effectiveRadius) {
        
        // Use the current position for collision response
        float dist = sqrtf(distSquared);
        if (dist < 0.0001f) dist = 0.0001f; // Avoid division by zero
        
        // Calculate normal vector (from obstacle to object)
        float nx = dx / dist;
        float ny = dy / dist;
        
        // Calculate relative velocity along normal
        float velAlongNormal = obj->vx * nx + obj->vy * ny;
        
        // Calculate bounce response (even if not moving toward obstacle)
        // This ensures objects don't get stuck inside obstacles
        float impulse = -(1.0f + RESTITUTION) * velAlongNormal;
        
        // Apply stronger impulse for objects moving toward the obstacle
        if (velAlongNormal < 0) {
            impulse *= 1.2f;  // Stronger bounce for incoming objects
        }
        
        // Apply impulse to object velocity
        obj->vx += impulse * nx;
        obj->vy += impulse * ny;
        
        // Calculate tangent vector
        float tx = -ny;
        float ty = nx;
        
        // Calculate velocity along tangent
        float velAlongTangent = obj->vx * tx + obj->vy * ty;
        
        // Apply friction impulse along tangent
        float frictionImpulse = -velAlongTangent * FRICTION;
        obj->vx += frictionImpulse * tx;
        obj->vy += frictionImpulse * ty;
        
        // Add some random rotation on collision
        obj->angularVelocity += randomRange(-1.0f, 1.0f);
        
        // Move object out of collision with extra safety margin
        float penetration = effectiveRadius - dist;
        obj->cx += nx * penetration * 1.2f; // Stronger push to avoid sticking/clipping
        obj->cy += ny * penetration * 1.2f;
    }
}

// Update all physics objects
void updatePhysics(float dt) {
    float halfWidth = canvasWidth / 2.0f;
    float halfHeight = canvasHeight / 2.0f;
    
    // First, update positions and handle wall collisions
    for (int i = 0; i < PHYSICS_COUNT; i++) {
        if (!objects[i].active) continue;
        
        // Apply gravity with scaling factor
        objects[i].vy -= GRAVITY_ACCELERATION * gravityScale * dt;
        
        // Update position
        objects[i].cx += objects[i].vx * dt;
        objects[i].cy += objects[i].vy * dt;
        
        // Update rotation
        objects[i].angle += objects[i].angularVelocity * dt;
        
        // Handle collisions with ground (bottom)
        if (objects[i].cy < -halfHeight) {
            objects[i].cy = -halfHeight;
            objects[i].vy = -objects[i].vy * RESTITUTION;  // Bounce with energy loss
            objects[i].vx *= FRICTION;                    // Apply friction
            
            // Apply a small random spin on bounce
            objects[i].angularVelocity += randomRange(-0.5f, 0.5f);
        }
        
        // Handle collisions with ceiling (top)
        if (objects[i].cy > halfHeight) {
            objects[i].cy = halfHeight;
            objects[i].vy = -objects[i].vy * RESTITUTION;  // Bounce with energy loss
            objects[i].vx *= FRICTION;                    // Apply friction
        }
        
        // Handle collisions with left wall
        if (objects[i].cx < -halfWidth) {
            objects[i].cx = -halfWidth;
            objects[i].vx = -objects[i].vx * RESTITUTION;  // Bounce with energy loss
            objects[i].vy *= FRICTION;                    // Apply friction
            
            // Apply a small random spin on bounce
            objects[i].angularVelocity += randomRange(-0.5f, 0.5f);
        }
        
        // Handle collisions with right wall
        if (objects[i].cx > halfWidth) {
            objects[i].cx = halfWidth;
            objects[i].vx = -objects[i].vx * RESTITUTION;  // Bounce with energy loss
            objects[i].vy *= FRICTION;                    // Apply friction
            
            // Apply a small random spin on bounce
            objects[i].angularVelocity += randomRange(-0.5f, 0.5f);
        }
        
        // Handle collisions with obstacles
        for (int j = 0; j < OBSTACLE_COUNT; j++) {
            if (obstacles[j].active) {
                handleObstacleCollision(&objects[i], &obstacles[j]);
            }
        }
        
        // Add some damping to gradually slow objects down
        objects[i].vx *= 0.999f;
        objects[i].vy *= 0.999f;
        objects[i].angularVelocity *= 0.998f;
        
        // Deactivate objects that are nearly stopped and at the bottom
        if (fabsf(objects[i].vx) < 1.0f && 
            fabsf(objects[i].vy) < 1.0f && 
            objects[i].cy < -halfHeight + 2.0f) {
            
            // Small chance to deactivate stopped objects
            if (rand() % 100 < 2) {
                objects[i].active = false;
            }
        }
    }
    
    // Then, check for inter-object collisions
    // Only compare each pair once (i,j where i < j)
    for (int i = 0; i < PHYSICS_COUNT - 1; i++) {
        if (!objects[i].active) continue;
        
        for (int j = i + 1; j < PHYSICS_COUNT; j++) {
            if (!objects[j].active) continue;
            
            // Check for collision using circle approximation
            float dx = objects[i].cx - objects[j].cx;
            float dy = objects[i].cy - objects[j].cy;
            float distSquared = dx*dx + dy*dy;
            
            float r = objects[i].size + objects[j].size;
            float rSquared = r*r;
            
            // If objects are overlapping
            if (distSquared < rSquared) {
                float dist = sqrtf(distSquared);
                if (dist < 0.0001f) dist = 0.0001f;  // Avoid division by zero
                
                // Calculate normal vector (from j to i)
                float nx = dx / dist;
                float ny = dy / dist;
                
                // Calculate relative velocity
                float dvx = objects[i].vx - objects[j].vx;
                float dvy = objects[i].vy - objects[j].vy;
                
                // Calculate velocity along normal
                float velAlongNormal = dvx * nx + dvy * ny;
                
                // Only resolve if objects are moving toward each other
                if (velAlongNormal < 0) {
                    // Calculate impulse scalar
                    float impulse = -(1.0f + RESTITUTION) * velAlongNormal;
                    impulse /= 2.0f;  // Split impulse evenly between objects
                    
                    // Apply impulse to object velocities
                    objects[i].vx += impulse * nx;
                    objects[i].vy += impulse * ny;
                    objects[j].vx -= impulse * nx;
                    objects[j].vy -= impulse * ny;
                    
                    // Add friction to perpendicular component
                    // Calculate tangent vector (perpendicular to normal)
                    float tx = -ny;
                    float ty = nx;
                    
                    // Calculate velocity along tangent
                    float velAlongTangent = dvx * tx + dvy * ty;
                    
                    // Apply friction impulse along tangent
                    float frictionImpulse = -velAlongTangent * FRICTION;
                    frictionImpulse /= 2.0f;  // Split impulse evenly
                    
                    objects[i].vx += frictionImpulse * tx;
                    objects[i].vy += frictionImpulse * ty;
                    objects[j].vx -= frictionImpulse * tx;
                    objects[j].vy -= frictionImpulse * ty;
                }
                
                // Add some random rotation change on collision
                objects[i].angularVelocity += randomRange(-0.5f, 0.5f);
                objects[j].angularVelocity += randomRange(-0.5f, 0.5f);
                
                // Push objects apart to prevent sticking
                float overlap = (r - dist) * 0.55f;  // Slightly more separation to avoid repeat collisions
                objects[i].cx += nx * overlap * 0.5f;
                objects[i].cy += ny * overlap * 0.5f;
                objects[j].cx -= nx * overlap * 0.5f;
                objects[j].cy -= ny * overlap * 0.5f;
            }
        }
    }
}

// Render all physics objects
void renderPhysics(Canvas* canvas) {
    // First render the obstacles
    for (int i = 0; i < OBSTACLE_COUNT; i++) {
        if (obstacles[i].active) {
            drawRotatedRectangle(canvas, &obstacles[i]);
        }
    }
    
    // Then render the physics objects (triangles)
    for (int i = 0; i < PHYSICS_COUNT; i++) {
        if (!objects[i].active) continue;
        
        // Create a temporary triangle with the physics object properties
        Triangle t;
        t.cx = objects[i].cx;
        t.cy = objects[i].cy;
        t.size = objects[i].size;
        t.angle = objects[i].angle;
        t.color = objects[i].color;
        
        // Draw the triangle
        drawTriangle(canvas, &t);
    }
}
