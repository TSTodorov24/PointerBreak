// GameplayScreen.cpp - playing the game: input, timers, camera, hint and drawing.
#include "GameplayScreen.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "GameLogic.h"
#include "UiHelper.h"

// ---------- small local helpers ----------

static void FormatTime(int totalSeconds, char* buffer, size_t bufferSize) {
    std::snprintf(buffer, bufferSize, "%02d:%02d", totalSeconds / 60, totalSeconds % 60);
}

// Keeps the camera inside the maze. A maze smaller than the screen stays centered.
static float ClampCamera(float value, float worldLength, float viewLength) {
    if (worldLength <= viewLength) {
        return worldLength / 2.0f;
    }
    float half = viewLength / 2.0f;
    return std::max(half, std::min(value, worldLength - half));
}

static void DrawWallSegment(float x, float y, bool horizontal, float thickness, Color color) {
    float length = CELL_SIZE + thickness;   // a bit longer so the corners are filled
    if (horizontal) {
        DrawRectangleRec({x - thickness / 2.0f, y - thickness / 2.0f, length, thickness}, color);
    } else {
        DrawRectangleRec({x - thickness / 2.0f, y - thickness / 2.0f, thickness, length}, color);
    }
}

// Neighbouring cells share a wall, so every wall is drawn once:
// right and bottom sides always, left and top only on the outer border.
static bool ShouldDrawWall(int direction, int x, int y) {
    return direction == DIR_RIGHT || direction == DIR_DOWN ||
           (direction == DIR_LEFT && x == 0) || (direction == DIR_UP && y == 0);
}

static void DrawDarkOverlay() {
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, {0, 0, 0, 190});
}

// ---------- start of a game ----------

GameplayScreen::GameplayScreen(AvatarRenderer& renderer, ScoreBoard& scoreBoard)
    : m_renderer(renderer), m_scoreBoard(scoreBoard) {}

void GameplayScreen::StartNewGame(const GameSettings& settings) {
    m_settings = settings;

    m_maze.AllocateGrid(settings.mazeSize);
    m_maze.GenerateMaze(GameLogic::DoorCountFor(settings.mazeSize, settings.difficulty));

    m_player.Reset();
    m_player.SetSelectedAvatar(settings.avatarId);
    m_renderer.ResetPosition(0, 0);

    // The best path from the start is used for the time limit and the score.
    int exitCell = settings.mazeSize - 1;
    std::vector<Point> bestPath =
        GameLogic::FindShortestPath(m_maze.GetGrid(), settings.mazeSize, 0, 0, exitCell, exitCell);
    m_shortestSteps = bestPath.empty() ? 1 : static_cast<int>(bestPath.size()) - 1;
    m_timeLimit = GameLogic::CalculateTimeLimit(m_shortestSteps, settings.difficulty);

    m_phase = PHASE_PLAYING;
    m_elapsed = 0.0f;
    m_moveCooldown = 0.0f;
    m_hintUsed = false;
    m_hintTimeLeft = 0.0f;
    m_hintPath.clear();
    m_textMode = false;
    m_viewingMap = false;
    m_finalScore = 0;
    m_enteringName = false;
    m_nameInput.clear();

    UpdateCamera(0.0f, true);
}

// ---------- update (no drawing here) ----------

void GameplayScreen::Update(float deltaTime) {
    if (m_phase == PHASE_PLAYING) {
        if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
            m_phase = PHASE_PAUSED;
            m_viewingMap = false;
        } else {
            if (IsKeyPressed(KEY_T)) {
                m_textMode = !m_textMode;
            }
            m_viewingMap = IsKeyDown(KEY_M) && !m_textMode;

            HandleHint();
            HandleMovement(deltaTime);

            m_elapsed += deltaTime;                 // the timer keeps running while the map is open
            if (m_hintTimeLeft > 0.0f) {
                m_hintTimeLeft -= deltaTime;
            }
            CheckEndOfGame();
        }
    } else if (m_phase == PHASE_PAUSED) {
        if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
            m_phase = PHASE_PLAYING;
        }
    } else if (m_phase == PHASE_WON && m_enteringName) {
        HandleNameInput();
    }

    m_renderer.Update(deltaTime, m_player.GetX(), m_player.GetY());
    UpdateCamera(deltaTime, false);
}

