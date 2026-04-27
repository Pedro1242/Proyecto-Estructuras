#include "population.h"
#include "skilltree.h"
#include <QtMath>
#include <QRandomGenerator>

Population::Population(SkillTree* tree, QObject* parent)
    : QObject(parent), skillTree(tree), dnaAccumulator(0.0f)
{
    timer = new QTimer(this);
    timer->setInterval(800);
    connect(timer, &QTimer::timeout, this, &Population::tick);

    ticksPerDay = 15;
    reset();
}

void Population::reset() {
    total          = 7'900'000'000LL;
    healthy        = total;
    infected       = 0;
    dead           = 0;
    pendingDna     = 0;
    dnaAccumulator = 0.0f;
    day            = 0;
    tickCounter    = 0;
    cureProgress   = 0.0f;
}

void Population::start() { timer->start(); }
void Population::stop()  { timer->stop();  }

void Population::tick() {
    if (healthy <= 0 && infected <= 0) return;

    auto& s = skillTree->stats;

    // --- Avance de dia ---
    tickCounter++;
    if (tickCounter >= ticksPerDay) {
        tickCounter = 0;
        day++;
        emit dayChanged(day);
    }

    // --- Infeccion ---
    float spreadFactor = infected > 0
                             ? (infected / (float)qMax(1LL, total))
                             : 0.0f;

    float infRate = (s.infectivity / 50000.0f) * (1.0f + spreadFactor * 10.0f);

    long long newInfected = (long long)(healthy * infRate);
    newInfected = qBound(0LL, newInfected, healthy);

    if (infected == 0 && healthy == total)
        newInfected = 100;

    // --- Muertes ---
    long long newDead = 0;
    if (s.lethality > 5.0f) {
        float deathRate = (s.lethality / 200000.0f) * (s.severity / 50.0f);
        newDead = (long long)(infected * deathRate);
        newDead = qBound(0LL, newDead, infected);
    }

    healthy  -= newInfected;
    infected += newInfected - newDead;
    dead     += newDead;
    healthy   = qMax(0LL, healthy);
    infected  = qMax(0LL, infected);
    // Win condition: 95% exterminado antes de que llegue la cura
    if (dead >= (long long)(total * 0.95f) && cureProgress < 100.0f) {
        stop();
        emit playerWon();
        return;
    }

    // --- ADN ---
    float dnaGain = (newInfected / 50'000'000.0f)
                    + (newDead     / 20'000'000.0f);
    dnaAccumulator += dnaGain;

    if (dnaAccumulator >= 1.0f) {
        int earned = (int)dnaAccumulator;
        dnaAccumulator -= earned;
        skillTree->dnaPoints += earned;
        emit dnaEarned(earned);
    }

    // --- Barra de Cura ---
    // Solo empieza cuando hay al menos 1000 infectados (la humanidad lo detecta)
    if (infected >= 1000 && cureProgress < 100.0f) {

        float infectedRatio = infected / (float)total; // 0.0 a 1.0

        // Base: la humanidad siempre investiga aunque sea lento
        float baseRate = 0.012f;

        // Sigilo frena la cura (max reduccion 80% con sigilo=100)
        float stealthPenalty = 1.0f - (s.stealth / 100.0f) * 0.80f;

        // Caos: mas infectados = menos recursos medicos disponibles
        // Poca infeccion -> cura rapida, mucha infeccion -> cura lenta
        float chaosPenalty = 1.0f - (infectedRatio * 0.70f);
        chaosPenalty = qMax(chaosPenalty, 0.10f); // minimo 10% de velocidad

        // Muchos muertos = colapso hospitalario = cura mas lenta
        float deadRatio = dead / (float)total;
        float collapsePenalty = 1.0f - (deadRatio * 0.50f);
        collapsePenalty = qMax(collapsePenalty, 0.20f);

        float cureRate = baseRate * stealthPenalty * chaosPenalty * collapsePenalty;

        cureProgress += cureRate;
        cureProgress  = qMin(cureProgress, 100.0f);

        emit cureUpdated(cureProgress);

        if (cureProgress >= 100.0f) {
            stop();
            emit cureCompleted();
        }
    }

    emit populationUpdated();
}