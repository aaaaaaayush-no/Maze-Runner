#include "Maze.h"
#include <algorithm>
#include <chrono>

Maze::Maze(int width, int height, unsigned int seed)
    : width(width | 1), height(height | 1) // ensure odd dimensions
{
    if (seed == 0)
        seed = static_cast<unsigned int>(
            std::chrono::steady_clock::now().time_since_epoch().count());
    rng.seed(seed);
}

void Maze::generate() {
    // Initialize grid: all walls
    grid.assign(height, std::vector<CellType>(width, CellType::WALL));

    // Carve paths using recursive backtracking starting at (1,1)
    recursiveBacktrack(1, 1);

    // Ensure start and exit are open
    auto [sx, sy] = getStart();
    grid[sy][sx] = CellType::PATH;

    auto [ex, ey] = getExit();
    grid[ey][ex] = CellType::PATH;
    // Also open adjacent cell towards exit if needed
    if (ex > 0 && grid[ey][ex - 1] == CellType::WALL && ex - 2 >= 0 && grid[ey][ex - 2] == CellType::PATH)
        grid[ey][ex - 1] = CellType::PATH;
    if (ey > 0 && grid[ey - 1][ex] == CellType::WALL && ey - 2 >= 0 && grid[ey - 2][ex] == CellType::PATH)
        grid[ey - 1][ex] = CellType::PATH;
}

CellType Maze::getCell(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height)
        return CellType::WALL;
    return grid[y][x];
}

bool Maze::isWall(int x, int y) const {
    return getCell(x, y) == CellType::WALL;
}

std::pair<int,int> Maze::getExit() const {
    // Exit at the bottom-right area
    int ex = width - 2;
    int ey = height - 2;
    // Ensure coordinates are odd (cell centers in the maze grid)
    if (ex % 2 == 0) ex--;
    if (ey % 2 == 0) ey--;
    return {ex, ey};
}

void Maze::recursiveBacktrack(int cx, int cy) {
    grid[cy][cx] = CellType::PATH;

    auto neighbors = getNeighbors(cx, cy);
    std::shuffle(neighbors.begin(), neighbors.end(), rng);

    for (auto [nx, ny] : neighbors) {
        if (grid[ny][nx] == CellType::WALL) {
            // Carve wall between current and neighbor
            int wx = cx + (nx - cx) / 2;
            int wy = cy + (ny - cy) / 2;
            grid[wy][wx] = CellType::PATH;
            recursiveBacktrack(nx, ny);
        }
    }
}

std::vector<std::pair<int,int>> Maze::getNeighbors(int cx, int cy) const {
    std::vector<std::pair<int,int>> neighbors;
    if (cx - 2 >= 1) neighbors.push_back({cx - 2, cy});
    if (cx + 2 < width - 1) neighbors.push_back({cx + 2, cy});
    if (cy - 2 >= 1) neighbors.push_back({cx, cy - 2});
    if (cy + 2 < height - 1) neighbors.push_back({cx, cy + 2});
    return neighbors;
}

std::vector<std::pair<int,int>> Maze::findDeadEnds() const {
    std::vector<std::pair<int,int>> deadEnds;
    for (int y = 1; y < height - 1; y += 2) {
        for (int x = 1; x < width - 1; x += 2) {
            if (grid[y][x] != CellType::PATH) continue;
            int openCount = 0;
            if (!isWall(x - 1, y)) openCount++;
            if (!isWall(x + 1, y)) openCount++;
            if (!isWall(x, y - 1)) openCount++;
            if (!isWall(x, y + 1)) openCount++;
            if (openCount == 1) {
                // Skip start and exit
                auto [sx, sy] = getStart();
                auto [ex, ey] = getExit();
                if (!(x == sx && y == sy) && !(x == ex && y == ey))
                    deadEnds.push_back({x, y});
            }
        }
    }
    return deadEnds;
}

std::vector<std::pair<int,int>> Maze::getItemPositions(int count) const {
    // Prefer dead ends, fill with random path cells
    auto deadEnds = findDeadEnds();
    std::vector<std::pair<int,int>> candidates;

    // Collect all path cells (excluding start/exit)
    auto [sx, sy] = getStart();
    auto [ex, ey] = getExit();
    std::vector<std::pair<int,int>> allPaths;
    for (int y = 1; y < height - 1; y += 2) {
        for (int x = 1; x < width - 1; x += 2) {
            if (grid[y][x] == CellType::PATH &&
                !(x == sx && y == sy) && !(x == ex && y == ey))
                allPaths.push_back({x, y});
        }
    }

    // Shuffle both
    std::mt19937 rngCopy = rng;
    std::shuffle(deadEnds.begin(), deadEnds.end(), rngCopy);
    std::shuffle(allPaths.begin(), allPaths.end(), rngCopy);

    // Take dead ends first
    for (auto& p : deadEnds) {
        if ((int)candidates.size() >= count) break;
        candidates.push_back(p);
    }
    // Fill remaining from other paths
    for (auto& p : allPaths) {
        if ((int)candidates.size() >= count) break;
        bool alreadyUsed = false;
        for (auto& c : candidates) {
            if (c.first == p.first && c.second == p.second) {
                alreadyUsed = true;
                break;
            }
        }
        if (!alreadyUsed) candidates.push_back(p);
    }
    return candidates;
}
