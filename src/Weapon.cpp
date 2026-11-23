#include "Weapon.hpp"
#include "Config.hpp"
#include <cmath>
#include <random>
#include <algorithm>

namespace {
    // Thread-local pseudo-random generator for bullet spread calculations
    std::mt19937& getRandomEngine() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        return gen;
    }

    constexpr float DEG_TO_RAD = 3.1415926535f / 180.0f;
}

Weapon::Weapon(WeaponType type, const WeaponStats& stats, bool unlockedByDefault)
    : m_type(type)
    , m_stats(stats)
    , m_currentMag(stats.magazineCapacity)
    , m_currentReserve(stats.maxReserve)
    , m_fireCooldownTimer(0.0f)
    , m_reloadTimer(0.0f)
    , m_currentSpreadDegrees(stats.baseSpreadDegrees)
    , m_isReloading(false)
    , m_isUnlocked(unlockedByDefault)
{
}

Weapon Weapon::createPistol() {
    WeaponStats stats;
    stats.name = Config::Weapons::Pistol::NAME;
    stats.damage = Config::Weapons::Pistol::DAMAGE;
    stats.fireRate = Config::Weapons::Pistol::FIRE_RATE;
    stats.magazineCapacity = Config::Weapons::Pistol::MAGAZINE_CAPACITY;
    stats.maxReserve = Config::Weapons::Pistol::START_RESERVE;
    stats.reloadTime = Config::Weapons::Pistol::RELOAD_TIME;
    stats.bulletSpeed = Config::Weapons::Pistol::BULLET_SPEED;
    stats.maxRange = Config::Weapons::Pistol::MAX_RANGE;
    stats.baseSpreadDegrees = Config::Weapons::Pistol::SPREAD_DEGREES;
    stats.maxSpreadDegrees = Config::Weapons::Pistol::SPREAD_DEGREES;
    stats.spreadGrowthPerShot = 0.0f;
    stats.spreadRecoverySpeed = 0.0f;
    stats.pellets = Config::Weapons::Pistol::PELLETS;
    stats.isAutomatic = Config::Weapons::Pistol::IS_AUTOMATIC;

    // Player starts with Pistol unlocked
    return Weapon(WeaponType::Pistol, stats, true);
}

Weapon Weapon::createShotgun() {
    WeaponStats stats;
    stats.name = Config::Weapons::Shotgun::NAME;
    stats.damage = Config::Weapons::Shotgun::DAMAGE;
    stats.fireRate = Config::Weapons::Shotgun::FIRE_RATE;
    stats.magazineCapacity = Config::Weapons::Shotgun::MAGAZINE_CAPACITY;
    stats.maxReserve = Config::Weapons::Shotgun::START_RESERVE;
    stats.reloadTime = Config::Weapons::Shotgun::RELOAD_TIME;
    stats.bulletSpeed = Config::Weapons::Shotgun::BULLET_SPEED;
    stats.maxRange = Config::Weapons::Shotgun::MAX_RANGE;
    stats.baseSpreadDegrees = Config::Weapons::Shotgun::SPREAD_DEGREES;
    stats.maxSpreadDegrees = Config::Weapons::Shotgun::SPREAD_DEGREES;
    stats.spreadGrowthPerShot = 0.0f;
    stats.spreadRecoverySpeed = 0.0f;
    stats.pellets = Config::Weapons::Shotgun::PELLETS;
    stats.isAutomatic = Config::Weapons::Shotgun::IS_AUTOMATIC;

    // Shotgun starts locked until unlocked in gameplay or via debug F1
    return Weapon(WeaponType::Shotgun, stats, false);
}

Weapon Weapon::createAssaultRifle() {
    WeaponStats stats;
    stats.name = Config::Weapons::AssaultRifle::NAME;
    stats.damage = Config::Weapons::AssaultRifle::DAMAGE;
    stats.fireRate = Config::Weapons::AssaultRifle::FIRE_RATE;
    stats.magazineCapacity = Config::Weapons::AssaultRifle::MAGAZINE_CAPACITY;
    stats.maxReserve = Config::Weapons::AssaultRifle::START_RESERVE;
    stats.reloadTime = Config::Weapons::AssaultRifle::RELOAD_TIME;
    stats.bulletSpeed = Config::Weapons::AssaultRifle::BULLET_SPEED;
    stats.maxRange = Config::Weapons::AssaultRifle::MAX_RANGE;
    stats.baseSpreadDegrees = Config::Weapons::AssaultRifle::BASE_SPREAD_DEGREES;
    stats.maxSpreadDegrees = Config::Weapons::AssaultRifle::MAX_SPREAD_DEGREES;
    stats.spreadGrowthPerShot = Config::Weapons::AssaultRifle::SPREAD_GROWTH_PER_SHOT;
    stats.spreadRecoverySpeed = Config::Weapons::AssaultRifle::SPREAD_RECOVERY_SPEED;
    stats.pellets = Config::Weapons::AssaultRifle::PELLETS;
    stats.isAutomatic = Config::Weapons::AssaultRifle::IS_AUTOMATIC;

    // Assault Rifle starts locked until unlocked in gameplay or via debug F1
    return Weapon(WeaponType::AssaultRifle, stats, false);
}

