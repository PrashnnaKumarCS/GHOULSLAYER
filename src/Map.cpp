#include "Map.hpp"
#include "ResourceManager.hpp"
#include "Config.hpp"
#include <cmath>
#include <algorithm>

Map::Map(const ResourceManager& resources)
    : m_width(Config::Map::WIDTH)
    , m_height(Config::Map::HEIGHT)
    , m_tileSize(Config::Map::TILE_SIZE)
{
    m_cols = static_cast<int>(std::ceil(m_width / m_tileSize));
    m_rows = static_cast<int>(std::ceil(m_height / m_tileSize));
    m_tileGrid.resize(m_cols * m_rows, TileType::Asphalt);

    // Cache sprites for fast tile drawing
    m_asphaltSprite.setTexture(resources.getTexture("tile_asphalt"));
    m_roadLineSprite.setTexture(resources.getTexture("tile_road_line"));
    m_sidewalkSprite.setTexture(resources.getTexture("tile_sidewalk"));

    generateWorldLayout(resources);
    addBoundaryWalls();
}

void Map::generateWorldLayout(const ResourceManager& resources) {
    // Generate a city street grid
    // Every 10 tiles (~640px) is an avenue (4 tiles wide)
    // Between avenues are sidewalk plazas and buildings
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            int gridC = c % 10;
            int gridR = r % 10;

            // Roads are columns 4..7 and rows 4..7
            bool isVerticalRoad = (gridC >= 4 && gridC <= 7);
            bool isHorizontalRoad = (gridR >= 4 && gridR <= 7);

            if (isVerticalRoad || isHorizontalRoad) {
                // Outer edges of road are sidewalks
                if (gridC == 4 || gridC == 7 || gridR == 4 || gridR == 7) {
                    m_tileGrid[r * m_cols + c] = TileType::Sidewalk;
                } else if (gridC == 5 || gridR == 5) {
                    // Center lane has road markings
                    m_tileGrid[r * m_cols + c] = TileType::RoadLine;
                } else {
                    m_tileGrid[r * m_cols + c] = TileType::Asphalt;
                }
            } else {
                // Non-road areas are city plaza sidewalks with buildings on top
                m_tileGrid[r * m_cols + c] = TileType::Sidewalk;
            }
        }
    }

    // Place static obstacles:
    const sf::Texture& texBuilding = resources.getTexture("obstacle_building");
    const sf::Texture& texCar = resources.getTexture("obstacle_car");
    const sf::Texture& texBarrier = resources.getTexture("obstacle_barrier");

    // 1. City block buildings (placed in the non-road plaza blocks)
    for (int blockY = 0; blockY < m_rows / 10; ++blockY) {
        for (int blockX = 0; blockX < m_cols / 10; ++blockX) {
            float originX = blockX * 10.0f * m_tileSize;
            float originY = blockY * 10.0f * m_tileSize;

            // Place a 2x2 building block (256x256) inside the plaza area
            float bX = originX + 1.0f * m_tileSize;
            float bY = originY + 1.0f * m_tileSize;

            Obstacle b1;
            b1.sprite.setTexture(texBuilding);
            b1.sprite.setScale(2.0f, 2.0f); // 128x128 scaled to 256x256
            b1.sprite.setPosition(bX, bY);
            b1.bounds = sf::FloatRect(bX, bY, 256.0f, 256.0f);
            m_obstacles.push_back(b1);
            m_collisionBoxes.push_back(b1.bounds);
        }
    }

    // 2. Ruined Cars scattered along the avenues
    // Deterministic placement along road intersections
    for (int r = 5; r < m_rows; r += 10) {
        for (int c = 6; c < m_cols; c += 10) {
            float carX = c * m_tileSize + 16.0f;
            float carY = r * m_tileSize + 8.0f;

            // Skip placing right at player spawn (2000, 2000)
            float distFromSpawn = std::hypot(carX - Config::Player::START_X, carY - Config::Player::START_Y);
            if (distFromSpawn < 250.0f) continue;

            Obstacle car;
            car.sprite.setTexture(texCar);
            // Rotate some cars for variation
            bool rotate = ((c + r) % 20 == 0);
            if (rotate) {
                car.sprite.setRotation(90.0f);
                car.sprite.setPosition(carX + 48.0f, carY);
                car.bounds = sf::FloatRect(carX, carY, 48.0f, 96.0f);
            } else {
                car.sprite.setPosition(carX, carY);
                car.bounds = sf::FloatRect(carX, carY, 96.0f, 48.0f);
            }
            m_obstacles.push_back(car);
            m_collisionBoxes.push_back(car.bounds);
        }
    }

    // 3. Concrete barriers blocking some streets and alleys
    for (int r = 4; r < m_rows; r += 10) {
        for (int c = 5; c < m_cols; c += 10) {
            if ((r + c) % 3 != 0) continue;
            float barX = c * m_tileSize;
            float barY = (r + 2) * m_tileSize;

            float distFromSpawn = std::hypot(barX - Config::Player::START_X, barY - Config::Player::START_Y);
            if (distFromSpawn < 250.0f) continue;

            Obstacle barrier;
            barrier.sprite.setTexture(texBarrier);
            barrier.sprite.setPosition(barX, barY);
            barrier.bounds = sf::FloatRect(barX, barY, 64.0f, 32.0f);
            m_obstacles.push_back(barrier);
            m_collisionBoxes.push_back(barrier.bounds);
        }
    }
}

