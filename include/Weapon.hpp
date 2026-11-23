#pragma once

#include <string>
#include <vector>
#include <SFML/System/Vector2.hpp>
#include "Bullet.hpp"

// Identification for weapon types
enum class WeaponType {
    Pistol,
    Shotgun,
    AssaultRifle
};

// Data definition defining the behavior and balance of each weapon
struct WeaponStats {
    std::string name;
    float damage;
    float fireRate;              // shots per second
    int magazineCapacity;
    int maxReserve;
    float reloadTime;            // in seconds
    float bulletSpeed;
    float maxRange;
    float baseSpreadDegrees;
    float maxSpreadDegrees;
    float spreadGrowthPerShot;
    float spreadRecoverySpeed;
    int pellets;                 // number of projectiles fired per shot
    bool isAutomatic;            // hold-to-fire vs semi-automatic click-per-shot
};

// Represents a firearm carried by the player.
// Manages fire cooldowns, current ammo, reserve ammo, reload sequences,
// spread expansion/recovery, and bullet generation.
class Weapon {
public:
    Weapon(WeaponType type, const WeaponStats& stats, bool unlockedByDefault);

    // Factory functions to create preconfigured weapons
    static Weapon createPistol();
    static Weapon createShotgun();
    static Weapon createAssaultRifle();

    // Advance timers (fire cooldown, reload timer, spread recovery)
    void update(float dt);

    // Checks if the weapon is ready to discharge a shot
    bool canFire() const;

    // Discharges weapon from muzzle, deducting ammo and returning generated bullets
    std::vector<Bullet> fire(sf::Vector2f muzzlePosition, sf::Vector2f aimDirection);

    // Initiates reload sequence. Returns true if reload started.
    bool startReload();

    // Cancels reload in progress (e.g., when switching weapons)
    void cancelReload();

    // Queries
    WeaponType getType() const { return m_type; }
    const std::string& getName() const { return m_stats.name; }
    int getMagazine() const { return m_currentMag; }
    int getMaxMagazine() const { return m_stats.magazineCapacity; }
    int getReserve() const { return m_currentReserve; }
    bool isReloading() const { return m_isReloading; }
    bool isAutomatic() const { return m_stats.isAutomatic; }
    bool isUnlocked() const { return m_isUnlocked; }
    void unlock() { m_isUnlocked = true; }

    // Returns reload progress from 0.0 (just started) to 1.0 (ready to finish)
    float getReloadProgress() const;

private:
    WeaponType m_type;
    WeaponStats m_stats;

    int m_currentMag;
    int m_currentReserve;
    float m_fireCooldownTimer;
    float m_reloadTimer;
    float m_currentSpreadDegrees;
    bool m_isReloading;
    bool m_isUnlocked;
};
