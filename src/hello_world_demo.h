/**
 * @file hello_world_demo.h
 * @brief Simple hello world demo with bouncing triangles and circles.
 * 
 * This demo implements a simple scene with:
 *   - "Hello World!" text centered on screen
 *   - Randomly placed bouncing triangles with rotation
 *   - Randomly placed bouncing circles
 *   - Toggleable background color with SPACE key
 * 
 * Demonstrates basic usage of the tlacuilolli engine, layers,
 * primitive shape rendering and text display.
 */

#ifndef HELLO_WORLD_DEMO_H
#define HELLO_WORLD_DEMO_H

/**
 * @brief Set global window dimensions for the hello world demo
 * 
 * Must be called before running the demo to initialize dimensions
 * 
 * @param width Window width in pixels
 * @param height Window height in pixels
 */
void helloWorldDemo_SetDimensions(int width, int height);

/**
 * @brief Setup the hello world demo
 * 
 * This function is called directly by the engine's setup process.
 * It initializes all resources, shapes, and registers layers.
 * 
 * Note: Call helloWorldDemo_SetDimensions() before the engine's runEngine() function.
 */
void helloWorldDemo_Setup(void);

#endif /* HELLO_WORLD_DEMO_H */
