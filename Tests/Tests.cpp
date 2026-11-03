// Tests.cpp - QA test functions. They use only the logic classes (no window needed).
// Run the PointerBreakTests program: every check prints PASS or FAIL.
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "GameLogic.h"
#include "MazeGenerator.h"
#include "PlayerAvatar.h"
#include "ScoreBoard.h"

static int g_passed = 0;
static int g_failed = 0;

static void Check(bool condition, const std::string& name) {
    if (condition) {
        g_passed++;
        std::cout << "[PASS] " << name << "\n";
    } else {
        g_failed++;
        std::cout << "[FAIL] " << name << "\n";
    }
}

// Generates a maze of the given size with a few doors.
static void MakeMaze(MazeGenerator& maze, int size, int doors) {
    maze.AllocateGrid(size);
    maze.GenerateMaze(doors);
}

void TestSizeValidation() {
    Check(!GameLogic::IsValidSize(9), "Size 9 is rejected");
    Check(GameLogic::IsValidSize(10), "Size 10 is accepted");
    Check(GameLogic::IsValidSize(50), "Size 50 is accepted");
    Check(!GameLogic::IsValidSize(51), "Size 51 is rejected");
    Check(!GameLogic::IsValidSize(-5), "Negative size is rejected");
}

void TestEveryMazeIsSolvable() {
    bool allSolvable = true;
    bool pathsAreValid = true;

    for (int size = MIN_MAZE_SIZE; size <= MAX_MAZE_SIZE; size++) {
        MazeGenerator maze;
        MakeMaze(maze, size, size / 2);
        if (!maze.IsValidPath()) {
            allSolvable = false;
        }

        std::vector<Point> path = GameLogic::FindShortestPath(maze.GetGrid(), size, 0, 0, size - 1, size - 1);
        if (path.empty() || path.front().x != 0 || path.front().y != 0 ||
            path.back().x != size - 1 || path.back().y != size - 1) {
            pathsAreValid = false;
            continue;
        }
        // every step must go to a neighbour cell and must not cross a wall
        for (size_t i = 1; i < path.size(); i++) {
            int dx = path[i].x - path[i - 1].x;
            int dy = path[i].y - path[i - 1].y;
            int direction = -1;
            for (int d = 0; d < DIR_COUNT; d++) {
                if (DIR_DX[d] == dx && DIR_DY[d] == dy) direction = d;
            }
            if (direction < 0 || GameLogic::CheckWallCollision(maze.GetGrid(), path[i - 1].x, path[i - 1].y, direction)) {
                pathsAreValid = false;
            }
        }
    }
    Check(allSolvable, "Mazes of every size 10..50 have a path from start to exit");
    Check(pathsAreValid, "Shortest path starts at (0,0), ends at the exit and never crosses walls");
}

void TestBorderAndSymmetry() {
    MazeGenerator maze;
    MakeMaze(maze, 20, 10);
    int** grid = maze.GetGrid();
    int size = maze.GetSize();

    bool borderClosed = true;
    bool symmetric = true;
    bool noBorderDoors = true;
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            if (y == 0 && !(grid[y][x] & WallBit(DIR_UP))) borderClosed = false;
            if (y == size - 1 && !(grid[y][x] & WallBit(DIR_DOWN))) borderClosed = false;
            if (x == 0 && !(grid[y][x] & WallBit(DIR_LEFT))) borderClosed = false;
            if (x == size - 1 && !(grid[y][x] & WallBit(DIR_RIGHT))) borderClosed = false;

            for (int d = 0; d < DIR_COUNT; d++) {
                int nx = x + DIR_DX[d];
                int ny = y + DIR_DY[d];
                bool inside = nx >= 0 && ny >= 0 && nx < size && ny < size;
                if (!inside) {
                    if (grid[y][x] & DoorBit(d)) noBorderDoors = false;
                    continue;
                }
                int opposite = (d + 2) % DIR_COUNT;
                bool wallA = (grid[y][x] & WallBit(d)) != 0;
                bool wallB = (grid[ny][nx] & WallBit(opposite)) != 0;
                bool doorA = (grid[y][x] & DoorBit(d)) != 0;
                bool doorB = (grid[ny][nx] & DoorBit(opposite)) != 0;
                if (wallA != wallB || doorA != doorB) symmetric = false;
            }
        }
    }
    Check(borderClosed, "The outer border of the maze is closed");
    Check(symmetric, "Walls and doors are the same seen from both neighbouring cells");
    Check(noBorderDoors, "No door is placed in the outer border");
}

