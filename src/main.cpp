#include "Game.hpp"
#include <iostream>

// Application entry point for GHOULSLAYER
// Creates the Game engine instance and begins the main loop.
int main() {
    try {
        Game game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "[GHOULSLAYER FATAL ERROR] " << e.what() << "\n";
        return 1;
    }
    return 0;
}
