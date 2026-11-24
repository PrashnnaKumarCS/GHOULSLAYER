#include "Game.hpp"
#include "PlaceholderArt.hpp"
#include "CollisionSystem.hpp"
#include "Config.hpp"
#include <iostream>
#include <random>
#include <cmath>
#include <algorithm>

namespace {
    std::mt19937& getGameRandom() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        return gen;
    }
}

Game::Game()
    : m_window(
        sf::VideoMode(Config::Window::WIDTH, Config::Window::HEIGHT),
        Config::Window::TITLE,
        sf::Style::Titlebar | sf::Style::Close
    )
    , m_uiView(sf::FloatRect(0.0f, 0.0f, static_cast<float>(Config::Window::WIDTH), static_cast<float>(Config::Window::HEIGHT)))
    , m_isWindowFocused(true)
    , m_state(GameState::MainMenu)
    , m_nextState(GameState::MainMenu)
    , m_isTransitioning(false)
    , m_fadingOut(false)
    , m_fadeAlpha(0.0f)
    , m_mainMenu(m_resources)
    , m_instructionsScreen(m_resources)
    , m_settingsScreen(m_resources)
    , m_pauseMenu(m_resources)
    , m_gameOverScreen(m_resources)
    , m_score(0)
    , m_kills(0)
{
    m_window.setFramerateLimit(Config::Window::FPS_LIMIT);
    m_window.setVerticalSyncEnabled(Config::Window::VSYNC_ENABLED);
    m_window.requestFocus();

    // 1. Generate procedural pixel art textures into ResourceManager
    PlaceholderArt::generateAll(m_resources);

    // 2. Initialize world, player, camera, and HUD
    m_map = std::make_unique<Map>(m_resources);
    m_player = std::make_unique<Player>(m_resources);
    m_hud = std::make_unique<HUD>(m_resources);

    m_camera.initialize(
        m_player->getPosition(),
        sf::Vector2f(static_cast<float>(Config::Window::WIDTH), static_cast<float>(Config::Window::HEIGHT))
    );

    // 3. Screen overlays (atmospheric darkness, vignette, damage flash, fader)
    float W = static_cast<float>(Config::Window::WIDTH);
    float H = static_cast<float>(Config::Window::HEIGHT);

    m_darkOverlay.setSize(sf::Vector2f(W, H));
    m_darkOverlay.setFillColor(Config::Darkness::OVERLAY_COLOR);

    const sf::Texture& vignetteTex = m_resources.getTexture("vignette");
    m_vignetteSprite.setTexture(vignetteTex);
    m_vignetteSprite.setScale(W / 512.0f, H / 512.0f);

    m_damageFlashOverlay.setSize(sf::Vector2f(W, H));
    m_damageFlashOverlay.setFillColor(Config::Darkness::DAMAGE_FLASH_COLOR);

    m_fadeRect.setSize(sf::Vector2f(W, H));
    m_fadeRect.setFillColor(sf::Color(0, 0, 0, 0));

    // 4. Restore fullscreen setting if saved previously
    if (m_settingsScreen.isFullscreen()) {
        setFullscreen(true);
    }

    std::cout << "[GHOULSLAYER] Stage 5: Menus, Polished Screens, State Transitions active.\n";
}

void Game::setFullscreen(bool fullscreen) {
    if (fullscreen) {
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        m_window.create(desktop, Config::Window::TITLE, sf::Style::Fullscreen);
    } else {
        m_window.create(
            sf::VideoMode(Config::Window::WIDTH, Config::Window::HEIGHT),
            Config::Window::TITLE,
            sf::Style::Titlebar | sf::Style::Close
        );
    }

    m_window.setFramerateLimit(Config::Window::FPS_LIMIT);
    m_window.setVerticalSyncEnabled(Config::Window::VSYNC_ENABLED);
    m_window.requestFocus();

    // Maintain consistent 1280x720 virtual resolution across all display modes
    m_uiView.reset(sf::FloatRect(0.0f, 0.0f, static_cast<float>(Config::Window::WIDTH), static_cast<float>(Config::Window::HEIGHT)));
}

void Game::transitionTo(GameState nextState, bool instant) {
    if (instant) {
        m_state = nextState;
        m_isTransitioning = false;
        m_fadingOut = false;
        m_fadeAlpha = 0.0f;
        if (m_state == GameState::Paused) {
            m_pauseMenu.resetSelection();
        }
        return;
    }

    m_nextState = nextState;
    m_isTransitioning = true;
    m_fadingOut = true;
}

