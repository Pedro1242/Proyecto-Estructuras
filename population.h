#pragma once
#include <QObject>
#include <QTimer>

class SkillTree;

/*!
 * \brief Simulates the global population dynamics.
 *
 * Drives infection, death, DNA generation and cure progress.
 * Non-copyable. Communicates via Qt signals.
 */
class Population : public QObject {
    Q_OBJECT

public:
    // ── Public constants ─────────────────────────────────────────────────────
    static constexpr long long WORLD_POPULATION   { 7'900'000'000LL };
    static constexpr float     WIN_THRESHOLD      { 0.95f           };
    static constexpr int       TICKS_PER_DAY      { 15              };
    static constexpr int       TICK_INTERVAL_MS   { 800             };
    static constexpr long long INITIAL_INFECTED   { 100LL           };
    static constexpr int       CURE_START_INFECTED { 1000           };

    // ── Read-only state (accessed by MainWindow) ──────────────────────────────
    long long  m_total    { WORLD_POPULATION };
    long long  m_healthy  { WORLD_POPULATION };
    long long  m_infected { 0                };
    long long  m_dead     { 0                };
    int        m_day      { 0                };
    float      m_cureProgress { 0.0f         };

    SkillTree* m_skillTree { nullptr };

    explicit Population(SkillTree* tree, QObject* parent = nullptr);

    Population(const Population&)            = delete;
    Population& operator=(const Population&) = delete;

    void start();
    void stop();
    void reset();

signals:
    void populationUpdated();
    void dnaEarned(int amount);
    void dayChanged(int day);
    void cureUpdated(float progress);
    void cureCompleted();
    void playerWon();

private slots:
    void tick();

private:
    QTimer* m_timer           { nullptr };
    float   m_dnaAccumulator  { 0.0f   };
    int     m_tickCounter     { 0      };

    void processInfection();
    void processCure();
    void processDna(long long newInfected, long long newDead);
};