void Map::addBoundaryWalls() {
    // 4 thick collision boxes forming an impassable boundary around the 4000x4000 map
    const float THICKNESS = 128.0f;
    // Left boundary
    m_collisionBoxes.emplace_back(-THICKNESS, -THICKNESS, THICKNESS, m_height + 2.0f * THICKNESS);
    // Right boundary
    m_collisionBoxes.emplace_back(m_width, -THICKNESS, THICKNESS, m_height + 2.0f * THICKNESS);
    // Top boundary
    m_collisionBoxes.emplace_back(-THICKNESS, -THICKNESS, m_width + 2.0f * THICKNESS, THICKNESS);
    // Bottom boundary
    m_collisionBoxes.emplace_back(-THICKNESS, m_height, m_width + 2.0f * THICKNESS, THICKNESS);
}

void Map::draw(sf::RenderTarget& target, const sf::View& cameraView) const {
    sf::Vector2f center = cameraView.getCenter();
    sf::Vector2f size = cameraView.getSize();

    // Frustum Culling for Tiles: calculate visible tile column and row ranges
    float left = center.x - size.x * 0.5f - m_tileSize;
    float top = center.y - size.y * 0.5f - m_tileSize;
    float right = center.x + size.x * 0.5f + m_tileSize;
    float bottom = center.y + size.y * 0.5f + m_tileSize;

    int minCol = std::max(0, static_cast<int>(left / m_tileSize));
    int maxCol = std::min(m_cols - 1, static_cast<int>(right / m_tileSize));
    int minRow = std::max(0, static_cast<int>(top / m_tileSize));
    int maxRow = std::min(m_rows - 1, static_cast<int>(bottom / m_tileSize));

    // Draw only tiles currently within the viewport
    for (int r = minRow; r <= maxRow; ++r) {
        for (int c = minCol; c <= maxCol; ++c) {
            float posX = c * m_tileSize;
            float posY = r * m_tileSize;

            TileType type = m_tileGrid[r * m_cols + c];
            if (type == TileType::Asphalt) {
                // Non-const sprite drawing by setting position on our cached sprite
                const_cast<sf::Sprite&>(m_asphaltSprite).setPosition(posX, posY);
                target.draw(m_asphaltSprite);
            } else if (type == TileType::RoadLine) {
                const_cast<sf::Sprite&>(m_roadLineSprite).setPosition(posX, posY);
                target.draw(m_roadLineSprite);
            } else {
                const_cast<sf::Sprite&>(m_sidewalkSprite).setPosition(posX, posY);
                target.draw(m_sidewalkSprite);
            }
        }
    }

    // Frustum Culling for Obstacles: only draw obstacles visible on screen
    sf::FloatRect viewRect(center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y);
    for (const auto& obs : m_obstacles) {
        if (obs.bounds.intersects(viewRect)) {
            target.draw(obs.sprite);
        }
    }
}
