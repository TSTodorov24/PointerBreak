// Screens.cpp - the simple screens: main menu, help, size, difficulty, avatar, scores.
#include "Screens.h"

#include <cmath>
#include <cstdio>

#include "GameLogic.h"
#include "UiHelper.h"

static const float CENTER_X = WINDOW_WIDTH / 2.0f;

static void DrawTitle(const char* text) {
    Ui::DrawLabelCentered(text, CENTER_X, 50, 40, COLOR_EXIT);
}

ScreenState ShowMainMenu(const AvatarRenderer& renderer) {
    Ui::DrawLabelCentered("POINTERBREAK", CENTER_X, 50, 64, COLOR_EXIT);
    Ui::DrawLabelCentered("ESCAPE THE MAZE", CENTER_X, 125, 16, COLOR_DIM);

    // A row with all six avatars as decoration.
    float startX = (WINDOW_WIDTH - (AVATAR_COUNT * 64 + (AVATAR_COUNT - 1) * 24)) / 2.0f;
    for (int i = 0; i < AVATAR_COUNT; i++) {
        renderer.DrawAvatar(i, {startX + i * 88.0f, 165.0f, 64.0f, 64.0f});
    }

    if (Ui::DrawButton({440, 270, 400, 64}, "NEW GAME")) return SCREEN_SIZE_SELECT;
    if (Ui::DrawButton({440, 350, 400, 64}, "HOW TO PLAY")) return SCREEN_HELP;
    if (Ui::DrawButton({440, 430, 400, 64}, "BEST SCORES")) return SCREEN_SCORES;
    if (Ui::DrawButton({440, 510, 400, 64}, "EXIT")) return SCREEN_EXIT;
    return SCREEN_MAIN_MENU;
}

ScreenState ShowHelp() {
    DrawTitle("HOW TO PLAY");

    const char* lines[] = {
        "GOAL: GO FROM THE TOP-LEFT CORNER TO THE EXIT IN THE BOTTOM-RIGHT.",
        "ARROW KEYS MOVE THE PLAYER. YOU CANNOT WALK THROUGH WALLS.",
        "BLINKING ORANGE WALLS ARE DOORS: CROSS THEM AS A SHORTCUT.",
        "HOLD M TO SEE THE WHOLE MAZE (YOU CANNOT MOVE, TIME KEEPS RUNNING).",
        "H SHOWS THE WAY TO THE EXIT FOR 3 SECONDS. ONCE PER GAME, -20% SCORE.",
        "T SWITCHES BETWEEN GRAPHIC AND TEXT VIEW.  P OR ESC PAUSES THE GAME.",
        "EASY: NO TIME LIMIT, MANY DOORS.",
        "MEDIUM: GENEROUS TIME LIMIT, SOME DOORS.",
        "HARD: TIGHT TIME LIMIT, FEW DOORS.",
        "SCORE: FEWER MOVES AND LESS TIME GIVE MORE POINTS (MAX 1000).",
    };
    for (int i = 0; i < 10; i++) {
        Color color = (i == 2) ? COLOR_DOOR : COLOR_TEXT;
        Ui::DrawLabel(lines[i], 80, 140.0f + i * 44.0f, 16, color);
    }

    if (Ui::DrawButton({440, 620, 400, 56}, "BACK") || IsKeyPressed(KEY_ESCAPE)) {
        return SCREEN_MAIN_MENU;
    }
    return SCREEN_HELP;
}

// Changes the size only if the new value stays inside 10..50 (input validation).
static void ChangeSize(GameSettings& settings, int newSize) {
    if (GameLogic::IsValidSize(newSize)) {
        settings.mazeSize = newSize;
    }
}

ScreenState ShowSizeSelect(GameSettings& settings) {
    DrawTitle("CHOOSE SIZE");

    char text[32];
    std::snprintf(text, sizeof(text), "%d x %d", settings.mazeSize, settings.mazeSize);
    Ui::DrawLabelCentered(text, CENTER_X, 211, 48, COLOR_TEXT);
    Ui::DrawLabelCentered("USE - AND + OR THE LEFT AND RIGHT KEYS", CENTER_X, 300, 16, COLOR_DIM);

    if (Ui::DrawButton({360, 190, 90, 90}, "-") || IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) {
        ChangeSize(settings, settings.mazeSize - 1);
    }
    if (Ui::DrawButton({830, 190, 90, 90}, "+") || IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) {
        ChangeSize(settings, settings.mazeSize + 1);
    }

    // Quick sizes.
    for (int i = 0; i < 5; i++) {
        char label[8];
        int size = MIN_MAZE_SIZE + i * 10;
        std::snprintf(label, sizeof(label), "%d", size);
        if (Ui::DrawButton({300.0f + i * 140.0f, 360, 120, 56}, label)) {
            ChangeSize(settings, size);
        }
    }

    if (Ui::DrawButton({440, 490, 400, 64}, "NEXT") || IsKeyPressed(KEY_ENTER)) return SCREEN_DIFFICULTY_SELECT;
    if (Ui::DrawButton({440, 575, 400, 56}, "BACK") || IsKeyPressed(KEY_ESCAPE)) return SCREEN_MAIN_MENU;
    return SCREEN_SIZE_SELECT;
}

