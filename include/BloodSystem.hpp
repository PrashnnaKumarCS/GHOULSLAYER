#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

// An individual blood droplet particle
struct BloodParticle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float lifetime;
    float maxLifetime;
};

// Manages blood splatters generated when projectiles impact infected targets.
// Simulates velocity dampening and alpha fading over time.
class BloodSystem {
public:
    BloodSystem() = default;

    // Emit a splatter burst along the impact direction
    void spawnSplatter(sf::Vector2f position, sf::Vector2f impactDirection);

    // Advance particle physics and prune expired droplets
    void update(float dt);

    // Render active droplets to world space
    void draw(sf::RenderTarget& target) const;

private:
    std::vector<BloodParticle> m_particles;
};
