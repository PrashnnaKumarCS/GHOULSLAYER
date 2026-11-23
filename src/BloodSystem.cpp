#include "BloodSystem.hpp"
#include "Config.hpp"
#include <cmath>
#include <random>
#include <algorithm>

namespace {
    std::mt19937& getBloodRandomEngine() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        return gen;
    }
}

void BloodSystem::spawnSplatter(sf::Vector2f position, sf::Vector2f impactDirection) {
    float length = std::hypot(impactDirection.x, impactDirection.y);
    if (length > 0.0001f) {
        impactDirection /= length;
    } else {
        impactDirection = sf::Vector2f(1.0f, 0.0f);
    }

    float baseAngle = std::atan2(impactDirection.y, impactDirection.x);
    std::uniform_real_distribution<float> angleSpread(-0.85f, 0.85f);
    std::uniform_real_distribution<float> speedDist(Config::Blood::MIN_SPEED, Config::Blood::MAX_SPEED);
    std::uniform_real_distribution<float> lifeDist(Config::Blood::LIFETIME * 0.7f, Config::Blood::LIFETIME * 1.3f);

    for (int i = 0; i < Config::Blood::PARTICLES_PER_HIT; ++i) {
        float angle = baseAngle + angleSpread(getBloodRandomEngine());
        float speed = speedDist(getBloodRandomEngine());
        float lifetime = lifeDist(getBloodRandomEngine());

        BloodParticle p;
        p.position = position;
        p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
        p.lifetime = lifetime;
        p.maxLifetime = lifetime;

        m_particles.push_back(p);
    }
}

void BloodSystem::update(float dt) {
    for (auto& p : m_particles) {
        p.position += p.velocity * dt;
        p.velocity *= 0.90f; // Rapid deceleration as blood hits the ground/air
        p.lifetime -= dt;
    }

    // Erase dead particles
    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
            [](const BloodParticle& p) { return p.lifetime <= 0.0f; }),
        m_particles.end()
    );
}

void BloodSystem::draw(sf::RenderTarget& target) const {
    sf::RectangleShape drop(sf::Vector2f(Config::Blood::PARTICLE_SIZE, Config::Blood::PARTICLE_SIZE));

    for (const auto& p : m_particles) {
        float alphaFactor = std::clamp(p.lifetime / p.maxLifetime, 0.0f, 1.0f);
        sf::Color c = Config::Blood::COLOR;
        c.a = static_cast<sf::Uint8>(alphaFactor * 240.0f);

        drop.setPosition(p.position);
        drop.setFillColor(c);
        target.draw(drop);
    }
}
