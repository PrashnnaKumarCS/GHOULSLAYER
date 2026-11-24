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

// Primary game states
enum class GameState {
    MainMenu,
    Playing,
    Paused,
    GameOver
};

// Game is the primary engine controller owning the window, main loop,
// state transitions, delta timing, input distribution, world simulation, and rendering.
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

    // Reset all game systems to start a fresh playthrough
    void resetGame();

    // Spawns a single zombie outside screen bounds safely
    void spawnZombie(ZombieType type, float healthMultiplier, float speedMultiplier);

    // Spawns random debug wave
    void spawnDebugZombieWave(int count);

    // Drops calculation when a zombie is slain
    void handleZombieDrop(ZombieType type, sf::Vector2f position);

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

    // Entities
    std::vector<Bullet> m_bullets;
    std::vector<Zombie> m_zombies;
    std::vector<Pickup> m_pickups;
    BloodSystem m_bloodSystem;

    // Progression & UI
    WaveManager m_waveManager;
    std::unique_ptr<HUD> m_hud;

    // Stats
    int m_score;
    int m_kills;

    // Screen-space atmospheric & damage effects
    sf::RectangleShape m_darkOverlay;
    sf::Sprite m_vignetteSprite;
    sf::RectangleShape m_damageFlashOverlay;

    // Game Over UI elements
    sf::RectangleShape m_gameOverOverlay;
    sf::RectangleShape m_gameOverBox;
    sf::Text m_gameOverTitleText;
    sf::Text m_gameOverSubtitleText;
    sf::Text m_gameOverStatsText;
    sf::Text m_gameOverPromptText;
};
