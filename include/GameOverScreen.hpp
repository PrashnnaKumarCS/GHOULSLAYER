#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Button.hpp"

class ResourceManager;

// GameOverScreen displays post-death statistics, high-score tracking persisted
// in "highscore.dat", and interactive buttons to Restart or return to the Main Menu.
class GameOverScreen {
public:
    enum class Action {
        None,
        Restart,
        MainMenu
    };

    explicit GameOverScreen(const ResourceManager& resources);

    // Configures final gameplay results and checks/updates high score persistence
    void setFinalStats(int score, int wave, int kills);

    void update(float dt, sf::Vector2f mousePos);

    // Handles keyboard navigation (Up/Down + Enter) and mouse clicks
    Action handleEvent(const sf::Event& event, sf::Vector2f mousePos);

    void draw(sf::RenderTarget& target) const;

    int getHighScore() const { return m_highScore; }

private:
    int loadHighScore();
    void saveHighScore(int score);
    void updateStatsText();

    const sf::Font& m_font;

    // Visual background & panel
    sf::RectangleShape m_darkOverlay;
    sf::RectangleShape m_panel;
    sf::Text m_titleShadow;
    sf::Text m_titleText;
    sf::Text m_newHighScoreText;
    sf::Text m_statsText;

    // Interactive buttons
    std::vector<Button> m_buttons;
    int m_selectedIndex;

    // Stats
    int m_finalScore;
    int m_finalWave;
    int m_finalKills;
    int m_highScore;
    bool m_isNewHighScore;

    // Animation / pulse timer for new high score
    float m_pulseTimer;
};
