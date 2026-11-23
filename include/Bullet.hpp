#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

// Represents an active projectile fired by a weapon.
// Moves with delta-time integration along a normalized trajectory vector.
// Deactivates upon exceeding max range, colliding with map obstacles, or leaving map bounds.
// Draws an illuminated tracer streak aligned with velocity.
class Bullet {
public:
    Bullet(sf::Vector2f startPosition, sf::Vector2f direction, float speed, float damage, float maxRange);

    // Update position and check collision with obstacles and map boundaries
    void update(float dt, const std::vector<sf::FloatRect>& obstacles, float mapWidth, float mapHeight);

    // Render the tracer streak in world space
    void draw(sf::RenderTarget& target) const;

    bool isAlive() const { return m_isAlive; }
    float getDamage() const { return m_damage; }
    sf::Vector2f getPosition() const { return m_position; }

private:
    sf::Vector2f m_position;
    sf::Vector2f m_direction; // Guaranteed unit length
    float m_speed;
    float m_damage;
    float m_maxRange;
    float m_distanceTraveled;
    bool m_isAlive;
};
