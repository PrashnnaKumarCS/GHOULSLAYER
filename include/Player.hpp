#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class ResourceManager;

// Player represents the playable survivor.
// Features:
// - 8-directional WASD movement with diagonal normalization
// - Smooth 360-degree rotation aiming toward mouse cursor in world coordinates
// - Axis-separated collision response allowing smooth sliding against obstacles
// - 4-frame animated leg/body bob cycle that triggers only while moving
class Player {
public:
    explicit Player(const ResourceManager& resources);

    // Update movement, collision, mouse aiming, and animation
    void update(float dt, const std::vector<sf::FloatRect>& obstacles, sf::Vector2f worldMousePos);

    // Render player sprite to current view
    void draw(sf::RenderTarget& target) const;

    // Getters
    sf::Vector2f getPosition() const { return m_position; }
    float getHealth() const { return m_health; }
    const sf::FloatRect& getCollisionBox() const { return m_collisionBox; }

private:
    void updateCollisionBox();

    sf::Vector2f m_position;
    float m_health;
    bool m_isMoving;

    // Bounding box for world collision
    sf::FloatRect m_collisionBox;
    const float m_halfSize = 12.0f;

    // Animation frames and timer
    std::vector<const sf::Texture*> m_frameTextures;
    int m_currentFrame;
    float m_animTimer;

    // Render sprite
    sf::Sprite m_sprite;
};
