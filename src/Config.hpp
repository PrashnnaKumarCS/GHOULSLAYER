#pragma once
#include <string>
#include <SFML/Graphics/Color.hpp>

// Central configuration file for GHOULSLAYER.
// All tunable numbers, gameplay speeds, dimensions, weapon stats,
// zombie stats, and engine settings are kept here in one place.
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
        constexpr float COLLISION_RADIUS = 14.0f; // Radius for bounding box
        constexpr float ANIM_FRAME_TIME = 0.12f;  // Seconds per walking animation frame

        // Gun muzzle offset from player center (forward along aim, right perpendicular)
        constexpr float MUZZLE_FORWARD_OFFSET = 20.0f;
        constexpr float MUZZLE_SIDE_OFFSET = 6.0f;

        // Combat & Damage response
        constexpr float INVULNERABILITY_DURATION = 0.40f; // Seconds of invulnerability after being struck
        constexpr float DAMAGE_FLASH_DURATION = 0.30f;    // Seconds of red screen flash
    }

    namespace Weapons {
        namespace Pistol {
            inline const std::string NAME = "Pistol";
            constexpr float DAMAGE = 25.0f;
            constexpr float FIRE_RATE = 3.5f;          // ~3.5 shots per second
            constexpr int MAGAZINE_CAPACITY = 12;
            constexpr int START_RESERVE = 48;
            constexpr float RELOAD_TIME = 1.2f;        // 1.2 seconds
            constexpr float BULLET_SPEED = 950.0f;     // pixels per second
            constexpr float MAX_RANGE = 1100.0f;
            constexpr float SPREAD_DEGREES = 2.5f;     // small spread
            constexpr int PELLETS = 1;
            constexpr bool IS_AUTOMATIC = false;       // semi-auto (click per shot)
        }

        namespace Shotgun {
            inline const std::string NAME = "Shotgun";
            constexpr float DAMAGE = 12.0f;            // 12 per pellet
            constexpr float FIRE_RATE = 1.0f;          // ~1 shot per second
            constexpr int MAGAZINE_CAPACITY = 6;
            constexpr int START_RESERVE = 24;
            constexpr float RELOAD_TIME = 2.0f;        // 2.0 seconds
            constexpr float BULLET_SPEED = 800.0f;
            constexpr float MAX_RANGE = 480.0f;        // short range
            constexpr float SPREAD_DEGREES = 14.0f;    // wide spread cone
            constexpr int PELLETS = 6;                 // 6 pellets per blast
            constexpr bool IS_AUTOMATIC = false;       // semi-auto
        }

        namespace AssaultRifle {
            inline const std::string NAME = "Assault Rifle";
            constexpr float DAMAGE = 18.0f;
            constexpr float FIRE_RATE = 9.0f;          // ~9 shots per second
            constexpr int MAGAZINE_CAPACITY = 30;
            constexpr int START_RESERVE = 90;
            constexpr float RELOAD_TIME = 1.8f;        // 1.8 seconds
            constexpr float BULLET_SPEED = 1050.0f;
            constexpr float MAX_RANGE = 1300.0f;
            constexpr float BASE_SPREAD_DEGREES = 2.0f;
            constexpr float MAX_SPREAD_DEGREES = 7.5f;
            constexpr float SPREAD_GROWTH_PER_SHOT = 0.9f;
            constexpr float SPREAD_RECOVERY_SPEED = 15.0f; // degrees per second
            constexpr int PELLETS = 1;
            constexpr bool IS_AUTOMATIC = true;        // full-auto (hold left mouse)
        }
    }

    namespace BulletVisuals {
        constexpr float TRAIL_LENGTH = 16.0f;
        constexpr float TRAIL_THICKNESS = 3.0f;
        inline const sf::Color COLOR_HEAD = sf::Color(255, 255, 200);
        inline const sf::Color COLOR_TAIL = sf::Color(255, 160, 40, 180);
    }

    namespace MuzzleFlash {
        constexpr float DURATION = 0.05f; // seconds flash stays visible
        constexpr float RADIUS = 10.0f;
        inline const sf::Color COLOR = sf::Color(255, 220, 100);
    }

    namespace Zombies {
        namespace Walker {
            inline const std::string NAME = "Walker";
            constexpr float SPEED = 60.0f;             // ~60 px/s
            constexpr float MAX_HEALTH = 60.0f;        // 60 HP
            constexpr float DAMAGE = 10.0f;            // 10 damage per hit
            constexpr float RADIUS = 15.0f;            // standard size
            constexpr float ATTACK_RANGE = 32.0f;
            constexpr float ATTACK_COOLDOWN = 1.0f;    // seconds between attacks
            constexpr float ANIM_FRAME_TIME = 0.18f;
        }

        namespace Runner {
            inline const std::string NAME = "Runner";
            constexpr float SPEED = 150.0f;            // ~150 px/s (fast)
            constexpr float MAX_HEALTH = 35.0f;        // 35 HP (fragile)
            constexpr float DAMAGE = 8.0f;             // 8 damage per hit
            constexpr float RADIUS = 12.0f;            // smaller size
            constexpr float ATTACK_RANGE = 28.0f;
            constexpr float ATTACK_COOLDOWN = 0.6f;    // fast attack
            constexpr float WOBBLE_FREQUENCY = 6.0f;   // erratic movement speed
            constexpr float WOBBLE_AMPLITUDE = 35.0f;  // lateral sway
            constexpr float ANIM_FRAME_TIME = 0.10f;
        }

        namespace Tank {
            inline const std::string NAME = "Tank";
            constexpr float SPEED = 45.0f;             // ~45 px/s (slow brute)
            constexpr float MAX_HEALTH = 400.0f;       // 400 HP (bullet sponge)
            constexpr float DAMAGE = 30.0f;            // 30 damage per heavy smash
            constexpr float RADIUS = 26.0f;            // large size
            constexpr float ATTACK_RANGE = 44.0f;
            constexpr float ATTACK_COOLDOWN = 2.0f;    // slower attack
            constexpr float WINDUP_TIME = 0.50f;       // visible attack preparation
            constexpr float SHOTGUN_STAGGER_TIME = 0.35f; // brief stagger when hit by shotgun
            constexpr float ANIM_FRAME_TIME = 0.24f;
        }

        // Shared zombie behavior settings
        constexpr float SEPARATION_FORCE = 70.0f;       // Soft push-apart speed between colliding zombies
        constexpr float HIT_FLASH_DURATION = 0.08f;     // Red/white damage flash duration
        constexpr float DEATH_FADE_DURATION = 0.50f;    // Fade out duration before removal
    }

    namespace Blood {
        constexpr int PARTICLES_PER_HIT = 7;
        constexpr float MIN_SPEED = 50.0f;
        constexpr float MAX_SPEED = 180.0f;
        constexpr float LIFETIME = 0.35f;
        constexpr float PARTICLE_SIZE = 3.5f;
        inline const sf::Color COLOR = sf::Color(180, 20, 20);
    }

    namespace Darkness {
        // Semi-transparent atmospheric tint
        inline const sf::Color OVERLAY_COLOR = sf::Color(10, 10, 18, 120);
        // Vignette intensity
        inline const sf::Color VIGNETTE_COLOR = sf::Color(0, 0, 0, 200);
        // Red screen-edge damage flash
        inline const sf::Color DAMAGE_FLASH_COLOR = sf::Color(180, 0, 0, 110);
    }

} // namespace Config
