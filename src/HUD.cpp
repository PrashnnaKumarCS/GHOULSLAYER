#include "HUD.hpp"
#include "Player.hpp"
#include "WaveManager.hpp"
#include "ResourceManager.hpp"
#include "Config.hpp"
#include <cmath>
#include <algorithm>

HUD::HUD(const ResourceManager& resources)
    // TODO: Bundle a custom pixel font (such as PressStart2P.ttf) in assets/fonts/ in a later stage.
    // For now, we utilize the Windows system font fallback from ResourceManager.
    : m_font(resources.getFont("default"))
    , m_lowHealthPulseTimer(0.0f)
{
    // 1. Health Bar
    m_healthBarBg.setSize(sf::Vector2f(220.0f, 22.0f));
    m_healthBarBg.setPosition(24.0f, static_cast<float>(Config::Window::HEIGHT) - 46.0f);
    m_healthBarBg.setFillColor(sf::Color(25, 12, 12, 230));
    m_healthBarBg.setOutlineColor(sf::Color(120, 40, 40, 200));
    m_healthBarBg.setOutlineThickness(1.5f);

    m_healthBarFill.setSize(sf::Vector2f(220.0f, 22.0f));
    m_healthBarFill.setPosition(24.0f, static_cast<float>(Config::Window::HEIGHT) - 46.0f);
    m_healthBarFill.setFillColor(sf::Color(210, 35, 35));

    m_healthText.setFont(m_font);
    m_healthText.setCharacterSize(14);
    m_healthText.setFillColor(sf::Color(255, 255, 255));
    m_healthText.setPosition(30.0f, static_cast<float>(Config::Window::HEIGHT) - 44.0f);

    // 2. Weapon and Ammo
    m_weaponNameText.setFont(m_font);
    m_weaponNameText.setCharacterSize(18);
    m_weaponNameText.setFillColor(sf::Color(245, 245, 250));
    m_weaponNameText.setPosition(24.0f, static_cast<float>(Config::Window::HEIGHT) - 98.0f);

    m_ammoText.setFont(m_font);
    m_ammoText.setCharacterSize(15);
    m_ammoText.setFillColor(sf::Color(220, 225, 230));
    m_ammoText.setPosition(24.0f, static_cast<float>(Config::Window::HEIGHT) - 74.0f);

    m_reloadText.setFont(m_font);
    m_reloadText.setCharacterSize(13);
    m_reloadText.setFillColor(sf::Color(80, 220, 255));
    m_reloadText.setPosition(250.0f, static_cast<float>(Config::Window::HEIGHT) - 44.0f);

    // 3. Stats Panel (Top-Right)
    m_statsPanelBg.setSize(sf::Vector2f(230.0f, 88.0f));
    m_statsPanelBg.setPosition(static_cast<float>(Config::Window::WIDTH) - 254.0f, 20.0f);
    m_statsPanelBg.setFillColor(sf::Color(12, 14, 18, 220));
    m_statsPanelBg.setOutlineColor(sf::Color(70, 85, 105, 180));
    m_statsPanelBg.setOutlineThickness(1.5f);

    m_waveText.setFont(m_font);
    m_waveText.setCharacterSize(16);
    m_waveText.setPosition(static_cast<float>(Config::Window::WIDTH) - 242.0f, 26.0f);

    m_scoreText.setFont(m_font);
    m_scoreText.setCharacterSize(15);
    m_scoreText.setFillColor(sf::Color(255, 215, 60)); // Gold score
    m_scoreText.setPosition(static_cast<float>(Config::Window::WIDTH) - 242.0f, 50.0f);

    m_killsText.setFont(m_font);
    m_killsText.setCharacterSize(14);
    m_killsText.setFillColor(sf::Color(200, 210, 220));
    m_killsText.setPosition(static_cast<float>(Config::Window::WIDTH) - 242.0f, 74.0f);

    // 4. Center Announcement Banner
    m_bannerBg.setSize(sf::Vector2f(660.0f, 76.0f));
    m_bannerBg.setOrigin(330.0f, 38.0f);
    m_bannerBg.setPosition(static_cast<float>(Config::Window::WIDTH) * 0.5f, 170.0f);
    m_bannerBg.setFillColor(sf::Color(10, 12, 16, 235));
    m_bannerBg.setOutlineThickness(2.0f);

    m_bannerTitleText.setFont(m_font);
    m_bannerTitleText.setCharacterSize(22);

    m_bannerSubtitleText.setFont(m_font);
    m_bannerSubtitleText.setCharacterSize(15);

    // 5. Bottom Controls Hint
    m_controlsHintText.setFont(m_font);
    m_controlsHintText.setCharacterSize(13);
    m_controlsHintText.setFillColor(sf::Color(150, 165, 180));
    m_controlsHintText.setPosition(
        static_cast<float>(Config::Window::WIDTH) * 0.5f - 240.0f,
        static_cast<float>(Config::Window::HEIGHT) - 24.0f
    );
    m_controlsHintText.setString("[1,2,3/Wheel: Switch Weapon | R: Reload | F1: Unlock All | F2: Spawn 5]");
}