void Game::updateTransition(float dt) {
    if (!m_isTransitioning) return;

    constexpr float FADE_SPEED = 900.0f; // Fast, smooth cinematic fade (~0.28s)

    if (m_fadingOut) {
        m_fadeAlpha += FADE_SPEED * dt;
        if (m_fadeAlpha >= 255.0f) {
            m_fadeAlpha = 255.0f;
            m_fadingOut = false; // Peak black reached; switch state and fade back in

            m_state = m_nextState;

            if (m_state == GameState::Paused) {
                m_pauseMenu.resetSelection();
            }
        }
    } else {
        m_fadeAlpha -= FADE_SPEED * dt;
        if (m_fadeAlpha <= 0.0f) {
            m_fadeAlpha = 0.0f;
            m_isTransitioning = false;
        }
    }
}

void Game::resetGame() {
    m_player = std::make_unique<Player>(m_resources);
    m_bullets.clear();
    m_zombies.clear();
    m_pickups.clear();
    m_bloodSystem = BloodSystem();
    m_waveManager.reset();
    m_score = 0;
    m_kills = 0;

    m_camera.initialize(
        m_player->getPosition(),
        sf::Vector2f(static_cast<float>(Config::Window::WIDTH), static_cast<float>(Config::Window::HEIGHT))
    );

    std::cout << "[GHOULSLAYER] Run initialized. Clean game state restored.\n";
}

void Game::spawnZombie(ZombieType type, float healthMultiplier, float speedMultiplier) {
    if (!m_player) return;

    sf::Vector2f playerPos = m_player->getPosition();
    std::uniform_real_distribution<float> angleDist(0.0f, 6.2831853f);
    std::uniform_real_distribution<float> distDist(650.0f, 950.0f);
    const auto& obstacles = m_map->getObstacles();

    for (int attempt = 0; attempt < 25; ++attempt) {
        float angle = angleDist(getGameRandom());
        float distance = distDist(getGameRandom());
        sf::Vector2f candidate(
            playerPos.x + std::cos(angle) * distance,
            playerPos.y + std::sin(angle) * distance
        );

        if (candidate.x < 150.0f || candidate.x > Config::Map::WIDTH - 150.0f ||
            candidate.y < 150.0f || candidate.y > Config::Map::HEIGHT - 150.0f) {
            continue;
        }

        bool insideObstacle = false;
        for (const auto& obs : obstacles) {
            if (CollisionSystem::checkRectCircle(obs, candidate, 32.0f)) {
                insideObstacle = true;
                break;
            }
        }

        if (!insideObstacle) {
            m_zombies.emplace_back(type, candidate, m_resources, healthMultiplier, speedMultiplier);
            return;
        }
    }
}

void Game::spawnDebugZombieWave(int count) {
    for (int i = 0; i < count; ++i) {
        ZombieType type = m_waveManager.pickZombieType();
        spawnZombie(type, m_waveManager.getHealthMultiplier(), m_waveManager.getSpeedMultiplier());
    }
}

void Game::handleZombieDrop(ZombieType type, sf::Vector2f position) {
    std::uniform_real_distribution<float> roll(0.0f, 1.0f);
    float val = roll(getGameRandom());

    if (type == ZombieType::Tank) {
        // Tanks drop valuable supplies with 100% total chance
        if (val < Config::Drops::TANK_HEALTH_CHANCE) {
            m_pickups.emplace_back(PickupType::HealthKit, position, m_resources);
        } else if (val < (Config::Drops::TANK_HEALTH_CHANCE + Config::Drops::TANK_AMMO_CHANCE)) {
            m_pickups.emplace_back(PickupType::AmmoBox, position, m_resources);
        } else {
            m_pickups.emplace_back(PickupType::Coin, position, m_resources);
        }
    } else {
        // Regular zombies drop chance
        if (val < Config::Drops::HEALTH_CHANCE) {
            m_pickups.emplace_back(PickupType::HealthKit, position, m_resources);
        } else if (val < (Config::Drops::HEALTH_CHANCE + Config::Drops::AMMO_CHANCE)) {
            m_pickups.emplace_back(PickupType::AmmoBox, position, m_resources);
        } else if (val < (Config::Drops::HEALTH_CHANCE + Config::Drops::AMMO_CHANCE + Config::Drops::COIN_CHANCE)) {
            m_pickups.emplace_back(PickupType::Coin, position, m_resources);
        }
    }
}

