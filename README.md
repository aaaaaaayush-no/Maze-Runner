# Maze Runner

A 3D first-person maze exploration game built with OpenGL 3.3, featuring procedural generation, physics simulation, collectible items and a minimap system.

![Maze Runner](https://img.shields.io/badge/OpenGL-3.3-blue) ![C++17](https://img.shields.io/badge/C%2B%2B-17-brightgreen)

## Features

- **Procedural Maze Generation** – Recursive backtracking algorithm creates a unique maze every game (configurable 21×21 grid)
- **First-Person 3D Rendering** – Flat-shaded geometry with fog for depth perception, wireframe toggle
- **Physics** – Gravity (9.8 m/s²) and jump mechanics with AABB wall collision
- **Minimap** – Top-right overlay with fog-of-war, player arrow, item markers and exit indicator (toggle with M)
- **Collectible Items** – Keys (golden cubes), Artifacts (rotating pyramids) and Orbs (spheres) placed at dead ends
- **HUD** – Item counter and elapsed timer rendered with a segment-display font
- **Win Condition** – Collect all items then reach the green exit portal

## Controls

| Key | Action |
|-----|--------|
| **W A S D** | Move forward / left / backward / right |
| **Mouse** | Look around |
| **Space** | Jump |
| **M** | Toggle minimap |
| **R** | Restart with a new maze |
| **F1** | Toggle wireframe rendering |
| **ESC** | Quit |

## Requirements

- C++17 compiler (GCC 9+, Clang 10+, or MSVC 2019+)
- CMake 3.10+
- OpenGL 3.3 capable GPU and drivers
- GLFW 3.3+ development library
- GLM development library

GLAD (OpenGL loader) is bundled in the repository.

### Installing dependencies

**Ubuntu / Debian:**
```bash
sudo apt-get install build-essential cmake libglfw3-dev libglm-dev libgl-dev
```

**macOS (Homebrew):**
```bash
brew install cmake glfw glm
```

**Windows (vcpkg):**
```powershell
vcpkg install glfw3 glm
```
Or open the folder in Visual Studio 2019+ which supports CMake projects natively.

## Building

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

The executable `MazeRunner` is placed in the `build/` directory. Shader files are automatically copied there during configuration.

### Visual Studio

1. Open Visual Studio and select **Open a local folder** → choose the repository root.
2. Visual Studio auto-detects `CMakeLists.txt` and configures the project.
3. Select **MazeRunner** as the startup target and press **F5** to build and run.

## Project Structure

```
├── CMakeLists.txt
├── README.md
├── include/
│   ├── glad/          # GLAD OpenGL loader (bundled)
│   └── KHR/           # Khronos platform header
├── shaders/
│   ├── vertex.glsl            # 3D scene vertex shader (MVP + fog)
│   ├── fragment.glsl          # 3D scene fragment shader (lighting + fog)
│   ├── minimap_vertex.glsl    # 2D overlay vertex shader
│   ├── minimap_fragment.glsl  # 2D overlay fragment shader
│   ├── hud_vertex.glsl        # HUD vertex shader
│   └── hud_fragment.glsl      # HUD fragment shader
└── src/
    ├── main.cpp         # Game loop, input, HUD, win logic
    ├── Maze.h/cpp       # Procedural maze generation & storage
    ├── Player.h/cpp     # First-person camera, movement, physics
    ├── Renderer.h/cpp   # OpenGL rendering pipeline
    ├── Minimap.h/cpp    # 2D minimap overlay with fog of war
    ├── Collectible.h/cpp# Item placement & collection
    ├── Shader.h/cpp     # GLSL shader loading & uniform helpers
    └── glad.c           # GLAD implementation
```

## Technical Details

| Component | Implementation |
|-----------|---------------|
| Maze algorithm | Recursive backtracking on an odd-dimensioned grid |
| Rendering | VBO/VAO with per-vertex position + color + normal (stride 9 floats) |
| Shading | Directional light (Gouraud) + exponential fog |
| Collision | AABB with separate X/Z axis resolution |
| Physics | Fixed 60 Hz timestep, gravity 9.8 m/s², jump impulse 6.5 m/s |
| Minimap | Dynamic quad generation in NDC, fog-of-war via explored cell set |

## License

This project is provided as-is for educational purposes.