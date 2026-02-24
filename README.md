# Maze Runner

A 3D first-person maze exploration game built with OpenGL 3.3, featuring procedural generation, physics simulation, collectible items, a minimap system, difficulty levels and a star rating system.

![Maze Runner](https://img.shields.io/badge/OpenGL-3.3-blue) ![C++17](https://img.shields.io/badge/C%2B%2B-17-brightgreen)

## Features

- **Procedural Maze Generation** – Recursive backtracking algorithm creates a unique maze every game
- **Difficulty Levels** – EASY (15×15), MEDIUM (21×21), HARD (31×31) and NIGHTMARE (41×41) with scaled item counts and time thresholds
- **First-Person 3D Rendering** – Textured geometry with procedural stone-brick wall textures, warm torch lighting, exponential fog, wireframe toggle
- **Physics** – Gravity (9.8 m/s²) and jump mechanics with AABB wall collision
- **Minimap** – North-up top-right overlay with fog-of-war, compass rose, player arrow, item markers and exit indicator (toggle with M); scrolling viewport for larger mazes; legend panel (toggle with L)
- **Collectible Items** – Keys (golden cubes), Artifacts (rotating pyramids) and Orbs (spheres) placed at dead ends
- **Star Rating System** – Time-based 1–3 star rating on completion with animated win screen; perfect run bonus for collecting all items under the 3-star time
- **Highscores** – Top 10 scores persisted to CSV with per-difficulty best-star tracking
- **HUD** – Item counter, elapsed timer, difficulty indicator and star preview rendered with a segment-display font
- **Win Condition** – Collect all items then reach the green exit portal

## Controls

| Key | Action |
|-----|--------|
| **W A S D** | Move forward / left / backward / right |
| **Mouse** | Look around |
| **Space** | Jump |
| **M** | Toggle minimap |
| **L** | Toggle minimap legend |
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

### Running

```bash
cd build
./MazeRunner
```

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
│   ├── vertex.glsl            # 3D scene vertex shader (MVP + fog + UV)
│   ├── fragment.glsl          # 3D scene fragment shader (lighting + fog + texture sampling)
│   ├── minimap_vertex.glsl    # 2D overlay vertex shader
│   ├── minimap_fragment.glsl  # 2D overlay fragment shader
│   ├── hud_vertex.glsl        # HUD vertex shader
│   └── hud_fragment.glsl      # HUD fragment shader
└── src/
    ├── main.cpp           # Game loop, input, HUD, win logic
    ├── Maze.h/cpp         # Procedural maze generation & storage
    ├── Player.h/cpp       # First-person camera, movement, physics
    ├── Renderer.h/cpp     # OpenGL rendering pipeline & procedural textures
    ├── Minimap.h/cpp      # 2D minimap overlay with fog of war
    ├── Collectible.h/cpp  # Item placement & collection
    ├── Shader.h/cpp       # GLSL shader loading & uniform helpers
    ├── TitleScreen.h/cpp  # Animated title screen & difficulty selection menu
    ├── HandRenderer.h/cpp # First-person hand with walk/jump animation
    ├── StarRating.h       # Difficulty config, star calculation & rendering
    ├── Highscore.h        # CSV highscore load/save with backward compat
    └── glad.c             # GLAD implementation
```

## Technical Details

| Component | Implementation |
|-----------|---------------|
| Maze algorithm | Recursive backtracking on an odd-dimensioned grid |
| Difficulty | 4 tiers (EASY–NIGHTMARE) scaling maze size, items and time thresholds |
| Rendering | VBO/VAO with per-vertex position + color + normal + texcoord (stride 11 floats for maze, 9 for objects) |
| Textures | Procedural 128×128 stone-brick texture generated at init; `useTexture` uniform toggles sampling in fragment shader |
| Shading | Directional light (Gouraud) + exponential fog + warm torch tint |
| Collision | AABB with separate X/Z axis resolution |
| Physics | Fixed 60 Hz timestep, gravity 9.8 m/s², jump impulse 6.5 m/s |
| Minimap | North-up dynamic quads in NDC, scrolling viewport, fog-of-war via explored cell set |
| Star rating | Time-based 1–3 stars, animated win screen with particle bursts |
| Highscores | CSV persistence, top 10, backward-compatible 5→7 column format |

## Textures

Wall and floor surfaces use a **procedurally generated** stone-brick texture created
at runtime in `Renderer::generateWallTexture()`. No external image files are needed.

### How it works

1. A 128×128 RGB pixel buffer is filled with a brick pattern (4 rows, offset every
   other row) including mortar lines, per-brick colour variation and per-pixel noise.
2. The buffer is uploaded to an OpenGL texture with mipmaps (`GL_LINEAR_MIPMAP_LINEAR`).
3. The maze mesh is built with an 11-float vertex stride
   (`pos[3] + color[3] + normal[3] + texcoord[2]`) so every face carries UV
   coordinates.
4. In the fragment shader, the `useTexture` boolean uniform controls whether the
   `wallTexture` sampler modulates the base vertex colour. Collectibles and the
   exit portal set `useTexture = false` and use vertex colour only.

### Customising the texture

To change how walls look, edit `Renderer::generateWallTexture()` in `src/Renderer.cpp`:

* **Brick size** – adjust `brickH` and `brickW` to make bricks taller or wider.
* **Mortar thickness** – change `mortarSize`.
* **Colour palette** – modify the base RGB values (`140, 135, 125`) or mortar colour
  (`60, 58, 55`).
* **Noise intensity** – tweak `bVar` and `pNoise` ranges for rougher or smoother
  stone.

### Loading an external texture

If you prefer to load a texture from a file (e.g. PNG) instead of generating one
procedurally:

1. Add [stb_image](https://github.com/nothings/stb) (`stb_image.h`) to `include/`.
2. At the top of `src/Renderer.cpp` (outside any function), add:
   ```cpp
   #define STB_IMAGE_IMPLEMENTATION
   #include "stb_image.h"
   ```
3. In `generateWallTexture()`, replace the pixel-generation loop with:
   ```cpp
   int w, h, channels;
   unsigned char* data = stbi_load("textures/wall.png", &w, &h, &channels, 3);
   ```
4. Pass `data`, `w` and `h` to `glTexImage2D` instead of the `pixels` vector.
5. Call `stbi_image_free(data)` after uploading.

The rest of the rendering pipeline (shaders, UVs, `useTexture` uniform) stays the
same.

## License

This project is provided as-is for educational purposes.