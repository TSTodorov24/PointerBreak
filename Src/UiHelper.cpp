// UiHelper.cpp - font loading, text drawing and buttons.
#include "UiHelper.h"

static const char* FONT_PATH = "Assets/PressStart2P-Regular.ttf";
static Font s_font;
static bool s_fontLoaded = false;

// Use Raylib's default font when the custom font is unavailable.
static Font GetUiFont() {
    return s_fontLoaded ? s_font : GetFontDefault();
}

void Ui::LoadUiFont() {
    if (FileExists(FONT_PATH)) {
        s_font = LoadFontEx(FONT_PATH, 32, nullptr, 0);
        s_fontLoaded = true;
    }
}

void Ui::UnloadUiFont() {
    if (s_fontLoaded) {
        UnloadFont(s_font);
        s_fontLoaded = false;
    }
}

void Ui::DrawLabel(const char* text, float x, float y, int fontSize, Color color) {
    DrawTextEx(GetUiFont(), text, {x, y}, static_cast<float>(fontSize), 0.0f, color);
}

void Ui::DrawLabelCentered(const char* text, float centerX, float y, int fontSize, Color color) {
    float width = MeasureTextEx(GetUiFont(), text, static_cast<float>(fontSize), 0.0f).x;
    DrawLabel(text, centerX - width / 2.0f, y, fontSize, color);
}

bool Ui::DrawButton(Rectangle bounds, const char* text) {
    bool hovered = CheckCollisionPointRec(GetMousePosition(), bounds);

    DrawRectangleRec(bounds, hovered ? COLOR_BUTTON_HOVER : COLOR_BUTTON);
    DrawRectangleLinesEx(bounds, 3.0f, hovered ? COLOR_TEXT : COLOR_DIM);

    const int fontSize = 24;
    DrawLabelCentered(text, bounds.x + bounds.width / 2.0f,
                      bounds.y + (bounds.height - fontSize) / 2.0f, fontSize, COLOR_TEXT);

    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}
