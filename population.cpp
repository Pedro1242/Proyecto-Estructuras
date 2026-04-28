#include "population.h"
#include "skilltree.h"
#include <QtMath>

Population::Population(SkillTree* tree, QObject* parent)
    : QObject(parent)
    , m_skillTree(tree)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(TICK_INTERVAL_MS);
    connect(m_timer, &QTimer::timeout, this, &Population::tick);
    reset();
}

void Population::reset() {
    m_total          = WORLD_POPULATION;
    m_healthy        = m_total;
    m_infected       = 0;
    m_dead           = 0;
    m_dnaAccumulator = 0.0f;
    m_day            = 0;
    m_tickCounter    = 0;
    m_cureProgress   = 0.0f;
}

void Population::start() { m_timer->start(); }
void Population::stop()  { m_timer->stop();  }

void Population::tick() {
    if (m_healthy <= 0 && m_infected <= 0) return;

    // ── Day counter ──────────────────────────────────────────────────────────
    if (++m_tickCounter >= TICKS_PER_DAY) {
        m_tickCounter = 0;
        emit dayChanged(++m_day);
    }

    processInfection();
    processCure();
    emit populationUpdated();
}

void Population::processInfection() {
    const auto& s = m_skillTree->m_stats;

    // Initial patient zero
    if (m_infected == 0 && m_healthy == m_total) {
        m_infected  = INITIAL_INFECTED;
        m_healthy  -= INITIAL_INFECTED;
        return;
    }

    // ── Spread ───────────────────────────────────────────────────────────────
    const float spreadFactor = m_infected / static_cast<float>(qMax(1LL, m_total));
    const float infRate      = (s.infectivity / 50000.0f)
                          * (1.0f + spreadFactor * 10.0f);

    long long newInfected = qBound(0LL,
                                   static_cast<long long>(m_healthy * infRate), m_healthy);

    // ── Deaths ───────────────────────────────────────────────────────────────
    long long newDead = 0LL;
    if (s.lethality > 5.0f) {
        const float deathRate = (s.lethality / 200000.0f)
        * (s.severity / 50.0f);
        newDead = qBound(0LL,
                         static_cast<long long>(m_infected * deathRate), m_infected);
    }

    m_healthy  = qMax(0LL, m_healthy  - newInfected);
    m_infected = qMax(0LL, m_infected + newInfected - newDead);
    m_dead    += newDead;

    // ── Win condition ────────────────────────────────────────────────────────
    if (m_dead >= static_cast<long long>(m_total * WIN_THRESHOLD)
        && m_cureProgress < 100.0f)
    {
        stop();
        emit playerWon();
        return;
    }

    processDna(newInfected, newDead);
}

void Population::processDna(long long newInfected, long long newDead) {
    static constexpr float DNA_PER_INFECTED { 50'000'000.0f };
    static constexpr float DNA_PER_DEAD     { 20'000'000.0f };

    m_dnaAccumulator += (newInfected / DNA_PER_INFECTED)
                        + (newDead     / DNA_PER_DEAD);

    if (m_dnaAccumulator >= 1.0f) {
        const int earned = static_cast<int>(m_dnaAccumulator);
        m_dnaAccumulator -= earned;
        m_skillTree->m_dnaPoints += earned;
        emit dnaEarned(earned);
    }
}

void Population::processCure() {
    if (m_infected < CURE_START_INFECTED || m_cureProgress >= 100.0f) return;

    const auto& s = m_skillTree->m_stats;

    static constexpr float BASE_CURE_RATE    { 0.012f };
    static constexpr float MAX_STEALTH_BLOCK { 0.80f  };
    static constexpr float MAX_CHAOS_BLOCK   { 0.70f  };
    static constexpr float MIN_CHAOS_FACTOR  { 0.10f  };
    static constexpr float MAX_DEAD_BLOCK    { 0.50f  };
    static constexpr float MIN_DEAD_FACTOR   { 0.20f  };

    const float infectedRatio  = m_infected / static_cast<float>(m_total);
    const float deadRatio      = m_dead     / static_cast<float>(m_total);
    const float stealthPenalty = 1.0f - (s.stealth / 100.0f) * MAX_STEALTH_BLOCK;
    const float chaosPenalty   = qMax(MIN_CHAOS_FACTOR, 1.0f - infectedRatio * MAX_CHAOS_BLOCK);
    const float deadPenalty    = qMax(MIN_DEAD_FACTOR,  1.0f - deadRatio     * MAX_DEAD_BLOCK);

    m_cureProgress = qMin(100.0f,
                          m_cureProgress + BASE_CURE_RATE * stealthPenalty * chaosPenalty * deadPenalty);

    emit cureUpdated(m_cureProgress);

    if (m_cureProgress >= 100.0f) {
        stop();
        emit cureCompleted();
    }
}