void Game::run() {
    while (m_window.isOpen()) {
        handleEvents();

        float dt = m_clock.restart().asSeconds();
        if (dt > Config::Engine::MAX_DELTA_TIME) {
            dt = Config::Engine::MAX_DELTA_TIME;
        }

        if (m_isWindowFocused) {
            update(dt);
        }

        render();
    }

    std::cout << "[GHOULSLAYER] Shutdown complete. Clean exit.\n";
}

void Game::handleEvents() {
    sf::Vector2i mousePixel = sf::Mouse::getPosition(m_window);
    sf::Vector2f mouseScreenPos = m_window.mapPixelToCoords(mousePixel, m_uiView);

    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_window.close();
        }

        if (event.type == sf::Event::LostFocus) {
            m_isWindowFocused = false;
        }
        if (event.type == sf::Event::GainedFocus) {
            m_isWindowFocused = true;
            m_clock.restart();
        }

        // Delegate discrete events based on active GameState
        switch (m_state) {
            case GameState::MainMenu: {
                MainMenu::Action action = m_mainMenu.handleEvent(event, mouseScreenPos);
                if (action == MainMenu::Action::Play) {
                    resetGame();
                    transitionTo(GameState::Playing);
                } else if (action == MainMenu::Action::Instructions) {
                    transitionTo(GameState::Instructions);
                } else if (action == MainMenu::Action::Settings) {
                    transitionTo(GameState::Settings);
                } else if (action == MainMenu::Action::Exit) {
                    m_window.close();
                }
                break;
            }

            case GameState::Instructions: {
                if (m_instructionsScreen.handleEvent(event, mouseScreenPos)) {
                    transitionTo(GameState::MainMenu);
                }
                break;
            }

            case GameState::Settings: {
                if (m_settingsScreen.handleEvent(event, mouseScreenPos)) {
                    transitionTo(GameState::MainMenu);
                }
                if (m_settingsScreen.consumeFullscreenToggled()) {
                    setFullscreen(m_settingsScreen.isFullscreen());
                }
                break;
            }

            case GameState::Playing: {
                // In-game ESC opens Pause Menu instead of closing the game
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                    transitionTo(GameState::Paused, true);
                    break;
                }

#if DEBUG_FEATURES
                // DEBUG: F3 instantly slays all living zombies and advances to the next wave
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F3) {
                    for (auto& zombie : m_zombies) {
                        zombie.applyDamage(999999.0f, false);
                    }
                    m_zombies.clear();
                    m_bullets.clear();
                    m_waveManager.skipToNextWave();
                    std::cout << "[DEBUG] F3 Triggered: Slayed all zombies, skipped to Wave "
                              << m_waveManager.getCurrentWave() << "!\n";
                }

                // DEBUG: F2 spawns 5 zombies
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F2) {
                    spawnDebugZombieWave(5);
                    std::cout << "[DEBUG] Spawned 5 zombies.\n";
                }
#endif

                if (m_isWindowFocused && m_player) {
                    m_player->handleEvent(event);
                }
                break;
            }

            case GameState::Paused: {
                PauseMenu::Action action = m_pauseMenu.handleEvent(event, mouseScreenPos);
                if (action == PauseMenu::Action::Resume) {
                    transitionTo(GameState::Playing, true);
                } else if (action == PauseMenu::Action::Restart) {
                    resetGame();
                    transitionTo(GameState::Playing);
                } else if (action == PauseMenu::Action::MainMenu) {
                    transitionTo(GameState::MainMenu);
                }
                break;
            }

            case GameState::GameOver: {
                GameOverScreen::Action action = m_gameOverScreen.handleEvent(event, mouseScreenPos);
                if (action == GameOverScreen::Action::Restart) {
                    resetGame();
                    transitionTo(GameState::Playing);
                } else if (action == GameOverScreen::Action::MainMenu) {
                    transitionTo(GameState::MainMenu);
                }
                break;
            }
        }
    }
}

