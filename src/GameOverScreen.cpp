#include "GameOverScreen.hpp"
#include "ResourceManager.hpp"
#include "Config.hpp"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

namespace {
    constexpr const char* HIGHSCORE_FILE = "highscore.dat";
}

GameOverScreen::GameOverScreen(const ResourceManager& resources)
    : m_font(resources.getFont("default"))
    , m_selectedIndex(0)
    , m_finalScore(0)
    , m_finalWave(1)
    , m_finalKills(0)
    , m_highScore(0)
    , m_isNewHighScore(false)
    , m_pulseTimer(0.0f)
{
    float W = static_cast<float>(Config::Window::WIDTH);
    float H = static_cast<float>(Config::Window::HEIGHT);

    // Initial load of previous high score
    m_highScore = loadHighScore();

    // Dark reddish overlay
    m_darkOverlay.setSize(sf::Vector2f(W, H));
    m_darkOverlay.setFillColor(sf::Color(14, 8, 10, 230));

    // Panel
    sf::Vector2f panelSize(560.0f, 500.0f);
    m_panel.setSize(panelSize);
    m_panel.setOrigin(panelSize.x * 0.5f, panelSize.y * 0.5f);
    m_panel.setPosition(W * 0.5f, H * 0.5f);
    m_panel.setFillColor(sf::Color(22, 14, 18, 245));
    m_panel.setOutlineThickness(2.0f);
    m_panel.setOutlineColor(sf::Color(180, 30, 30, 200));

    // Blood-red Title: YOU DIED
    m_titleShadow.setFont(m_font);
    m_titleShadow.setCharacterSize(48);
    m_titleShadow.setString("YOU DIED");
    m_titleShadow.setFillColor(sf::Color(30, 0, 0, 220));
    sf::FloatRect sb = m_titleShadow.getLocalBounds();
    m_titleShadow.setOrigin(sb.left + sb.width * 0.5f, sb.top + sb.height * 0.5f);
    m_titleShadow.setPosition(W * 0.5f + 3.0f, H * 0.5f - 180.0f + 3.0f);

    m_titleText.setFont(m_font);
    m_titleText.setCharacterSize(48);
    m_titleText.setString("YOU DIED");
    m_titleText.setFillColor(sf::Color(220, 28, 28)); // Blood Red
    m_titleText.setOrigin(sb.left + sb.width * 0.5f, sb.top + sb.height * 0.5f);
    m_titleText.setPosition(W * 0.5f, H * 0.5f - 180.0f);

    // New High Score Banner
    m_newHighScoreText.setFont(m_font);
    m_newHighScoreText.setCharacterSize(15);
    m_newHighScoreText.setString("*** NEW HIGH SCORE! ***");
    m_newHighScoreText.setFillColor(sf::Color(255, 215, 60)); // Gold
    sf::FloatRect nb = m_newHighScoreText.getLocalBounds();
    m_newHighScoreText.setOrigin(nb.left + nb.width * 0.5f, nb.top + nb.height * 0.5f);
    m_newHighScoreText.setPosition(W * 0.5f, H * 0.5f - 125.0f);

    // Stats block
    m_statsText.setFont(m_font);
    m_statsText.setCharacterSize(14);
    m_statsText.setFillColor(sf::Color(220, 225, 235));
    m_statsText.setLineSpacing(1.7f);

    // Buttons
    sf::Vector2f btnSize(240.0f, 44.0f);
    float btnY = H * 0.5f + 160.0f;
    m_buttons.emplace_back("RESTART", sf::Vector2f(W * 0.5f - 135.0f, btnY), btnSize, m_font, 15);
    m_buttons.emplace_back("MAIN MENU", sf::Vector2f(W * 0.5f + 135.0f, btnY), btnSize, m_font, 15);

    m_buttons[0].setSelected(true);
}

int GameOverScreen::loadHighScore() {
    std::ifstream file(HIGHSCORE_FILE);
    if (!file.is_open()) {
        return 0; // File does not exist yet
    }

    int score = 0;
    if (file >> score) {
        return std::max(0, score);
    }
    return 0; // Fallback if file is corrupted or empty
}

