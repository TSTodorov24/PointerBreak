// AvatarRenderer.cpp - loading and drawing the six avatars.
#include "AvatarRenderer.h"

static const char* AVATAR_FILES[AVATAR_COUNT] = {
    "Assets/AvatarRed.png",  "Assets/AvatarBlue.png",   "Assets/AvatarGreen.png",
    "Assets/AvatarPink.png", "Assets/AvatarPurple.png", "Assets/AvatarYellow.png"};

// Colors are used as a frame in the menu and as a fallback if an image is missing.
static const Color AVATAR_COLORS[AVATAR_COUNT] = {
    {230, 40, 40, 255},  {30, 160, 220, 255}, {40, 230, 170, 255},
    {250, 100, 200, 255}, {150, 90, 250, 255}, {250, 210, 80, 255}};

void AvatarRenderer::LoadAvatarTextures() {
    for (int i = 0; i < AVATAR_COUNT; i++) {
        if (FileExists(AVATAR_FILES[i])) {
            m_textures[i] = LoadTexture(AVATAR_FILES[i]);
            // Point filter keeps the pixel art sharp when it is scaled.
            SetTextureFilter(m_textures[i], TEXTURE_FILTER_POINT);
        }
    }
}

void AvatarRenderer::UnloadAvatarTextures() {
    for (int i = 0; i < AVATAR_COUNT; i++) {
        if (m_textures[i].id != 0) {
            UnloadTexture(m_textures[i]);
            m_textures[i].id = 0;
        }
    }
}

void AvatarRenderer::ResetPosition(int cellX, int cellY) {
    m_drawX = static_cast<float>(cellX);
    m_drawY = static_cast<float>(cellY);
}

// Smooth the drawn position as the player moves between cells.
void AvatarRenderer::Update(float deltaTime, int cellX, int cellY) {
    float step = deltaTime * 18.0f;
    if (step > 1.0f) {
        step = 1.0f;
    }
    m_drawX += (cellX - m_drawX) * step;
    m_drawY += (cellY - m_drawY) * step;
}

Vector2 AvatarRenderer::GetCenter() const {
    return {(m_drawX + 0.5f) * CELL_SIZE, (m_drawY + 0.5f) * CELL_SIZE};
}

void AvatarRenderer::DrawPlayer(int avatarId) const {
    Rectangle destination = {m_drawX * CELL_SIZE, m_drawY * CELL_SIZE,
                             static_cast<float>(CELL_SIZE), static_cast<float>(CELL_SIZE)};
    DrawAvatar(avatarId, destination);
}

void AvatarRenderer::DrawAvatar(int avatarId, Rectangle destination) const {
    const Texture2D& texture = m_textures[avatarId];
    if (texture.id == 0) {
        DrawRectangleRec(destination, AVATAR_COLORS[avatarId]);   // image missing: colored square
        return;
    }
    // Scale the texture to fit the destination rectangle.
    Rectangle source = {0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height)};
    DrawTexturePro(texture, source, destination, {0.0f, 0.0f}, 0.0f, WHITE);
}

Color AvatarRenderer::GetAvatarColor(int avatarId) const {
    return AVATAR_COLORS[avatarId];
}
