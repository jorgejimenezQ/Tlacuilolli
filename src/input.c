#include "../include/input.h"
#include <SDL2/SDL.h>

// Current state
static bool currentKeys[SDL_NUM_SCANCODES];
static bool previousKeys[SDL_NUM_SCANCODES];
static bool currentMouseButtons[5];
static int mouseX, mouseY;  // Canvas coordinates (0,0 at center)
static int screenWidth, screenHeight;

// Initialize input system
void inputInit(int width, int height) {
    screenWidth = width;
    screenHeight = height;
    
    // Initialize key arrays
    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        currentKeys[i] = false;
        previousKeys[i] = false;
    }
    
    // Initialize mouse buttons
    for (int i = 0; i < 5; i++) {
        currentMouseButtons[i] = false;
    }
    
    // Initialize mouse position
    mouseX = 0;
    mouseY = 0;
}

void inputUpdate(void) {
    // Copy current state to previous state
    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        previousKeys[i] = currentKeys[i];
    }

    // Update current state from SDL
    const Uint8* keyboardState = SDL_GetKeyboardState(NULL);
    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        currentKeys[i] = keyboardState[i];
    }
    
    // Update mouse state
    Uint32 buttonState = SDL_GetMouseState(&mouseX, &mouseY);
    
    // Convert from screen coordinates to canvas coordinates
    mouseX = mouseX - screenWidth / 2;
    mouseY = screenHeight / 2 - mouseY;
    
    // Update mouse button states
    currentMouseButtons[0] = (buttonState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    currentMouseButtons[1] = (buttonState & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
    currentMouseButtons[2] = (buttonState & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    currentMouseButtons[3] = (buttonState & SDL_BUTTON(SDL_BUTTON_X1)) != 0;
    currentMouseButtons[4] = (buttonState & SDL_BUTTON(SDL_BUTTON_X2)) != 0;
}

// Keyboard functions
bool isKeyPressed(int scancode) {
    if (scancode < 0 || scancode >= SDL_NUM_SCANCODES) return false;
    return currentKeys[scancode];
}

bool wasKeyJustPressed(int scancode) {
    if (scancode < 0 || scancode >= SDL_NUM_SCANCODES) return false;
    return currentKeys[scancode] && !previousKeys[scancode];
}

bool wasKeyJustReleased(int scancode) {
    if (scancode < 0 || scancode >= SDL_NUM_SCANCODES) return false;
    return !currentKeys[scancode] && previousKeys[scancode];
}

// Mouse functions
bool isMousePressed(void) {
    return currentMouseButtons[0] || currentMouseButtons[1] || 
           currentMouseButtons[2] || currentMouseButtons[3] || 
           currentMouseButtons[4];
}

bool isLeftMousePressed(void) {
    return currentMouseButtons[0];
}

bool isRightMousePressed(void) {
    return currentMouseButtons[2];
}

int getMouseX(void) {
    return mouseX;
}

int getMouseY(void) {
    return mouseY;
}
