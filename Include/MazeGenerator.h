// MazeGenerator.h - owns the dynamic maze grid and generates the maze.
#pragma once

#include <vector>

#include "Constants.h"

class MazeGenerator {
public:
    MazeGenerator();
    ~MazeGenerator();

    // The grid is raw dynamic memory, so copying is forbidden (it would be freed twice).
    MazeGenerator(const MazeGenerator&) = delete;
    MazeGenerator& operator=(const MazeGenerator&) = delete;

    void AllocateGrid(int size);        // new int*[size], then new int[size] for every row
    void FreeGrid();                    // delete[] everything
    void GenerateMaze(int doorCount);   // carve the maze, then add shortcut doors
    bool IsValidPath() const;           // is there a path from (0,0) to the exit?

    int** GetGrid() const { return m_grid; }
    int GetSize() const { return m_size; }

private:
    int** m_grid;
    int m_size;

    void RemoveWall(int x, int y, int direction);
    int PlaceDoors(int doorCount);
};