void Weapon::update(float dt) {
    // 1. Advance fire rate cooldown
    if (m_fireCooldownTimer > 0.0f) {
        m_fireCooldownTimer = std::max(0.0f, m_fireCooldownTimer - dt);
    }

    // 2. Recover weapon spread over time when not firing
    if (m_currentSpreadDegrees > m_stats.baseSpreadDegrees) {
        m_currentSpreadDegrees = std::max(
            m_stats.baseSpreadDegrees,
            m_currentSpreadDegrees - m_stats.spreadRecoverySpeed * dt
        );
    }

    // 3. Process ongoing reload sequence
    if (m_isReloading) {
        m_reloadTimer -= dt;
        if (m_reloadTimer <= 0.0f) {
            // Reload completed: move missing ammo from reserve to magazine
            int needed = m_stats.magazineCapacity - m_currentMag;
            int toTransfer = std::min(needed, m_currentReserve);
            m_currentMag += toTransfer;
            m_currentReserve -= toTransfer;
            m_isReloading = false;
            m_reloadTimer = 0.0f;
        }
    }
}

bool Weapon::canFire() const {
    return !m_isReloading && (m_fireCooldownTimer <= 0.0f) && (m_currentMag > 0);
}

std::vector<Bullet> Weapon::fire(sf::Vector2f muzzlePosition, sf::Vector2f aimDirection) {
    std::vector<Bullet> bullets;
    if (!canFire()) return bullets;

    // Reset fire rate cooldown
    m_fireCooldownTimer = 1.0f / m_stats.fireRate;

    // Deduct one round from magazine
    m_currentMag--;

    // Increase spread under sustained fire (clamped to max spread)
    m_currentSpreadDegrees = std::min(
        m_stats.maxSpreadDegrees,
        m_currentSpreadDegrees + m_stats.spreadGrowthPerShot
    );

    // Calculate base aim angle in radians
    float baseAngleRad = std::atan2(aimDirection.y, aimDirection.x);

    // Uniform distribution for spread angle offsets
    std::uniform_real_distribution<float> spreadDist(
        -m_currentSpreadDegrees * DEG_TO_RAD,
        m_currentSpreadDegrees * DEG_TO_RAD
    );

    // Spawn pellets (1 for pistol/rifle, 6 for shotgun)
    bool isShotgun = (m_type == WeaponType::Shotgun);
    for (int i = 0; i < m_stats.pellets; ++i) {
        float pelletAngleRad = baseAngleRad + spreadDist(getRandomEngine());
        sf::Vector2f pelletDir(std::cos(pelletAngleRad), std::sin(pelletAngleRad));

        bullets.emplace_back(
            muzzlePosition,
            pelletDir,
            m_stats.bulletSpeed,
            m_stats.damage,
            m_stats.maxRange,
            isShotgun
        );
    }

    // Auto-reload on empty magazine if reserve ammo is available
    if (m_currentMag == 0 && m_currentReserve > 0) {
        startReload();
    }

    return bullets;
}

bool Weapon::startReload() {
    // Can only reload if not currently reloading, magazine isn't already full, and reserve ammo exists
    if (!m_isReloading && m_currentMag < m_stats.magazineCapacity && m_currentReserve > 0) {
        m_isReloading = true;
        m_reloadTimer = m_stats.reloadTime;
        return true;
    }
    return false;
}

void Weapon::cancelReload() {
    m_isReloading = false;
    m_reloadTimer = 0.0f;
}

float Weapon::getReloadProgress() const {
    if (!m_isReloading || m_stats.reloadTime <= 0.0f) return 0.0f;
    return std::clamp(1.0f - (m_reloadTimer / m_stats.reloadTime), 0.0f, 1.0f);
}