ScreenState ShowDifficultySelect(GameSettings& settings) {
    DrawTitle("CHOOSE DIFFICULTY");

    const char* descriptions[DIFFICULTY_COUNT] = {
        "NO TIME LIMIT, MANY DOORS",
        "GENEROUS TIME LIMIT, SOME DOORS",
        "TIGHT TIME LIMIT, FEW DOORS"};

    for (int i = 0; i < DIFFICULTY_COUNT; i++) {
        Rectangle button = {360, 160.0f + i * 125.0f, 560, 70};
        if (Ui::DrawButton(button, DIFFICULTY_NAMES[i])) {
            settings.difficulty = static_cast<Difficulty>(i);
            return SCREEN_AVATAR_SELECT;
        }
        if (settings.difficulty == i) {
            DrawRectangleLinesEx(button, 5.0f, COLOR_EXIT);   // the current choice
        }
        Ui::DrawLabelCentered(descriptions[i], CENTER_X, button.y + 82, 16, COLOR_DIM);
    }

    if (Ui::DrawButton({440, 575, 400, 56}, "BACK") || IsKeyPressed(KEY_ESCAPE)) return SCREEN_SIZE_SELECT;
    return SCREEN_DIFFICULTY_SELECT;
}

ScreenState ShowAvatarSelect(GameSettings& settings, const AvatarRenderer& renderer) {
    DrawTitle("CHOOSE YOUR AVATAR");

    // 3 x 2 cards with the avatars.
    for (int i = 0; i < AVATAR_COUNT; i++) {
        Rectangle card = {340.0f + (i % 3) * 210.0f, 120.0f + (i / 3) * 210.0f, 180, 180};
        bool hovered = CheckCollisionPointRec(GetMousePosition(), card);

        DrawRectangleRec(card, COLOR_PANEL);
        if (settings.avatarId == i) {
            DrawRectangleLinesEx(card, 6.0f, renderer.GetAvatarColor(i));
        } else {
            DrawRectangleLinesEx(card, 2.0f, COLOR_DIM);
        }

        // The avatar under the mouse bounces a little.
        float size = 128.0f;
        if (hovered) {
            size += 12.0f + 6.0f * std::sin(static_cast<float>(GetTime()) * 8.0f);
        }
        float centerX = card.x + card.width / 2.0f;
        float centerY = card.y + card.height / 2.0f;
        renderer.DrawAvatar(i, {centerX - size / 2.0f, centerY - size / 2.0f, size, size});

        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            settings.avatarId = i;
        }
    }

    if (Ui::DrawButton({440, 545, 400, 64}, "START") || IsKeyPressed(KEY_ENTER)) return SCREEN_GAMEPLAY;
    if (Ui::DrawButton({440, 625, 400, 56}, "BACK") || IsKeyPressed(KEY_ESCAPE)) return SCREEN_DIFFICULTY_SELECT;
    return SCREEN_AVATAR_SELECT;
}

ScreenState ShowScores(const ScoreBoard& scoreBoard) {
    DrawTitle("BEST SCORES");

    for (int d = 0; d < DIFFICULTY_COUNT; d++) {
        float columnX = 80.0f + d * 400.0f;
        Ui::DrawLabel(DIFFICULTY_NAMES[d], columnX, 150, 24, COLOR_HINT);

        const std::vector<ScoreEntry>& entries = scoreBoard.GetEntries(d);
        for (int i = 0; i < MAX_SCORES_PER_TABLE; i++) {
            char line[48];
            if (i < static_cast<int>(entries.size())) {
                std::snprintf(line, sizeof(line), "%d %s %4d%s", i + 1, entries[i].name.c_str(),
                              entries[i].score, entries[i].hintUsed ? " *" : "");
                Ui::DrawLabel(line, columnX, 210.0f + i * 50.0f, 20, COLOR_TEXT);
            } else {
                std::snprintf(line, sizeof(line), "%d ---", i + 1);
                Ui::DrawLabel(line, columnX, 210.0f + i * 50.0f, 20, COLOR_DIM);
            }
        }
    }
    Ui::DrawLabelCentered("* = HINT USED", CENTER_X, 500, 16, COLOR_DIM);

    if (Ui::DrawButton({440, 590, 400, 56}, "BACK") || IsKeyPressed(KEY_ESCAPE)) return SCREEN_MAIN_MENU;
    return SCREEN_SCORES;
}
