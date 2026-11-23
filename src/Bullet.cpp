#include "Bullet.hpp"
#include "Config.hpp"
#include <cmath>

Bullet::Bullet(sf::Vector2f startPosition, sf::Vector2f direction, float speed, float damage, float maxRange, bool isShotgunPellet)
    : m_position(startPosition)
    , m_speed(speed)
    , m_damage(damage)
    , m_maxRange(maxRange)
    , m_distanceTraveled(0.0f)
    , m_isShotgunPellet(isShotgunPellet)
    , m_isAlive(true)
{
    // Guard against zero-length direction vectors to prevent division by zero or NaN coordinates
    float length = std::hypot(direction.x, direction.y);
    if (length > 0.0001f) {
        m_direction = direction / length;
    } else {
        // Fallback default direction (rightwards)
        m_direction = sf::Vector2f(1.0f, 0.0f);
    }
}

void Bullet::update(float dt, const std::vector<sf::FloatRect>& obstacles, float mapWidth, float mapHeight) {
    if (!m_isAlive) return;

    // Advance position by delta time
    float step = m_speed * dt;
    m_position += m_direction * step;
    m_distanceTraveled += step;

    // 1. Exceeded maximum ballistic range
    if (m_distanceTraveled >= m_maxRange) {
        m_isAlive = false;
        return;
    }

    // 2. Hit outer map boundaries
    if (m_position.x <= 0.0f || m_position.x >= mapWidth ||
        m_position.y <= 0.0f || m_position.y >= mapHeight) {
        m_isAlive = false;
        return;
    }

    // 3. Hit an obstacle (ruined car, barrier, building wall)
    for (const auto& obs : obstacles) {
        if (obs.contains(m_position)) {
            m_isAlive = false;
            return;
        }
    }
}

void Bullet::draw(sf::RenderTarget& target) const {
    if (!m_isAlive) return;

    // Render an elongated glowing tracer along the bullet's direction of travel
    sf::RectangleShape tracer;
    tracer.setSize(sf::Vector2f(Config::BulletVisuals::TRAIL_LENGTH, Config::BulletVisuals::TRAIL_THICKNESS));
    // Set origin to the leading tip of the tracer
    tracer.setOrigin(Config::BulletVisuals::TRAIL_LENGTH, Config::BulletVisuals::TRAIL_THICKNESS * 0.5f);
    tracer.setPosition(m_position);

    float angleDeg = std::atan2(m_direction.y, m_direction.x) * (180.0f / 3.1415926535f);
    tracer.setRotation(angleDeg);
    tracer.setFillColor(Config::BulletVisuals::COLOR_HEAD);

    target.draw(tracer);
}
