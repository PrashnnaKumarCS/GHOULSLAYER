#include "SettingsScreen.hpp"
#include "ResourceManager.hpp"
#include "Config.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

SettingsScreen::SettingsScreen(const ResourceManager& resources)
    : m_font(resources.getFont("default"))
    , m_masterVolume(100)
    , m_musicVolume(80)
    , m_sfxVolume(80)
    , m_fullscreen(false)
    , m_fullscreenToggled(false)
    , m_masterDecBtn("-", sf::Vector2f(660.0f, 160.0f), sf::Vector2f(44.0f, 36.0f), m_font, 16)
    , m_masterIncBtn("+", sf::Vector2f(800.0f, 160.0f), sf::Vector2f(44.0f, 36.0f), m_font, 16)
    , m_musicDecBtn("-", sf::Vector2f(660.0f, 230.0f), sf::Vector2f(44.0f, 36.0f), m_font, 16)
    , m_musicIncBtn("+", sf::Vector2f(800.0f, 230.0f), sf::Vector2f(44.0f, 36.0f), m_font, 16)
    , m_sfxDecBtn("-", sf::Vector2f(660.0f, 300.0f), sf::Vector2f(44.0f, 36.0f), m_font, 16)
    , m_sfxIncBtn("+", sf::Vector2f(800.0f, 300.0f), sf::Vector2f(44.0f, 36.0f), m_font, 16)
    , m_fullscreenBtn("FULLSCREEN: OFF", sf::Vector2f(static_cast<float>(Config::Window::WIDTH) * 0.5f, 390.0f), sf::Vector2f(320.0f, 44.0f), m_font, 14)
    , m_backBtn("BACK & SAVE", sf::Vector2f(static_cast<float>(Config::Window::WIDTH) * 0.5f, 490.0f), sf::Vector2f(240.0f, 44.0f), m_font, 15)
    , m_selectedNavIndex(0)
{
    float W = static_cast<float>(Config::Window::WIDTH);
    float H = static_cast<float>(Config::Window::HEIGHT);

    m_darkOverlay.setSize(sf::Vector2f(W, H));
    m_darkOverlay.setFillColor(sf::Color(10, 10, 16, 235));

    m_contentPanel.setSize(sf::Vector2f(680.0f, 520.0f));
    m_contentPanel.setOrigin(340.0f, 260.0f);
    m_contentPanel.setPosition(W * 0.5f, H * 0.5f);
    m_contentPanel.setFillColor(sf::Color(16, 18, 24, 240));
    m_contentPanel.setOutlineColor(sf::Color(70, 85, 110, 180));
    m_contentPanel.setOutlineThickness(1.5f);

    m_titleText.setFont(m_font);
    m_titleText.setCharacterSize(26);
    m_titleText.setFillColor(sf::Color(255, 215, 60)); // Gold
    m_titleText.setString("SYSTEM SETTINGS");
    sf::FloatRect tb = m_titleText.getLocalBounds();
    m_titleText.setOrigin(tb.left + tb.width * 0.5f, tb.top + tb.height * 0.5f);
    m_titleText.setPosition(W * 0.5f, 75.0f);

    // Row texts
    m_masterLabel.setFont(m_font);
    m_masterLabel.setCharacterSize(15);
    m_masterLabel.setFillColor(sf::Color(230, 235, 240));
    m_masterLabel.setPosition(360.0f, 150.0f);

    m_musicLabel.setFont(m_font);
    m_musicLabel.setCharacterSize(15);
    m_musicLabel.setFillColor(sf::Color(230, 235, 240));
    m_musicLabel.setPosition(360.0f, 220.0f);

    m_sfxLabel.setFont(m_font);
    m_sfxLabel.setCharacterSize(15);
    m_sfxLabel.setFillColor(sf::Color(230, 235, 240));
    m_sfxLabel.setPosition(360.0f, 290.0f);

    // Setup navigation button list
    m_navButtons = {
        &m_masterDecBtn, &m_masterIncBtn,
        &m_musicDecBtn,  &m_musicIncBtn,
        &m_sfxDecBtn,    &m_sfxIncBtn,
        &m_fullscreenBtn,
        &m_backBtn
    };

    loadFromFile();
    updateLabels();
}