// H: show the way to the exit for 3 seconds. Only once per game.
void GameplayScreen::HandleHint() {
    if (m_textMode || m_hintUsed || !IsKeyPressed(KEY_H)) {
        return;
    }
    m_hintUsed = true;
    m_hintTimeLeft = HINT_DURATION;

    int size = m_maze.GetSize();
    m_hintPath = GameLogic::FindShortestPath(m_maze.GetGrid(), size, m_player.GetX(), m_player.GetY(),
                                             size - 1, size - 1);
}

// Arrow keys. The logic moves one cell at a time; holding a key repeats the step.
void GameplayScreen::HandleMovement(float deltaTime) {
    if (m_moveCooldown > 0.0f) {
        m_moveCooldown -= deltaTime;
    }
    if (m_viewingMap || m_moveCooldown > 0.0f) {
        return;   // no moving while looking at the map
    }

    int direction = -1;
    if (IsKeyDown(KEY_UP)) direction = DIR_UP;
    else if (IsKeyDown(KEY_RIGHT)) direction = DIR_RIGHT;
    else if (IsKeyDown(KEY_DOWN)) direction = DIR_DOWN;
    else if (IsKeyDown(KEY_LEFT)) direction = DIR_LEFT;
    if (direction < 0) {
        return;
    }

    if (GameLogic::CheckWallCollision(m_maze.GetGrid(), m_player.GetX(), m_player.GetY(), direction)) {
        return;   // a wall is in the way
    }
    m_player.MovePlayer(DIR_DX[direction], DIR_DY[direction]);
    m_moveCooldown = MOVE_COOLDOWN;
}

void GameplayScreen::CheckEndOfGame() {
    int size = m_maze.GetSize();

    if (GameLogic::CheckWinCondition(m_player.GetX(), m_player.GetY(), size, size)) {
        m_phase = PHASE_WON;
        m_viewingMap = false;
        m_hintTimeLeft = 0.0f;
        m_finalScore = GameLogic::CalculateScore(m_shortestSteps, m_player.GetMoves(), m_elapsed, m_hintUsed);
        m_enteringName = m_scoreBoard.QualifiesForTable(m_settings.difficulty, m_finalScore);
        m_nameInput.clear();
    } else if (m_timeLimit > 0 && m_elapsed >= m_timeLimit) {
        m_phase = PHASE_LOST;
        m_viewingMap = false;
        m_hintTimeLeft = 0.0f;
    }
}

// Typing the name for the score table: only letters A-Z, exactly 3.
void GameplayScreen::HandleNameInput() {
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 'a' && key <= 'z') {
            key -= 32;   // make it uppercase
        }
        if (key >= 'A' && key <= 'Z' && m_nameInput.size() < 3) {
            m_nameInput += static_cast<char>(key);
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && !m_nameInput.empty()) {
        m_nameInput.pop_back();
    }

    if (IsKeyPressed(KEY_ENTER) && ScoreBoard::IsValidName(m_nameInput)) {
        m_scoreBoard.AddScore(m_settings.difficulty, {m_nameInput, m_finalScore, m_hintUsed});
        m_scoreBoard.SaveToFile("Scores.txt");
        m_enteringName = false;
    }
}

