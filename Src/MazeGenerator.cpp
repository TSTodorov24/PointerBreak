// MazeGenerator.cpp - dynamic grid + Recursive Backtracker + shortcut doors.
#include "MazeGenerator.h"

#include <cstdlib>

#include "GameLogic.h"

MazeGenerator::MazeGenerator() : m_grid(nullptr), m_size(0) {}

MazeGenerator::~MazeGenerator() {
    FreeGrid();
}

void MazeGenerator::AllocateGrid(int size) {
    FreeGrid();   // never leak an old grid

    m_size = size;
    m_grid = new int*[m_size];
    for (int y = 0; y < m_size; y++) {
        m_grid[y] = new int[m_size];
        for (int x = 0; x < m_size; x++) {
            m_grid[y][x] = ALL_WALLS;   // start with walls on all four sides
        }
    }
}

void MazeGenerator::FreeGrid() {
    if (m_grid != nullptr) {
        for (int y = 0; y < m_size; y++) {
            delete[] m_grid[y];
        }
        delete[] m_grid;
    }
    m_grid = nullptr;
    m_size = 0;
}

// Removes the wall between a cell and its neighbour (on both sides).
void MazeGenerator::RemoveWall(int x, int y, int direction) {
    int nextX = x + DIR_DX[direction];
    int nextY = y + DIR_DY[direction];
    int opposite = (direction + 2) % DIR_COUNT;

    m_grid[y][x] &= ~WallBit(direction);
    m_grid[nextY][nextX] &= ~WallBit(opposite);
}

void MazeGenerator::GenerateMaze(int doorCount) {
    if (m_grid == nullptr) {
        return;
    }

    // Recursive Backtracker, written with a stack instead of recursion:
    // walk to a random unvisited neighbour, remove the wall, repeat.
    // When stuck, step back. This gives a path from (0,0) to every cell.
    std::vector<Point> stack;
    m_grid[0][0] |= VISITED_BIT;
    stack.push_back({0, 0});

    while (!stack.empty()) {
        Point current = stack.back();

        int options[DIR_COUNT];
        int optionCount = 0;
        for (int d = 0; d < DIR_COUNT; d++) {
            int nextX = current.x + DIR_DX[d];
            int nextY = current.y + DIR_DY[d];
            bool inside = nextX >= 0 && nextY >= 0 && nextX < m_size && nextY < m_size;
            if (inside && !(m_grid[nextY][nextX] & VISITED_BIT)) {
                options[optionCount++] = d;
            }
        }

        if (optionCount == 0) {
            stack.pop_back();   // dead end, go back
            continue;
        }

        int direction = options[std::rand() % optionCount];
        RemoveWall(current.x, current.y, direction);

        Point next = {current.x + DIR_DX[direction], current.y + DIR_DY[direction]};
        m_grid[next.y][next.x] |= VISITED_BIT;
        stack.push_back(next);
    }

    // The visited flags are no longer needed.
    for (int y = 0; y < m_size; y++) {
        for (int x = 0; x < m_size; x++) {
            m_grid[y][x] &= ~VISITED_BIT;
        }
    }

    PlaceDoors(doorCount);
}

// Turns random inner walls into doors, but only if the door is a real shortcut:
// the two cells next to it must be far apart when walking around the wall.
// Returns how many doors were placed (it can be less than requested).
int MazeGenerator::PlaceDoors(int doorCount) {
    int placed = 0;
    int attempts = 0;

    while (placed < doorCount && attempts < doorCount * 100) {
        attempts++;

        int x = std::rand() % m_size;
        int y = std::rand() % m_size;
        int direction = std::rand() % DIR_COUNT;
        int nextX = x + DIR_DX[direction];
        int nextY = y + DIR_DY[direction];

        bool inside = nextX >= 0 && nextY >= 0 && nextX < m_size && nextY < m_size;
        if (!inside) {
            continue;   // never put a door in the outer border
        }
        bool isWall = (m_grid[y][x] & WallBit(direction)) != 0;
        bool isDoor = (m_grid[y][x] & DoorBit(direction)) != 0;
        if (!isWall || isDoor) {
            continue;
        }

        std::vector<Point> detour = GameLogic::FindShortestPath(m_grid, m_size, x, y, nextX, nextY);
        int stepsAround = static_cast<int>(detour.size()) - 1;
        if (stepsAround >= MIN_SHORTCUT_LENGTH) {
            int opposite = (direction + 2) % DIR_COUNT;
            m_grid[y][x] |= DoorBit(direction);
            m_grid[nextY][nextX] |= DoorBit(opposite);
            placed++;
        }
    }
    return placed;
}

bool MazeGenerator::IsValidPath() const {
    if (m_grid == nullptr) {
        return false;
    }
    return !GameLogic::FindShortestPath(m_grid, m_size, 0, 0, m_size - 1, m_size - 1).empty();
}
