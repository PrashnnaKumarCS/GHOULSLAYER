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
    // Apply window configuration: 60 FPS cap, VSync, and initial keyboard focus
    m_window.setFramerateLimit(Config::Window::FPS_LIMIT);
    m_window.setVerticalSyncEnabled(Config::Window::VSYNC_ENABLED);
    m_window.requestFocus();

    // 1. Generate all procedural pixel art assets into ResourceManager
    PlaceholderArt::generateAll(m_resources);

    // 2. Initialize the game world and player
    m_map = std::make_unique<Map>(m_resources);
    m_player = std::make_unique<Player>(m_resources);

    // 3. Initialize camera centered on the player
    m_camera.initialize(
        m_player->getPosition(),
        sf::Vector2f(static_cast<float>(Config::Window::WIDTH), static_cast<float>(Config::Window::HEIGHT))
    );

    // 4. Setup dark atmospheric overlay and soft vignette in screen-space
    m_darkOverlay.setSize(sf::Vector2f(
        static_cast<float>(Config::Window::WIDTH),
        static_cast<float>(Config::Window::HEIGHT)
    ));
    m_darkOverlay.setFillColor(Config::Darkness::OVERLAY_COLOR);

    const sf::Texture& vignetteTex = m_resources.getTexture("vignette");
    m_vignetteSprite.setTexture(vignetteTex);
    // Scale 512x512 vignette to fit screen resolution
    m_vignetteSprite.setScale(
        static_cast<float>(Config::Window::WIDTH) / 512.0f,
        static_cast<float>(Config::Window::HEIGHT) / 512.0f
    );

    std::cout << "[GHOULSLAYER] Engine Core and Systems initialized successfully.\n";
}

void Game::run() {
    while (m_window.isOpen()) {
        handleEvents();

        // Calculate delta time in seconds
        float dt = m_clock.restart().asSeconds();

        // Clamp delta time to maximum threshold to prevent physics tunneling
        // during window moves, loading delays, or OS scheduling hitches
        if (dt > Config::Engine::MAX_DELTA_TIME) {
            dt = Config::Engine::MAX_DELTA_TIME;
        }

        // Only update world simulation when the window has focus
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
        // Window close button (X)
        if (event.type == sf::Event::Closed) {
            m_window.close();
        }

        // ESC key closes the game
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            m_window.close();
        }

        // Window focus handling
        if (event.type == sf::Event::LostFocus) {
            m_isWindowFocused = false;
        }
        if (event.type == sf::Event::GainedFocus) {
            m_isWindowFocused = true;
            // Reset clock so the pause duration doesn't cause a large delta spike on resume
            m_clock.restart();
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
    // 1. Get mouse position on screen and convert to world coordinates through camera view
    sf::Vector2i mousePixel = sf::Mouse::getPosition(m_window);
    sf::Vector2f worldMousePos = m_window.mapPixelToCoords(mousePixel, m_camera.getView());

    // 2. Update player with inputs, collision boxes, and mouse aiming
    m_player->update(dt, m_map->getObstacles(), worldMousePos);

    // 3. Smoothly update camera to follow player position
    m_camera.update(m_player->getPosition(), dt);
}

// Stubs for upcoming stages
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
    // 1. World-space rendering using the smoothly interpolated camera view
    m_window.setView(m_camera.getView());
    m_map->draw(m_window, m_camera.getView());
    m_player->draw(m_window);

    // 2. Screen-space rendering using the default view (HUD and darkness overlay)
    m_window.setView(m_window.getDefaultView());
    m_window.draw(m_darkOverlay);
    m_window.draw(m_vignetteSprite);
}

// Stubs for upcoming stages
void Game::renderMainMenu() {}
void Game::renderPaused() {}
void Game::renderGameOver() {}
