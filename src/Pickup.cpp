#include "Pickup.hpp"
#include "ResourceManager.hpp"
#include "Config.hpp"
#include <cmath>

Pickup::Pickup(PickupType type, sf::Vector2f position, const ResourceManager& resources)
    : m_type(type)
    , m_position(position)
    , m_lifetime(Config::Drops::LIFETIME)
    , m_bobTimer(0.0f)
    , m_isCollected(false)
{
    std::string texName = "pickup_health";
    if (m_type == PickupType::AmmoBox) {
        texName = "pickup_ammo";
    } else if (m_type == PickupType::Coin) {
        texName = "pickup_coin";
    }

    m_sprite.setTexture(resources.getTexture(texName));
    m_sprite.setOrigin(8.0f, 8.0f); // 16x16 center
    m_sprite.setPosition(m_position);
}

void Pickup::update(float dt) {
    if (m_isCollected) return;

    m_lifetime -= dt;
    m_bobTimer += dt * 4.5f;
}

bool Pickup::checkCollection(sf::Vector2f playerPos, float playerRadius) {
    if (m_isCollected || isExpired()) return false;

    float dx = playerPos.x - m_position.x;
    float dy = playerPos.y - m_position.y;
    float distSq = dx * dx + dy * dy;

    float totalReach = Config::Drops::PICKUP_RADIUS + playerRadius;
    if (distSq <= (totalReach * totalReach)) {
        m_isCollected = true;
        return true;
    }
    return false;
}

void Pickup::draw(sf::RenderTarget& target) const {
    if (m_isCollected || isExpired()) return;

    // Blink warning in the last 3 seconds of lifetime
    if (m_lifetime <= Config::Drops::BLINK_THRESHOLD) {
        if (static_cast<int>(m_lifetime * 9.0f) % 2 == 0) {
            return; // Skip rendering this frame for a rapid flicker effect
        }
    }

    // Apply smooth sinusoidal vertical bobbing
    float bobOffsetY = std::sin(m_bobTimer) * 3.5f;

    sf::Sprite renderSprite = m_sprite;
    renderSprite.setPosition(m_position.x, m_position.y + bobOffsetY);
    target.draw(renderSprite);
}
