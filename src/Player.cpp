#include "Player.hpp"
#include "ResourceManager.hpp"
#include "Config.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

Player::Player(const ResourceManager& resources)
    : m_position(Config::Player::START_X, Config::Player::START_Y)
    , m_aimDirection(1.0f, 0.0f)
    , m_health(Config::Player::MAX_HEALTH)
    , m_isMoving(false)
    , m_isDead(false)
    , m_deathLogged(false)
    , m_invulnerabilityTimer(0.0f)
    , m_damageFlashTimer(0.0f)
    , m_currentFrame(0)
    , m_animTimer(0.0f)
    , m_currentWeaponIndex(0)
    , m_triggerSemiAuto(false)
    , m_muzzleFlashTimer(0.0f)
{
    // Cache the 4 walking animation frames
    for (int i = 0; i < 4; ++i) {
        m_frameTextures.push_back(&resources.getTexture("player_" + std::to_string(i)));
    }

    if (!m_frameTextures.empty() && m_frameTextures[0] != nullptr) {
        m_sprite.setTexture(*m_frameTextures[0]);
    }
    m_sprite.setOrigin(16.0f, 16.0f);
    m_sprite.setPosition(m_position);

    // Initialize weapon inventory:
    // Slot 0: Pistol (unlocked)
    // Slot 1: Shotgun (starts locked)
    // Slot 2: Assault Rifle (starts locked)
    m_weapons.push_back(Weapon::createPistol());
    m_weapons.push_back(Weapon::createShotgun());
    m_weapons.push_back(Weapon::createAssaultRifle());

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

sf::Vector2f Player::calculateMuzzlePosition() const {
    // Offset gun muzzle forward along aim direction and slightly to the right hand side
    sf::Vector2f perpRight(-m_aimDirection.y, m_aimDirection.x);
    return m_position
         + m_aimDirection * Config::Player::MUZZLE_FORWARD_OFFSET
         + perpRight * Config::Player::MUZZLE_SIDE_OFFSET;
}

void Player::takeDamage(float damage) {
    if (m_isDead || m_invulnerabilityTimer > 0.0f) return;

    m_health = std::max(0.0f, m_health - damage);
    m_invulnerabilityTimer = Config::Player::INVULNERABILITY_DURATION;
    m_damageFlashTimer = Config::Player::DAMAGE_FLASH_DURATION;

    if (m_health <= 0.0f) {
        m_isDead = true;
        if (!m_deathLogged) {
            std::cout << "[GHOULSLAYER] PLAYER DIED\n";
            m_deathLogged = true;
        }
    }
}

void Player::restoreHealth(float amount) {
    if (m_isDead) return;
    m_health = std::min(Config::Player::MAX_HEALTH, m_health + amount);
}

void Player::addAmmo(int amount) {
    m_weapons[m_currentWeaponIndex].addReserve(amount);
}

void Player::unlockWeaponType(WeaponType type, int starterAmmo) {
    for (auto& w : m_weapons) {
        if (w.getType() == type) {
            w.unlock();
            if (starterAmmo > 0) {
                w.addReserve(starterAmmo);
            }
            break;
        }
    }
}

void Player::handleEvent(const sf::Event& event) {
    if (m_isDead) return; // Prevent actions when dead

    if (event.type == sf::Event::KeyPressed) {
        // Direct weapon slot selection via number keys (only switches if unlocked)
        if (event.key.code == sf::Keyboard::Num1) {
            selectWeapon(0);
        } else if (event.key.code == sf::Keyboard::Num2) {
            selectWeapon(1);
        } else if (event.key.code == sf::Keyboard::Num3) {
            selectWeapon(2);
        }

        // Manual reload trigger
        if (event.key.code == sf::Keyboard::R) {
            m_weapons[m_currentWeaponIndex].startReload();
        }

#if DEBUG_FEATURES
        // DEBUG: Press F1 to unlock all weapons for immediate testing
        if (event.key.code == sf::Keyboard::F1) {
            unlockAllWeapons();
            std::cout << "[DEBUG] All weapons unlocked (Pistol, Shotgun, Assault Rifle).\n";
        }
#endif
    }

    // Weapon switching via mouse scroll wheel
    if (event.type == sf::Event::MouseWheelScrolled) {
        if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
            if (event.mouseWheelScroll.delta > 0.0f) {
                cycleWeapon(-1); // Previous unlocked weapon
            } else if (event.mouseWheelScroll.delta < 0.0f) {
                cycleWeapon(1);  // Next unlocked weapon
            }
        }
    }

    // Semi-automatic mouse click registration
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            if (!m_weapons[m_currentWeaponIndex].isAutomatic()) {
                m_triggerSemiAuto = true;
            }
        }
    }
}

