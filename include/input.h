#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

// Initialize input system with screen dimensions
void inputInit(int width, int height);

// Call each frame before update()
void inputUpdate(void);

// Keyboard
bool isKeyPressed(int scancode);      // held down
bool wasKeyJustPressed(int scancode); // transitioned this frame
bool wasKeyJustReleased(int scancode);

// Mouse
bool isMousePressed(void);            // any button
bool isLeftMousePressed(void);
bool isRightMousePressed(void);
int  getMouseX(void);                 // canvas coords
int  getMouseY(void);

#endif // INPUT_H