void Game::update(float dt) {
    sf::Vector2i mousePixel = sf::Mouse::getPosition(m_window);
    sf::Vector2f mouseScreenPos = m_window.mapPixelToCoords(mousePixel, m_uiView);
    sf::Vector2f mouseWorldPos = m_window.mapPixelToCoords(mousePixel, m_camera.getView());

    updateTransition(dt);

    switch (m_state) {
        case GameState::MainMenu:
            updateMainMenu(dt, mouseScreenPos);
            break;
        case GameState::Instructions:
            updateInstructions(dt, mouseScreenPos);
            break;
        case GameState::Settings:
            updateSettings(dt, mouseScreenPos);
            break;
        case GameState::Playing:
            updatePlaying(dt, mouseWorldPos);
            break;
        case GameState::Paused:
            updatePaused(dt, mouseScreenPos);
            break;
        case GameState::GameOver:
            updateGameOver(dt, mouseScreenPos);
            break;
    }
}

void Game::updateMainMenu(float dt, sf::Vector2f mouseScreenPos) {
    m_mainMenu.update(dt, mouseScreenPos);
}

void Game::updateInstructions(float /*dt*/, sf::Vector2f mouseScreenPos) {
    m_instructionsScreen.update(mouseScreenPos);
}

void Game::updateSettings(float /*dt*/, sf::Vector2f mouseScreenPos) {
    m_settingsScreen.update(mouseScreenPos);
    if (m_settingsScreen.consumeFullscreenToggled()) {
        setFullscreen(m_settingsScreen.isFullscreen());
    }
}

void Game::updatePaused(float /*dt*/, sf::Vector2f mouseScreenPos) {
    // True freeze: world simulation is skipped entirely!
    m_pauseMenu.update(mouseScreenPos);
}

void Game::updateGameOver(float dt, sf::Vector2f mouseScreenPos) {
    m_gameOverScreen.update(dt, mouseScreenPos);
}

