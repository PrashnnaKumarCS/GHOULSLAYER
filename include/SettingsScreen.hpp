#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "Button.hpp"

class ResourceManager;

// SettingsScreen manages volume configuration and fullscreen display settings.
// Automatically persists and restores settings from "settings.cfg" with fallback safety.
class SettingsScreen {
public:
    explicit SettingsScreen(const ResourceManager& resources);

    void update(sf::Vector2f mousePos);

    // Returns true when the user presses Back to return to the Main Menu
    bool handleEvent(const sf::Event& event, sf::Vector2f mousePos);

    void draw(sf::RenderTarget& target) const;

    // Checks and clears the flag indicating window recreation is needed
    bool consumeFullscreenToggled();
    bool isFullscreen() const { return m_fullscreen; }

    int getMasterVolume() const { return m_masterVolume; }
    int getMusicVolume() const { return m_musicVolume; }
    int getSfxVolume() const { return m_sfxVolume; }

    void loadFromFile();
    void saveToFile() const;

private:
    void updateLabels();

    const sf::Font& m_font;

    // Setting values
    int m_masterVolume;
    int m_musicVolume;
    int m_sfxVolume;
    bool m_fullscreen;
    bool m_fullscreenToggled;

    // Visuals
    sf::RectangleShape m_darkOverlay;
    sf::RectangleShape m_contentPanel;
    sf::Text m_titleText;

    // Row texts and controls
    sf::Text m_masterLabel;
    sf::Text m_musicLabel;
    sf::Text m_sfxLabel;

    Button m_masterDecBtn;
    Button m_masterIncBtn;
    Button m_musicDecBtn;
    Button m_musicIncBtn;
    Button m_sfxDecBtn;
    Button m_sfxIncBtn;

    Button m_fullscreenBtn;
    Button m_backBtn;

    std::vector<Button*> m_navButtons;
    int m_selectedNavIndex;
};
