#pragma once

#include <string>
#include "Zombie.hpp"

// Phases of a wave's lifecycle
enum class WavePhase {
    Spawning,     // Batches are spawning over time
    InProgress,   // All zombies for the wave spawned; waiting for player to eliminate them
    Intermission  // Wave cleared; 5-second countdown to next wave
};

// WaveManager governs the gameplay progression loop:
// - Calculates enemy count and batch intervals per wave
// - Governs zombie health & speed scaling per wave
// - Controls archetype mix (Walkers on waves 1-2, Runners on 3+, Tanks on 5+)
// - Triggers Horde Waves every 5th wave
// - Manages intermission countdowns, wave-clear bonuses, and progression weapon unlocks
class WaveManager {
public:
    WaveManager();

    // Reset the manager to Wave 1 with clean state
    void reset();

    // Advance wave timers and progression state
    void update(float dt, int aliveZombieCount);

    // Queries if a batch of zombies should be spawned right now
    bool shouldSpawnBatch(float dt);

    // Determines how many zombies to spawn in the current batch
    int getBatchSize();

    // Selects which zombie type to spawn based on current wave mix rules
    ZombieType pickZombieType() const;

    // Multipliers for zombie scaling
    float getHealthMultiplier() const;
    float getSpeedMultiplier() const;

    // Progression unlocks and bonuses
    bool checkAndConsumeWaveClearBonus(int& outBonusPoints);
    bool checkAndConsumeShotgunUnlock();
    bool checkAndConsumeRifleUnlock();

    // Wave status queries
    int getCurrentWave() const { return m_currentWave; }
    WavePhase getPhase() const { return m_phase; }
    bool isHordeWave() const;
    float getIntermissionRemaining() const { return m_intermissionTimer; }
    int getTotalZombiesForWave() const { return m_totalZombiesForWave; }
    int getZombiesRemainingToSpawn() const { return m_totalZombiesForWave - m_zombiesSpawned; }

    // DEBUG: Skip immediately to the next wave
    void skipToNextWave();

    // Banner message queries
    const std::string& getBannerTitle() const { return m_bannerTitle; }
    const std::string& getBannerSubtitle() const { return m_bannerSubtitle; }
    bool hasActiveBanner() const { return m_bannerTimer > 0.0f; }

    // Display a temporary center screen banner
    void setBanner(const std::string& title, const std::string& subtitle, float duration = 3.5f);

private:
    void initWave(int waveNumber);

    int m_currentWave;
    int m_totalZombiesForWave;
    int m_zombiesSpawned;
    float m_spawnBatchTimer;

    WavePhase m_phase;
    float m_intermissionTimer;

    // Banner state
    std::string m_bannerTitle;
    std::string m_bannerSubtitle;
    float m_bannerTimer;

    // Progression trigger flags
    bool m_pendingWaveClearBonus;
    int  m_bonusAwardAmount;
    bool m_pendingShotgunUnlock;
    bool m_pendingRifleUnlock;
    bool m_shotgunAwarded;
    bool m_rifleAwarded;
};