void Game::updatePlaying(float dt, sf::Vector2f mouseWorldPos) {
    // Check death condition
    if (m_player->isDead()) {
        if (!m_isTransitioning) {
            m_gameOverScreen.setFinalStats(m_score, m_waveManager.getCurrentWave(), m_kills);
            transitionTo(GameState::GameOver);
        }
        return;
    }

    const auto& obstacles = m_map->getObstacles();

    // 1. Player Movement, Aiming, and Firing
    m_player->update(dt, obstacles, mouseWorldPos);

    // Retrieve pending bullets created by player
    std::vector<Bullet> newBullets = m_player->extractPendingBullets();
    if (!newBullets.empty()) {
        m_bullets.insert(m_bullets.end(), newBullets.begin(), newBullets.end());
    }

    // 2. Bullets update against obstacles and map boundaries
    for (auto& bullet : m_bullets) {
        bullet.update(dt, obstacles, Config::Map::WIDTH, Config::Map::HEIGHT);
    }
    m_bullets.erase(
        std::remove_if(m_bullets.begin(), m_bullets.end(),
            [](const Bullet& b) { return !b.isAlive(); }),
        m_bullets.end()
    );

    // 3. Blood particle splatters
    m_bloodSystem.update(dt);

    // 4. Pickups animation and collection
    sf::Vector2f playerPos = m_player->getPosition();
    float playerRadius = m_player->getRadius();

    for (auto& pickup : m_pickups) {
        pickup.update(dt);

        if (pickup.checkCollection(playerPos, playerRadius)) {
            if (pickup.getType() == PickupType::HealthKit) {
                m_player->restoreHealth(Config::Drops::HEALTH_RESTORE);
                m_hud->showPickupMessage("+" + std::to_string(static_cast<int>(Config::Drops::HEALTH_RESTORE)) + " HEALTH", sf::Color(60, 220, 80));
            } else if (pickup.getType() == PickupType::AmmoBox) {
                m_player->addAmmo(Config::Drops::AMMO_BOX_ROUNDS);
                m_hud->showPickupMessage("+" + std::to_string(Config::Drops::AMMO_BOX_ROUNDS) + " AMMO", sf::Color(255, 215, 60));
            } else if (pickup.getType() == PickupType::Coin) {
                m_score += Config::Drops::COIN_SCORE;
                m_hud->showPickupMessage("+" + std::to_string(Config::Drops::COIN_SCORE) + " SCORE", sf::Color(255, 230, 90));
            }
        }
    }

    m_pickups.erase(
        std::remove_if(m_pickups.begin(), m_pickups.end(),
            [](const Pickup& p) { return p.isCollected() || p.isExpired(); }),
        m_pickups.end()
    );

    // 5. Update Zombies (AI, pathfinding, wall collisions)
    bool playerAlive = !m_player->isDead();
    for (auto& zombie : m_zombies) {
        zombie.update(dt, playerPos, playerAlive, obstacles);
    }

    // 6. Bullet-to-Zombie Collision
    for (auto& bullet : m_bullets) {
        if (!bullet.isAlive()) continue;

        for (auto& zombie : m_zombies) {
            if (zombie.isDead()) continue;

            if (CollisionSystem::checkPointCircle(bullet.getPosition(), zombie.getPosition(), zombie.getRadius())) {
                bullet.kill();

                bool isShotgun = bullet.isShotgunPellet();
                bool wasAlive = !zombie.isDead();
                zombie.applyDamage(bullet.getDamage(), isShotgun);

                sf::Vector2f hitDir = bullet.getDirection();
                m_bloodSystem.spawnSplatter(bullet.getPosition(), hitDir);

                if (wasAlive && zombie.isDead()) {
                    m_kills++;
                    int killScore = 0;
                    if (zombie.getType() == ZombieType::Walker) killScore = Config::Score::WALKER_KILL;
                    else if (zombie.getType() == ZombieType::Runner) killScore = Config::Score::RUNNER_KILL;
                    else if (zombie.getType() == ZombieType::Tank) killScore = Config::Score::TANK_KILL;

                    m_score += killScore;
                    handleZombieDrop(zombie.getType(), zombie.getPosition());
                }
                break;
            }
        }
    }

    // 7. Zombie-to-Player Attack & Damage
    if (playerAlive) {
        for (auto& zombie : m_zombies) {
            if (zombie.isDead()) continue;

            if (zombie.canAttack(playerPos, playerRadius)) {
                float damage = zombie.performAttack();
                m_player->takeDamage(damage);
            }
        }
    }

    // 9. Zombie-to-zombie soft push-apart separation
    int zombieCount = static_cast<int>(m_zombies.size());
    for (int i = 0; i < zombieCount; ++i) {
        if (m_zombies[i].isDead()) continue;

        for (int j = i + 1; j < zombieCount; ++j) {
            if (m_zombies[j].isDead()) continue;

            sf::Vector2f diff = m_zombies[i].getPosition() - m_zombies[j].getPosition();
            float distSq = diff.x * diff.x + diff.y * diff.y;
            float minDist = m_zombies[i].getRadius() + m_zombies[j].getRadius();

            if (distSq < (minDist * minDist) && distSq > 0.0001f) {
                float dist = std::sqrt(distSq);
                sf::Vector2f normal = diff / dist;
                float overlap = (minDist - dist) * 0.5f;

                sf::Vector2f push = normal * overlap * Config::Zombies::SEPARATION_FORCE * dt;
                m_zombies[i].applySeparation(push, m_map->getObstacles());
                m_zombies[j].applySeparation(-push, m_map->getObstacles());
            }
        }
    }

    // 10. Erase dead zombies whose fade-out animation completed
    m_zombies.erase(
        std::remove_if(m_zombies.begin(), m_zombies.end(),
            [](const Zombie& z) { return z.isReadyForRemoval(); }),
        m_zombies.end()
    );

    // 11. Wave Manager Update & Spawning
    int aliveZombies = 0;
    for (const auto& z : m_zombies) {
        if (!z.isDead()) aliveZombies++;
    }

    m_waveManager.update(dt, aliveZombies);

    // Check wave clear bonus award
    int bonusAward = 0;
    if (m_waveManager.checkAndConsumeWaveClearBonus(bonusAward)) {
        m_score += bonusAward;
        m_hud->showPickupMessage("+" + std::to_string(bonusAward) + " WAVE BONUS", sf::Color(255, 215, 60));
    }

    // Check weapon unlocks
    if (m_waveManager.checkAndConsumeShotgunUnlock()) {
        m_player->unlockWeaponType(WeaponType::Shotgun, Config::Waves::SHOTGUN_STARTER_AMMO);
        m_hud->showPickupMessage("SHOTGUN UNLOCKED! (PRESS 2)", sf::Color(255, 215, 60));
    }
    if (m_waveManager.checkAndConsumeRifleUnlock()) {
        m_player->unlockWeaponType(WeaponType::AssaultRifle, Config::Waves::RIFLE_STARTER_AMMO);
        m_hud->showPickupMessage("ASSAULT RIFLE UNLOCKED! (PRESS 3)", sf::Color(255, 215, 60));
    }

    // Spawn timed batches
    if (m_waveManager.shouldSpawnBatch(dt)) {
        int count = m_waveManager.getBatchSize();
        float hpMult = m_waveManager.getHealthMultiplier();
        float spdMult = m_waveManager.getSpeedMultiplier();

        for (int i = 0; i < count; ++i) {
            ZombieType type = m_waveManager.pickZombieType();
            spawnZombie(type, hpMult, spdMult);
        }
    }

    // 12. Camera tracking
    m_camera.update(m_player->getPosition(), dt);

    // 13. Update HUD timers
    m_hud->update(dt);
}

