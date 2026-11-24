#include "MainMenu.hpp"
#include "ResourceManager.hpp"
#include "Config.hpp"
#include <cmath>
#include <random>

namespace {
    std::mt19937& getMenuRandom() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        return gen;
    }
}

MainMenu::MainMenu(const ResourceManager& resources)
    : m_font(resources.getFont("default"))
    , m_titleFlickerTimer(0.0f)
    , m_selectedIndex(0)
{
    // 1. Sky Gradient (smoky night sky)
    m_skyGradient.setPrimitiveType(sf::Quads);
    m_skyGradient.resize(4);

    float W = static_cast<float>(Config::Window::WIDTH);
    float H = static_cast<float>(Config::Window::HEIGHT);

    m_skyGradient[0].position = sf::Vector2f(0.0f, 0.0f);
    m_skyGradient[0].color = sf::Color(10, 10, 18);
    m_skyGradient[1].position = sf::Vector2f(W, 0.0f);
    m_skyGradient[1].color = sf::Color(12, 12, 22);
    m_skyGradient[2].position = sf::Vector2f(W, H);
    m_skyGradient[2].color = sf::Color(32, 22, 34);
    m_skyGradient[3].position = sf::Vector2f(0.0f, H);
    m_skyGradient[3].color = sf::Color(28, 18, 30);

    // 2. Dim Moon and soft halo glow
    m_moonGlow.setRadius(55.0f);
    m_moonGlow.setOrigin(55.0f, 55.0f);
    m_moonGlow.setPosition(1020.0f, 130.0f);
    m_moonGlow.setFillColor(sf::Color(180, 205, 235, 45));

    m_moon.setRadius(40.0f);
    m_moon.setOrigin(40.0f, 40.0f);
    m_moon.setPosition(1020.0f, 130.0f);
    m_moon.setFillColor(sf::Color(215, 225, 240, 190));

    // 3. City skyline
    initCitySkyline();

    // 4. Initialize drifting embers
    m_embers.resize(MAX_EMBERS);
    for (auto& e : m_embers) {
        spawnEmber(e, true);
    }

    // 5. Title & Tagline
    m_titleText.setFont(m_font);
    m_titleText.setCharacterSize(42);
    m_titleText.setString("GHOULSLAYER");
    sf::FloatRect titleB = m_titleText.getLocalBounds();
    m_titleText.setOrigin(titleB.left + titleB.width * 0.5f, titleB.top + titleB.height * 0.5f);
    m_titleText.setPosition(W * 0.5f, 150.0f);

    m_taglineText.setFont(m_font);
    m_taglineText.setCharacterSize(15);
    m_taglineText.setFillColor(sf::Color(255, 215, 60)); // Gold
    m_taglineText.setString("RISE. FIGHT. SURVIVE.");
    sf::FloatRect tagB = m_taglineText.getLocalBounds();
    m_taglineText.setOrigin(tagB.left + tagB.width * 0.5f, tagB.top + tagB.height * 0.5f);
    m_taglineText.setPosition(W * 0.5f, 215.0f);

    // 6. Navigation Buttons
    sf::Vector2f btnSize(280.0f, 46.0f);
    float startY = 320.0f;
    float spacing = 62.0f;

    m_buttons.emplace_back("PLAY", sf::Vector2f(W * 0.5f, startY + spacing * 0), btnSize, m_font, 16);
    m_buttons.emplace_back("INSTRUCTIONS", sf::Vector2f(W * 0.5f, startY + spacing * 1), btnSize, m_font, 14);
    m_buttons.emplace_back("SETTINGS", sf::Vector2f(W * 0.5f, startY + spacing * 2), btnSize, m_font, 14);
    m_buttons.emplace_back("EXIT", sf::Vector2f(W * 0.5f, startY + spacing * 3), btnSize, m_font, 16);

    m_buttons[0].setSelected(true);
}

void MainMenu::initCitySkyline() {
    float W = static_cast<float>(Config::Window::WIDTH);
    float H = static_cast<float>(Config::Window::HEIGHT);

    // Layer 1: Distant buildings (dark gray/blue)
    float curX = -20.0f;
    std::uniform_real_distribution<float> bWidthDist(80.0f, 140.0f);
    std::uniform_real_distribution<float> bHeightDist(220.0f, 360.0f);

    while (curX < W + 40.0f) {
        float bW = bWidthDist(getMenuRandom());
        float bH = bHeightDist(getMenuRandom());
        sf::RectangleShape b(sf::Vector2f(bW, bH));
        b.setPosition(curX, H - bH);
        b.setFillColor(sf::Color(20, 22, 30));
        m_backBuildings.push_back(b);
        curX += bW - 10.0f;
    }

    // Layer 2: Foreground silhouette buildings (pitch dark)
    curX = -20.0f;
    std::uniform_real_distribution<float> fWidthDist(70.0f, 120.0f);
    std::uniform_real_distribution<float> fHeightDist(120.0f, 240.0f);

    while (curX < W + 40.0f) {
        float bW = fWidthDist(getMenuRandom());
        float bH = fHeightDist(getMenuRandom());
        sf::RectangleShape b(sf::Vector2f(bW, bH));
        b.setPosition(curX, H - bH);
        b.setFillColor(sf::Color(10, 12, 16));
        m_frontBuildings.push_back(b);

        // Add a few faint amber window lights
        if (bH > 160.0f) {
            sf::RectangleShape win(sf::Vector2f(6.0f, 8.0f));
            win.setPosition(curX + bW * 0.35f, H - bH + 40.0f);
            win.setFillColor(sf::Color(230, 175, 45, 140));
            m_buildingWindows.push_back(win);

            sf::RectangleShape win2(sf::Vector2f(6.0f, 8.0f));
            win2.setPosition(curX + bW * 0.65f, H - bH + 75.0f);
            win2.setFillColor(sf::Color(230, 175, 45, 120));
            m_buildingWindows.push_back(win2);
        }

        curX += bW - 8.0f;
    }
}

