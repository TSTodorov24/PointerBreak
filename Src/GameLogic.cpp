// GameLogic.cpp - pure calculations: collisions, win, shortest path, score, time limit.
#include "GameLogic.h"

#include <algorithm>
#include <queue>

bool GameLogic::IsValidSize(int size) {
    return size >= MIN_MAZE_SIZE && size <= MAX_MAZE_SIZE;
}

bool GameLogic::CheckWallCollision(int** grid, int x, int y, int direction) {
    int cell = grid[y][x];
    bool hasWall = (cell & WallBit(direction)) != 0;
    bool hasDoor = (cell & DoorBit(direction)) != 0;
    return hasWall && !hasDoor;   // a door is a wall we are allowed to cross
}

bool GameLogic::CheckWinCondition(int x, int y, int width, int height) {
    return x == width - 1 && y == height - 1;
}

std::vector<Point> GameLogic::FindShortestPath(int** grid, int size, int startX, int startY,
                                               int endX, int endY) {
    const int NOT_VISITED = -2;
    const int NO_PARENT = -1;

    // Every cell is numbered y * size + x. parent[] remembers where we came from.
    std::vector<int> parent(size * size, NOT_VISITED);
    std::queue<int> queue;

    int start = startY * size + startX;
    int end = endY * size + endX;
    parent[start] = NO_PARENT;
    queue.push(start);

    while (!queue.empty() && parent[end] == NOT_VISITED) {
        int current = queue.front();
        queue.pop();
        int x = current % size;
        int y = current / size;

        for (int d = 0; d < DIR_COUNT; d++) {
            if (CheckWallCollision(grid, x, y, d)) {
                continue;
            }
            int next = (y + DIR_DY[d]) * size + (x + DIR_DX[d]);
            if (parent[next] == NOT_VISITED) {
                parent[next] = current;
                queue.push(next);
            }
        }
    }

    std::vector<Point> path;
    if (parent[end] == NOT_VISITED) {
        return path;   // no way to the end
    }

    // Walk back from the end to the start, then reverse.
    for (int cell = end; cell != NO_PARENT; cell = parent[cell]) {
        path.push_back({cell % size, cell / size});
    }
    std::reverse(path.begin(), path.end());
    return path;
}

int GameLogic::DoorCountFor(int size, Difficulty difficulty) {
    if (difficulty == DIFFICULTY_EASY) {
        return size;                        // many doors
    }
    if (difficulty == DIFFICULTY_MEDIUM) {
        return size / 2;
    }
    return std::max(2, size / 5);           // few doors
}

int GameLogic::CalculateTimeLimit(int shortestSteps, Difficulty difficulty) {
    if (difficulty == DIFFICULTY_EASY) {
        return 0;                           // no limit
    }
    float secondsPerStep = (difficulty == DIFFICULTY_MEDIUM) ? MEDIUM_SECONDS_PER_STEP
                                                             : HARD_SECONDS_PER_STEP;
    return static_cast<int>(shortestSteps * secondsPerStep);
}

int GameLogic::CalculateScore(int shortestSteps, int moves, float seconds, bool hintUsed) {
    // Half of the score is how close the route was to the best one,
    // the other half is how fast the player was.
    float pathPart = 0.0f;
    if (moves > 0) {
        pathPart = std::min(1.0f, static_cast<float>(shortestSteps) / moves);
    }
    float timePart = 1.0f;
    if (seconds > 0.0f) {
        timePart = std::min(1.0f, shortestSteps * BEST_SECONDS_PER_STEP / seconds);
    }

    float score = MAX_SCORE * (0.5f * pathPart + 0.5f * timePart);
    if (hintUsed) {
        score *= HINT_SCORE_FACTOR;
    }
    return static_cast<int>(score);
}

std::vector<std::string> GameLogic::BuildTextMaze(int** grid, int size, int playerX, int playerY) {
    // Every cell becomes one character with a wall character between it and its neighbours.
    int textSize = 2 * size + 1;
    std::vector<std::string> rows(textSize, std::string(textSize, '#'));

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int cell = grid[y][x];
            rows[2 * y + 1][2 * x + 1] = ' ';

            if (x < size - 1) {
                if (cell & DoorBit(DIR_RIGHT)) {
                    rows[2 * y + 1][2 * x + 2] = 'D';
                } else if (!(cell & WallBit(DIR_RIGHT))) {
                    rows[2 * y + 1][2 * x + 2] = ' ';
                }
            }
            if (y < size - 1) {
                if (cell & DoorBit(DIR_DOWN)) {
                    rows[2 * y + 2][2 * x + 1] = 'D';
                } else if (!(cell & WallBit(DIR_DOWN))) {
                    rows[2 * y + 2][2 * x + 1] = ' ';
                }
            }
        }
    }

    rows[2 * size - 1][2 * size - 1] = 'E';
    rows[2 * playerY + 1][2 * playerX + 1] = '@';
    return rows;
}