// The camera follows the player. While M is held, the hint is shown, or the game
// is over, it zooms out so the whole maze is visible.
void GameplayScreen::UpdateCamera(float deltaTime, bool jumpToTarget) {
    float worldSize = static_cast<float>(m_maze.GetSize() * CELL_SIZE);
    bool showWholeMaze = m_viewingMap || m_hintTimeLeft > 0.0f || m_phase == PHASE_WON || m_phase == PHASE_LOST;

    float desiredZoom = 1.0f;
    Vector2 desiredTarget;
    if (showWholeMaze) {
        float fit = std::min(WINDOW_WIDTH / worldSize, VIEW_HEIGHT / worldSize) * 0.95f;
        desiredZoom = std::min(1.0f, fit);
        desiredTarget = {worldSize / 2.0f, worldSize / 2.0f};
    } else {
        Vector2 center = m_renderer.GetCenter();
        desiredTarget = {ClampCamera(center.x, worldSize, static_cast<float>(WINDOW_WIDTH)),
                         ClampCamera(center.y, worldSize, static_cast<float>(VIEW_HEIGHT))};
    }

    float step = jumpToTarget ? 1.0f : std::min(1.0f, deltaTime * 8.0f);
    m_cameraZoom += (desiredZoom - m_cameraZoom) * step;
    m_cameraTarget.x += (desiredTarget.x - m_cameraTarget.x) * step;
    m_cameraTarget.y += (desiredTarget.y - m_cameraTarget.y) * step;
}

// ---------- drawing ----------

ScreenState GameplayScreen::Render() {
    ScreenState next = SCREEN_GAMEPLAY;

    if (m_phase == PHASE_PAUSED) {
        next = DrawPauseOverlay();   // the maze is hidden while paused
    } else {
        if (m_textMode) {
            DrawTextMaze();
        } else {
            DrawMaze();
        }
        DrawHud();

        if (m_phase == PHASE_WON) {
            next = DrawWonOverlay();
        } else if (m_phase == PHASE_LOST) {
            next = DrawLostOverlay();
        }
    }
    return next;
}

void GameplayScreen::DrawMaze() {
    int** grid = m_maze.GetGrid();
    int size = m_maze.GetSize();
    float worldSize = static_cast<float>(size * CELL_SIZE);

    Camera2D camera = {};
    camera.offset = {WINDOW_WIDTH / 2.0f, HUD_HEIGHT + VIEW_HEIGHT / 2.0f};
    camera.target = m_cameraTarget;
    camera.zoom = m_cameraZoom;

    // Walls get thicker when zoomed out, so they stay visible.
    float wallThickness = std::max(4.0f, 2.0f / m_cameraZoom);
    float pulse = std::sin(static_cast<float>(GetTime()) * 6.0f);

    Color doorColor = COLOR_DOOR;                     // doors blink
    doorColor.a = static_cast<unsigned char>(170 + 85 * pulse);
    Color exitColor = COLOR_EXIT;
    exitColor.a = static_cast<unsigned char>(190 + 60 * pulse);

    BeginScissorMode(0, HUD_HEIGHT, WINDOW_WIDTH, VIEW_HEIGHT);   // draw only below the top bar
    BeginMode2D(camera);

    DrawRectangle(0, 0, static_cast<int>(worldSize), static_cast<int>(worldSize), COLOR_PANEL);

    // exit
    float exitStart = (size - 1) * static_cast<float>(CELL_SIZE) + 10.0f;
    DrawRectangleRec({exitStart, exitStart, CELL_SIZE - 20.0f, CELL_SIZE - 20.0f}, exitColor);

    // hint path
    float hintThickness = std::max(8.0f, 3.0f / m_cameraZoom);
    for (size_t i = 1; i < m_hintPath.size(); i++) {
        Vector2 from = {(m_hintPath[i - 1].x + 0.5f) * CELL_SIZE, (m_hintPath[i - 1].y + 0.5f) * CELL_SIZE};
        Vector2 to = {(m_hintPath[i].x + 0.5f) * CELL_SIZE, (m_hintPath[i].y + 0.5f) * CELL_SIZE};
        if (m_hintTimeLeft > 0.0f) {
            DrawLineEx(from, to, hintThickness, COLOR_HINT);
        }
    }

    // player
    m_renderer.DrawPlayer(m_player.GetAvatarId());

    // walls and doors
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            for (int d = 0; d < DIR_COUNT; d++) {
                if (!(grid[y][x] & WallBit(d)) || !ShouldDrawWall(d, x, y)) {
                    continue;
                }
                float cellX = static_cast<float>(x * CELL_SIZE);
                float cellY = static_cast<float>(y * CELL_SIZE);
                if (d == DIR_RIGHT) cellX += CELL_SIZE;
                if (d == DIR_DOWN) cellY += CELL_SIZE;

                bool horizontal = (d == DIR_UP || d == DIR_DOWN);
                Color color = (grid[y][x] & DoorBit(d)) ? doorColor : COLOR_WALL;
                DrawWallSegment(cellX, cellY, horizontal, wallThickness, color);
            }
        }
    }

    EndMode2D();
    EndScissorMode();
}

