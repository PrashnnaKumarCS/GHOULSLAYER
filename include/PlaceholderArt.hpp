#pragma once

class ResourceManager;

// PlaceholderArt generates all required game textures directly in code
// using pixel-by-pixel rendering into sf::Image, then stores them in ResourceManager.
// This allows the entire game to look like a gritty top-down retro pixel game
// without needing any external asset files downloaded.
class PlaceholderArt {
public:
    // Generates and registers all textures needed for Stage 1
    static void generateAll(ResourceManager& resources);

private:
    static void generatePlayerWalkFrames(ResourceManager& resources);
    static void generateZombieWalkFrames(ResourceManager& resources);
    static void generateTiles(ResourceManager& resources);
    static void generateObstacles(ResourceManager& resources);
    static void generateVignette(ResourceManager& resources);
};
