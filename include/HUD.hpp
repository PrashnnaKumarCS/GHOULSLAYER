#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class Player;
class WaveManager;
class ResourceManager;

// Temporary floating notification for collected drops
struct PickupMessage {
    std::string text;
    float timer;
    sf::Color color;
};

// HUD renders all player statistics, combat status, wave progression,
// and announcement banners in screen space using the default viewport.
class HUD {
public:
    explicit HUD(const ResourceManager& resources);

    // Advance HUD timers (low-health pulse, pickup message decays)
    void update(float dt);

    // Trigger a temporary floating drop notification (e.g. "+25 HEALTH", "+AMMO")
    void showPickupMessage(const std::string& text, sf::Color color = sf::Color(255, 230, 90));

    // Render complete HUD overlay
    void render(sf::RenderTarget& target, const Player& player, const WaveManager& waveManager, int score, int kills);

private:
    const sf::Font& m_font;

    // Health Bar
    sf::RectangleShape m_healthBarBg;
    sf::RectangleShape m_healthBarFill;
    sf::Text m_healthText;

    // Weapon & Ammo
    sf::Text m_weaponNameText;
    sf::Text m_ammoText;
    sf::Text m_reloadText;
    sf::RectangleShape m_reloadBarBg;
    sf::RectangleShape m_reloadBarFill;

    // Stats (Wave, Score, Kills)
    sf::RectangleShape m_statsPanelBg;
    sf::Text m_waveText;
    sf::Text m_killsText;
    sf::Text m_scoreText;

    // Center Announcement Banners
    sf::RectangleShape m_bannerBg;
    sf::Text m_bannerTitleText;
    sf::Text m_bannerSubtitleText;

    // Bottom controls hint
    sf::Text m_controlsHintText;

    // Active floating pickup messages
    std::vector<PickupMessage> m_pickupMessages;

    float m_lowHealthPulseTimer;
};
