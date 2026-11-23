#include "Game.hpp"
#include "PlaceholderArt.hpp"
#include "Config.hpp"
#include <iostream>
#include <algorithm>

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

    // 5. Setup temporary debug HUD for Stage 2
    const sf::Font& hudFont = m_resources.getFont("default");

    m_debugBox.setSize(sf::Vector2f(340.0f, 105.0f));
    m_debugBox.setPosition(20.0f, 20.0f);
    m_debugBox.setFillColor(sf::Color(10, 12, 16, 210));
    m_debugBox.setOutlineColor(sf::Color(80, 100, 120, 180));
    m_debugBox.setOutlineThickness(1.5f);

    m_debugText.setFont(hudFont);
    m_debugText.setCharacterSize(16);
    m_debugText.setFillColor(sf::Color(240, 240, 245));
    m_debugText.setPosition(30.0f, 26.0f);

    m_hintText.setFont(hudFont);
    m_hintText.setCharacterSize(13);
    m_hintText.setFillColor(sf::Color(160, 180, 200));
    m_hintText.setPosition(20.0f, static_cast<float>(Config::Window::HEIGHT) - 30.0f);
    m_hintText.setString("[1,2,3 or Wheel: Switch Weapon | R: Reload | F1: Unlock All (Debug)]");

    std::cout << "[GHOULSLAYER] Stage 2: Weapons, Bullets, Ammo, and Reloading active.\n";
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

    // 4. Update all active bullets and check collisions
    for (auto& bullet : m_bullets) {
        bullet.update(dt, m_map->getObstacles(), m_map->getWidth(), m_map->getHeight());
    }

    // 5. Erase dead bullets safely using erase-remove idiom
    m_bullets.erase(
        std::remove_if(m_bullets.begin(), m_bullets.end(),
            [](const Bullet& b) { return !b.isAlive(); }),
        m_bullets.end()
    );

    // 6. Smoothly track player with camera
    m_camera.update(m_player->getPosition(), dt);

    // 7. Update debug HUD text
    const Weapon& weapon = m_player->getCurrentWeapon();
    std::string hudString = "WEAPON: " + weapon.getName() + "\n";
    hudString += "AMMO:   " + std::to_string(weapon.getMagazine()) + " / " + std::to_string(weapon.getReserve()) + "\n";

    if (weapon.isReloading()) {
        int progressPct = static_cast<int>(weapon.getReloadProgress() * 100.0f);
        hudString += "STATUS: RELOADING... (" + std::to_string(progressPct) + "%)\n";
    } else if (weapon.getMagazine() == 0) {
        if (weapon.getReserve() > 0) {
            hudString += "STATUS: EMPTY! (PRESS R TO RELOAD)\n";
        } else {
            hudString += "STATUS: NO AMMO!\n";
        }
    } else {
        hudString += "STATUS: READY\n";
    }

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

    m_map->draw(m_window, m_camera.getView());

    for (const auto& bullet : m_bullets) {
        bullet.draw(m_window);
    }

    m_player->draw(m_window);

    // 2. Screen space rendering (default view)
    m_window.setView(m_window.getDefaultView());

    // Dark atmosphere and vignette
    m_window.draw(m_darkOverlay);
    m_window.draw(m_vignetteSprite);

    // Debug ammo and controls HUD
    m_window.draw(m_debugBox);
    m_window.draw(m_debugText);
    m_window.draw(m_hintText);
}

void Game::renderMainMenu() {}
void Game::renderPaused() {}
void Game::renderGameOver() {}
