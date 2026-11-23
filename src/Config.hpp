#pragma once
#include <string>
#include <SFML/Graphics/Color.hpp>

// Central configuration file for GHOULSLAYER.
// All tunable numbers, gameplay speeds, dimensions, and settings
// are kept here in one place so they can easily be tweaked.
namespace Config {

    namespace Window {
        constexpr unsigned int WIDTH = 1280;
        constexpr unsigned int HEIGHT = 720;
        constexpr unsigned int FPS_LIMIT = 60;
        constexpr bool VSYNC_ENABLED = true;
        inline const std::string TITLE = "GHOULSLAYER - RISE. FIGHT. SURVIVE.";
    }

    namespace Engine {
        // Delta time clamp to prevent physics jumps or tunneling during lag spikes or window drags
        constexpr float MAX_DELTA_TIME = 0.05f;
    }

    namespace Map {
        constexpr float WIDTH = 4000.0f;
        constexpr float HEIGHT = 4000.0f;
        constexpr float TILE_SIZE = 64.0f;
    }

    namespace Camera {
        // How fast the camera glides toward the player (higher = snappier)
        constexpr float LERP_SPEED = 6.0f;
    }

    namespace Player {
        constexpr float START_X = 2000.0f; // Center of the map
        constexpr float START_Y = 2000.0f;
        constexpr float SPEED = 250.0f;     // Movement speed in pixels per second
        constexpr float MAX_HEALTH = 100.0f;
        constexpr float COLLISION_RADIUS = 16.0f; // Radius for bounding box
        constexpr float ANIM_FRAME_TIME = 0.12f;  // Seconds per walking animation frame
    }

    namespace Darkness {
        // Semi-transparent atmospheric tint
        inline const sf::Color OVERLAY_COLOR = sf::Color(10, 10, 18, 120);
        // Vignette intensity
        inline const sf::Color VIGNETTE_COLOR = sf::Color(0, 0, 0, 200);
    }

} // namespace Config