void SettingsScreen::loadFromFile() {
    std::ifstream file("settings.cfg");
    if (!file.is_open()) {
        std::cerr << "[Settings INFO] No settings.cfg found. Using default settings.\n";
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = line.substr(0, eqPos);
        std::string val = line.substr(eqPos + 1);

        try {
            if (key == "master_volume") {
                m_masterVolume = std::clamp(std::stoi(val), 0, 100);
            } else if (key == "music_volume") {
                m_musicVolume = std::clamp(std::stoi(val), 0, 100);
            } else if (key == "sfx_volume") {
                m_sfxVolume = std::clamp(std::stoi(val), 0, 100);
            } else if (key == "fullscreen") {
                m_fullscreen = (std::stoi(val) != 0);
            }
        } catch (...) {
            std::cerr << "[Settings WARNING] Corrupt line in settings.cfg ignored.\n";
        }
    }
    std::cout << "[Settings] Loaded config (Master: " << m_masterVolume << "%, Music: " 
              << m_musicVolume << "%, SFX: " << m_sfxVolume << "%, Fullscreen: " << m_fullscreen << ")\n";
}

void SettingsScreen::saveToFile() const {
    std::ofstream file("settings.cfg");
    if (file.is_open()) {
        file << "master_volume=" << m_masterVolume << "\n";
        file << "music_volume=" << m_musicVolume << "\n";
        file << "sfx_volume=" << m_sfxVolume << "\n";
        file << "fullscreen=" << (m_fullscreen ? 1 : 0) << "\n";
    }
}

void SettingsScreen::updateLabels() {
    m_masterLabel.setString("MASTER VOLUME :  " + std::to_string(m_masterVolume) + "%");
    m_musicLabel.setString("MUSIC VOLUME  :  " + std::to_string(m_musicVolume) + "%");
    m_sfxLabel.setString("SFX VOLUME    :  " + std::to_string(m_sfxVolume) + "%");

    m_fullscreenBtn.setLabel(m_fullscreen ? "FULLSCREEN: [ ON ]" : "FULLSCREEN: [ OFF ]");
}

bool SettingsScreen::consumeFullscreenToggled() {
    bool toggled = m_fullscreenToggled;
    m_fullscreenToggled = false;
    return toggled;
}

void SettingsScreen::update(sf::Vector2f mousePos) {
    for (size_t i = 0; i < m_navButtons.size(); ++i) {
        m_navButtons[i]->update(mousePos);
        if (m_navButtons[i]->isHovered()) {
            m_selectedNavIndex = static_cast<int>(i);
        }
        m_navButtons[i]->setSelected(m_selectedNavIndex == static_cast<int>(i));
    }
}

bool SettingsScreen::handleEvent(const sf::Event& event, sf::Vector2f mousePos) {
    // Keyboard arrow navigation
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            saveToFile();
            return true; // ESC returns and saves
        }

        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
            m_selectedNavIndex = (m_selectedNavIndex - 1 + static_cast<int>(m_navButtons.size())) % static_cast<int>(m_navButtons.size());
        } else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
            m_selectedNavIndex = (m_selectedNavIndex + 1) % static_cast<int>(m_navButtons.size());
        }
    }

    for (size_t i = 0; i < m_navButtons.size(); ++i) {
        m_navButtons[i]->setSelected(m_selectedNavIndex == static_cast<int>(i));
    }

    // Handle volume adjustment buttons
    if (m_masterDecBtn.handleEvent(event, mousePos)) {
        m_masterVolume = std::max(0, m_masterVolume - 10);
        updateLabels();
    }
    if (m_masterIncBtn.handleEvent(event, mousePos)) {
        m_masterVolume = std::min(100, m_masterVolume + 10);
        updateLabels();
    }
    if (m_musicDecBtn.handleEvent(event, mousePos)) {
        m_musicVolume = std::max(0, m_musicVolume - 10);
        updateLabels();
    }
    if (m_musicIncBtn.handleEvent(event, mousePos)) {
        m_musicVolume = std::min(100, m_musicVolume + 10);
        updateLabels();
    }
    if (m_sfxDecBtn.handleEvent(event, mousePos)) {
        m_sfxVolume = std::max(0, m_sfxVolume - 10);
        updateLabels();
    }
    if (m_sfxIncBtn.handleEvent(event, mousePos)) {
        m_sfxVolume = std::min(100, m_sfxVolume + 10);
        updateLabels();
    }

    // Fullscreen toggle
    if (m_fullscreenBtn.handleEvent(event, mousePos)) {
        m_fullscreen = !m_fullscreen;
        m_fullscreenToggled = true;
        updateLabels();
        saveToFile();
    }

    // Back button
    if (m_backBtn.handleEvent(event, mousePos)) {
        saveToFile();
        return true;
    }

    return false;
}

void SettingsScreen::draw(sf::RenderTarget& target) const {
    target.draw(m_darkOverlay);
    target.draw(m_contentPanel);
    target.draw(m_titleText);

    target.draw(m_masterLabel);
    target.draw(m_musicLabel);
    target.draw(m_sfxLabel);

    for (const auto* btn : m_navButtons) {
        btn->draw(target);
    }
}
