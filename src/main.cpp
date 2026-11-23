#include "Config.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>

// Application entry point for GHOULSLAYER.
// Stage 0: Skeleton window verification and clean shutdown test.
int main() {
    std::cout << "========================================\n";
    std::cout << "  GHOULSLAYER: RISE. FIGHT. SURVIVE.\n";
    std::cout << "  Stage 0 Build Skeleton Verified!\n";
    std::cout << "========================================\n";

    // Create the SFML render window using settings from Config.hpp
    sf::RenderWindow window(
        sf::VideoMode(Config::Window::WIDTH, Config::Window::HEIGHT),
        Config::Window::TITLE,
        sf::Style::Titlebar | sf::Style::Close
    );

    // Limit framerate to avoid burning 100% CPU on an empty loop
    window.setFramerateLimit(Config::Window::FPS_LIMIT);

    // Ensure the window gains keyboard focus upon launch
    window.requestFocus();

    std::cout << "[GHOULSLAYER] Window created ("
              << Config::Window::WIDTH << "x" << Config::Window::HEIGHT
              << "). Close window or press ESC to exit.\n";

    // Simple event and render loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            // Handle window close button (X)
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // Close window when ESC key is pressed
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
        }

        // Clear with dark atmospheric background
        window.clear(sf::Color(18, 18, 22));

        // Present to screen
        window.display();
    }

    std::cout << "[GHOULSLAYER] Application closed cleanly.\n";
    return 0;
}
