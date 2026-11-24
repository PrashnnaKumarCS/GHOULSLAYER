#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Config.hpp"
#include "Weapon.hpp"
#include "Bullet.hpp"

class ResourceManager;

// Enables debug shortcuts (F1 to unlock all weapons)
#define DEBUG_FEATURES 1

// Player represents the playable survivor.
// Features:
// - 8-directional WASD movement with diagonal normalization and wall sliding
// - 360-degree rotation aiming toward mouse cursor in world coordinates
// - Weapon inventory (Pistol, Shotgun, Assault Rifle) with unlocking and switching (1, 2, 3, Wheel)
// - Semi-auto and full-auto shooting from gun muzzle with spread and cooldown
// - Reloading sequence (R key and auto-reload on empty) with visual progress indicator
// - Muzzle flash rendering
class Player {
public:
    explicit Player(const ResourceManager& resources);

    // Process discrete events: weapon switching (1, 2, 3, Wheel), reload (R), debug unlock (F1), clicks
    void handleEvent(const sf::Event& event);

    // Update movement, collision, mouse aiming, weapon state, and shooting
    void update(float dt, const std::vector<sf::FloatRect>& obstacles, sf::Vector2f worldMousePos);

    // Render player sprite, muzzle flash, and reload progress bar in world space
    void draw(sf::RenderTarget& target) const;

    // Retrieve bullets created during this frame to add to the world simulation
    std::vector<Bullet> extractPendingBullets();

    // Inflict damage upon the player, enforcing invulnerability frames
    void takeDamage(float damage);

    // Health and ammo restoration from pickups
    void restoreHealth(float amount);
    void addAmmo(int amount);

    // Progression: unlock specific weapon and grant starter ammo
    void unlockWeaponType(WeaponType type, int starterAmmo);

    // Getters
    sf::Vector2f getPosition() const { return m_position; }
    float getHealth() const { return m_health; }
    float getMaxHealth() const { return Config::Player::MAX_HEALTH; }
    float getRadius() const { return Config::Player::COLLISION_RADIUS; }
    float getDamageFlashTimer() const { return m_damageFlashTimer; }
    bool isDead() const { return m_isDead; }
    const sf::FloatRect& getCollisionBox() const { return m_collisionBox; }
    const Weapon& getCurrentWeapon() const { return m_weapons[m_currentWeaponIndex]; }

    // Weapon switching and debug unlocking
    void selectWeapon(int index);
    void cycleWeapon(int step);
    void unlockAllWeapons();

private:
    void updateCollisionBox();
    sf::Vector2f calculateMuzzlePosition() const;

    sf::Vector2f m_position;
    sf::Vector2f m_aimDirection;
    float m_health;
    bool m_isMoving;
    bool m_isDead;
    bool m_deathLogged;
    float m_invulnerabilityTimer;
    float m_damageFlashTimer;

    // Bounding box for world collision
    sf::FloatRect m_collisionBox;
    const float m_halfSize = 12.0f;

    // Animation frames and timer
    std::vector<const sf::Texture*> m_frameTextures;
    int m_currentFrame;
    float m_animTimer;
    sf::Sprite m_sprite;

    // Weapon inventory
    std::vector<Weapon> m_weapons;
    int m_currentWeaponIndex;
    bool m_triggerSemiAuto; // Single-shot trigger on mouse press

    // Newly spawned bullets waiting to enter the game world
    std::vector<Bullet> m_pendingBullets;

    // Muzzle flash visual state
    float m_muzzleFlashTimer;
    sf::Vector2f m_muzzleFlashPos;
};
