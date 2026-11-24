#include "InstructionsScreen.hpp"
#include "ResourceManager.hpp"
#include "Config.hpp"

InstructionsScreen::InstructionsScreen(const ResourceManager& resources)
    : m_font(resources.getFont("default"))
    , m_backButton(
        "BACK",
        sf::Vector2f(static_cast<float>(Config::Window::WIDTH) * 0.5f, static_cast<float>(Config::Window::HEIGHT) - 55.0f),
        sf::Vector2f(220.0f, 44.0f),
        m_font,
        15
    )
{
    float W = static_cast<float>(Config::Window::WIDTH);
    float H = static_cast<float>(Config::Window::HEIGHT);

    // Dark backdrop overlay
    m_darkOverlay.setSize(sf::Vector2f(W, H));
    m_darkOverlay.setFillColor(sf::Color(10, 10, 16, 235));

    // Content box
    m_contentPanel.setSize(sf::Vector2f(W - 140.0f, H - 150.0f));
    m_contentPanel.setOrigin((W - 140.0f) * 0.5f, (H - 150.0f) * 0.5f);
    m_contentPanel.setPosition(W * 0.5f, H * 0.5f - 30.0f);
    m_contentPanel.setFillColor(sf::Color(16, 18, 24, 240));
    m_contentPanel.setOutlineColor(sf::Color(70, 85, 110, 180));
    m_contentPanel.setOutlineThickness(1.5f);

    // Header Title
    m_titleText.setFont(m_font);
    m_titleText.setCharacterSize(28);
    m_titleText.setFillColor(sf::Color(255, 215, 60)); // Gold
    m_titleText.setString("SURVIVOR'S FIELD MANUAL");
    sf::FloatRect tb = m_titleText.getLocalBounds();
    m_titleText.setOrigin(tb.left + tb.width * 0.5f, tb.top + tb.height * 0.5f);
    m_titleText.setPosition(W * 0.5f, 48.0f);

    // Section 1: CONTROLS
    m_controlsTitle.setFont(m_font);
    m_controlsTitle.setCharacterSize(16);
    m_controlsTitle.setFillColor(sf::Color(80, 215, 255)); // Cyan
    m_controlsTitle.setString("--- TACTICAL CONTROLS ---");
    m_controlsTitle.setPosition(100.0f, 105.0f);

    std::string ctrlStr =
        "W, A, S, D       : Movement (smooth wall sliding)\n"
        "MOUSE CURSOR     : 360-Degree Aiming\n"
        "LEFT CLICK / HOLD: Fire Weapon (Semi-Auto / Full-Auto)\n"
        "R                : Tactical Reload\n"
        "1, 2, 3 or WHEEL : Switch Unlocked Firearms\n"
        "ESC              : Pause / Resume Game";
    m_controlsText.setFont(m_font);
    m_controlsText.setCharacterSize(12);
    m_controlsText.setFillColor(sf::Color(220, 225, 235));
    m_controlsText.setString(ctrlStr);
    m_controlsText.setPosition(100.0f, 135.0f);

    // Section 2: ZOMBIE THREATS
    m_zombiesTitle.setFont(m_font);
    m_zombiesTitle.setCharacterSize(16);
    m_zombiesTitle.setFillColor(sf::Color(255, 90, 80)); // Red
    m_zombiesTitle.setString("--- INFECTED ARCHETYPES ---");
    m_zombiesTitle.setPosition(100.0f, 260.0f);

    std::string zombStr =
        "WALKER (Green) : Slow rotting shambler (60 HP, 10 DMG). Standard horde enemy.\n"
        "RUNNER (Red)   : Rapid feral sprinter (35 HP, 8 DMG). Erratic lateral weave.\n"
        "TANK   (Purple): Massive armored brute (400 HP, 30 DMG). Telegraphs heavy smash.\n"
        "                 *TIP*: Direct Shotgun blasts stagger the Tank and interrupt its strike!";
    m_zombiesText.setFont(m_font);
    m_zombiesText.setCharacterSize(12);
    m_zombiesText.setFillColor(sf::Color(220, 225, 235));
    m_zombiesText.setString(zombStr);
    m_zombiesText.setPosition(100.0f, 290.0f);

    // Section 3: SUPPLY DROPS
    m_pickupsTitle.setFont(m_font);
    m_pickupsTitle.setCharacterSize(16);
    m_pickupsTitle.setFillColor(sf::Color(100, 255, 120)); // Green
    m_pickupsTitle.setString("--- SUPPLY DROPS (EXPIRE IN 15 SECONDS) ---");
    m_pickupsTitle.setPosition(100.0f, 400.0f);

    std::string pickStr =
        "HEALTH KIT (White/Red): Restores +25 Health (Capped at 100 HP).\n"
        "AMMO BOX   (Olive/Gold): Adds +24 Reserve Rounds to active firearm.\n"
        "COIN       (Gold Coin) : Grants +50 Bonus Score.\n"
        "*NOTE*: Pickups blink rapidly in the final 3 seconds before vanishing!";
    m_pickupsText.setFont(m_font);
    m_pickupsText.setCharacterSize(12);
    m_pickupsText.setFillColor(sf::Color(220, 225, 235));
    m_pickupsText.setString(pickStr);
    m_pickupsText.setPosition(100.0f, 430.0f);

    m_backButton.setSelected(true);
}

void InstructionsScreen::update(sf::Vector2f mousePos) {
    m_backButton.update(mousePos);
}

bool InstructionsScreen::handleEvent(const sf::Event& event, sf::Vector2f mousePos) {
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
        return true; // ESC returns back to menu
    }
    return m_backButton.handleEvent(event, mousePos);
}

void InstructionsScreen::draw(sf::RenderTarget& target) const {
    target.draw(m_darkOverlay);
    target.draw(m_contentPanel);
    target.draw(m_titleText);

    target.draw(m_controlsTitle);
    target.draw(m_controlsText);

    target.draw(m_zombiesTitle);
    target.draw(m_zombiesText);

    target.draw(m_pickupsTitle);
    target.draw(m_pickupsText);

    m_backButton.draw(target);
}
