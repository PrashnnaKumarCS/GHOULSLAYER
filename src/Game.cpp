#include "Game.hpp"
#include "PlaceholderArt.hpp"
#include "CollisionSystem.hpp"
#include "Config.hpp"
#include <iostream>
#include <random>
#include <cmath>
#include <algorithm>

namespace {
    std::mt19937& getGameRandomEngine() {
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
{
    m_window.setFramerateLimit(Config::Window::FPS_LIMIT);
    m_window.setVerticalSyncEnabled(Config::Window::VSYNC_ENABLED);
    m_window.requestFocus();

    // 1. Generate procedural pixel art textures into ResourceManager
    PlaceholderArt::generateAll(m_resources);

    // 2. Initialize world and player
    m_map = std::make_unique<Map>(m_resources);
    m_player = std::make_unique<Player>(m_resources);

    // 3. Setup camera centered on player
    m_camera.initialize(
        m_player->getPosition(),
        sf::Vector2f(static_cast<float>(Config::Window::WIDTH), static_cast<float>(Config::Window::HEIGHT))
    );

    // 4. Setup atmospheric dark overlay and vignette
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

    // Damage flash overlay (red border flash on player hurt)
    m_damageFlashOverlay.setSize(sf::Vector2f(
        static_cast<float>(Config::Window::WIDTH),
        static_cast<float>(Config::Window::HEIGHT)
    ));
    m_damageFlashOverlay.setFillColor(Config::Darkness::DAMAGE_FLASH_COLOR);

    // 5. Setup temporary debug HUD for Stage 3
    const sf::Font& hudFont = m_resources.getFont("default");

    m_debugBox.setSize(sf::Vector2f(340.0f, 135.0f));
    m_debugBox.setPosition(20.0f, 20.0f);
    m_debugBox.setFillColor(sf::Color(10, 12, 16, 210));
    m_debugBox.setOutlineColor(sf::Color(80, 100, 120, 180));
    m_debugBox.setOutlineThickness(1.5f);

    m_debugText.setFont(hudFont);
    m_debugText.setCharacterSize(15);
    m_debugText.setFillColor(sf::Color(240, 240, 245));
    m_debugText.setPosition(30.0f, 26.0f);

    m_hintText.setFont(hudFont);
    m_hintText.setCharacterSize(13);
    m_hintText.setFillColor(sf::Color(160, 180, 200));
    m_hintText.setPosition(20.0f, static_cast<float>(Config::Window::HEIGHT) - 30.0f);
    m_hintText.setString("[1,2,3/Wheel: Weapons | R: Reload | F1: Unlock Weapons | F2: Spawn 5 Zombies]");

    std::cout << "[GHOULSLAYER] Stage 3: Zombies, AI, Combat & Blood Active.\n";
}

void Game::spawnZombieWave(int count) {
    if (!m_player) return;

    sf::Vector2f playerPos = m_player->getPosition();
    std::uniform_real_distribution<float> angleDist(0.0f, 6.2831853f);
    std::uniform_real_distribution<float> distDist(650.0f, 950.0f);
    std::uniform_int_distribution<int> typeRoll(0, 9);

    const auto& obstacles = m_map->getObstacles();

    for (int i = 0; i < count; ++i) {
        sf::Vector2f spawnPos;
        bool validSpotFound = false;

        // Try up to 25 times to find a spawn position free of obstacles
        for (int attempt = 0; attempt < 25; ++attempt) {
            float angle = angleDist(getGameRandomEngine());
            float distance = distDist(getGameRandomEngine());
            sf::Vector2f candidate(
                playerPos.x + std::cos(angle) * distance,
                playerPos.y + std::sin(angle) * distance
            );

            // Clamp safely inside map edges
            if (candidate.x < 150.0f || candidate.x > Config::Map::WIDTH - 150.0f ||
                candidate.y < 150.0f || candidate.y > Config::Map::HEIGHT - 150.0f) {
                continue;
            }

            // Test candidate position against all obstacles
            bool insideObstacle = false;
            for (const auto& obs : obstacles) {
                if (CollisionSystem::checkRectCircle(obs, candidate, 32.0f)) {
                    insideObstacle = true;
                    break;
                }
            }

            if (!insideObstacle) {
                spawnPos = candidate;
                validSpotFound = true;
                break;
            }
        }

        if (validSpotFound) {
            // Type distribution: 50% Walker (0-4), 30% Runner (5-7), 20% Tank (8-9)
            int roll = typeRoll(getGameRandomEngine());
            ZombieType zType = ZombieType::Walker;
            if (roll >= 5 && roll <= 7) {
                zType = ZombieType::Runner;
            } else if (roll >= 8) {
                zType = ZombieType::Tank;
            }

            m_zombies.emplace_back(zType, spawnPos, m_resources);
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

    std::cout << "[GHOULSLAYER] Exited cleanly.\n";
}

void Game::handleEvents() {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_window.close();
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
        // DEBUG: Press F2 to spawn 5 random zombies in a ring outside screen
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F2) {
            spawnZombieWave(5);
            std::cout << "[DEBUG] Spawned 5 zombies.\n";
        }
#endif

        // Forward input events to player (weapon selection, reload, debug unlock, mouse clicks)
        if (m_isWindowFocused && m_player) {
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
    // 1. Mouse to world coordinates
    sf::Vector2i mousePixel = sf::Mouse::getPosition(m_window);
    sf::Vector2f worldMousePos = m_window.mapPixelToCoords(mousePixel, m_camera.getView());

    // 2. Update player
    m_player->update(dt, m_map->getObstacles(), worldMousePos);

    // 3. Collect newly discharged bullets from player
    auto newBullets = m_player->extractPendingBullets();
    if (!newBullets.empty()) {
        m_bullets.insert(m_bullets.end(), newBullets.begin(), newBullets.end());
    }

    // 4. Update all active bullets
    for (auto& bullet : m_bullets) {
        bullet.update(dt, m_map->getObstacles(), m_map->getWidth(), m_map->getHeight());
    }

    // 5. Update blood splatters
    m_bloodSystem.update(dt);

    // 6. Combat Collision: Bullets vs Live Zombies
    for (auto& bullet : m_bullets) {
        if (!bullet.isAlive()) continue;

        for (auto& zombie : m_zombies) {
            if (zombie.isDead()) continue; // Dead zombies do not block bullets

            if (CollisionSystem::checkPointCircle(bullet.getPosition(), zombie.getPosition(), zombie.getRadius())) {
                zombie.applyDamage(bullet.getDamage(), bullet.isShotgunPellet());
                bullet.kill();
                m_bloodSystem.spawnSplatter(bullet.getPosition(), bullet.getDirection());
                break; // Bullet hits only one zombie
            }
        }
    }

    // 7. Erase dead bullets safely
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

        // Attack player if in range
        if (playerAlive && zombie.canAttack(playerPos, playerRadius)) {
            float damage = zombie.performAttack();
            m_player->takeDamage(damage);
        }
    }

    // 9. Zombie-to-zombie soft push-apart separation (prevents stacking into one blob)
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

    // 10. Erase dead zombies whose fade animation has completed
    m_zombies.erase(
        std::remove_if(m_zombies.begin(), m_zombies.end(),
            [](const Zombie& z) { return z.isReadyForRemoval(); }),
        m_zombies.end()
    );

    // 11. Smoothly track player with camera
    m_camera.update(m_player->getPosition(), dt);

    // 12. Update debug HUD text with Player Health & Zombie Count
    const Weapon& weapon = m_player->getCurrentWeapon();
    int liveZombies = 0;
    for (const auto& z : m_zombies) {
        if (!z.isDead()) liveZombies++;
    }

    std::string hudString = "WEAPON: " + weapon.getName() + "\n";
    hudString += "AMMO:   " + std::to_string(weapon.getMagazine()) + " / " + std::to_string(weapon.getReserve()) + "\n";

    if (m_player->isDead()) {
        hudString += "STATUS: *** PLAYER DEAD ***\n";
    } else if (weapon.isReloading()) {
        int progressPct = static_cast<int>(weapon.getReloadProgress() * 100.0f);
        hudString += "STATUS: RELOADING... (" + std::to_string(progressPct) + "%)\n";
    } else if (weapon.getMagazine() == 0) {
        hudString += (weapon.getReserve() > 0) ? "STATUS: EMPTY! (PRESS R)\n" : "STATUS: NO AMMO!\n";
    } else {
        hudString += "STATUS: READY\n";
    }

    if (m_player->isDead()) {
        hudString += "HEALTH: 0 / 100 (DEAD)\n";
    } else {
        hudString += "HEALTH: " + std::to_string(static_cast<int>(m_player->getHealth())) + " / 100\n";
    }
    hudString += "ZOMBIES ALIVE: " + std::to_string(liveZombies);

    m_debugText.setString(hudString);
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

    // Map tiles and static obstacles
    m_map->draw(m_window, m_camera.getView());

    // Blood splatters
    m_bloodSystem.draw(m_window);

    // Bullets
    for (const auto& bullet : m_bullets) {
        bullet.draw(m_window);
    }

    // Zombies (with viewport frustum culling)
    sf::Vector2f camCenter = m_camera.getCenter();
    sf::FloatRect viewBounds(
        camCenter.x - Config::Window::WIDTH * 0.5f - 80.0f,
        camCenter.y - Config::Window::HEIGHT * 0.5f - 80.0f,
        Config::Window::WIDTH + 160.0f,
        Config::Window::HEIGHT + 160.0f
    );

    for (const auto& zombie : m_zombies) {
        if (viewBounds.contains(zombie.getPosition())) {
            zombie.draw(m_window);
        }
    }

    // Player
    m_player->draw(m_window);

    // 2. Screen space rendering (default view)
    m_window.setView(m_window.getDefaultView());

    // Darkness & vignette
    m_window.draw(m_darkOverlay);
    m_window.draw(m_vignetteSprite);

    // Red screen-edge damage flash on player hurt
    if (m_player->getDamageFlashTimer() > 0.0f) {
        float flashPct = m_player->getDamageFlashTimer() / Config::Player::DAMAGE_FLASH_DURATION;
        sf::Color flashColor = Config::Darkness::DAMAGE_FLASH_COLOR;
        flashColor.a = static_cast<sf::Uint8>(flashPct * 140.0f);
        m_damageFlashOverlay.setFillColor(flashColor);
        m_window.draw(m_damageFlashOverlay);
    }

    // Debug HUD
    m_window.draw(m_debugBox);
    m_window.draw(m_debugText);
    m_window.draw(m_hintText);
}

void Game::renderMainMenu() {}
void Game::renderPaused() {}
void Game::renderGameOver() {}
