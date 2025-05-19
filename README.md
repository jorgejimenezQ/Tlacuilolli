# Tlacuilolli Engine

**Tlacuilolli** is a minimalist, CPU-rendered 2D game engine written in C using SDL2. It is built for rapid experimentation, visualization, and simulation using geometric primitives, with a focus on simplicity, flexibility, and real-time control.

---

## ✨ Features

- 🖼️ Software-based pixel drawing with double buffering
- 🔺 Triangle, line, and primitive rendering
- 🎯 Input abstraction (keyboard & mouse)
- 🧠 Layer-based update/render loop
- 💥 Triangle explosion & physics demos
- 📝 Text rendering with TTF font support
- 🧩 Modular demo integration (setup/update)
- 🧠 Clean C architecture for learning and expansion

---

## 🚀 Getting Started

### Prerequisites

Install SDL2 and SDL_ttf:

```bash
# Ubuntu/Debian
sudo apt install libsdl2-dev libsdl2-ttf-dev

# macOS
brew install sdl2 sdl2_ttf
```

### Build the Project

```bash
make
./renderer
```

---

## 📚 Engine Usage Tutorial

### Step 1: Create a Demo

Create `include/my_demo.h`:

```c
#ifndef MY_DEMO_H
#define MY_DEMO_H
void myDemo_Setup(void);
#endif
```

Create `src/my_demo.c`:

```c
#include "../include/engine.h"
#include "../include/canvas.h"
#include "../include/input.h"
#include "../include/my_demo.h"

static void update(float dt) {
    if (wasKeyJustPressed(SDL_SCANCODE_SPACE)) {
        printf("Spacebar pressed!\n");
    }
}

static void render(void) {
    Canvas* canvas = getCanvas();
    Triangle t = { 0, 0, 30, {255, 0, 0}, 0, 0 };
    drawTriangle(canvas, &t);
}

static Layer demoLayer = {
    "MyDemo",
    update,
    render,
    true
};

void myDemo_Setup(void) {
    registerLayer(&demoLayer);
}
```

### Step 2: Register Your Demo in `main.c`

```c
#include "../include/engine.h"
#include "../include/my_demo.h"

void setup(void) {
    myDemo_Setup();
}

int main(void) {
    return runEngine("My Demo", 800, 600, 60);
}
```

---

## 🔑 Core APIs

### Drawing

- `Canvas_PutPixel(canvas, x, y, Color)`
- `drawTriangle(Canvas*, Triangle*)`
- `Canvas_Update()` – already called by engine

### Input

- `isKeyPressed(SDL_SCANCODE_X)`
- `wasKeyJustPressed(SDL_SCANCODE_X)`
- `getMouseX()`, `getMouseY()`, `isLeftMousePressed()`

### Text

```c
textInit("assets/fonts/Ribeye-Regular.ttf", 24);
textDraw(canvas, 0, 100, "Hello World!", (Color){255,255,255});
```

---

## 🔄 Built-in Demos

| Demo      | Header             | Description                        |
| --------- | ------------------ | ---------------------------------- |
| Explosion | `explosion_demo.h` | Click to spawn triangle fireworks  |
| Physics   | `physics_demo.h`   | Basic triangle gravity + collision |
| Text      | `text.h`           | Draw bitmap text on screen         |

---

## 📌 Roadmap Ideas

- Geometry-based RTS battles
- Symbol-overlaid units
- Scenario challenges using shapes
- Morale/health tracking
- Replay mode
- Editor interface for layout

---

## 🔣 Name Meaning

**Tlacuilolli** is Nahuatl for "something painted or inscribed" — a reference to abstract, symbolic drawing and design.

---