void TestDoors() {
    MazeGenerator maze;
    MakeMaze(maze, 30, 15);
    int** grid = maze.GetGrid();
    int doors = 0;
    bool doorsOnWallsOnly = true;
    for (int y = 0; y < 30; y++) {
        for (int x = 0; x < 30; x++) {
            for (int d = 0; d < DIR_COUNT; d++) {
                if (grid[y][x] & DoorBit(d)) {
                    doors++;
                    if (!(grid[y][x] & WallBit(d))) doorsOnWallsOnly = false;
                }
            }
        }
    }
    Check(doors % 2 == 0 && doors / 2 <= 15, "Door count is not more than requested");
    Check(doorsOnWallsOnly, "Every door sits on a wall");
}

void TestWallCollision() {
    MazeGenerator maze;
    maze.AllocateGrid(10);              // all walls closed
    int** grid = maze.GetGrid();

    Check(GameLogic::CheckWallCollision(grid, 5, 5, DIR_RIGHT), "A wall blocks the player");
    grid[5][5] &= ~WallBit(DIR_RIGHT);
    Check(!GameLogic::CheckWallCollision(grid, 5, 5, DIR_RIGHT), "An open side does not block");
    grid[5][5] |= WallBit(DIR_DOWN) | DoorBit(DIR_DOWN);
    Check(!GameLogic::CheckWallCollision(grid, 5, 5, DIR_DOWN), "A door can be crossed");
}

void TestWinCondition() {
    Check(GameLogic::CheckWinCondition(9, 9, 10, 10), "Exit cell wins");
    Check(!GameLogic::CheckWinCondition(0, 0, 10, 10), "Start cell does not win");
    Check(!GameLogic::CheckWinCondition(9, 8, 10, 10), "Cell next to the exit does not win");
}

void TestPlayerAvatar() {
    PlayerAvatar player;
    Check(player.GetX() == 0 && player.GetY() == 0 && player.GetMoves() == 0, "Player starts at (0,0)");
    player.MovePlayer(1, 0);
    player.MovePlayer(0, 1);
    Check(player.GetX() == 1 && player.GetY() == 1 && player.GetMoves() == 2, "MovePlayer updates position and moves");
    player.Reset();
    Check(player.GetX() == 0 && player.GetMoves() == 0, "Reset returns to the start");
    player.SetSelectedAvatar(3);
    player.SetSelectedAvatar(99);
    Check(player.GetAvatarId() == 3, "Invalid avatar id is ignored");
}

void TestTimeLimitAndDoorCount() {
    Check(GameLogic::CalculateTimeLimit(100, DIFFICULTY_EASY) == 0, "Easy has no time limit");
    Check(GameLogic::CalculateTimeLimit(100, DIFFICULTY_HARD) > 0, "Hard has a time limit");
    Check(GameLogic::CalculateTimeLimit(100, DIFFICULTY_HARD) < GameLogic::CalculateTimeLimit(100, DIFFICULTY_MEDIUM),
          "Hard limit is shorter than Medium limit");
    Check(GameLogic::CalculateTimeLimit(200, DIFFICULTY_HARD) > GameLogic::CalculateTimeLimit(100, DIFFICULTY_HARD),
          "Time limit grows with the path length");
    Check(GameLogic::DoorCountFor(20, DIFFICULTY_EASY) > GameLogic::DoorCountFor(20, DIFFICULTY_MEDIUM) &&
              GameLogic::DoorCountFor(20, DIFFICULTY_MEDIUM) > GameLogic::DoorCountFor(20, DIFFICULTY_HARD),
          "Easy has more doors than Medium, Medium more than Hard");
}

