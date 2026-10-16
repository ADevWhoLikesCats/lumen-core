# Photonix

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
- Arcade vehicle: dynamic chassis, velocity-only control,
  physics-driven gravity and ramp response
- glTF/GLB loading with PBR materials and material fixups
- Ramps: car climbs and launches off them

Planned:
- Full Asphalt-style track (Blender map in progress)
- Wheel spin animation tuning
- Character controller toggle (FPS <-> car)
- Editor / scene format
- Loop-de-loop / upside-down driving (needs track-normal force)

## Requirements

- C++17 compiler (g++ 10+, clang 12+, MSVC 2019+)
- OpenGL 3.3 core
- GLFW 3.3+
- GLM
- Bullet 3.x
- Linux, macOS, or Windows

## Build

### CMake (recommended, cross-platform)

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j
    ./build/lumen

### Make (Linux/macOS)

    make -j$(nproc)
    ./lumen

### Dependencies

Debian/Ubuntu:

    sudo apt install build-essential cmake libglfw3-dev libglm-dev libbullet-dev

Fedora:

    sudo dnf install gcc-c++ cmake glfw-devel glm-devel bullet-devel

Arch:

    sudo pacman -S base-devel cmake glfw glm bullet

macOS:

    brew install cmake glfw glm bullet

Windows (vcpkg):

    vcpkg install glfw3 glm bullet3
    cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake
    cmake --build build --config Release

## Run

    ./lumen

Assets are loaded relative to the working directory. Run from the
repository root.

## Controls

    W       Accelerate
    S       Brake / reverse
    A       Steer left
    D       Steer right
    Space   Handbrake
    C       Toggle chase / orbit camera
    P       Physics debug overlay
    V       Vehicle debug logging
    K       Cycle car modelFix (debugging)
    F       Play test beep
    Escape  Quit

## Project layout

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

## Architecture notes

**Rendering.** Four-attachment G-buffer (lit color, albedo,
normal+roughness, emissive). Geometry pass writes to it, then
SSAO/SSGI/SSR/fog composite on top before TAA and ACES tonemapping.

**Physics.** Bullet with a DBVT broadphase. The player controller
sweeps a capsule against the world. Dynamic bodies are added via
`PhysicsWorld`. Contact events feed the audio system.

**Vehicle.** The chassis is a *dynamic* rigid body. The code sets the
body's **velocity** each frame but never its transform. Physics
integrates position, so gravity, ramps, jumps, and wall collisions
are all handled by Bullet. Code owns throttle, brake, steering, and
lateral grip — applied as forces and velocity modifications. This is
the pattern that lets physics push the car up a ramp without fighting
the driver's input.

**Audio.** miniaudio engine, one-shot 3D spatialized sounds. Physics
contacts above a velocity threshold are pushed to an impact queue,
drained each frame, and played at the contact point.

## Known limitations

- Wheels spin visually but aren't physics-simulated
- Per-primitive glb texture loads duplicate uploads
- Glass rendered opaque (deferred pipeline has no transparency pass)
- Some assets (spec-gloss exports from 3DS Max) need per-file material
  fixups
- Steering model is arcade, not simulated — no slip-angle tire model
- No support for inverted driving surfaces (loop-de-loops)

## Roadmap

1. **Track**: load a Blender-authored glb track, split into visual
   mesh + physics colliders.
2. **Content**: multiple ramps, banked turns, a full lap.
3. **Wheel polish**: suspension travel visual, tire smoke, skid marks.
4. **Character**: bring back the FPS controller on a toggle key.
5. **Lighting**: real sun position, moving time of day, skybox.
6. **Post**: bloom, motion blur, depth of field.

## License

Apache 2.0 License

## Name

Photonix — the engine formerly known as Lumen. It grew a physics body
and an audio system, and picked up a name that sounded less like a
graphics-only tool.
