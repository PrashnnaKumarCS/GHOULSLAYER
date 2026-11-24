#include "WaveManager.hpp"
#include "Config.hpp"
#include <random>
#include <algorithm>

namespace {
    std::mt19937& getWaveRandomEngine() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        return gen;
    }
}

WaveManager::WaveManager()
    : m_currentWave(1)
    , m_totalZombiesForWave(0)
    , m_zombiesSpawned(0)
    , m_spawnBatchTimer(0.0f)
    , m_phase(WavePhase::Spawning)
    , m_intermissionTimer(0.0f)
    , m_bannerTimer(0.0f)
    , m_pendingWaveClearBonus(false)
    , m_bonusAwardAmount(0)
    , m_pendingShotgunUnlock(false)
    , m_pendingRifleUnlock(false)
    , m_shotgunAwarded(false)
    , m_rifleAwarded(false)
{
    initWave(1);
}

void WaveManager::reset() {
    m_currentWave = 1;
    m_shotgunAwarded = false;
    m_rifleAwarded = false;
    m_pendingShotgunUnlock = false;
    m_pendingRifleUnlock = false;
    m_pendingWaveClearBonus = false;
    m_bonusAwardAmount = 0;
    initWave(1);
}

void WaveManager::initWave(int waveNumber) {
    m_currentWave = waveNumber;

    // Calculate zombies for this wave: 5 + N*3 (scaled higher on horde waves)
    int count = Config::Waves::BASE_ZOMBIES + m_currentWave * Config::Waves::ZOMBIES_PER_WAVE;
    if (isHordeWave()) {
        count = static_cast<int>(count * Config::Waves::HORDE_MULTIPLIER);
    }
    m_totalZombiesForWave = std::min(Config::Waves::MAX_ZOMBIES_PER_WAVE, count);
    m_zombiesSpawned = 0;
    m_spawnBatchTimer = 0.5f; // First batch arrives promptly
    m_phase = WavePhase::Spawning;

    if (isHordeWave()) {
        setBanner("HORDE WAVE " + std::to_string(m_currentWave) + "!", "MASSIVE SWARM DETECTED - SURVIVE!", 3.5f);
    } else {
        setBanner("WAVE " + std::to_string(m_currentWave), "ELIMINATE ALL THREATS", 3.0f);
    }
}

bool WaveManager::isHordeWave() const {
    return (m_currentWave % Config::Waves::HORDE_WAVE_INTERVAL == 0);
}

void WaveManager::setBanner(const std::string& title, const std::string& subtitle, float duration) {
    m_bannerTitle = title;
    m_bannerSubtitle = subtitle;
    m_bannerTimer = duration;
}

bool WaveManager::shouldSpawnBatch(float dt) {
    if (m_phase != WavePhase::Spawning) return false;

    m_spawnBatchTimer -= dt;
    if (m_spawnBatchTimer <= 0.0f && m_zombiesSpawned < m_totalZombiesForWave) {
        m_spawnBatchTimer = Config::Waves::BATCH_SPAWN_INTERVAL;
        return true;
    }
    return false;
}

int WaveManager::getBatchSize() {
    int remaining = m_totalZombiesForWave - m_zombiesSpawned;
    int count = std::min(Config::Waves::BATCH_SIZE, remaining);
    m_zombiesSpawned += count;

    if (m_zombiesSpawned >= m_totalZombiesForWave) {
        m_phase = WavePhase::InProgress;
    }
    return count;
}

ZombieType WaveManager::pickZombieType() const {
    std::uniform_int_distribution<int> roll(0, 99);
    int val = roll(getWaveRandomEngine());

    // Waves 1-2: Walkers only
    if (m_currentWave <= 2) {
        return ZombieType::Walker;
    }
    // Waves 3-4: Walkers + Runners
    else if (m_currentWave <= 4) {
        return (val < 60) ? ZombieType::Walker : ZombieType::Runner;
    }
    // Waves 5+: Walkers, Runners, and Tanks
    else {
        int tankChance = std::min(25, 10 + (m_currentWave - 5) * 3);
        int runnerChance = 35;
        if (val < tankChance) {
            return ZombieType::Tank;
        } else if (val < (tankChance + runnerChance)) {
            return ZombieType::Runner;
        } else {
            return ZombieType::Walker;
        }
    }
}

float WaveManager::getHealthMultiplier() const {
    return 1.0f + static_cast<float>(m_currentWave - 1) * Config::Waves::HEALTH_SCALE_PER_WAVE;
}

float WaveManager::getSpeedMultiplier() const {
    float scaled = 1.0f + static_cast<float>(m_currentWave - 1) * Config::Waves::SPEED_SCALE_PER_WAVE;
    return std::min(Config::Waves::MAX_SPEED_MULTIPLIER, scaled);
}

void WaveManager::update(float dt, int aliveZombieCount) {
    // 1. Advance banner display timer
    if (m_bannerTimer > 0.0f) {
        m_bannerTimer -= dt;
    }

    // 2. Wave completion check
    if (m_phase == WavePhase::InProgress) {
        if (aliveZombieCount == 0 && m_zombiesSpawned >= m_totalZombiesForWave) {
            // Wave cleared! Transition to intermission
            m_phase = WavePhase::Intermission;
            m_intermissionTimer = Config::Waves::INTERMISSION_DURATION;

            // Trigger wave clear bonus
            m_pendingWaveClearBonus = true;
            m_bonusAwardAmount = Config::Score::WAVE_CLEAR_BASE * m_currentWave;

            // Check progression weapon unlocks
            if (m_currentWave == Config::Waves::SHOTGUN_UNLOCK_WAVE && !m_shotgunAwarded) {
                m_pendingShotgunUnlock = true;
                m_shotgunAwarded = true;
                setBanner(
                    "WAVE " + std::to_string(m_currentWave) + " CLEARED!",
                    "NEW WEAPON UNLOCKED: SHOTGUN (PRESS 2)!",
                    4.5f
                );
            } else if (m_currentWave == Config::Waves::RIFLE_UNLOCK_WAVE && !m_rifleAwarded) {
                m_pendingRifleUnlock = true;
                m_rifleAwarded = true;
                setBanner(
                    "WAVE " + std::to_string(m_currentWave) + " CLEARED!",
                    "NEW WEAPON UNLOCKED: ASSAULT RIFLE (PRESS 3)!",
                    4.5f
                );
            } else {
                setBanner(
                    "WAVE " + std::to_string(m_currentWave) + " CLEARED!",
                    "WAVE BONUS: +" + std::to_string(m_bonusAwardAmount) + " PTS",
                    3.5f
                );
            }
        }
    }

    // 3. Intermission countdown to next wave
    if (m_phase == WavePhase::Intermission) {
        m_intermissionTimer -= dt;
        if (m_intermissionTimer <= 0.0f) {
            initWave(m_currentWave + 1);
        }
    }
}

bool WaveManager::checkAndConsumeWaveClearBonus(int& outBonusPoints) {
    if (m_pendingWaveClearBonus) {
        outBonusPoints = m_bonusAwardAmount;
        m_pendingWaveClearBonus = false;
        return true;
    }
    return false;
}

bool WaveManager::checkAndConsumeShotgunUnlock() {
    if (m_pendingShotgunUnlock) {
        m_pendingShotgunUnlock = false;
        return true;
    }
    return false;
}

bool WaveManager::checkAndConsumeRifleUnlock() {
    if (m_pendingRifleUnlock) {
        m_pendingRifleUnlock = false;
        return true;
    }
    return false;
}