void Player::selectWeapon(int index) {
    if (index >= 0 && index < static_cast<int>(m_weapons.size())) {
        if (m_weapons[index].isUnlocked() && index != m_currentWeaponIndex) {
            // Switching weapons immediately cancels any active reload
            m_weapons[m_currentWeaponIndex].cancelReload();
            m_currentWeaponIndex = index;
            m_triggerSemiAuto = false;
        }
    }
}

void Player::cycleWeapon(int step) {
    int total = static_cast<int>(m_weapons.size());
    for (int i = 1; i <= total; ++i) {
        int candidate = (m_currentWeaponIndex + step * i) % total;
        if (candidate < 0) candidate += total;

        if (m_weapons[candidate].isUnlocked()) {
            selectWeapon(candidate);
            break;
        }
    }
}

void Player::unlockAllWeapons() {
    for (auto& weapon : m_weapons) {
        weapon.unlock();
    }
}

void Player::update(float dt, const std::vector<sf::FloatRect>& obstacles, sf::Vector2f worldMousePos) {
    // Decrement damage response timers
    if (m_invulnerabilityTimer > 0.0f) {
        m_invulnerabilityTimer = std::max(0.0f, m_invulnerabilityTimer - dt);
    }
    if (m_damageFlashTimer > 0.0f) {
        m_damageFlashTimer = std::max(0.0f, m_damageFlashTimer - dt);
    }

    // Stop updating player controls and movement if dead
    if (m_isDead) {
        m_isMoving = false;
        return;
    }

    // 1. Update current weapon timers (fire cooldown, reload, spread recovery)
    m_weapons[m_currentWeaponIndex].update(dt);

    // 2. Gather WASD input
    sf::Vector2f moveDir(0.0f, 0.0f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) moveDir.y -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) moveDir.y += 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) moveDir.x -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) moveDir.x += 1.0f;

    // 3. Diagonal normalization
    float lengthSq = moveDir.x * moveDir.x + moveDir.y * moveDir.y;
    if (lengthSq > 0.0001f) {
        float invLen = 1.0f / std::sqrt(lengthSq);
        moveDir.x *= invLen;
        moveDir.y *= invLen;
        m_isMoving = true;
    } else {
        m_isMoving = false;
    }

    // 4. Axis-Separated Collision Resolution (slides smoothly along walls)
    // --- Step X ---
    if (moveDir.x != 0.0f) {
        m_position.x += moveDir.x * Config::Player::SPEED * dt;
        updateCollisionBox();
        for (const auto& obs : obstacles) {
            if (m_collisionBox.intersects(obs)) {
                if (moveDir.x > 0.0f) {
                    m_position.x = obs.left - m_halfSize;
                } else if (moveDir.x < 0.0f) {
                    m_position.x = obs.left + obs.width + m_halfSize;
                }
                updateCollisionBox();
            }
        }
    }

    // --- Step Y ---
    if (moveDir.y != 0.0f) {
        m_position.y += moveDir.y * Config::Player::SPEED * dt;
        updateCollisionBox();
        for (const auto& obs : obstacles) {
            if (m_collisionBox.intersects(obs)) {
                if (moveDir.y > 0.0f) {
                    m_position.y = obs.top - m_halfSize;
                } else if (moveDir.y < 0.0f) {
                    m_position.y = obs.top + obs.height + m_halfSize;
                }
                updateCollisionBox();
            }
        }
    }

    // 5. Clamp to world boundaries
    m_position.x = std::clamp(m_position.x, m_halfSize, Config::Map::WIDTH - m_halfSize);
    m_position.y = std::clamp(m_position.y, m_halfSize, Config::Map::HEIGHT - m_halfSize);
    updateCollisionBox();

    // 6. Aim Direction toward mouse
    sf::Vector2f aimDiff = worldMousePos - m_position;
    float aimLen = std::hypot(aimDiff.x, aimDiff.y);
    if (aimLen > 0.0001f) {
        m_aimDirection = aimDiff / aimLen;
    }

    float aimAngleRad = std::atan2(m_aimDirection.y, m_aimDirection.x);
    float aimAngleDeg = aimAngleRad * (180.0f / 3.1415926535f);
    m_sprite.setRotation(aimAngleDeg);

    // 7. Walking bob animation
    if (m_isMoving) {
        m_animTimer += dt;
        if (m_animTimer >= Config::Player::ANIM_FRAME_TIME) {
            m_animTimer = 0.0f;
            m_currentFrame = (m_currentFrame + 1) % static_cast<int>(m_frameTextures.size());
            m_sprite.setTexture(*m_frameTextures[m_currentFrame]);
        }
    } else {
        m_currentFrame = 0;
        m_animTimer = 0.0f;
        m_sprite.setTexture(*m_frameTextures[0]);
    }
    m_sprite.setPosition(m_position);

    // 8. Shooting Logic
    bool wantsToShoot = false;
    if (m_weapons[m_currentWeaponIndex].isAutomatic()) {
        wantsToShoot = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    } else {
        wantsToShoot = m_triggerSemiAuto;
        m_triggerSemiAuto = false; // Reset single click trigger
    }

    if (wantsToShoot) {
        if (m_weapons[m_currentWeaponIndex].canFire()) {
            sf::Vector2f muzzlePos = calculateMuzzlePosition();
            auto bullets = m_weapons[m_currentWeaponIndex].fire(muzzlePos, m_aimDirection);
            m_pendingBullets.insert(m_pendingBullets.end(), bullets.begin(), bullets.end());

            // Trigger muzzle flash
            m_muzzleFlashTimer = Config::MuzzleFlash::DURATION;
            m_muzzleFlashPos = muzzlePos;
        } else if (m_weapons[m_currentWeaponIndex].getMagazine() == 0 && m_weapons[m_currentWeaponIndex].getReserve() > 0) {
            m_weapons[m_currentWeaponIndex].startReload();
        }
    }

    // 9. Muzzle flash timer decay
    if (m_muzzleFlashTimer > 0.0f) {
        m_muzzleFlashTimer -= dt;
    }
}

