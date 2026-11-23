#include "Player.hpp"
#include "ResourceManager.hpp"
#include "Config.hpp"
#include <cmath>
#include <algorithm>

Player::Player(const ResourceManager& resources)
    : m_position(Config::Player::START_X, Config::Player::START_Y)
    , m_health(Config::Player::MAX_HEALTH)
    , m_isMoving(false)
    , m_currentFrame(0)
    , m_animTimer(0.0f)
{
    // Cache the 4 walking animation frames
    for (int i = 0; i < 4; ++i) {
        m_frameTextures.push_back(&resources.getTexture("player_" + std::to_string(i)));
    }

    // Configure sprite
    if (!m_frameTextures.empty() && m_frameTextures[0] != nullptr) {
        m_sprite.setTexture(*m_frameTextures[0]);
    }
    // Origin at center (16, 16) for symmetrical 360-degree rotation
    m_sprite.setOrigin(16.0f, 16.0f);
    m_sprite.setPosition(m_position);

    updateCollisionBox();
}

void Player::updateCollisionBox() {
    m_collisionBox = sf::FloatRect(
        m_position.x - m_halfSize,
        m_position.y - m_halfSize,
        m_halfSize * 2.0f,
        m_halfSize * 2.0f
    );
}

void Player::update(float dt, const std::vector<sf::FloatRect>& obstacles, sf::Vector2f worldMousePos) {
    // 1. Gather directional input from WASD keys
    sf::Vector2f moveDir(0.0f, 0.0f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) moveDir.y -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) moveDir.y += 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) moveDir.x -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) moveDir.x += 1.0f;

    // 2. Normalize input direction (guard against division by zero on idle)
    float lengthSq = moveDir.x * moveDir.x + moveDir.y * moveDir.y;
    if (lengthSq > 0.0001f) {
        float invLength = 1.0f / std::sqrt(lengthSq);
        moveDir.x *= invLength;
        moveDir.y *= invLength;
        m_isMoving = true;
    } else {
        m_isMoving = false;
    }

    // 3. Axis-Separated Movement & Collision Resolution (Slides smoothly along walls)
    // --- Step X Axis ---
    if (moveDir.x != 0.0f) {
        m_position.x += moveDir.x * Config::Player::SPEED * dt;
        updateCollisionBox();

        for (const auto& obs : obstacles) {
            if (m_collisionBox.intersects(obs)) {
                // Moving right, push back to left edge of obstacle
                if (moveDir.x > 0.0f) {
                    m_position.x = obs.left - m_halfSize;
                }
                // Moving left, push back to right edge of obstacle
                else if (moveDir.x < 0.0f) {
                    m_position.x = obs.left + obs.width + m_halfSize;
                }
                updateCollisionBox();
            }
        }
    }

    // --- Step Y Axis ---
    if (moveDir.y != 0.0f) {
        m_position.y += moveDir.y * Config::Player::SPEED * dt;
        updateCollisionBox();

        for (const auto& obs : obstacles) {
            if (m_collisionBox.intersects(obs)) {
                // Moving down, push back to top edge of obstacle
                if (moveDir.y > 0.0f) {
                    m_position.y = obs.top - m_halfSize;
                }
                // Moving up, push back to bottom edge of obstacle
                else if (moveDir.y < 0.0f) {
                    m_position.y = obs.top + obs.height + m_halfSize;
                }
                updateCollisionBox();
            }
        }
    }

    // 4. Clamp within map boundaries
    m_position.x = std::clamp(m_position.x, m_halfSize, Config::Map::WIDTH - m_halfSize);
    m_position.y = std::clamp(m_position.y, m_halfSize, Config::Map::HEIGHT - m_halfSize);
    updateCollisionBox();

    // 5. 360-Degree Aim Rotation Toward Mouse in World Coordinates
    sf::Vector2f aimDiff = worldMousePos - m_position;
    float aimAngleRad = std::atan2(aimDiff.y, aimDiff.x);
    float aimAngleDeg = aimAngleRad * (180.0f / 3.1415926535f);
    m_sprite.setRotation(aimAngleDeg);

    // 6. Walking Bob Animation (updates only while moving)
    if (m_isMoving) {
        m_animTimer += dt;
        if (m_animTimer >= Config::Player::ANIM_FRAME_TIME) {
            m_animTimer = 0.0f;
            m_currentFrame = (m_currentFrame + 1) % static_cast<int>(m_frameTextures.size());
            m_sprite.setTexture(*m_frameTextures[m_currentFrame]);
        }
    } else {
        // Return to resting idle frame when stopped
        m_currentFrame = 0;
        m_animTimer = 0.0f;
        m_sprite.setTexture(*m_frameTextures[0]);
    }

    m_sprite.setPosition(m_position);
}

void Player::draw(sf::RenderTarget& target) const {
    target.draw(m_sprite);
}