// Text view: a window of characters around the player, drawn with the monospace font.
void GameplayScreen::DrawTextMaze() {
    std::vector<std::string> lines =
        GameLogic::BuildTextMaze(m_maze.GetGrid(), m_maze.GetSize(), m_player.GetX(), m_player.GetY());

    const int fontSize = 24;
    int width = static_cast<int>(lines[0].size());
    int height = static_cast<int>(lines.size());
    int visibleCols = std::min(WINDOW_WIDTH / fontSize, width);
    int visibleRows = std::min(VIEW_HEIGHT / fontSize, height);

    // Center the window on the player (text position of the player = 2 * cell + 1).
    int startCol = std::max(0, std::min(2 * m_player.GetX() + 1 - visibleCols / 2, width - visibleCols));
    int startRow = std::max(0, std::min(2 * m_player.GetY() + 1 - visibleRows / 2, height - visibleRows));

    float offsetX = (WINDOW_WIDTH - visibleCols * fontSize) / 2.0f;
    float offsetY = HUD_HEIGHT + (VIEW_HEIGHT - visibleRows * fontSize) / 2.0f;

    for (int row = 0; row < visibleRows; row++) {
        for (int col = 0; col < visibleCols; col++) {
            char letter = lines[startRow + row][startCol + col];
            if (letter == ' ') {
                continue;
            }
            Color color = COLOR_WALL;
            if (letter == '@') color = COLOR_HINT;
            if (letter == 'E') color = COLOR_EXIT;
            if (letter == 'D') color = COLOR_DOOR;

            char text[2] = {letter, '\0'};
            Ui::DrawLabel(text, offsetX + col * fontSize, offsetY + row * fontSize, fontSize, color);
        }
    }
}

void GameplayScreen::DrawHud() {
    DrawRectangle(0, 0, WINDOW_WIDTH, HUD_HEIGHT, COLOR_PANEL);

    char text[64];
    char timeText[16];

    // With a time limit we show the time left, otherwise the time passed.
    if (m_timeLimit > 0) {
        int remaining = std::max(0, static_cast<int>(std::ceil(m_timeLimit - m_elapsed)));
        FormatTime(remaining, timeText, sizeof(timeText));
        std::snprintf(text, sizeof(text), "TIME LEFT %s", timeText);
        Ui::DrawLabel(text, 16, 6, 20, remaining <= 10 ? COLOR_DANGER : COLOR_TEXT);
    } else {
        FormatTime(static_cast<int>(m_elapsed), timeText, sizeof(timeText));
        std::snprintf(text, sizeof(text), "TIME %s", timeText);
        Ui::DrawLabel(text, 16, 6, 20, COLOR_TEXT);
    }

    std::snprintf(text, sizeof(text), "MOVES %d", m_player.GetMoves());
    Ui::DrawLabel(text, 440, 6, 20, COLOR_TEXT);

    Ui::DrawLabel(m_hintUsed ? "HINT USED" : "HINT READY", 760, 6, 20, m_hintUsed ? COLOR_DIM : COLOR_HINT);

    Ui::DrawLabel("ARROWS MOVE  M MAP  H HINT  T TEXT  P PAUSE", 16, 34, 16, COLOR_DIM);

    std::snprintf(text, sizeof(text), "%s %dx%d", DIFFICULTY_NAMES[m_settings.difficulty],
                  m_settings.mazeSize, m_settings.mazeSize);
    Ui::DrawLabel(text, 1264.0f - static_cast<float>(std::strlen(text)) * 16.0f, 34, 16, COLOR_DIM);   // right aligned
}

