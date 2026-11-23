#pragma once

#include <SFML/Graphics.hpp>

// Camera wraps sf::View and provides smooth interpolation (lerp) toward
// the player's position, while clamping so that the viewport never shows
// areas outside the 4000x4000 world boundaries.
class Camera {
public:
    Camera();

    // Setup initial camera view dimensions and starting center
    void initialize(sf::Vector2f startPosition, sf::Vector2f viewSize);

    // Smoothly glide camera toward target position based on delta time
    void update(sf::Vector2f targetPosition, float dt);

    // Access the underlying SFML view
    const sf::View& getView() const { return m_view; }

    // Direct access to current camera center
    sf::Vector2f getCenter() const { return m_currentCenter; }

private:
    sf::View m_view;
    sf::Vector2f m_currentCenter;
};