void MainMenu::spawnEmber(Ember& e, bool randomY) {
    float W = static_cast<float>(Config::Window::WIDTH);
    float H = static_cast<float>(Config::Window::HEIGHT);

    std::uniform_real_distribution<float> xDist(0.0f, W);
    std::uniform_real_distribution<float> yDist(0.0f, H);
    std::uniform_real_distribution<float> vxDist(10.0f, 32.0f);
    std::uniform_real_distribution<float> vyDist(-22.0f, -48.0f);
    std::uniform_real_distribution<float> lifeDist(4.0f, 9.0f);

    e.pos.x = xDist(getMenuRandom());
    e.pos.y = randomY ? yDist(getMenuRandom()) : (H + 10.0f);
    e.vel.x = vxDist(getMenuRandom());
    e.vel.y = vyDist(getMenuRandom());
    e.maxLife = lifeDist(getMenuRandom());
    e.life = randomY ? (lifeDist(getMenuRandom()) * 0.5f) : e.maxLife;
}

void MainMenu::update(float dt, sf::Vector2f mousePos) {
    // 1. Title glow & gentle flicker
    m_titleFlickerTimer += dt * 3.5f;
    float flicker = (std::sin(m_titleFlickerTimer) + 1.0f) * 0.5f;
    sf::Uint8 redVal = static_cast<sf::Uint8>(210 + flicker * 45);
    sf::Uint8 greenVal = static_cast<sf::Uint8>(40 + flicker * 30);
    m_titleText.setFillColor(sf::Color(redVal, greenVal, greenVal));

    // 2. Drifting embers update
    float W = static_cast<float>(Config::Window::WIDTH);
    for (auto& e : m_embers) {
        e.pos += e.vel * dt;
        e.life -= dt;
        if (e.life <= 0.0f || e.pos.x > W + 20.0f || e.pos.y < -20.0f) {
            spawnEmber(e, false);
        }
    }

    // 3. Update buttons
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        m_buttons[i].update(mousePos);
        if (m_buttons[i].isHovered()) {
            m_selectedIndex = static_cast<int>(i);
        }
        m_buttons[i].setSelected(m_selectedIndex == static_cast<int>(i));
    }
}

MainMenu::Action MainMenu::handleEvent(const sf::Event& event, sf::Vector2f mousePos) {
    // Keyboard navigation (Up/Down arrows)
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
            m_selectedIndex = (m_selectedIndex - 1 + static_cast<int>(m_buttons.size())) % static_cast<int>(m_buttons.size());
        } else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
            m_selectedIndex = (m_selectedIndex + 1) % static_cast<int>(m_buttons.size());
        }
    }

    // Update button visual selections
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        m_buttons[i].setSelected(m_selectedIndex == static_cast<int>(i));
    }

    // Check button click or Enter key
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        if (m_buttons[i].handleEvent(event, mousePos)) {
            if (i == 0) return Action::Play;
            if (i == 1) return Action::Instructions;
            if (i == 2) return Action::Settings;
            if (i == 3) return Action::Exit;
        }
    }

    return Action::None;
}

void MainMenu::draw(sf::RenderTarget& target) const {
    // 1. Sky & Moon
    target.draw(m_skyGradient);
    target.draw(m_moonGlow);
    target.draw(m_moon);

    // 2. City Silhouette Layers
    for (const auto& b : m_backBuildings) target.draw(b);
    for (const auto& b : m_frontBuildings) target.draw(b);
    for (const auto& w : m_buildingWindows) target.draw(w);

    // 3. Drifting Embers
    sf::RectangleShape emberShape(sf::Vector2f(3.0f, 3.0f));
    for (const auto& e : m_embers) {
        float lifePct = std::clamp(e.life / e.maxLife, 0.0f, 1.0f);
        sf::Color c(255, 145, 45, static_cast<sf::Uint8>(lifePct * 200.0f));
        emberShape.setPosition(e.pos);
        emberShape.setFillColor(c);
        target.draw(emberShape);
    }

    // 4. Title and Tagline
    target.draw(m_titleText);
    target.draw(m_taglineText);

    // 5. Buttons
    for (const auto& btn : m_buttons) {
        btn.draw(target);
    }
}