void Game::render() {
    m_window.clear(sf::Color(14, 14, 18));

    switch (m_state) {
        case GameState::MainMenu:
            renderMainMenu();
            break;
        case GameState::Instructions:
            renderInstructions();
            break;
        case GameState::Settings:
            renderSettings();
            break;
        case GameState::Playing:
            renderPlaying();
            break;
        case GameState::Paused:
            renderPaused();
            break;
        case GameState::GameOver:
            renderGameOver();
            break;
    }

    // State transition fade-to-black overlay
    if (m_fadeAlpha > 0.0f) {
        m_window.setView(m_uiView);
        m_fadeRect.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(std::clamp(m_fadeAlpha, 0.0f, 255.0f))));
        m_window.draw(m_fadeRect);
    }

    m_window.display();
}

void Game::renderMainMenu() {
    m_window.setView(m_uiView);
    m_mainMenu.draw(m_window);
}

void Game::renderInstructions() {
    m_window.setView(m_uiView);
    m_mainMenu.draw(m_window); // Menu backdrop behind overlay
    m_instructionsScreen.draw(m_window);
}

void Game::renderSettings() {
    m_window.setView(m_uiView);
    m_mainMenu.draw(m_window); // Menu backdrop behind overlay
    m_settingsScreen.draw(m_window);
}

void Game::renderPlaying() {
    // 1. World space rendering (camera view)
    m_window.setView(m_camera.getView());

    m_map->draw(m_window, m_camera.getView());

    // Render Pickups
    for (const auto& p : m_pickups) {
        p.draw(m_window);
    }

    // Render Blood splatters
    m_bloodSystem.draw(m_window);

    // Render Bullets
    for (const auto& b : m_bullets) {
        b.draw(m_window);
    }

    // Render Zombies (with camera view frustum culling)
    sf::Vector2f camCenter = m_camera.getCenter();
    sf::FloatRect viewBounds(
        camCenter.x - Config::Window::WIDTH * 0.5f - 80.0f,
        camCenter.y - Config::Window::HEIGHT * 0.5f - 80.0f,
        Config::Window::WIDTH + 160.0f,
        Config::Window::HEIGHT + 160.0f
    );

    for (const auto& z : m_zombies) {
        if (viewBounds.contains(z.getPosition())) {
            z.draw(m_window);
        }
    }

    // Render Player
    m_player->draw(m_window);

    // 2. Screen space rendering (UI view)
    m_window.setView(m_uiView);

    // Dark atmosphere & vignette
    m_window.draw(m_darkOverlay);
    m_window.draw(m_vignetteSprite);

    // Red screen-edge damage flash
    if (m_player->getDamageFlashTimer() > 0.0f) {
        float flashPct = m_player->getDamageFlashTimer() / Config::Player::DAMAGE_FLASH_DURATION;
        sf::Color flashColor = Config::Darkness::DAMAGE_FLASH_COLOR;
        flashColor.a = static_cast<sf::Uint8>(flashPct * 140.0f);
        m_damageFlashOverlay.setFillColor(flashColor);
        m_window.draw(m_damageFlashOverlay);
    }

    // Full In-game HUD
    m_hud->render(m_window, *m_player, m_waveManager, m_score, m_kills);
}

void Game::renderPaused() {
    // Frozen game world in background
    renderPlaying();

    // Semi-transparent pause menu on top
    m_window.setView(m_uiView);
    m_pauseMenu.draw(m_window);
}

void Game::renderGameOver() {
    // Frozen game world in background
    renderPlaying();

    // Polished game over screen on top
    m_window.setView(m_uiView);
    m_gameOverScreen.draw(m_window);
}
