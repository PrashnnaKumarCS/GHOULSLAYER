#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Button.hpp"

class ResourceManager;

// Cinematic main menu screen with code-generated apocalyptic skyline,
// drifting ember particles, atmospheric moon, flickering title, and button navigation.
class MainMenu {
public:
    enum class Action {
        None,
        Play,
        Instructions,
        Settings,
        Exit
    };

    explicit MainMenu(const ResourceManager& resources);

    // Advance ambient background animations (embers, title flicker)
    void update(float dt, sf::Vector2f mousePos);

    // Process user input for menu navigation (returns an Action if a button was clicked)
    Action handleEvent(const sf::Event& event, sf::Vector2f mousePos);

    // Render cinematic menu background, title, and buttons
    void draw(sf::RenderTarget& target) const;

private:
    struct Ember {
        sf::Vector2f pos;
        sf::Vector2f vel;
        float alpha;
        float maxLife;
        float life;
    };

    void initCitySkyline();
    void spawnEmber(Ember& e, bool randomY = false);

    const sf::Font& m_font;

    // Atmospheric elements
    sf::VertexArray m_skyGradient;
    sf::CircleShape m_moon;
    sf::CircleShape m_moonGlow;
    std::vector<sf::RectangleShape> m_backBuildings;
    std::vector<sf::RectangleShape> m_frontBuildings;
    std::vector<sf::RectangleShape> m_buildingWindows;

    // Drifting embers (hard capped for performance)
    static constexpr size_t MAX_EMBERS = 45;
    std::vector<Ember> m_embers;

    // Title & Tagline
    sf::Text m_titleText;
    sf::Text m_taglineText;
    float m_titleFlickerTimer;

    // Navigation buttons
    std::vector<Button> m_buttons;
    int m_selectedIndex;
};