void GameOverScreen::saveHighScore(int score) {
    std::ofstream file(HIGHSCORE_FILE, std::ios::trunc);
    if (file.is_open()) {
        file << score << "\n";
    }
}

void GameOverScreen::setFinalStats(int score, int wave, int kills) {
    m_finalScore = score;
    m_finalWave = wave;
    m_finalKills = kills;

    int oldHigh = loadHighScore();
    if (score > oldHigh) {
        m_isNewHighScore = true;
        m_highScore = score;
        saveHighScore(score);
    } else {
        m_isNewHighScore = false;
        m_highScore = oldHigh;
    }

    m_selectedIndex = 0;
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        m_buttons[i].setSelected(m_selectedIndex == static_cast<int>(i));
    }

    updateStatsText();
}

void GameOverScreen::updateStatsText() {
    std::ostringstream ss;
    ss << "FINAL SCORE:       " << m_finalScore << "\n"
       << "WAVES SURVIVED:    " << m_finalWave << "\n"
       << "ZOMBIES SLAIN:     " << m_finalKills << "\n"
       << "ALL-TIME BEST:     " << m_highScore;

    m_statsText.setString(ss.str());

    sf::FloatRect sb = m_statsText.getLocalBounds();
    m_statsText.setOrigin(sb.left + sb.width * 0.5f, sb.top + sb.height * 0.5f);

    float W = static_cast<float>(Config::Window::WIDTH);
    float H = static_cast<float>(Config::Window::HEIGHT);
    m_statsText.setPosition(W * 0.5f, H * 0.5f + 15.0f);
}

void GameOverScreen::update(float dt, sf::Vector2f mousePos) {
    m_pulseTimer += dt * 4.0f;

    // Pulse new high score notice if active
    if (m_isNewHighScore) {
        float alpha = 180.0f + 75.0f * std::sin(m_pulseTimer);
        m_newHighScoreText.setFillColor(sf::Color(255, 215, 60, static_cast<sf::Uint8>(std::clamp(alpha, 0.0f, 255.0f))));
    }

    for (size_t i = 0; i < m_buttons.size(); ++i) {
        m_buttons[i].update(mousePos);
        if (m_buttons[i].isHovered()) {
            m_selectedIndex = static_cast<int>(i);
        }
        m_buttons[i].setSelected(m_selectedIndex == static_cast<int>(i));
    }
}

GameOverScreen::Action GameOverScreen::handleEvent(const sf::Event& event, sf::Vector2f mousePos) {
    // Keyboard navigation (Left/Right or Up/Down)
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Up ||
            event.key.code == sf::Keyboard::A || event.key.code == sf::Keyboard::W) {
            m_selectedIndex = (m_selectedIndex - 1 + static_cast<int>(m_buttons.size())) % static_cast<int>(m_buttons.size());
        } else if (event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::Down ||
                   event.key.code == sf::Keyboard::D || event.key.code == sf::Keyboard::S) {
            m_selectedIndex = (m_selectedIndex + 1) % static_cast<int>(m_buttons.size());
        }
    }

    for (size_t i = 0; i < m_buttons.size(); ++i) {
        m_buttons[i].setSelected(m_selectedIndex == static_cast<int>(i));
    }

    // Button activation
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        if (m_buttons[i].handleEvent(event, mousePos)) {
            if (i == 0) return Action::Restart;
            if (i == 1) return Action::MainMenu;
        }
    }

    return Action::None;
}

void GameOverScreen::draw(sf::RenderTarget& target) const {
    target.draw(m_darkOverlay);
    target.draw(m_panel);
    target.draw(m_titleShadow);
    target.draw(m_titleText);

    if (m_isNewHighScore) {
        target.draw(m_newHighScoreText);
    }

    target.draw(m_statsText);

    for (const auto& btn : m_buttons) {
        btn.draw(target);
    }
}
