// GameManager.cpp - window, game loop and switching between screens.
#include "GameManager.h"

#include <cstdlib>
#include <ctime>

#include "Screens.h"
#include "UiHelper.h"
#include "raylib.h"

static const char* SCORES_FILE = "Scores.txt";

GameManager::GameManager()
    : m_state(SCREEN_MAIN_MENU),
      m_settings({20, DIFFICULTY_MEDIUM, 0}),
      m_running(true),
      m_gameplay(m_renderer, m_scoreBoard) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));   // a new maze every run
}

void GameManager::InitializeEngine() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "PointerBreak");
    SetExitKey(KEY_NULL);   // ESC goes back / pauses, it must not close the window
    SetTargetFPS(60);

    // Assets are found next to the program, no matter where it was started from.
    ChangeDirectory(GetApplicationDirectory());

    Ui::LoadUiFont();
    m_renderer.LoadAvatarTextures();
    m_scoreBoard.LoadFromFile(SCORES_FILE);
}

void GameManager::RunGameLoop() {
    while (m_running && !WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        UpdateScreen(deltaTime);
        RenderScreen();
    }
}

// Menus use immediate-mode buttons (a click is detected while drawing),
// so only the gameplay needs a separate update step.
void GameManager::UpdateScreen(float deltaTime) {
    if (m_state == SCREEN_GAMEPLAY) {
        m_gameplay.Update(deltaTime);
    }
}

void GameManager::RenderScreen() {
    BeginDrawing();
    ClearBackground(COLOR_BACKGROUND);

    ScreenState nextState = m_state;
    switch (m_state) {
        case SCREEN_MAIN_MENU:
            nextState = ShowMainMenu(m_renderer);
            break;
        case SCREEN_SIZE_SELECT:
            nextState = ShowSizeSelect(m_settings);
            break;
        case SCREEN_DIFFICULTY_SELECT:
            nextState = ShowDifficultySelect(m_settings);
            break;
        case SCREEN_AVATAR_SELECT:
            nextState = ShowAvatarSelect(m_settings, m_renderer);
            if (nextState == SCREEN_GAMEPLAY) {
                m_gameplay.StartNewGame(m_settings);   // build the maze when START is pressed
            }
            break;
        case SCREEN_GAMEPLAY:
            nextState = m_gameplay.Render();
            break;
        case SCREEN_HELP:
            nextState = ShowHelp();
            break;
        case SCREEN_SCORES:
            nextState = ShowScores(m_scoreBoard);
            break;
        case SCREEN_EXIT:
            m_running = false;
            break;
    }

    EndDrawing();

    if (nextState == SCREEN_EXIT) {
        m_running = false;
    }
    m_state = nextState;
}

void GameManager::ShutdownEngine() {
    m_renderer.UnloadAvatarTextures();   // textures must be freed while the window is still open
    Ui::UnloadUiFont();
    CloseWindow();
}
