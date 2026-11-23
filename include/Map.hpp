#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class ResourceManager;

// Represents an individual static visual obstacle with sprite and collision bounding box
struct Obstacle {
    sf::Sprite sprite;
    sf::FloatRect bounds;
};

// Represents the game world: 4000x4000 pixel dark apocalyptic city.
// Owns the tile grid, static obstacles, and collision rects.
// Implements view-based frustum culling so only tiles and obstacles
// visible on screen are submitted to the GPU.
class Map {
public:
    explicit Map(const ResourceManager& resources);

    // Render only visible tiles and obstacles within the camera's view
    void draw(sf::RenderTarget& target, const sf::View& cameraView) const;

    // Get all collision bounding boxes (obstacles and outer boundaries)
    const std::vector<sf::FloatRect>& getObstacles() const { return m_collisionBoxes; }

    // Map dimensions
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }

private:
    void generateWorldLayout(const ResourceManager& resources);
    void addBoundaryWalls();

    float m_width;
    float m_height;
    float m_tileSize;
    int m_cols;
    int m_rows;

    // Tile grid storing sprite and position for each map cell
    enum class TileType { Asphalt, RoadLine, Sidewalk };
    std::vector<TileType> m_tileGrid;

    // Cached sprites for rendering tiles
    sf::Sprite m_asphaltSprite;
    sf::Sprite m_roadLineSprite;
    sf::Sprite m_sidewalkSprite;

    // Static obstacles with graphics
    std::vector<Obstacle> m_obstacles;

    // All collision boxes (obstacles + outer map boundary walls)
    std::vector<sf::FloatRect> m_collisionBoxes;
};
