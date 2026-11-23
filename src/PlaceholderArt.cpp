#include "PlaceholderArt.hpp"
#include "ResourceManager.hpp"
#include "Config.hpp"
#include <cmath>
#include <vector>

void PlaceholderArt::generateAll(ResourceManager& resources) {
    generatePlayerWalkFrames(resources);
    generateTiles(resources);
    generateObstacles(resources);
    generateVignette(resources);
}

void PlaceholderArt::generatePlayerWalkFrames(ResourceManager& resources) {
    // 4 walking animation frames: 32x32 pixels each
    // Facing RIGHT (positive X axis, matching SFML's 0-degree rotation convention)
    const int W = 32;
    const int H = 32;

    for (int frame = 0; frame < 4; ++frame) {
        sf::Image img;
        img.create(W, H, sf::Color(0, 0, 0, 0)); // Transparent background

        // Leg offsets for the walking bob
        int leftLegOffsetX = (frame == 1) ? 2 : (frame == 3 ? -2 : 0);
        int rightLegOffsetX = (frame == 1) ? -2 : (frame == 3 ? 2 : 0);
        int bodyBobY = (frame % 2 == 1) ? 1 : 0;

        // Draw Boots / Legs
        sf::Color bootColor(35, 30, 25);
        for (int y = 7 + bodyBobY; y <= 10 + bodyBobY; ++y) {
            for (int x = 11 + leftLegOffsetX; x <= 14 + leftLegOffsetX; ++x) {
                img.setPixel(x, y, bootColor);
            }
        }
        for (int y = 21 + bodyBobY; y <= 24 + bodyBobY; ++y) {
            for (int x = 11 + rightLegOffsetX; x <= 14 + rightLegOffsetX; ++x) {
                img.setPixel(x, y, bootColor);
            }
        }

        // Draw Torso (Military Green Jacket)
        sf::Color jacketColor(50, 75, 45);
        sf::Color jacketShade(38, 58, 34);
        for (int y = 11 + bodyBobY; y <= 20 + bodyBobY; ++y) {
            for (int x = 10; x <= 17; ++x) {
                bool isBorder = (x == 10 || x == 17 || y == 11 + bodyBobY || y == 20 + bodyBobY);
                img.setPixel(x, y, isBorder ? jacketShade : jacketColor);
            }
        }

        // Draw Head (Hair and Skin Tone)
        sf::Color skinColor(225, 185, 150);
        sf::Color hairColor(65, 45, 30);
        for (int y = 13 + bodyBobY; y <= 18 + bodyBobY; ++y) {
            for (int x = 11; x <= 16; ++x) {
                img.setPixel(x, y, skinColor);
            }
        }
        // Hair cap on the back and top
        for (int y = 13 + bodyBobY; y <= 18 + bodyBobY; ++y) {
            img.setPixel(11, y, hairColor);
            img.setPixel(12, y, hairColor);
        }

        // Hands & Gun pointing to the right (positive X)
        sf::Color gunColor(45, 48, 52);
        sf::Color gunHighlight(75, 80, 85);
        sf::Color stockColor(90, 55, 30);

        // Right arm & hands holding shotgun
        for (int x = 17; x <= 20; ++x) {
            img.setPixel(x, 18 + bodyBobY, skinColor);
        }
        // Gun stock
        for (int x = 15; x <= 18; ++x) {
            img.setPixel(x, 19 + bodyBobY, stockColor);
        }
        // Gun barrel
        for (int x = 18; x <= 28; ++x) {
            img.setPixel(x, 19 + bodyBobY, gunColor);
            img.setPixel(x, 20 + bodyBobY, gunHighlight);
        }
        // Gun muzzle tip
        img.setPixel(29, 19 + bodyBobY, sf::Color(30, 30, 32));
        img.setPixel(29, 20 + bodyBobY, sf::Color(30, 30, 32));

        sf::Texture tex;
        tex.loadFromImage(img);
        tex.setSmooth(false); // Sharp, crisp retro pixels
        resources.insertTexture("player_" + std::to_string(frame), tex);
    }
}

