#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "ResourceManager.hpp"
#include "Camera.hpp"
#include "Map.hpp"
#include "Player.hpp"
#include "Bullet.hpp"
#include "Zombie.hpp"
#include "BloodSystem.hpp"
#include "Pickup.hpp"
#include "WaveManager.hpp"
#include "HUD.hpp"

// Menu & Screen states
#include "MainMenu.hpp"
#include "InstructionsScreen.hpp"
#include "SettingsScreen.hpp"
#include "PauseMenu.hpp"
#include "GameOverScreen.hpp"

// Primary game states
enum class GameState {
    MainMenu,
    Instructions,
    Settings,
    Playing,
    Paused,
    GameOver
};

// Game is the primary engine controller owning the window, main loop,
// state machine, delta timing, input routing, world simulation, and rendering.
class Game {
public:
    Game();
    ~Game() = default;

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    // Run the main game loop
    void run();

private:
    void handleEvents();
    void update(float dt);
    void render();

    // Reset all gameplay systems to start a fresh playthrough
    void resetGame();

    // Spawns a single zombie outside screen bounds safely
    void spawnZombie(ZombieType type, float healthMultiplier, float speedMultiplier);

    // Spawns random debug wave
    void spawnDebugZombieWave(int count);

    // Drops calculation when a zombie is slain
    void handleZombieDrop(ZombieType type, sf::Vector2f position);

    // State machine transitions (fade-to-black or instant)
    void transitionTo(GameState nextState, bool instant = false);
    void updateTransition(float dt);

    // Window recreation for Fullscreen / Windowed modes
    void setFullscreen(bool fullscreen);

    // State machine updates
    void updateMainMenu(float dt, sf::Vector2f mouseScreenPos);
    void updateInstructions(float dt, sf::Vector2f mouseScreenPos);
    void updateSettings(float dt, sf::Vector2f mouseScreenPos);
    void updatePlaying(float dt, sf::Vector2f mouseWorldPos);
    void updatePaused(float dt, sf::Vector2f mouseScreenPos);
    void updateGameOver(float dt, sf::Vector2f mouseScreenPos);

    // State machine renders
    void renderMainMenu();
    void renderInstructions();
    void renderSettings();
    void renderPlaying();
    void renderPaused();
    void renderGameOver();

    // Window, Timing & Views
    sf::RenderWindow m_window;
    sf::Clock m_clock;
    sf::View m_uiView;
    bool m_isWindowFocused;

    // State Machine
    GameState m_state;
    GameState m_nextState;
    bool m_isTransitioning;
    bool m_fadingOut;
    float m_fadeAlpha;
    sf::RectangleShape m_fadeRect;

    // Resources & Game Systems
    ResourceManager m_resources;
    std::unique_ptr<Map> m_map;
    std::unique_ptr<Player> m_player;
    Camera m_camera;

    // Entities
    std::vector<Bullet> m_bullets;
    std::vector<Zombie> m_zombies;
    std::vector<Pickup> m_pickups;
    BloodSystem m_bloodSystem;

    // Progression & In-game UI
    WaveManager m_waveManager;
    std::unique_ptr<HUD> m_hud;

    // Menus & Polished Screens
    MainMenu m_mainMenu;
    InstructionsScreen m_instructionsScreen;
    SettingsScreen m_settingsScreen;
    PauseMenu m_pauseMenu;
    GameOverScreen m_gameOverScreen;

    // Stats
    int m_score;
    int m_kills;

    // Screen-space atmospheric & damage effects
    sf::RectangleShape m_darkOverlay;
    sf::Sprite m_vignetteSprite;
    sf::RectangleShape m_damageFlashOverlay;
};
