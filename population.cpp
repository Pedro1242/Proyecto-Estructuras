#include "population.h"
#include "skilltree.h"
#include <QtMath>
#include <QRandomGenerator>

Population::Population(SkillTree* tree, QObject* parent)
    : QObject(parent), skillTree(tree), dnaAccumulator(0.0f)
{
    timer = new QTimer(this);
    timer->setInterval(800); // tick cada 800ms
    connect(timer, &QTimer::timeout, this, &Population::tick);
    reset();
}

void Population::reset() {
    total     = 7'900'000'000LL; // 7.9 mil millones
    healthy   = total;
    infected  = 0;
    dead      = 0;
    pendingDna     = 0;
    dnaAccumulator = 0.0f;
}

void Population::start() { timer->start(); }
void Population::stop()  { timer->stop();  }

void Population::tick() {
    if (healthy <= 0 && infected <= 0) return;

    auto& s = skillTree->stats;

    // ── Nuevos infectados ─────────────────────────────────────────────────
    // Tasa base de infección: infectividad / 10000 por tick sobre sanos
    float infRate = (s.infectivity / 10000.0f)
                    * (1.0f + infected / (float)qMax(1LL, total));
    long long newInfected = (long long)(healthy * infRate);
    newInfected = qBound(0LL, newInfected, healthy);

    // Si no hay infectados aún, seed inicial pequeño
    if (infected == 0 && healthy == total)
        newInfected = qMax(1LL, newInfected);

    // ── Muertes ───────────────────────────────────────────────────────────
    float deathRate = (s.lethality / 100000.0f)
                      * (s.severity / 50.0f);
    long long newDead = (long long)(infected * deathRate);
    newDead = qBound(0LL, newDead, infected);

    // ── Aplicar cambios ───────────────────────────────────────────────────
    healthy  -= newInfected;
    infected += newInfected - newDead;
    dead     += newDead;

    healthy  = qMax(0LL, healthy);
    infected = qMax(0LL, infected);

    // ── Ganar ADN: 1 punto por cada 50M infectados acumulados ─────────────
    float dnaGain = (newInfected / 50'000'000.0f)
                    + (newDead     / 20'000'000.0f);
    dnaAccumulator += dnaGain;

    if (dnaAccumulator >= 1.0f) {
        int earned = (int)dnaAccumulator;
        dnaAccumulator -= earned;
        skillTree->dnaPoints += earned;
        emit dnaEarned(earned);
    }

    emit populationUpdated();
}