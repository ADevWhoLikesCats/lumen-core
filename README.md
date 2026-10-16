```markdown
# Lumen Core

A deferred OpenGL renderer with integrated Bullet physics and
cross-platform audio. Currently a hobby engine — renders, simulates,
and plays sound. Drives a car.

## Status

Working:
- Deferred PBR pipeline
- Cascaded shadow maps (3 cascades)
- SSAO, SSGI, SSR
- Spherical-harmonics ambient + SG probe specular
- Volumetric fog (half-res raymarch)
- TAA
- ACES tonemapping
- Bullet physics: static world, dynamic rigid bodies, contact events
- miniaudio: 3D spatialized one-shots, cross-platform
- Arcade vehicle: kinematic chassis, code-driven motion, drift model
- glTF/GLB loading with PBR materials and material fixups

Planned:
- Ramps and a proper track
- Vertical velocity + ground raycast (jumps)
- Wheel spin, suspension, proper vehicle physics
- Character controller toggle (FPS ↔ car)
- Editor / scene format

## Requirements

- C++17 compiler (g++ 10+, clang 12+, MSVC 2019+)
- OpenGL 3.3 core
- GLFW 3.3+
- GLM
- Bullet 3.x
- Linux, macOS, or Windows

## Build

### CMake (recommended, cross-platform)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/lumen
```

### Make (Linux/macOS)

```bash
make -j$(nproc)
./lumen
```

### Dependencies

**Debian/Ubuntu:**

```bash
sudo apt install build-essential cmake libglfw3-dev libglm-dev libbullet-dev
```

**Fedora:**

```bash
sudo dnf install gcc-c++ cmake glfw-devel glm-devel bullet-devel
```

**Arch:**

```bash
sudo pacman -S base-devel cmake glfw glm bullet
```

**macOS:**

```bash
brew install cmake glfw glm bullet
```

**Windows (vcpkg):**

```
vcpkg install glfw3 glm bullet3
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

## Run

```bash
./lumen
```

The engine loads assets relative to the working directory. Run from the
repository root.

## Controls

| Key | Action |
|-----|--------|
| W | Accelerate |
| S | Brake / reverse |
| A | Steer left |
| D | Steer right |
| Space | Handbrake |
| C | Toggle chase / orbit camera |
| P | Physics debug overlay |
| V | Vehicle debug logging |
| K | Cycle car modelFix (debugging) |
| F | Play test beep |
| Escape | Quit |

## Project layout

```
src/
  core/         asset loading, math, time, collision primitives
  render/       framebuffer, shaders, meshes, textures, shadowmap
  scene/        glb loading, map, probe grids, car renderer
  physics/      Bullet integration, vehicle, debug draw
  audio/        miniaudio wrapper, impact events
  main.cpp
include/        third-party headers (glad, glm, stb, miniaudio)
shaders/        GLSL sources
assets/         models, textures, sounds
```

## Architecture notes

**Rendering.** Four-attachment G-buffer (lit color, albedo, normal+roughness,
emissive). Geometry pass writes to it, then SSAO/SSGI/SSR/fog composite on
top before TAA and ACES tonemapping.

**Physics.** Bullet with a DBVT broadphase. The player controller sweeps a
capsule against the world. Dynamic bodies are added via `PhysicsWorld`.
Contact events feed the audio system.

**Vehicle.** The chassis is a *kinematic* rigid body — code owns its
position, yaw, and speed; Bullet only handles collision against ramps and
walls. This removes the friction/solver fighting that makes dynamic-vehicle
handling unpredictable. Drift is a grip factor that controls how fast the
velocity direction catches up to the car's facing.

**Audio.** miniaudio engine, one-shot 3D spatialized sounds. Physics
contacts above a velocity threshold are pushed to an impact queue, drained
each frame, and played at the contact point.

## Known limitations

- Vehicle has no vertical velocity yet — no jumps off ramps
- Wheels spin visually but aren't physics-simulated
- No track yet, just a flat grey plane
- Per-primitive glb texture loads duplicate uploads
- Glass rendered opaque (deferred pipeline has no transparency pass)
- Some assets (spec-gloss exports from 3DS Max) need per-file material fixups

## License

[Add a license before anyone else uses this.]

## Name

"Lumen Core" — the renderer that was originally "Lumen Engine" grew a
physics body and an audio system, so the name got shorter and the project
got bigger.
```

That's a working README for where the project actually is. Two things to decide before you save it:

1. **License** — replace the placeholder. MIT is the standard permissive choice if you want people to use this freely. If you're not sure, don't add one yet; the current text just says `[Add a license]`.

2. **The "Status" section** — I described what actually works today. If anything in there is inaccurate (e.g., a feature that's broken or half-wired), tell me and I'll fix it. It's better for the README to under-claim than over-claim.

Save it, then commit:

```bash
cd ~/Downloads/lumen-core
cat > README.md <<'EOF'
... (paste the above)
EOF

git add README.md
git commit -m "Rewrite README: current status, build, controls, architecture"
```

