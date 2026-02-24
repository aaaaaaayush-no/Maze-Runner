#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include <utility>
#include <random>

enum class CellType : uint8_t {
    WALL = 0,
    PATH = 1
};

class Maze {
public:
    Maze(int width, int height, unsigned int seed = 0);

    void generate();
    CellType getCell(int x, int y) const;
    bool isWall(int x, int y) const;
    int getWidth() const { return width; }
    int getHeight() const { return height; }

    std::pair<int,int> getStart() const { return {1, 1}; }
    std::pair<int,int> getExit() const;

    // Get positions suitable for placing items (dead ends preferred)
    std::vector<std::pair<int,int>> getItemPositions(int count) const;

    const std::vector<std::vector<CellType>>& getGrid() const { return grid; }

private:
    int width, height;
    std::vector<std::vector<CellType>> grid;
    std::mt19937 rng;

    void recursiveBacktrack(int cx, int cy);
    std::vector<std::pair<int,int>> getNeighbors(int cx, int cy) const;
    std::vector<std::pair<int,int>> findDeadEnds() const;
};

#endif
