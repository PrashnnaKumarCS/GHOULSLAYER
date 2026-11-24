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
    , m_isWindowFocused(true)
    , m_state(GameState::Playing)
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

    // 3. Screen overlays
    m_darkOverlay.setSize(sf::Vector2f(
        static_cast<float>(Config::Window::WIDTH),
        static_cast<float>(Config::Window::HEIGHT)
    ));
    m_darkOverlay.setFillColor(Config::Darkness::OVERLAY_COLOR);

    const sf::Texture& vignetteTex = m_resources.getTexture("vignette");
    m_vignetteSprite.setTexture(vignetteTex);
    m_vignetteSprite.setScale(
        static_cast<float>(Config::Window::WIDTH) / 512.0f,
        static_cast<float>(Config::Window::HEIGHT) / 512.0f
    );

    m_damageFlashOverlay.setSize(sf::Vector2f(
        static_cast<float>(Config::Window::WIDTH),
        static_cast<float>(Config::Window::HEIGHT)
    ));
    m_damageFlashOverlay.setFillColor(Config::Darkness::DAMAGE_FLASH_COLOR);

    // 4. Game Over UI
    const sf::Font& font = m_resources.getFont("default");

    m_gameOverOverlay.setSize(sf::Vector2f(
        static_cast<float>(Config::Window::WIDTH),
        static_cast<float>(Config::Window::HEIGHT)
    ));
    m_gameOverOverlay.setFillColor(sf::Color(10, 8, 12, 220));

    m_gameOverBox.setSize(sf::Vector2f(540.0f, 320.0f));
    m_gameOverBox.setOrigin(270.0f, 160.0f);
    m_gameOverBox.setPosition(
        static_cast<float>(Config::Window::WIDTH) * 0.5f,
        static_cast<float>(Config::Window::HEIGHT) * 0.5f
    );
    m_gameOverBox.setFillColor(sf::Color(16, 18, 22, 245));
    m_gameOverBox.setOutlineColor(sf::Color(180, 40, 40, 220));
    m_gameOverBox.setOutlineThickness(2.0f);

    m_gameOverTitleText.setFont(font);
    m_gameOverTitleText.setCharacterSize(42);
    m_gameOverTitleText.setFillColor(sf::Color(230, 45, 45));
    m_gameOverTitleText.setString("GAME OVER");
    sf::FloatRect titleB = m_gameOverTitleText.getLocalBounds();
    m_gameOverTitleText.setOrigin(titleB.width * 0.5f, titleB.height * 0.5f);
    m_gameOverTitleText.setPosition(static_cast<float>(Config::Window::WIDTH) * 0.5f, 240.0f);

    m_gameOverSubtitleText.setFont(font);
    m_gameOverSubtitleText.setCharacterSize(15);
    m_gameOverSubtitleText.setFillColor(sf::Color(160, 170, 180));
    m_gameOverSubtitleText.setString("THE INFECTION CONSUMED YOU.");
    sf::FloatRect subB = m_gameOverSubtitleText.getLocalBounds();
    m_gameOverSubtitleText.setOrigin(subB.width * 0.5f, subB.height * 0.5f);
    m_gameOverSubtitleText.setPosition(static_cast<float>(Config::Window::WIDTH) * 0.5f, 280.0f);

    m_gameOverStatsText.setFont(font);
    m_gameOverStatsText.setCharacterSize(18);
    m_gameOverStatsText.setFillColor(sf::Color(255, 255, 255));

    m_gameOverPromptText.setFont(font);
    m_gameOverPromptText.setCharacterSize(16);
    m_gameOverPromptText.setFillColor(sf::Color(255, 215, 60));
    m_gameOverPromptText.setString("[ENTER] RESTART RUN    |    [ESC] QUIT");
    sf::FloatRect promptB = m_gameOverPromptText.getLocalBounds();
    m_gameOverPromptText.setOrigin(promptB.width * 0.5f, promptB.height * 0.5f);
    m_gameOverPromptText.setPosition(static_cast<float>(Config::Window::WIDTH) * 0.5f, 470.0f);

    std::cout << "[GHOULSLAYER] Stage 4: Wave Manager, Drops, Score, and HUD active.\n";
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

    m_state = GameState::Playing;
    std::cout << "[GHOULSLAYER] Run restarted. Clean state restored.\n";
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
    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_window.close();
        }

        if (m_state == GameState::GameOver) {
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Return) {
                    resetGame();
                } else if (event.key.code == sf::Keyboard::Escape) {
                    m_window.close();
                }
            }
            continue;
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            m_window.close();
        }

        if (event.type == sf::Event::LostFocus) {
            m_isWindowFocused = false;
        }
        if (event.type == sf::Event::GainedFocus) {
            m_isWindowFocused = true;
            m_clock.restart();
        }

