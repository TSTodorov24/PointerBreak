// Screens.h - the simple menu screens. Each function draws its screen,
// handles the input and returns the screen that should be shown next.
#pragma once

#include "AvatarRenderer.h"
#include "Constants.h"
#include "ScoreBoard.h"

ScreenState ShowMainMenu(const AvatarRenderer& renderer);
ScreenState ShowHelp();
ScreenState ShowSizeSelect(GameSettings& settings);
ScreenState ShowDifficultySelect(GameSettings& settings);
ScreenState ShowAvatarSelect(GameSettings& settings, const AvatarRenderer& renderer);
ScreenState ShowScores(const ScoreBoard& scoreBoard);
