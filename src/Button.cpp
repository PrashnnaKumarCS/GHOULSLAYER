#include "Button.hpp"

Button::Button(const std::string& label, sf::Vector2f position, sf::Vector2f size, const sf::Font& font, unsigned int charSize)
    : m_isHovered(false)
    , m_isSelected(false)
{
    m_box.setSize(size);
    m_box.setOrigin(size.x * 0.5f, size.y * 0.5f);
    m_box.setPosition(position);
    m_box.setOutlineThickness(1.5f);

    m_text.setFont(font);
    m_text.setCharacterSize(charSize);
    m_text.setString(label);

    updateVisuals();
}

void Button::setPosition(sf::Vector2f position) {
    m_box.setPosition(position);
    updateVisuals();
}

void Button::setLabel(const std::string& label) {
    m_text.setString(label);
    updateVisuals();
}

sf::FloatRect Button::getBounds() const {
    return m_box.getGlobalBounds();
}

void Button::setSelected(bool selected) {
    m_isSelected = selected;
    updateVisuals();
}

void Button::update(sf::Vector2f mousePos) {
    bool hoveredNow = getBounds().contains(mousePos);
    if (hoveredNow != m_isHovered) {
        m_isHovered = hoveredNow;
        updateVisuals();
    }
}

bool Button::handleEvent(const sf::Event& event, sf::Vector2f mousePos) {
    update(mousePos);

    // Mouse click activation
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left && m_isHovered) {
            return true;
        }
    }

    // Keyboard activation (Enter or Space when selected)
    if (event.type == sf::Event::KeyPressed && m_isSelected) {
        if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space) {
            return true;
        }
    }

    return false;
}

void Button::updateVisuals() {
    // Center text inside box
    sf::FloatRect textBounds = m_text.getLocalBounds();
    m_text.setOrigin(
        textBounds.left + textBounds.width * 0.5f,
        textBounds.top + textBounds.height * 0.5f
    );
    m_text.setPosition(m_box.getPosition());

    if (m_isHovered || m_isSelected) {
        m_box.setFillColor(sf::Color(45, 52, 68, 240));
        m_box.setOutlineColor(sf::Color(255, 215, 60)); // Gold outline on hover/select
        m_text.setFillColor(sf::Color(255, 255, 255));
    } else {
        m_box.setFillColor(sf::Color(16, 18, 24, 220));
        m_box.setOutlineColor(sf::Color(75, 90, 110, 180));
        m_text.setFillColor(sf::Color(200, 210, 220));
    }
}

void Button::draw(sf::RenderTarget& target) const {
    target.draw(m_box);
    target.draw(m_text);
}
