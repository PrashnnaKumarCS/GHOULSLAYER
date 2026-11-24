#pragma once

#include <SFML/Graphics.hpp>
#include <string>

// Reusable interactive UI button supporting mouse hover, mouse clicks,
// and keyboard navigation (Up/Down selection + Enter/Space activation).
class Button {
public:
    Button(const std::string& label, sf::Vector2f position, sf::Vector2f size, const sf::Font& font, unsigned int charSize = 14);

    // Update hover state based on mouse cursor position
    void update(sf::Vector2f mousePos);

    // Handles mouse and keyboard events. Returns true when activated (clicked or Enter pressed while selected).
    bool handleEvent(const sf::Event& event, sf::Vector2f mousePos);

    // Render button to screen
    void draw(sf::RenderTarget& target) const;

    // Selection management for keyboard navigation
    void setSelected(bool selected);
    bool isSelected() const { return m_isSelected; }
    bool isHovered() const { return m_isHovered; }

    // Position and label modifiers
    void setPosition(sf::Vector2f position);
    void setLabel(const std::string& label);
    sf::FloatRect getBounds() const;

private:
    void updateVisuals();

    sf::RectangleShape m_box;
    sf::Text m_text;
    bool m_isHovered;
    bool m_isSelected;
};
