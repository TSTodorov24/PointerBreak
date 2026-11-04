// Main.cpp - starts the game.
#include "GameManager.h"

int main() {
    GameManager game;
    game.InitializeEngine();
    game.RunGameLoop();
    game.ShutdownEngine();
    return 0;
}
