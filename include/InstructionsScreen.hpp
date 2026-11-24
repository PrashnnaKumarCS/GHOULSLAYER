#pragma once

#include <SFML/Graphics.hpp>
#include "Button.hpp"

class ResourceManager;

// InstructionsScreen displays full controls, zombie codex, and pickup guide,
// with an interactive Back button to return to the Main Menu.
class InstructionsScreen {
public:
    explicit InstructionsScreen(const ResourceManager& resources);

    void update(sf::Vector2f mousePos);

    // Returns true when the Back button is triggered
    bool handleEvent(const sf::Event& event, sf::Vector2f mousePos);

    void draw(sf::RenderTarget& target) const;

private:
    const sf::Font& m_font;

    sf::RectangleShape m_darkOverlay;
    sf::RectangleShape m_contentPanel;
    sf::Text m_titleText;

    // Sections
    sf::Text m_controlsTitle;
    sf::Text m_controlsText;

    sf::Text m_zombiesTitle;
    sf::Text m_zombiesText;

    sf::Text m_pickupsTitle;
    sf::Text m_pickupsText;

    Button m_backButton;
};
