#pragma once

#include <SFML/Graphics.hpp>

// CollisionSystem centralizes 2D geometric intersection calculations
// used by bullets, zombies, the player, and map obstacles.
class CollisionSystem {
public:
    // Check if two circles overlap (for zombie-to-zombie separation and zombie-to-player reach)
    static bool checkCircleCircle(sf::Vector2f posA, float radiusA, sf::Vector2f posB, float radiusB);

    // Check if an axis-aligned rectangle overlaps with a circle
    static bool checkRectCircle(const sf::FloatRect& rect, sf::Vector2f circleCenter, float circleRadius);

    // Check if a point lies within a circle (ideal for bullet-vs-zombie hit detection)
    static bool checkPointCircle(sf::Vector2f point, sf::Vector2f circleCenter, float circleRadius);
};
