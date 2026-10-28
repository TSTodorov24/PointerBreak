// UiHelper.h - colors, text and buttons used by every screen.
#pragma once

#include "raylib.h"

// Retro color palette.
const Color COLOR_BACKGROUND = {16, 14, 32, 255};
const Color COLOR_PANEL = {30, 27, 60, 255};
const Color COLOR_TEXT = {240, 240, 255, 255};
const Color COLOR_DIM = {140, 140, 180, 255};
const Color COLOR_WALL = {0, 200, 255, 255};
const Color COLOR_DOOR = {255, 170, 0, 255};
const Color COLOR_EXIT = {60, 255, 140, 255};
const Color COLOR_HINT = {255, 240, 60, 255};
const Color COLOR_DANGER = {255, 70, 70, 255};
const Color COLOR_BUTTON = {45, 42, 90, 255};
const Color COLOR_BUTTON_HOVER = {95, 70, 175, 255};

namespace Ui {

void LoadUiFont();      // Use the default font if Press Start 2P is missing.
void UnloadUiFont();

void DrawLabel(const char* text, float x, float y, int fontSize, Color color);
void DrawLabelCentered(const char* text, float centerX, float y, int fontSize, Color color);

// Returns true on the frame the button is clicked.
bool DrawButton(Rectangle bounds, const char* text);

}  // namespace Ui
