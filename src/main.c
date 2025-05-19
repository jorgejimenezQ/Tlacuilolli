/**
 * @file main.c
 * @brief Main entry point for the tlacuilolli engine demo
 * 
 * This file provides a minimal setup that delegates all demo functionality
 * to the hello_world_demo module. Instead of embedding all the demo code
 * directly in main.c, we've modularized it into a reusable component.
 * 
 * The hello_world_demo module contains all the implementation details for:
 * - Rendering bouncing triangles with random properties
 * - Rendering bouncing circles with physics
 * - Displaying "Hello World!" text
 * - Handling space key to toggle background color
 * 
 * @see hello_world_demo.h for the demo's API details
 */

#include <stdio.h>
#include "../include/engine.h"
#include "hello_world_demo.h" /* Import our hello world demo module */

/* Window configuration */
#define WINDOW_WIDTH   800 /* Window width in pixels */
#define WINDOW_HEIGHT  600 /* Window height in pixels */
#define TARGET_FPS     60  /* Target frame rate */

/**
 * @brief Setup function - automatically called by the engine at startup
 * 
 * This is the entry point for our application initialization.
 * We simply delegate to the hello_world_demo module's setup function.
 */
void setup(void) {
    /* Call the hello world demo's setup function directly */
    helloWorldDemo_Setup();
}

/**
 * @brief Main function - program entry point
 * 
 * Initializes the hello world demo with proper window dimensions,
 * then starts the engine which will call our setup() function.
 * 
 * @return Exit code (0 on success, non-zero on failure)
 */
int main(void) {
    /* Set dimensions for the hello world demo */
    helloWorldDemo_SetDimensions(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    /* Run the engine with the window parameters */
    return runEngine("Hello World Bouncing Shapes", WINDOW_WIDTH, WINDOW_HEIGHT, TARGET_FPS);
}
