#include "Camera.hpp"
#include "Config.hpp"
#include <algorithm>

Camera::Camera()
    : m_currentCenter(Config::Player::START_X, Config::Player::START_Y)
{
}

void Camera::initialize(sf::Vector2f startPosition, sf::Vector2f viewSize) {
    m_view.setSize(viewSize);
    m_currentCenter = startPosition;
    m_view.setCenter(m_currentCenter);
}

void Camera::update(sf::Vector2f targetPosition, float dt) {
    // Linear interpolation (lerp) toward the target
    // We clamp the interpolation factor to 1.0 to prevent overshooting during frame drops
    float lerpFactor = std::min(1.0f, Config::Camera::LERP_SPEED * dt);
    m_currentCenter.x += (targetPosition.x - m_currentCenter.x) * lerpFactor;
    m_currentCenter.y += (targetPosition.y - m_currentCenter.y) * lerpFactor;

    // Viewport clamping so the camera never reveals space beyond the 4000x4000 map
    sf::Vector2f halfSize = m_view.getSize() * 0.5f;

    // Clamp X axis
    if (m_view.getSize().x >= Config::Map::WIDTH) {
        m_currentCenter.x = Config::Map::WIDTH * 0.5f;
    } else {
        m_currentCenter.x = std::clamp(m_currentCenter.x, halfSize.x, Config::Map::WIDTH - halfSize.x);
    }

    // Clamp Y axis
    if (m_view.getSize().y >= Config::Map::HEIGHT) {
        m_currentCenter.y = Config::Map::HEIGHT * 0.5f;
    } else {
        m_currentCenter.y = std::clamp(m_currentCenter.y, halfSize.y, Config::Map::HEIGHT - halfSize.y);
    }

    m_view.setCenter(m_currentCenter);
}
