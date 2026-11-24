#include "PauseMenu.hpp"
#include "ResourceManager.hpp"
#include "Config.hpp"

PauseMenu::PauseMenu(const ResourceManager& resources)
    : m_font(resources.getFont("default"))
    , m_selectedIndex(0)
{
    float W = static_cast<float>(Config::Window::WIDTH);
    float H = static_cast<float>(Config::Window::HEIGHT);

    // Fullscreen dim overlay over the frozen game
    m_darkOverlay.setSize(sf::Vector2f(W, H));
    m_darkOverlay.setFillColor(sf::Color(10, 12, 16, 215));

    // Centered panel frame
    sf::Vector2f panelSize(400.0f, 380.0f);
    m_panel.setSize(panelSize);
    m_panel.setOrigin(panelSize.x * 0.5f, panelSize.y * 0.5f);
    m_panel.setPosition(W * 0.5f, H * 0.5f);
    m_panel.setFillColor(sf::Color(18, 22, 30, 240));
    m_panel.setOutlineThickness(2.0f);
    m_panel.setOutlineColor(sf::Color(220, 50, 50, 180)); // Crimson border

    // Title: PAUSED
    m_titleText.setFont(m_font);
    m_titleText.setCharacterSize(34);
    m_titleText.setString("PAUSED");
    m_titleText.setFillColor(sf::Color(255, 215, 60)); // Gold
    sf::FloatRect tb = m_titleText.getLocalBounds();
    m_titleText.setOrigin(tb.left + tb.width * 0.5f, tb.top + tb.height * 0.5f);
    m_titleText.setPosition(W * 0.5f, H * 0.5f - 130.0f);

    // Hint: ESC to resume
    m_hintText.setFont(m_font);
    m_hintText.setCharacterSize(12);
    m_hintText.setString("PRESS ESC TO RESUME");
    m_hintText.setFillColor(sf::Color(160, 175, 190));
    sf::FloatRect hb = m_hintText.getLocalBounds();
    m_hintText.setOrigin(hb.left + hb.width * 0.5f, hb.top + hb.height * 0.5f);
    m_hintText.setPosition(W * 0.5f, H * 0.5f - 85.0f);

    // Buttons
    sf::Vector2f btnSize(260.0f, 44.0f);
    float startY = H * 0.5f - 20.0f;
    float spacing = 58.0f;

    m_buttons.emplace_back("RESUME", sf::Vector2f(W * 0.5f, startY + spacing * 0), btnSize, m_font, 15);
    m_buttons.emplace_back("RESTART", sf::Vector2f(W * 0.5f, startY + spacing * 1), btnSize, m_font, 15);
    m_buttons.emplace_back("MAIN MENU", sf::Vector2f(W * 0.5f, startY + spacing * 2), btnSize, m_font, 15);

    resetSelection();
}

void PauseMenu::resetSelection() {
    m_selectedIndex = 0;
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        m_buttons[i].setSelected(m_selectedIndex == static_cast<int>(i));
    }
}

void PauseMenu::update(sf::Vector2f mousePos) {
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        m_buttons[i].update(mousePos);
        if (m_buttons[i].isHovered()) {
            m_selectedIndex = static_cast<int>(i);
        }
        m_buttons[i].setSelected(m_selectedIndex == static_cast<int>(i));
    }
}

PauseMenu::Action PauseMenu::handleEvent(const sf::Event& event, sf::Vector2f mousePos) {
    // Pressing ESC inside pause menu also resumes the game
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
        return Action::Resume;
    }

    // Keyboard navigation (Up/Down arrows or W/S)
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
            m_selectedIndex = (m_selectedIndex - 1 + static_cast<int>(m_buttons.size())) % static_cast<int>(m_buttons.size());
        } else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
            m_selectedIndex = (m_selectedIndex + 1) % static_cast<int>(m_buttons.size());
        }
    }

    for (size_t i = 0; i < m_buttons.size(); ++i) {
        m_buttons[i].setSelected(m_selectedIndex == static_cast<int>(i));
    }

    // Check button click or activation
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        if (m_buttons[i].handleEvent(event, mousePos)) {
            if (i == 0) return Action::Resume;
            if (i == 1) return Action::Restart;
            if (i == 2) return Action::MainMenu;
        }
    }

    return Action::None;
}

void PauseMenu::draw(sf::RenderTarget& target) const {
    target.draw(m_darkOverlay);
    target.draw(m_panel);
    target.draw(m_titleText);
    target.draw(m_hintText);

    for (const auto& btn : m_buttons) {
        btn.draw(target);
    }
}