void HUD::update(float dt) {
    // Pulse timer for low-health visual alert
    m_lowHealthPulseTimer += dt * 6.0f;

    // Advance floating pickup notifications
    for (auto& msg : m_pickupMessages) {
        msg.timer -= dt;
    }

    m_pickupMessages.erase(
        std::remove_if(m_pickupMessages.begin(), m_pickupMessages.end(),
            [](const PickupMessage& m) { return m.timer <= 0.0f; }),
        m_pickupMessages.end()
    );
}

void HUD::showPickupMessage(const std::string& text, sf::Color color) {
    PickupMessage msg;
    msg.text = text;
    msg.timer = 1.8f; // Displays for 1.8 seconds
    msg.color = color;
    m_pickupMessages.push_back(msg);
}

void HUD::render(sf::RenderTarget& target, const Player& player, const WaveManager& waveManager, int score, int kills) {
    // 1. Render Health Bar
    float hp = player.getHealth();
    float maxHp = player.getMaxHealth();
    float healthPct = std::clamp(hp / maxHp, 0.0f, 1.0f);

    m_healthBarFill.setSize(sf::Vector2f(220.0f * healthPct, 22.0f));

    // Low-health pulsing alert when HP <= 30
    if (hp <= 30.0f && hp > 0.0f) {
        float pulse = (std::sin(m_lowHealthPulseTimer) + 1.0f) * 0.5f;
        sf::Color pulseColor(
            static_cast<sf::Uint8>(200 + pulse * 55),
            static_cast<sf::Uint8>(30 + pulse * 80),
            static_cast<sf::Uint8>(30)
        );
        m_healthBarFill.setFillColor(pulseColor);
    } else {
        m_healthBarFill.setFillColor(sf::Color(210, 35, 35));
    }

    target.draw(m_healthBarBg);
    target.draw(m_healthBarFill);

    if (player.isDead()) {
        m_healthText.setString("HP: 0 / 100 (DEAD)");
    } else {
        m_healthText.setString("HP: " + std::to_string(static_cast<int>(hp)) + " / " + std::to_string(static_cast<int>(maxHp)));
    }
    target.draw(m_healthText);

    // 2. Render Weapon & Ammo
    const Weapon& weapon = player.getCurrentWeapon();
    m_weaponNameText.setString(weapon.getName());
    target.draw(m_weaponNameText);

    std::string ammoStr = "AMMO: " + std::to_string(weapon.getMagazine()) + " / " + std::to_string(weapon.getReserve());
    m_ammoText.setString(ammoStr);

    // Color ammo text red if magazine is low (<= 25% capacity)
    if (weapon.getMagazine() <= weapon.getMaxMagazine() / 4) {
        m_ammoText.setFillColor(sf::Color(255, 80, 70));
    } else {
        m_ammoText.setFillColor(sf::Color(220, 225, 230));
    }
    target.draw(m_ammoText);

    // Reload status feedback
    if (weapon.isReloading()) {
        int pct = static_cast<int>(weapon.getReloadProgress() * 100.0f);
        m_reloadText.setString("RELOADING... " + std::to_string(pct) + "%");
        m_reloadText.setFillColor(sf::Color(80, 220, 255));
        target.draw(m_reloadText);
    } else if (weapon.getMagazine() == 0 && !player.isDead()) {
        if (weapon.getReserve() > 0) {
            m_reloadText.setString("[PRESS R TO RELOAD]");
            m_reloadText.setFillColor(sf::Color(255, 210, 60));
        } else {
            m_reloadText.setString("[OUT OF AMMO]");
            m_reloadText.setFillColor(sf::Color(255, 60, 60));
        }
        target.draw(m_reloadText);
    }

    // 3. Render Stats Panel (Top-Right)
    target.draw(m_statsPanelBg);

    if (waveManager.isHordeWave()) {
        m_waveText.setString("HORDE WAVE " + std::to_string(waveManager.getCurrentWave()));
        m_waveText.setFillColor(sf::Color(255, 70, 70));
    } else {
        m_waveText.setString("WAVE " + std::to_string(waveManager.getCurrentWave()));
        m_waveText.setFillColor(sf::Color(255, 255, 255));
    }
    target.draw(m_waveText);

    m_scoreText.setString("SCORE: " + std::to_string(score));
    target.draw(m_scoreText);

    m_killsText.setString("KILLS: " + std::to_string(kills));
    target.draw(m_killsText);

    // 4. Render Active Center Announcement Banner
    if (waveManager.hasActiveBanner()) {
        // Outline color: Red for horde wave, Gold for weapon unlocks, Blue/Gray for wave clears
        if (waveManager.isHordeWave()) {
            m_bannerBg.setOutlineColor(sf::Color(220, 45, 45, 230));
        } else if (waveManager.getBannerSubtitle().find("NEW WEAPON") != std::string::npos) {
            m_bannerBg.setOutlineColor(sf::Color(255, 215, 60, 240));
        } else {
            m_bannerBg.setOutlineColor(sf::Color(70, 150, 240, 220));
        }
        target.draw(m_bannerBg);

        // Center Title
        m_bannerTitleText.setString(waveManager.getBannerTitle());
        m_bannerTitleText.setFillColor(sf::Color(255, 255, 255));
        sf::FloatRect titleBounds = m_bannerTitleText.getLocalBounds();
        m_bannerTitleText.setOrigin(titleBounds.width * 0.5f, titleBounds.height * 0.5f);
        m_bannerTitleText.setPosition(static_cast<float>(Config::Window::WIDTH) * 0.5f, 156.0f);
        target.draw(m_bannerTitleText);

        // Center Subtitle / Intermission countdown
        std::string sub = waveManager.getBannerSubtitle();
        if (waveManager.getPhase() == WavePhase::Intermission) {
            int countdown = static_cast<int>(std::ceil(waveManager.getIntermissionRemaining()));
            sub += " (NEXT WAVE IN " + std::to_string(countdown) + "...)";
        }
        m_bannerSubtitleText.setString(sub);
        m_bannerSubtitleText.setFillColor(sf::Color(255, 220, 90));
        sf::FloatRect subBounds = m_bannerSubtitleText.getLocalBounds();
        m_bannerSubtitleText.setOrigin(subBounds.width * 0.5f, subBounds.height * 0.5f);
        m_bannerSubtitleText.setPosition(static_cast<float>(Config::Window::WIDTH) * 0.5f, 186.0f);
        target.draw(m_bannerSubtitleText);
    }

    // 5. Render Floating Pickup Notifications
    float msgY = static_cast<float>(Config::Window::HEIGHT) - 130.0f;
    for (const auto& msg : m_pickupMessages) {
        sf::Text pText(msg.text, m_font, 14);
        float alphaFactor = std::clamp(msg.timer / 1.8f, 0.0f, 1.0f);
        sf::Color c = msg.color;
        c.a = static_cast<sf::Uint8>(alphaFactor * 255.0f);
        pText.setFillColor(c);
        pText.setPosition(24.0f, msgY);
        target.draw(pText);
        msgY -= 20.0f; // Stack messages upwards
    }

    // 6. Controls hint at screen bottom
    target.draw(m_controlsHintText);
}
