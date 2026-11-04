// GameManager.h - window, main loop and the state machine of the screens.
#pragma once

#include "AvatarRenderer.h"
#include "Constants.h"
#include "GameplayScreen.h"
#include "ScoreBoard.h"

class GameManager {
public:
    GameManager();

    void InitializeEngine();    // open the window and load assets
    void RunGameLoop();
    void ShutdownEngine();      // free assets and close the window

private:
    void UpdateScreen(float deltaTime);
    void RenderScreen();

    ScreenState m_state;
    GameSettings m_settings;
    bool m_running;

    // The order matters: GameplayScreen uses the two objects above it.
    AvatarRenderer m_renderer;
    ScoreBoard m_scoreBoard;
    GameplayScreen m_gameplay;
};
