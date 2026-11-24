#include "Zombie.hpp"
#include "ResourceManager.hpp"
#include "CollisionSystem.hpp"
#include "Config.hpp"
#include <cmath>
#include <algorithm>

Zombie::Zombie(ZombieType type, sf::Vector2f spawnPosition, const ResourceManager& resources, float healthMultiplier, float speedMultiplier)
    : m_type(type)
    , m_position(spawnPosition)
    , m_moveDirection(1.0f, 0.0f)
    , m_attackCooldownTimer(0.0f)
    , m_wobbleTimer(0.0f)
    , m_windupTimer(0.0f)
    , m_isWindingUp(false)
    , m_staggerTimer(0.0f)
    , m_hitFlashTimer(0.0f)
    , m_deathFadeTimer(Config::Zombies::DEATH_FADE_DURATION)
    , m_isDead(false)
    , m_isReadyForRemoval(false)
    , m_currentFrame(0)
    , m_animTimer(0.0f)
{
    std::string prefix;
    float originOffset = 16.0f;

    switch (m_type) {
        case ZombieType::Walker:
            m_speed = Config::Zombies::Walker::SPEED * speedMultiplier;
            m_health = m_maxHealth = Config::Zombies::Walker::MAX_HEALTH * healthMultiplier;
            m_damage = Config::Zombies::Walker::DAMAGE;
            m_radius = Config::Zombies::Walker::RADIUS;
            m_attackRange = Config::Zombies::Walker::ATTACK_RANGE;
            m_attackCooldown = Config::Zombies::Walker::ATTACK_COOLDOWN;
            m_frameDuration = Config::Zombies::Walker::ANIM_FRAME_TIME;
            prefix = "zombie_walker_";
            originOffset = 16.0f; // 32x32 center
            break;

        case ZombieType::Runner:
            m_speed = Config::Zombies::Runner::SPEED * speedMultiplier;
            m_health = m_maxHealth = Config::Zombies::Runner::MAX_HEALTH * healthMultiplier;
            m_damage = Config::Zombies::Runner::DAMAGE;
            m_radius = Config::Zombies::Runner::RADIUS;
            m_attackRange = Config::Zombies::Runner::ATTACK_RANGE;
            m_attackCooldown = Config::Zombies::Runner::ATTACK_COOLDOWN;
            m_frameDuration = Config::Zombies::Runner::ANIM_FRAME_TIME;
            prefix = "zombie_runner_";
            originOffset = 14.0f; // 28x28 center
            break;

        case ZombieType::Tank:
            m_speed = Config::Zombies::Tank::SPEED * speedMultiplier;
            m_health = m_maxHealth = Config::Zombies::Tank::MAX_HEALTH * healthMultiplier;
            m_damage = Config::Zombies::Tank::DAMAGE;
            m_radius = Config::Zombies::Tank::RADIUS;
            m_attackRange = Config::Zombies::Tank::ATTACK_RANGE;
            m_attackCooldown = Config::Zombies::Tank::ATTACK_COOLDOWN;
            m_frameDuration = Config::Zombies::Tank::ANIM_FRAME_TIME;
            prefix = "zombie_tank_";
            originOffset = 28.0f; // 56x56 center
            break;
    }

    // Cache animation textures
    for (int i = 0; i < 4; ++i) {
        m_animFrames.push_back(&resources.getTexture(prefix + std::to_string(i)));
    }

    if (!m_animFrames.empty() && m_animFrames[0] != nullptr) {
        m_sprite.setTexture(*m_animFrames[0]);
    }
    m_sprite.setOrigin(originOffset, originOffset);
    m_sprite.setPosition(m_position);
}

void Zombie::applyDamage(float damage, bool isShotgunPellet) {
    if (m_isDead) return;

    m_health -= damage;
    m_hitFlashTimer = Config::Zombies::HIT_FLASH_DURATION;

    // Shotgun blast staggers the Tank
    if (m_type == ZombieType::Tank && isShotgunPellet) {
        m_staggerTimer = Config::Zombies::Tank::SHOTGUN_STAGGER_TIME;
        m_isWindingUp = false; // Interrupts attack windup
    }

    if (m_health <= 0.0f) {
        m_health = 0.0f;
        m_isDead = true;
        m_deathFadeTimer = Config::Zombies::DEATH_FADE_DURATION;
    }
}

bool Zombie::canAttack(sf::Vector2f playerPos, float playerRadius) const {
    if (m_isDead || m_staggerTimer > 0.0f) return false;

    float distSq = (playerPos.x - m_position.x) * (playerPos.x - m_position.x)
                 + (playerPos.y - m_position.y) * (playerPos.y - m_position.y);
    float reach = m_attackRange + playerRadius;

    if (distSq <= (reach * reach)) {
        if (m_type == ZombieType::Tank) {
            // Tank attacks connect only when the wind-up completes
            return m_isWindingUp && m_windupTimer <= 0.0f;
        } else {
            return m_attackCooldownTimer <= 0.0f;
        }
    }
    return false;
}

float Zombie::performAttack() {
    m_attackCooldownTimer = m_attackCooldown;
    m_isWindingUp = false;
    m_windupTimer = 0.0f;
    return m_damage;
}

void Zombie::applySeparation(sf::Vector2f pushDelta, const std::vector<sf::FloatRect>& obstacles) {
    if (m_isDead) return;

    m_position += pushDelta;

    // Soft check against obstacles to prevent separation pushing zombies into walls
    for (const auto& obs : obstacles) {
        if (CollisionSystem::checkRectCircle(obs, m_position, m_radius)) {
            m_position -= pushDelta;
            break;
        }
    }
    m_sprite.setPosition(m_position);
}

