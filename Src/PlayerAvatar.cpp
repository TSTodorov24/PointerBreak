// PlayerAvatar.cpp - player data.
#include "PlayerAvatar.h"

#include "Constants.h"

PlayerAvatar::PlayerAvatar() : m_x(0), m_y(0), m_moves(0), m_avatarId(0) {}

void PlayerAvatar::Reset() {
    m_x = 0;
    m_y = 0;
    m_moves = 0;
}

void PlayerAvatar::SetSelectedAvatar(int id) {
    if (id >= 0 && id < AVATAR_COUNT) {
        m_avatarId = id;
    }
}

void PlayerAvatar::MovePlayer(int dx, int dy) {
    m_x += dx;
    m_y += dy;
    m_moves++;
}
