#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include "ResourceManager.hpp"
#include "Camera.hpp"
#include "Map.hpp"
#include "Player.hpp"

// Game states for state machine
enum class GameState {
    MainMenu,
    Playing,
    Paused,
    GameOver
};

// Game is the primary engine controller owning the window, main loop,
// state transitions, delta timing, input distribution, and rendering.
class Game {
public:
    Game();
    ~Game() = default;

    // Disallow copying
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    // Run the main game loop
    void run();

private:
    void handleEvents();
    void update(float dt);
    void render();

    // State machine updates
    void updateMainMenu(float dt);
    void updatePlaying(float dt);
    void updatePaused(float dt);
    void updateGameOver(float dt);

    // State machine renders
    void renderMainMenu();
    void renderPlaying();
    void renderPaused();
    void renderGameOver();

    // Window and Timing
    sf::RenderWindow m_window;
    sf::Clock m_clock;
    bool m_isWindowFocused;

    // State
    GameState m_state;

    // Resources & Game Systems
    ResourceManager m_resources;
    std::unique_ptr<Map> m_map;
    std::unique_ptr<Player> m_player;
    Camera m_camera;

    // Screen-space atmospheric effects
    sf::RectangleShape m_darkOverlay;
    sf::Sprite m_vignetteSprite;
};
