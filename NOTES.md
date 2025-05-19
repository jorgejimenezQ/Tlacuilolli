\*\* Use GPU-accelerated rendering instead of CPU-based line drawing:
Replace the software rendering with direct OpenGL or SDL's hardware accelerated primitive rendering
This would be significantly faster for 50,000+ triangles

** Batch rendering:
Group triangles together and render them in batches
This reduces API calls and improves performance
** Frustum culling:
Only render triangles that are visible in the current viewport
Skip triangles that are off-screen
** Level of detail (LOD):
Render fewer/simpler triangles when they're smaller or farther away
Full detail only for close-up triangles
** Implement spatial partitioning:
Organize triangles in a spatial data structure like a quadtree
Only process triangles in visible regions
** SIMD instructions:
Use SIMD (Single Instruction Multiple Data) to process multiple triangles in parallel
This would require some restructuring of your data for better memory alignment
** Multi-threading:
Distribute rendering across multiple CPU cores
Could divide the triangle array into chunks processed by different threads

Designing a clean, flexible sprite/spritesheet API—especially one that supports animations—means balancing ease of use, performance, and extensibility. Below is a high-level sketch of what such an API might look like, organized into **Core Concepts**, **Key Features**, **Example Usage**, and **Best Practices**.

---

## Core Concepts

1. **SpriteSheet**

   - Loads one (or more) texture atlases plus accompanying metadata (JSON, XML, etc.).
   - Knows how to extract individual frames (rectangles) by name or index.

2. **Frame**

   - Represents a single sub-image: its source rectangle, trimmed size, pivot/origin point.

3. **Animation**

   - A named sequence of frames with per-frame durations.
   - Configurable looping modes (loop, ping-pong, once, reverse).

4. **Sprite**

   - Holds a reference to a `SpriteSheet` and a current `Animation`.
   - Exposes control methods (play, pause, stop, gotoFrame, etc.).
   - Knows how to update its internal timer and draw itself.

5. **Animator** (optional)

   - A higher-level controller for sequencing multiple `Sprite` animations or blending between them.

---

## Key Features

- **Metadata-Driven**
  Load frame definitions and animation sequences from a standard JSON schema:

  ```jsonc
  {
    "frames": {
      "walk_0":  { "x":0,   "y":0,  "w":32, "h":48, "origin": [16,48] },
      "walk_1":  { "x":32,  "y":0,  "w":32, "h":48, "origin": [16,48] },
      ...
    },
    "animations": {
      "walk": { "frames": ["walk_0","walk_1","walk_2","walk_1"], "durations": [0.1,0.1,0.1,0.1], "mode":"pingpong" },
      "attack": { "frames": ["atk_0","atk_1","atk_2"], "durations": [0.05,0.05,0.1], "mode":"once" }
    }
  }
  ```

- **Simple Load & Retrieval**

  ```js
  const sheet = await SpriteSheet.load("assets/hero.png", "assets/hero.json");
  ```

- **Dynamic Animation Creation**

  ```js
  // programmatic
  const jumpAnim = sheet.createAnimation(
  	"jump",
  	["jump_0", "jump_1", "jump_2"],
  	0.08,
  	{ loop: false }
  );
  ```

- **Playback Control**

  ```js
  sprite.play("walk");
  sprite.pause();
  sprite.stop();
  sprite.gotoFrame(2);
  ```

- **Frame Callbacks / Events**

  ```js
  sprite.onFrame(1, () => {
  	/* spawn sound */
  });
  sprite.onAnimationEnd(() => {
  	/* switch to idle */
  });
  ```

- **Transform & Rendering Options**

  - Position, rotation, scale, color tint
  - Flipping (horizontal/vertical)
  - Custom shaders or blend modes

  ```js
  sprite.draw(ctx, x, y, {
  	rotation: Math.PI / 4,
  	scaleX: -1, // flip
  	tint: { r: 1, g: 0.8, b: 0.8 },
  });
  ```

- **Batching / Atlas Optimization**

  - Group multiple sprites from the same sheet into one draw call.
  - Auto-upload of texture to GPU when first used.

- **Resource Management**

  - Reference counting for shared sheets.
  - Unload idle atlases.

---

## Example Usage

```js
// 1. Load assets
const heroSheet = await SpriteSheet.load("hero.png", "hero.json");
const enemySheet = await SpriteSheet.load("enemy.png", "enemy.json");

// 2. Create sprites
const hero = new Sprite(heroSheet, "idle"); // starts in "idle" animation
const goblin = new Sprite(enemySheet, "walk");

// 3. Register events
hero.onAnimationEnd(() => hero.play("idle"));
hero.onFrame(3, () => playSound("hero_step.wav"));

// 4. Game loop
function gameLoop(dt) {
	hero.update(dt);
	goblin.update(dt);

	// Draw in layers
	renderer.clear();
	renderer.draw(hero, 100, 200);
	renderer.draw(goblin, 300, 200);
}
```

---

## Best Practices & Extensions

- **Data-First Approach**
  Keep all frame and animation data external (JSON/XML). Let artists update metadata without touching code.

- **Immutable Frames**
  Once loaded, `Frame` objects should be read-only to avoid state bugs.

- **Lean Animation Class**
  Separate timing logic from rendering. Animation should simply produce “current frame index” for a given elapsed time.

- **Pooling & Reuse**
  Avoid `new`/`delete` per frame. Recycle `Sprite` instances or use pooling for frequent spawns.

- **Editor Integration**
  Provide a small tool or script to generate the JSON metadata from common sprite editors (TexturePacker, ShoeBox, etc.).

- **Extensible Loop Modes**
  Aside from standard modes, allow custom interpolation functions—e.g., reverse once, staggered start, random pick.

- **Hierarchical Sprites**
  Support “composite” sprites (e.g., a character made of body + weapon layers), each with independent animations.

- **Integration Hooks**
  Expose low-level hooks for embedding in larger rendering pipelines or engines (e.g., Unity, Godot, or custom engines).