std::vector<Bullet> Player::extractPendingBullets() {
    std::vector<Bullet> bullets = std::move(m_pendingBullets);
    m_pendingBullets.clear();
    return bullets;
}

void Player::draw(sf::RenderTarget& target) const {
    // 1. Draw player sprite with hit or death tint
    sf::Sprite renderSprite = m_sprite;
    if (m_isDead) {
        renderSprite.setColor(sf::Color(120, 50, 50, 160));
    } else if (m_invulnerabilityTimer > 0.0f) {
        renderSprite.setColor(sf::Color(255, 120, 120, 210));
    }
    target.draw(renderSprite);

    // If dead, do not render weapon flashes or reload bars
    if (m_isDead) return;

    // 2. Draw muzzle flash if active
    if (m_muzzleFlashTimer > 0.0f) {
        sf::CircleShape flash(Config::MuzzleFlash::RADIUS);
        flash.setOrigin(Config::MuzzleFlash::RADIUS, Config::MuzzleFlash::RADIUS);
        flash.setPosition(m_muzzleFlashPos);
        flash.setFillColor(Config::MuzzleFlash::COLOR);
        target.draw(flash);
    }

    // 3. Draw reloading progress bar if currently reloading
    if (m_weapons[m_currentWeaponIndex].isReloading()) {
        float progress = m_weapons[m_currentWeaponIndex].getReloadProgress();

        const float BAR_WIDTH = 42.0f;
        const float BAR_HEIGHT = 6.0f;
        sf::Vector2f barPos(m_position.x - BAR_WIDTH * 0.5f, m_position.y - 28.0f);

        // Background box
        sf::RectangleShape bg(sf::Vector2f(BAR_WIDTH, BAR_HEIGHT));
        bg.setPosition(barPos);
        bg.setFillColor(sf::Color(20, 20, 20, 220));
        bg.setOutlineColor(sf::Color(255, 255, 255, 140));
        bg.setOutlineThickness(1.0f);
        target.draw(bg);

        // Progress fill
        sf::RectangleShape fill(sf::Vector2f(BAR_WIDTH * progress, BAR_HEIGHT));
        fill.setPosition(barPos);
        fill.setFillColor(sf::Color(80, 210, 255));
        target.draw(fill);
    }
}