void TestScore() {
    int perfect = GameLogic::CalculateScore(50, 50, 10.0f, false);
    int slow = GameLogic::CalculateScore(50, 200, 300.0f, false);
    int withHint = GameLogic::CalculateScore(50, 50, 10.0f, true);
    Check(perfect == MAX_SCORE, "A perfect run gives the maximum score");
    Check(slow < perfect && slow > 0, "A slow run gives a lower, positive score");
    Check(withHint == static_cast<int>(MAX_SCORE * HINT_SCORE_FACTOR), "Using the hint costs 20% of the score");
    Check(GameLogic::CalculateScore(50, 0, 0.0f, false) <= MAX_SCORE, "Score never goes above the maximum");
}

void TestScoreBoard() {
    ScoreBoard board;
    for (int i = 1; i <= 7; i++) {
        board.AddScore(DIFFICULTY_EASY, {"AAA", i * 100, false});
    }
    const std::vector<ScoreEntry>& list = board.GetEntries(DIFFICULTY_EASY);
    Check(list.size() == 5, "Table keeps only the best 5 scores");
    Check(list.front().score == 700 && list.back().score == 300, "Table is sorted from best to worst");
    Check(board.QualifiesForTable(DIFFICULTY_EASY, 900), "A high score qualifies");
    Check(!board.QualifiesForTable(DIFFICULTY_EASY, 100), "A low score does not qualify");
    Check(board.GetEntries(DIFFICULTY_HARD).empty(), "Tables of different difficulties are separate");

    Check(ScoreBoard::IsValidName("ABC"), "Name ABC is valid");
    Check(!ScoreBoard::IsValidName("AB"), "Too short name is invalid");
    Check(!ScoreBoard::IsValidName("ABCD"), "Too long name is invalid");
    Check(!ScoreBoard::IsValidName("A1C"), "Name with a digit is invalid");
    Check(!ScoreBoard::IsValidName("abc"), "Lowercase name is invalid");

    // A broken file must not crash the game: bad lines are skipped.
    const std::string fileName = "TestScores.txt";
    {
        std::ofstream file(fileName);
        file << "0 AAA 900 0\n";       // good
        file << "0 BB 800 0\n";        // bad name
        file << "1 CCC abc 0\n";       // score is not a number
        file << "7 DDD 100 0\n";       // bad difficulty
        file << "2 EEE 700 1\n";       // good
        file << "garbage line\n";
    }
    ScoreBoard loaded;
    loaded.LoadFromFile(fileName);
    Check(loaded.GetEntries(0).size() == 1 && loaded.GetEntries(1).empty() && loaded.GetEntries(2).size() == 1,
          "Broken lines in the score file are skipped");
    std::remove(fileName.c_str());

    ScoreBoard missing;
    missing.LoadFromFile("this_file_does_not_exist.txt");
    Check(missing.GetEntries(0).empty(), "A missing score file gives empty tables");
}

void TestTextMaze() {
    MazeGenerator maze;
    MakeMaze(maze, 10, 2);
    std::vector<std::string> text = GameLogic::BuildTextMaze(maze.GetGrid(), 10, 0, 0);
    Check(text.size() == 21 && text[0].size() == 21, "Text maze of size 10 is 21x21 characters");
    Check(text[1][1] == '@', "Player is shown at the start");
    Check(text[19][19] == 'E', "Exit is shown in the bottom-right corner");
    Check(text[0][0] == '#', "Corners are walls");
}

void TestMemoryCreateAndDestroy() {
    // Create and destroy many mazes. Run with AddressSanitizer to prove there are no leaks.
    for (int i = 0; i < 200; i++) {
        MazeGenerator maze;
        int size = MIN_MAZE_SIZE + (i * 7) % (MAX_MAZE_SIZE - MIN_MAZE_SIZE + 1);
        MakeMaze(maze, size, size / 5);
        maze.AllocateGrid(size);   // allocating again must free the old grid
    }
    Check(true, "200 mazes created and destroyed without crashing");
}

int main() {
    std::srand(12345);
    TestSizeValidation();
    TestEveryMazeIsSolvable();
    TestBorderAndSymmetry();
    TestDoors();
    TestWallCollision();
    TestWinCondition();
    TestPlayerAvatar();
    TestTimeLimitAndDoorCount();
    TestScore();
    TestScoreBoard();
    TestTextMaze();
    TestMemoryCreateAndDestroy();

    std::cout << "\nPassed: " << g_passed << "  Failed: " << g_failed << "\n";
    return g_failed == 0 ? 0 : 1;
}