#if DEBUG_FEATURES
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F2) {
            spawnDebugZombieWave(5);
            std::cout << "[DEBUG] Spawned 5 zombies.\n";
        }
#endif

        if (m_isWindowFocused && m_player && m_state == GameState::Playing) {
            m_player->handleEvent(event);
        }
    }
}

void Game::update(float dt) {
    switch (m_state) {
        case GameState::MainMenu:
            updateMainMenu(dt);
            break;
        case GameState::Playing:
            updatePlaying(dt);
            break;
        case GameState::Paused:
            updatePaused(dt);
            break;
        case GameState::GameOver:
            updateGameOver(dt);
            break;
    }
}

void Game::updatePlaying(float dt) {
    // 1. Check for player demise
    if (m_player->isDead()) {
        m_state = GameState::GameOver;
        return;
    }

    // 2. Mouse position to world coordinates
    sf::Vector2i mousePixel = sf::Mouse::getPosition(m_window);
    sf::Vector2f worldMousePos = m_window.mapPixelToCoords(mousePixel, m_camera.getView());

    // 3. Update player
    m_player->update(dt, m_map->getObstacles(), worldMousePos);

    // 4. Extract and update active bullets
    auto newBullets = m_player->extractPendingBullets();
    if (!newBullets.empty()) {
        m_bullets.insert(m_bullets.end(), newBullets.begin(), newBullets.end());
    }
    for (auto& bullet : m_bullets) {
        bullet.update(dt, m_map->getObstacles(), m_map->getWidth(), m_map->getHeight());
    }

    // 5. Update blood splatters
    m_bloodSystem.update(dt);

    // 6. Update pickups and process player collection
    for (auto& pickup : m_pickups) {
        pickup.update(dt);

        if (pickup.checkCollection(m_player->getPosition(), m_player->getRadius())) {
            if (pickup.getType() == PickupType::HealthKit) {
                m_player->restoreHealth(Config::Drops::HEALTH_RESTORE);
                m_hud->showPickupMessage("+25 HEALTH", sf::Color(80, 255, 120));
            } else if (pickup.getType() == PickupType::AmmoBox) {
                m_player->addAmmo(Config::Drops::AMMO_BOX_ROUNDS);
                m_hud->showPickupMessage("+AMMO", sf::Color(255, 215, 60));
            } else if (pickup.getType() == PickupType::Coin) {
                m_score += Config::Drops::COIN_SCORE;
                m_hud->showPickupMessage("+" + std::to_string(Config::Drops::COIN_SCORE) + " SCORE", sf::Color(255, 230, 80));
            }
        }
    }

    // Erase collected or expired pickups
    m_pickups.erase(
        std::remove_if(m_pickups.begin(), m_pickups.end(),
            [](const Pickup& p) { return p.isCollected() || p.isExpired(); }),
        m_pickups.end()
    );

    // 7. Combat Resolution: Bullets vs Live Zombies
    for (auto& bullet : m_bullets) {
        if (!bullet.isAlive()) continue;

        for (auto& zombie : m_zombies) {
            if (zombie.isDead()) continue;

            if (CollisionSystem::checkPointCircle(bullet.getPosition(), zombie.getPosition(), zombie.getRadius())) {
                bool wasDead = zombie.isDead();
                zombie.applyDamage(bullet.getDamage(), bullet.isShotgunPellet());
                bullet.kill();
                m_bloodSystem.spawnSplatter(bullet.getPosition(), bullet.getDirection());

                // Trigger kill awards once upon death
                if (!wasDead && zombie.isDead()) {
                    m_kills++;
                    if (zombie.getType() == ZombieType::Walker) {
                        m_score += Config::Score::WALKER_KILL;
                    } else if (zombie.getType() == ZombieType::Runner) {
                        m_score += Config::Score::RUNNER_KILL;
                    } else if (zombie.getType() == ZombieType::Tank) {
                        m_score += Config::Score::TANK_KILL;
                    }

                    // Roll for item drop
                    handleZombieDrop(zombie.getType(), zombie.getPosition());
                }
                break;
            }
        }
    }

    // Safe bullet removal
    m_bullets.erase(
        std::remove_if(m_bullets.begin(), m_bullets.end(),
            [](const Bullet& b) { return !b.isAlive(); }),
        m_bullets.end()
    );

    // 8. Update zombies and check attacks on player
    bool playerAlive = !m_player->isDead();
    sf::Vector2f playerPos = m_player->getPosition();
    float playerRadius = m_player->getRadius();

    for (auto& zombie : m_zombies) {
        zombie.update(dt, playerPos, playerAlive, m_map->getObstacles());

        if (playerAlive && zombie.canAttack(playerPos, playerRadius)) {
            float damage = zombie.performAttack();
            m_player->takeDamage(damage);
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

void Game::updateMainMenu(float /*dt*/) {}
void Game::updatePaused(float /*dt*/) {}
void Game::updateGameOver(float /*dt*/) {}

void Game::render() {
    m_window.clear(sf::Color(14, 14, 18));

    switch (m_state) {
        case GameState::MainMenu:
            renderMainMenu();
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

    m_window.display();
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

    // 2. Screen space rendering (default view)
    m_window.setView(m_window.getDefaultView());

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

    // Full HUD
    m_hud->render(m_window, *m_player, m_waveManager, m_score, m_kills);
}

void Game::renderGameOver() {
    // 1. Render the frozen world in the background
    m_window.setView(m_camera.getView());
    m_map->draw(m_window, m_camera.getView());
    for (const auto& p : m_pickups) p.draw(m_window);
    m_bloodSystem.draw(m_window);
    for (const auto& z : m_zombies) z.draw(m_window);
    m_player->draw(m_window);

    // 2. Screen space Game Over Screen
    m_window.setView(m_window.getDefaultView());
    m_window.draw(m_gameOverOverlay);
    m_window.draw(m_gameOverBox);
    m_window.draw(m_gameOverTitleText);
    m_window.draw(m_gameOverSubtitleText);

    std::string statsStr = "FINAL SCORE:    " + std::to_string(m_score) + "\n\n";
    statsStr += "WAVES REACHED:  " + std::to_string(m_waveManager.getCurrentWave()) + "\n\n";
    statsStr += "TOTAL KILLS:    " + std::to_string(m_kills);

    m_gameOverStatsText.setString(statsStr);
    sf::FloatRect statsB = m_gameOverStatsText.getLocalBounds();
    m_gameOverStatsText.setOrigin(statsB.width * 0.5f, statsB.height * 0.5f);
    m_gameOverStatsText.setPosition(static_cast<float>(Config::Window::WIDTH) * 0.5f, 360.0f);
    m_window.draw(m_gameOverStatsText);

    m_window.draw(m_gameOverPromptText);
}

void Game::renderMainMenu() {}
void Game::renderPaused() {}
