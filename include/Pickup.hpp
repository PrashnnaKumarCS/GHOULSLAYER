#pragma once

#include <SFML/Graphics.hpp>

class ResourceManager;

// Distinct collectible item types
enum class PickupType {
    HealthKit,
    AmmoBox,
    Coin
};

// Represents a collectible item dropped upon a zombie's demise.
// Features:
// - Gentle vertical sinusoidal bobbing animation
// - 15-second lifetime with rapid blink warning in the last 3 seconds
// - Safe one-time collection check preventing duplicate pickups
class Pickup {
public:
    Pickup(PickupType type, sf::Vector2f position, const ResourceManager& resources);

    // Advance lifetime and floating bob animation
    void update(float dt);

    // Check if player's collision circle collects this item
    bool checkCollection(sf::Vector2f playerPos, float playerRadius);

    // Render pickup sprite with bobbing and expiration blinking
    void draw(sf::RenderTarget& target) const;

    PickupType getType() const { return m_type; }
    bool isExpired() const { return m_lifetime <= 0.0f; }
    bool isCollected() const { return m_isCollected; }
    sf::Vector2f getPosition() const { return m_position; }

private:
    PickupType m_type;
    sf::Vector2f m_position;
    float m_lifetime;
    float m_bobTimer;
    bool m_isCollected;
    sf::Sprite m_sprite;
};
