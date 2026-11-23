#include "CollisionSystem.hpp"
#include <algorithm>

bool CollisionSystem::checkCircleCircle(sf::Vector2f posA, float radiusA, sf::Vector2f posB, float radiusB) {
    float dx = posA.x - posB.x;
    float dy = posA.y - posB.y;
    float distSq = dx * dx + dy * dy;
    float radSum = radiusA + radiusB;
    return distSq <= (radSum * radSum);
}

bool CollisionSystem::checkRectCircle(const sf::FloatRect& rect, sf::Vector2f circleCenter, float circleRadius) {
    // Find closest point on the axis-aligned rectangle to the circle center
    float closestX = std::clamp(circleCenter.x, rect.left, rect.left + rect.width);
    float closestY = std::clamp(circleCenter.y, rect.top, rect.top + rect.height);

    float dx = circleCenter.x - closestX;
    float dy = circleCenter.y - closestY;
    return (dx * dx + dy * dy) <= (circleRadius * circleRadius);
}

bool CollisionSystem::checkPointCircle(sf::Vector2f point, sf::Vector2f circleCenter, float circleRadius) {
    float dx = point.x - circleCenter.x;
    float dy = point.y - circleCenter.y;
    return (dx * dx + dy * dy) <= (circleRadius * circleRadius);
}
