#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Button.hpp"

class ResourceManager;

// PauseMenu provides an in-game freeze overlay displaying options to
// Resume current play, Restart cleanly from wave 1, or return to Main Menu.
class PauseMenu {
public:
    enum class Action {
        None,
        Resume,
        Restart,
        MainMenu
    };

    explicit PauseMenu(const ResourceManager& resources);

    void update(sf::Vector2f mousePos);

    // Handles keyboard navigation (Up/Down + Enter) and mouse clicks
    Action handleEvent(const sf::Event& event, sf::Vector2f mousePos);

    void draw(sf::RenderTarget& target) const;

    // Resets keyboard selection when opening menu
    void resetSelection();

private:
    const sf::Font& m_font;

    // Dim backdrop overlay and menu panel
    sf::RectangleShape m_darkOverlay;
    sf::RectangleShape m_panel;
    sf::Text m_titleText;
    sf::Text m_hintText;

    // Interactive buttons
    std::vector<Button> m_buttons;
    int m_selectedIndex;
};
