// Constants.h - values and small types shared by the whole project.
// This file does not use Raylib, so the game logic can include it freely.
#pragma once

// ----- Window and drawing -----
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const int HUD_HEIGHT = 56;                           // top bar with time and moves
const int VIEW_HEIGHT = WINDOW_HEIGHT - HUD_HEIGHT;  // area where the maze is drawn
const int CELL_SIZE = 64;                            // avatars are 192x192, so a cell is exactly one third

// ----- Maze size limits (chosen by the player) -----
const int MIN_MAZE_SIZE = 10;
const int MAX_MAZE_SIZE = 50;

// ----- Directions -----
enum Direction { DIR_UP = 0, DIR_RIGHT = 1, DIR_DOWN = 2, DIR_LEFT = 3 };
const int DIR_COUNT = 4;
const int DIR_DX[DIR_COUNT] = {0, 1, 0, -1};
const int DIR_DY[DIR_COUNT] = {-1, 0, 1, 0};

// ----- Cell bits -----
// Every cell of the grid is one int:
//   bits 0-3 : a wall on the side UP / RIGHT / DOWN / LEFT
//   bits 4-7 : a door on that side (a wall the player may cross = shortcut)
//   bit 8    : "visited" flag, used only while the maze is generated
inline int WallBit(int direction) { return 1 << direction; }
inline int DoorBit(int direction) { return 16 << direction; }
const int ALL_WALLS = 15;
const int VISITED_BIT = 256;

// ----- Game rules (change these numbers to tune the game) -----
const int MIN_SHORTCUT_LENGTH = 10;            // a door must save at least this many steps
const float MOVE_COOLDOWN = 0.13f;             // seconds between two steps when a key is held
const float HINT_DURATION = 3.0f;              // seconds the hint is shown
const float HINT_SCORE_FACTOR = 0.8f;          // using the hint costs 20% of the score
const float MEDIUM_SECONDS_PER_STEP = 4.0f;    // time limit = shortest path * this
const float HARD_SECONDS_PER_STEP = 2.0f;
const float BEST_SECONDS_PER_STEP = 0.4f;      // speed of a "perfect" player (for the score)
const int MAX_SCORE = 1000;
const int MAX_SCORES_PER_TABLE = 5;

// ----- Difficulty -----
enum Difficulty { DIFFICULTY_EASY = 0, DIFFICULTY_MEDIUM = 1, DIFFICULTY_HARD = 2 };
const int DIFFICULTY_COUNT = 3;
const char* const DIFFICULTY_NAMES[DIFFICULTY_COUNT] = {"EASY", "MEDIUM", "HARD"};

// ----- Avatars -----
const int AVATAR_COUNT = 6;

// ----- Screens (the state machine of GameManager) -----
enum ScreenState {
    SCREEN_MAIN_MENU,
    SCREEN_SIZE_SELECT,
    SCREEN_DIFFICULTY_SELECT,
    SCREEN_AVATAR_SELECT,
    SCREEN_GAMEPLAY,
    SCREEN_HELP,
    SCREEN_SCORES,
    SCREEN_EXIT
};

// ----- Small shared types -----
struct Point {
    int x;
    int y;
};

// What the player chose in the menus.
struct GameSettings {
    int mazeSize;
    Difficulty difficulty;
    int avatarId;
};