ScreenState GameplayScreen::DrawPauseOverlay() {
    ScreenState next = SCREEN_GAMEPLAY;
    DrawDarkOverlay();
    Ui::DrawLabelCentered("PAUSED", WINDOW_WIDTH / 2.0f, 180, 48, COLOR_TEXT);

    if (Ui::DrawButton({440, 290, 400, 60}, "RESUME")) m_phase = PHASE_PLAYING;
    if (Ui::DrawButton({440, 370, 400, 60}, "RESTART")) StartNewGame(m_settings);
    if (Ui::DrawButton({440, 450, 400, 60}, "MENU")) next = SCREEN_MAIN_MENU;
    return next;
}

ScreenState GameplayScreen::DrawWonOverlay() {
    ScreenState next = SCREEN_GAMEPLAY;
    float centerX = WINDOW_WIDTH / 2.0f;
    DrawDarkOverlay();

    Ui::DrawLabelCentered("YOU ESCAPED!", centerX, 110, 48, COLOR_EXIT);

    char timeText[16];
    char line[64];
    FormatTime(static_cast<int>(m_elapsed), timeText, sizeof(timeText));
    std::snprintf(line, sizeof(line), "MOVES %d   TIME %s", m_player.GetMoves(), timeText);
    Ui::DrawLabelCentered(line, centerX, 200, 24, COLOR_TEXT);

    std::snprintf(line, sizeof(line), "SCORE %d", m_finalScore);
    Ui::DrawLabelCentered(line, centerX, 250, 32, COLOR_HINT);
    if (m_hintUsed) {
        Ui::DrawLabelCentered("HINT USED: SCORE -20%", centerX, 300, 16, COLOR_DIM);
    }

    if (m_enteringName) {
        // The score is good enough for the table: ask for a 3-letter name.
        char nameText[16];
        std::snprintf(nameText, sizeof(nameText), "%c %c %c",
                      m_nameInput.size() > 0 ? m_nameInput[0] : '_',
                      m_nameInput.size() > 1 ? m_nameInput[1] : '_',
                      m_nameInput.size() > 2 ? m_nameInput[2] : '_');
        Ui::DrawLabelCentered("NEW HIGH SCORE!", centerX, 370, 24, COLOR_HINT);
        Ui::DrawLabelCentered("TYPE 3 LETTERS", centerX, 415, 16, COLOR_TEXT);
        Ui::DrawLabelCentered(nameText, centerX, 450, 48, COLOR_TEXT);
        if (ScoreBoard::IsValidName(m_nameInput)) {
            Ui::DrawLabelCentered("PRESS ENTER", centerX, 530, 16, COLOR_DIM);
        }
    } else {
        if (Ui::DrawButton({440, 370, 400, 60}, "PLAY AGAIN")) StartNewGame(m_settings);
        if (Ui::DrawButton({440, 450, 400, 60}, "BEST SCORES")) next = SCREEN_SCORES;
        if (Ui::DrawButton({440, 530, 400, 60}, "MENU")) next = SCREEN_MAIN_MENU;
    }
    return next;
}

ScreenState GameplayScreen::DrawLostOverlay() {
    ScreenState next = SCREEN_GAMEPLAY;
    float centerX = WINDOW_WIDTH / 2.0f;
    DrawDarkOverlay();

    Ui::DrawLabelCentered("TIME IS UP!", centerX, 200, 48, COLOR_DANGER);
    Ui::DrawLabelCentered("THE MAZE WAS TOO FAST FOR YOU", centerX, 280, 16, COLOR_DIM);

    if (Ui::DrawButton({440, 370, 400, 60}, "TRY AGAIN")) StartNewGame(m_settings);
    if (Ui::DrawButton({440, 450, 400, 60}, "MENU")) next = SCREEN_MAIN_MENU;
    return next;
}
