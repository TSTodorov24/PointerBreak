// GameplayScreen.h - the screen where the game is played.
// It keeps the state of one game: maze, player, timers, hint, camera.
#pragma once

#include <string>
#include <vector>

#include "AvatarRenderer.h"
#include "Constants.h"
#include "MazeGenerator.h"
#include "PlayerAvatar.h"
#include "ScoreBoard.h"
#include "raylib.h"

class GameplayScreen {
public:
    GameplayScreen(AvatarRenderer& renderer, ScoreBoard& scoreBoard);

    void StartNewGame(const GameSettings& settings);
    void Update(float deltaTime);   // input, timers, camera (no drawing)
    ScreenState Render();           // drawing; returns the next screen

private:
    enum Phase { PHASE_PLAYING, PHASE_PAUSED, PHASE_WON, PHASE_LOST };

    AvatarRenderer& m_renderer;
    ScoreBoard& m_scoreBoard;

    GameSettings m_settings = {20, DIFFICULTY_MEDIUM, 0};
    MazeGenerator m_maze;
    PlayerAvatar m_player;
    Phase m_phase = PHASE_PLAYING;

    int m_shortestSteps = 1;        // length of the best path from the start
    int m_timeLimit = 0;            // seconds, 0 = no limit
    float m_elapsed = 0.0f;
    float m_moveCooldown = 0.0f;

    bool m_hintUsed = false;
    float m_hintTimeLeft = 0.0f;
    std::vector<Point> m_hintPath;

    bool m_textMode = false;        // T switches between graphic and text view
    bool m_viewingMap = false;      // true while M is held

    Vector2 m_cameraTarget = {0.0f, 0.0f};
    float m_cameraZoom = 1.0f;

    int m_finalScore = 0;
    bool m_enteringName = false;
    std::string m_nameInput;

    // update helpers
    void HandleHint();
    void HandleMovement(float deltaTime);
    void CheckEndOfGame();
    void HandleNameInput();
    void UpdateCamera(float deltaTime, bool jumpToTarget);

    // drawing helpers
    void DrawMaze();
    void DrawTextMaze();
    void DrawHud();
    ScreenState DrawPauseOverlay();
    ScreenState DrawWonOverlay();
    ScreenState DrawLostOverlay();
};