void PlaceholderArt::generateTiles(ResourceManager& resources) {
    const unsigned int TILE_SIZE = 64;

    // 1. Dark cracked asphalt tile
    sf::Image asphalt;
    asphalt.create(TILE_SIZE, TILE_SIZE);
    for (unsigned int y = 0; y < TILE_SIZE; ++y) {
        for (unsigned int x = 0; x < TILE_SIZE; ++x) {
            // Subtle asphalt noise
            int noise = ((x * 13 + y * 29 + (x ^ y)) % 7) - 3;
            sf::Uint8 val = static_cast<sf::Uint8>(std::clamp(34 + noise, 0, 255));
            asphalt.setPixel(x, y, sf::Color(val, val + 1, val + 3));

            // Procedural cracks
            if ((x + y * 2) % 31 == 0 || (x * 2 + y) % 43 == 0) {
                asphalt.setPixel(x, y, sf::Color(16, 17, 19));
            }
        }
    }
    sf::Texture texAsphalt;
    texAsphalt.loadFromImage(asphalt);
    texAsphalt.setSmooth(false);
    resources.insertTexture("tile_asphalt", texAsphalt);

    // 2. Sidewalk concrete tile
    sf::Image sidewalk;
    sidewalk.create(TILE_SIZE, TILE_SIZE);
    for (unsigned int y = 0; y < TILE_SIZE; ++y) {
        for (unsigned int x = 0; x < TILE_SIZE; ++x) {
            int noise = ((x * 7 + y * 19) % 5) - 2;
            sf::Uint8 val = static_cast<sf::Uint8>(std::clamp(75 + noise, 0, 255));
            sidewalk.setPixel(x, y, sf::Color(val, val + 2, val + 2));

            // Tile grid border grooves
            if (x == 0 || y == 0 || x == TILE_SIZE - 1 || y == TILE_SIZE - 1) {
                sidewalk.setPixel(x, y, sf::Color(45, 46, 48));
            }
        }
    }
    sf::Texture texSidewalk;
    texSidewalk.loadFromImage(sidewalk);
    texSidewalk.setSmooth(false);
    resources.insertTexture("tile_sidewalk", texSidewalk);

    // 3. Road with dashed yellow line tile
    sf::Image roadLine;
    roadLine.create(TILE_SIZE, TILE_SIZE);
    for (unsigned int y = 0; y < TILE_SIZE; ++y) {
        for (unsigned int x = 0; x < TILE_SIZE; ++x) {
            roadLine.setPixel(x, y, asphalt.getPixel(x, y));
            // Dashed yellow center line (only middle 4 pixels wide and dash height 32)
            if (x >= 30 && x <= 33 && y >= 16 && y <= 48) {
                roadLine.setPixel(x, y, sf::Color(220, 185, 30));
            }
        }
    }
    sf::Texture texRoadLine;
    texRoadLine.loadFromImage(roadLine);
    texRoadLine.setSmooth(false);
    resources.insertTexture("tile_road_line", texRoadLine);
}

