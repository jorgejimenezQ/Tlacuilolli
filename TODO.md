# Tlacuilolli Graphics Engine TODO List

## Future Enhancements

### 1. Particle System Layer

Implement a particle system layer that takes advantage of our new layer architecture:

- [ ] Create a basic Particle struct with:
  - Position (x, y)
  - Velocity (vx, vy)
  - Color
  - Size
  - Lifetime
  - Alpha (optional, for fadeout effects)

- [ ] Implement particle management:
  - Particle pool/array for efficient reuse
  - Functions to spawn particles with various initial parameters
  - Batch rendering for performance

- [ ] Create particle effects:
  - Explosion effect (radial particles)
  - Dust/impact effects for collisions
  - Smoke/fire/water particle types
  - Particle emitters that continuously spawn particles

- [ ] Physics integration:
  - Make particles respect gravity
  - Optional collision with scene geometry
  - Wind or force field effects

- [ ] Performance optimizations:
  - View frustum culling for particles
  - Level-of-detail based on distance
  - Instanced rendering for similar particles

### 2. UI Layer

- [ ] Implement a UI layer that sits on top of all other layers
- [ ] Basic UI widgets (buttons, sliders, text)
- [ ] Input handling specific to UI elements

### 3. Debug/Development Layer

- [ ] Frame rate counter
- [ ] Physics visualization (collision boxes, vectors)
- [ ] Memory usage statistics
- [ ] Toggle-able via keyboard shortcut
