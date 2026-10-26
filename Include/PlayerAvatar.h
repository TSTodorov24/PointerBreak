// PlayerAvatar.h - data of the player: position, moves and chosen avatar.
// Only data and logic here. Drawing is done by AvatarRenderer.
#pragma once

class PlayerAvatar {
public:
    PlayerAvatar();

    void Reset();                       // back to (0,0) with 0 moves
    void SetSelectedAvatar(int id);     // ignores ids outside 0..5
    void MovePlayer(int dx, int dy);    // the caller checks walls first (GameLogic)

    int GetX() const { return m_x; }
    int GetY() const { return m_y; }
    int GetMoves() const { return m_moves; }
    int GetAvatarId() const { return m_avatarId; }

private:
    int m_x;
    int m_y;
    int m_moves;
    int m_avatarId;
};