void Zombie::update(float dt, sf::Vector2f playerPos, bool playerIsAlive, const std::vector<sf::FloatRect>& obstacles) {
    // 1. Death fade animation handling
    if (m_isDead) {
        m_deathFadeTimer -= dt;
        if (m_deathFadeTimer <= 0.0f) {
            m_isReadyForRemoval = true;
        }
        return;
    }

    // 2. Cooldown and effect timer decrements
    if (m_attackCooldownTimer > 0.0f) {
        m_attackCooldownTimer -= dt;
    }
    if (m_hitFlashTimer > 0.0f) {
        m_hitFlashTimer -= dt;
    }
    if (m_staggerTimer > 0.0f) {
        m_staggerTimer -= dt;
        return; // Pauses movement while staggered
    }

    // 3. Stop pursuing if player is dead
    if (!playerIsAlive) {
        return;
    }

    // 4. Direction calculation toward player (guarded against division by zero)
    sf::Vector2f toPlayer = playerPos - m_position;
    float dist = std::hypot(toPlayer.x, toPlayer.y);
    if (dist > 0.0001f) {
        m_moveDirection = toPlayer / dist;
    }

    // 5. Tank attack wind-up initiation
    if (m_type == ZombieType::Tank) {
        if (dist <= m_attackRange + 14.0f && m_attackCooldownTimer <= 0.0f && !m_isWindingUp) {
            m_isWindingUp = true;
            m_windupTimer = Config::Zombies::Tank::WINDUP_TIME;
        }

        if (m_isWindingUp) {
            m_windupTimer -= dt;
            // During windup, the Tank slows down significantly to telegraph the punch
            dt *= 0.20f;
        }
    }

    // 6. Runner erratic movement (sinusoidal lateral wobble)
    sf::Vector2f actualMove = m_moveDirection;
    if (m_type == ZombieType::Runner) {
        m_wobbleTimer += dt * Config::Zombies::Runner::WOBBLE_FREQUENCY;
        sf::Vector2f perp(-m_moveDirection.y, m_moveDirection.x);
        float swayFactor = std::sin(m_wobbleTimer) * (Config::Zombies::Runner::WOBBLE_AMPLITUDE / m_speed);
        actualMove = m_moveDirection + perp * swayFactor;
        float actualLen = std::hypot(actualMove.x, actualMove.y);
        if (actualLen > 0.0001f) actualMove /= actualLen;
    }

    // 7. Axis-Separated Movement & Collision Resolution (slide along obstacles)
    // --- Step X ---
    if (actualMove.x != 0.0f) {
        m_position.x += actualMove.x * m_speed * dt;
        for (const auto& obs : obstacles) {
            if (CollisionSystem::checkRectCircle(obs, m_position, m_radius)) {
                if (actualMove.x > 0.0f) {
                    m_position.x = obs.left - m_radius;
                } else if (actualMove.x < 0.0f) {
                    m_position.x = obs.left + obs.width + m_radius;
                }
            }
        }
    }

    // --- Step Y ---
    if (actualMove.y != 0.0f) {
        m_position.y += actualMove.y * m_speed * dt;
        for (const auto& obs : obstacles) {
            if (CollisionSystem::checkRectCircle(obs, m_position, m_radius)) {
                if (actualMove.y > 0.0f) {
                    m_position.y = obs.top - m_radius;
                } else if (actualMove.y < 0.0f) {
                    m_position.y = obs.top + obs.height + m_radius;
                }
            }
        }
    }

    // 8. Clamp within map boundaries
    m_position.x = std::clamp(m_position.x, m_radius, Config::Map::WIDTH - m_radius);
    m_position.y = std::clamp(m_position.y, m_radius, Config::Map::HEIGHT - m_radius);

    // 9. Rotation toward movement direction
    float angleDeg = std::atan2(m_moveDirection.y, m_moveDirection.x) * (180.0f / 3.1415926535f);
    m_sprite.setRotation(angleDeg);

    // 10. Walk animation step
    m_animTimer += dt;
    if (m_animTimer >= m_frameDuration) {
        m_animTimer = 0.0f;
        m_currentFrame = (m_currentFrame + 1) % static_cast<int>(m_animFrames.size());
        m_sprite.setTexture(*m_animFrames[m_currentFrame]);
    }

    m_sprite.setPosition(m_position);
}

void Zombie::draw(sf::RenderTarget& target) const {
    // Non-const tinting for visual damage & death feedback
    sf::Sprite renderSprite = m_sprite;

    if (m_isDead) {
        // Fade out during death animation
        float alphaPct = std::clamp(m_deathFadeTimer / Config::Zombies::DEATH_FADE_DURATION, 0.0f, 1.0f);
        renderSprite.setColor(sf::Color(100, 100, 100, static_cast<sf::Uint8>(alphaPct * 200.0f)));
    } else if (m_hitFlashTimer > 0.0f) {
        // Red hit flash on taking damage
        renderSprite.setColor(sf::Color(255, 70, 70));
    } else if (m_isWindingUp) {
        // Bright yellow/orange warning flash during Tank attack wind-up
        renderSprite.setColor(sf::Color(255, 210, 80));
    } else if (m_staggerTimer > 0.0f) {
        // Dim blueish tint while staggered by shotgun
        renderSprite.setColor(sf::Color(120, 160, 255));
    }

    target.draw(renderSprite);
}
