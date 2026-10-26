// GameLogic.h - pure game calculations.
// No Raylib and no cin/cout in this class (requirement of the assignment).
#pragma once

#include <string>
#include <vector>

#include "Constants.h"

class GameLogic {
public:
    static bool IsValidSize(int size);

    // True if the player cannot go from cell (x, y) in this direction (wall without a door).
    static bool CheckWallCollision(int** grid, int x, int y, int direction);
    static bool CheckWinCondition(int x, int y, int width, int height);

    // Breadth-first search. Returns the cells from start to end, or an empty list if there is no path.
    static std::vector<Point> FindShortestPath(int** grid, int size, int startX, int startY, int endX, int endY);

    static int DoorCountFor(int size, Difficulty difficulty);
    static int CalculateTimeLimit(int shortestSteps, Difficulty difficulty);  // 0 = no limit
    static int CalculateScore(int shortestSteps, int moves, float seconds, bool hintUsed);

    // Text version of the maze: '#' wall, ' ' path, 'D' door, '@' player, 'E' exit.
    static std::vector<std::string> BuildTextMaze(int** grid, int size, int playerX, int playerY);
};