void PlaceholderArt::generateObstacles(ResourceManager& resources) {
    // 1. Ruined Abandoned Car (96x48)
    sf::Image car;
    car.create(96, 48, sf::Color(0, 0, 0, 0));
    sf::Color bodyColor(115, 48, 38);       // Rusted reddish-brown
    sf::Color bodyShade(80, 32, 25);
    sf::Color glassColor(65, 80, 92);        // Dusty cracked glass
    sf::Color tireColor(22, 22, 24);

    // 4 Tires
    for (int y = 2; y <= 8; ++y) {
        for (int x = 16; x <= 26; ++x) car.setPixel(x, y, tireColor);
        for (int x = 70; x <= 80; ++x) car.setPixel(x, y, tireColor);
    }
    for (int y = 39; y <= 45; ++y) {
        for (int x = 16; x <= 26; ++x) car.setPixel(x, y, tireColor);
        for (int x = 70; x <= 80; ++x) car.setPixel(x, y, tireColor);
    }

    // Car Body
    for (int y = 8; y <= 39; ++y) {
        for (int x = 8; x <= 87; ++x) {
            bool isEdge = (x == 8 || x == 87 || y == 8 || y == 39);
            car.setPixel(x, y, isEdge ? bodyShade : bodyColor);
        }
    }

    // Windshield and rear windows
    for (int y = 12; y <= 35; ++y) {
        for (int x = 32; x <= 42; ++x) car.setPixel(x, y, glassColor);
        for (int x = 60; x <= 68; ++x) car.setPixel(x, y, glassColor);
    }

    // Roof between windshields
    for (int y = 12; y <= 35; ++y) {
        for (int x = 43; x <= 59; ++x) {
            car.setPixel(x, y, bodyShade);
        }
    }

    sf::Texture texCar;
    texCar.loadFromImage(car);
    texCar.setSmooth(false);
    resources.insertTexture("obstacle_car", texCar);

    // 2. Concrete Road Barrier (64x32)
    sf::Image barrier;
    barrier.create(64, 32);
    for (unsigned int y = 0; y < 32; ++y) {
        for (unsigned int x = 0; x < 64; ++x) {
            bool isStripe = ((x + y) / 8) % 2 == 0;
            sf::Color c = isStripe ? sf::Color(170, 150, 40) : sf::Color(45, 45, 50);
            if (x == 0 || y == 0 || x == 63 || y == 31) c = sf::Color(25, 25, 28);
            barrier.setPixel(x, y, c);
        }
    }
    sf::Texture texBarrier;
    texBarrier.loadFromImage(barrier);
    texBarrier.setSmooth(false);
    resources.insertTexture("obstacle_barrier", texBarrier);

    // 3. Building Block (128x128)
    sf::Image building;
    building.create(128, 128);
    for (unsigned int y = 0; y < 128; ++y) {
        for (unsigned int x = 0; x < 128; ++x) {
            // Rooftop dark gravel texture
            int n = ((x * 11 + y * 23) % 7) - 3;
            sf::Uint8 val = static_cast<sf::Uint8>(std::clamp(28 + n, 0, 255));
            building.setPixel(x, y, sf::Color(val, val, val + 2));

            // Parapet wall border
            if (x <= 6 || x >= 121 || y <= 6 || y >= 121) {
                building.setPixel(x, y, sf::Color(16, 17, 19));
            }
        }
    }
    sf::Texture texBuilding;
    texBuilding.loadFromImage(building);
    texBuilding.setSmooth(false);
    resources.insertTexture("obstacle_building", texBuilding);
}

void PlaceholderArt::generateVignette(ResourceManager& resources) {
    // 512x512 smooth radial vignette
    const unsigned int SIZE = 512;
    const float center = SIZE / 2.0f;
    const float maxRadius = center;

    sf::Image vignette;
    vignette.create(SIZE, SIZE);

    for (unsigned int y = 0; y < SIZE; ++y) {
        for (unsigned int x = 0; x < SIZE; ++x) {
            float dx = static_cast<float>(x) - center;
            float dy = static_cast<float>(y) - center;
            float dist = std::sqrt(dx * dx + dy * dy);

            // Normalized distance: 0 at center, 1 at edges
            float norm = std::clamp(dist / maxRadius, 0.0f, 1.0f);

            // Clear center (radius up to 0.45) then smooth exponential falloff to edge
            float alpha = 0.0f;
            if (norm > 0.40f) {
                float factor = (norm - 0.40f) / 0.60f;
                alpha = std::clamp(factor * factor * 230.0f, 0.0f, 230.0f);
            }

            vignette.setPixel(x, y, sf::Color(0, 0, 0, static_cast<sf::Uint8>(alpha)));
        }
    }

    sf::Texture texVignette;
    texVignette.loadFromImage(vignette);
    texVignette.setSmooth(true); // Smooth gradient interpolation
    resources.insertTexture("vignette", texVignette);
}
