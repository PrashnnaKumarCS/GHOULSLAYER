#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class ResourceManager;

// Distinct zombie archetypes
enum class ZombieType {
    Walker,
    Runner,
    Tank
};

// Represents an infected enemy.
// All three types share this single class, driven by data parameters in Config.hpp.
// Features:
// - Direct pathfinding toward the player with obstacle collision and sliding
// - Type-specific behaviors: Walker (shambling), Runner (erratic wobble), Tank (windup smash & shotgun stagger)
// - Red hit flash when struck by bullets
// - Smooth death fade-out animation before removal
class Zombie {
public:
    Zombie(ZombieType type, sf::Vector2f spawnPosition, const ResourceManager& resources, float healthMultiplier = 1.0f, float speedMultiplier = 1.0f);

    // Update AI behavior, pathing, collision resolution, and animation
    void update(float dt, sf::Vector2f playerPos, bool playerIsAlive, const std::vector<sf::FloatRect>& obstacles);

    // Checks if the zombie is currently in range to inflict an attack on the player
    bool canAttack(sf::Vector2f playerPos, float playerRadius) const;

    // Executes the attack, resets the cooldown timer, and returns damage dealt
    float performAttack();

    // Inflict bullet damage on the zombie
    void applyDamage(float damage, bool isShotgunPellet);

    // Applies a soft push vector away from neighboring zombies to prevent clustering into a single blob
    void applySeparation(sf::Vector2f pushDelta, const std::vector<sf::FloatRect>& obstacles);

    // Render zombie to world space
    void draw(sf::RenderTarget& target) const;

    // Getters
    sf::Vector2f getPosition() const { return m_position; }
    float getRadius() const { return m_radius; }
    float getDamage() const { return m_damage; }
    float getHealth() const { return m_health; }
    bool isDead() const { return m_isDead; }
    bool isReadyForRemoval() const { return m_isReadyForRemoval; }
    ZombieType getType() const { return m_type; }

private:
    ZombieType m_type;
    sf::Vector2f m_position;
    sf::Vector2f m_moveDirection;

    float m_health;
    float m_maxHealth;
    float m_speed;
    float m_damage;
    float m_radius;
    float m_attackRange;
    float m_attackCooldown;
    float m_attackCooldownTimer;

    // Type-specific state timers
    float m_wobbleTimer;        // Runner lateral weave
    float m_windupTimer;        // Tank attack wind-up
    bool  m_isWindingUp;        // Tank is preparing a smash
    float m_staggerTimer;       // Tank stunned by shotgun blast

    // Visual effect timers
    float m_hitFlashTimer;
    float m_deathFadeTimer;
    bool  m_isDead;
    bool  m_isReadyForRemoval;

    // Animation frames and rendering
    std::vector<const sf::Texture*> m_animFrames;
    int m_currentFrame;
    float m_animTimer;
    float m_frameDuration;
    sf::Sprite m_sprite;
};
