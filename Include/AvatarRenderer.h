// Loads avatar textures and draws avatars in the game and menus.
#pragma once

#include "Constants.h"
#include "raylib.h"

class AvatarRenderer {
public:
    void LoadAvatarTextures();      // call after the window is open
    void UnloadAvatarTextures();    // call before the window is closed

    void ResetPosition(int cellX, int cellY);           // jump to a cell without sliding
    void Update(float deltaTime, int cellX, int cellY); // slide smoothly towards a cell
    Vector2 GetCenter() const;                          // center of the avatar in world pixels

    void DrawPlayer(int avatarId) const;                      // drawn at the sliding position
    void DrawAvatar(int avatarId, Rectangle destination) const;  // drawn anywhere (menus)
    Color GetAvatarColor(int avatarId) const;

private:
    Texture2D m_textures[AVATAR_COUNT] = {};
    float m_drawX = 0.0f;   // position in cells (can be fractional while sliding)
    float m_drawY = 0.0f;
};
