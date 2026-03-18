# Maze Runner

First-person 3D maze exploration game written in modern C++17 and rendered with OpenGL 3.3. The project features procedural maze generation, collectible stacking, a minimap with fog-of-war, animated HUD, day/night lighting, and CSV-based high scores.

![Maze Runner](https://img.shields.io/badge/OpenGL-3.3-blue) ![C++17](https://img.shields.io/badge/C%2B%2B-17-brightgreen)

## Overview

- **Goal**: Escape the maze by reaching the exit gateway while collecting scattered items for a higher star rating and score.
- **Perspective**: First-person only; mouse-look + WASD movement with jump.
- **Key Systems**: Recursive-backtracking maze generation, collectible placement in dead-ends, stackable carried items, time-of-day sky/lighting, toggleable torch, minimap overlay with explored-cell tracking, difficulty-scaled timers, and persistent high scores.

## Technical Specifications

- **Language / Std**: C++17
- **Graphics**: OpenGL 3.3 core profile, GLSL 330 shaders
- **Build System**: CMake 3.10+
- **Dependencies**: GLFW 3.3+, GLM headers, GLAD loader (bundled)
- **Physics**: 60 Hz fixed-step loop, gravity 9.8 m/s², jump impulse controlled by `jumpForce` in `Player.cpp`, corridor width 2.0 units
- **Lighting**: Forward pipeline; directional sun/moon from a 90-second day/night cycle, exponential fog, optional torch glow (radius ~30 units with sine-wave flicker)
- **Textures**: Procedural stone brick walls + graffiti decals; optional external PNG overrides (see `textures/`)
- **Data**: High scores saved as CSV (`highscores.txt`, top 10, backward compatible 5→7 columns)

## Controls

### Gameplay

| Key | Action |
|-----|--------|
| **W / A / S / D** | Move |
| **Mouse** | Look |
| **Space** | Jump |
| **T** | Toggle torch |
| **R** | Restart with a new maze |
| **F1** | Toggle wireframe |
| **ESC** | Return to title / quit |

### Menus

| Key | Action |
|-----|--------|
| **↑ / ↓** | Navigate options |
| **← / →** | Change difficulty |
| **Enter** | Confirm |
| **ESC** | Quit |

## Building and Running

1. Install dependencies (example Ubuntu):  
   ```bash
   sudo apt-get install build-essential cmake libglfw3-dev libglm-dev libgl-dev
   ```
2. Configure & build:  
   ```bash
   mkdir -p build && cd build
   cmake ..
   make -j$(nproc)
   ```
3. Run:  
   ```bash
   ./MazeRunner
   ```

Shaders and textures are copied next to the executable at build time.

## Project Layout (file-by-file guide)

### Root
- `CMakeLists.txt` — build script that finds/links GLFW, OpenGL, and GLM, builds bundled GLAD, and copies shaders/textures post-build.
- `OUTDOOR_SCENE_SPECIFICATION.md` — art/reference specification for a ray-traced outdoor maze-runner scene.
- `report/MAZE_RUNNER_PRESENTATION.md` — slide-style project presentation.
- `report/MAZE_RUNNER_REPORT.md` — written project report and appendices.
- `README.md` — this document.

### Source (`src/`)
- `main.cpp` — entry point; window/context setup, fixed-timestep game loop, input handling, HUD text rendering, win/lose flow, difficulty selection glue, and high-score persistence.
- `Maze.h/cpp` — recursive-backtracking maze generator (odd dimensions), wall/path grid storage, exit cell selection, and dead-end queries for item placement.
- `Player.h/cpp` — first-person camera + movement; mouse look, WASD movement, jump, AABB wall collision, and carried-item bookkeeping.
- `Renderer.h/cpp` — builds meshes (walls, gift boxes, graffiti batches, exit gateway, carried stack) and renders maze, collectibles, exit zone, and wireframe mode; generates procedural wall & graffiti textures; manages VAOs/VBOs and texture loading.
- `Collectible.h/cpp` — item definitions, placement onto provided grid locations, rotation update, AABB pickup, and stacked-carry support.
- `Minimap.h/cpp` — north-up overlay with fog-of-war, scrolling viewport, difficulty color accents, legend toggle, and markers for player/items/exit.
- `TitleScreen.h/cpp` — animated title menu, difficulty selector, high-score list rendering, and start/quit handling.
- `HandRenderer.h/cpp` — first-person hand mesh with walk/jump bob animation.
- `SkyRenderer.h/cpp` — GPU sky dome with 90-second day/night cycle, sun/moon rendering, star field, and helpers for sun direction, sun color, fog color, and ambient light.
- `TorchLight.h/cpp` — toggleable torch attached to the player; sine-wave flicker, glow sprite billboard, and lighting parameters (color/intensity/radius).
- `Shader.h/cpp` — utility wrapper for compiling GLSL shaders and setting uniforms.
- `StarRating.h` — difficulty configs (EASY 15×15/5 items, MEDIUM 21×21/7, HARD 31×31/10, NIGHTMARE 41×41/14) plus time thresholds and star vertex helpers.
- `Highscore.h` — CSV load/save, top-10 sorting, backward compatibility, and per-difficulty best-star queries.
- `glad.c` — bundled GLAD OpenGL loader implementation.

### Shaders (`shaders/`)
- `vertex.glsl` / `fragment.glsl` — main 3D pipeline (MVP, lighting, fog, optional texture sampling, torch parameters).
- `minimap_vertex.glsl` / `minimap_fragment.glsl` — 2D overlay for minimap and HUD quads.
- `hud_vertex.glsl` / `hud_fragment.glsl` — text/stars/HUD rendering.

### Textures (`textures/`)
- `gift_box.png` — default giftbox texture; also used for stacked carried items.
- `graffiti/` — optional PNG decals (`graffiti_01.png` …) mixed with procedural graffiti when absent.
- (Optional) Place a `textures/wall.png` next to the executable to override the procedural stone texture.

## Gameplay & Systems

- **Difficulty & Timing**: Four tiers scale maze size, item count, and target times; stars awarded based on completion time and whether all items were collected (perfect run requires collecting everything within the 3-star time).
- **Collectibles**: Items spawn preferentially in dead-ends; pickups use generous AABB checks; collected items can be carried as a visible stack and contribute to score.
- **Exit Zone**: 6×6 gateway platform with animated energy curtain and expanding light rings; crossing immediately ends the run.
- **Minimap**: Shows explored cells, player arrow, collectible markers, exit marker, optional legend; scrolls smoothly for large mazes.
- **Lighting**: Dynamic sky controls directional light color/dir, fog color, and ambient level; optional torch adds warm close-range light and a billboarded glow.
- **HUD**: Timer, item counters, difficulty indicator, win-screen star animation, and CSV-backed high-score board on the title screen.

## Customization Tips

- **Wall & Graffiti textures**: Drop replacement PNGs into `textures/` (wall) or `textures/graffiti/` (up to 4 files). If absent, procedural textures are generated at startup.
- **Parameters to tweak**:
  - Maze scale & item counts: `getDifficultyConfig` in `StarRating.h`.
  - Movement/physics: `moveSpeed`, `gravity`, `jumpForce` in `Player.cpp`.
  - Torch color/radius: `baseColor` and `radius` in `TorchLight.cpp`.
  - Day/night speed: `DAY_CYCLE_SPEED` in `SkyRenderer.h` (default: full cycle in 90 seconds).

## Notes

- High scores are stored in `highscores.txt` beside the executable; the file is created on first run.
- If a dependency (e.g., GLFW) is missing, install it via your system package manager or point CMake to its install prefix (`glfw3_DIR` / `CMAKE_PREFIX_PATH`).

---

This README summarizes every file and the major specifications so you can build, run, and modify Maze Runner confidently.
