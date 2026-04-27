#pragma once
#include <QObject>
#include <QTimer>

class SkillTree;

class Population : public QObject {
    Q_OBJECT
public:
    explicit Population(SkillTree* tree, QObject* parent = nullptr);

    long long total;
    long long healthy;
    long long infected;
    long long dead;
    int       pendingDna;
    int       day;

    float     cureProgress;   // 0.0 a 100.0 — si llega a 100 el jugador pierde

    SkillTree* skillTree;

    void start();
    void stop();
    void reset();

signals:
    void populationUpdated();
    void dnaEarned(int amount);
    void dayChanged(int day);
    void cureUpdated(float progress);   // emite cada tick con el % actual
    void cureCompleted();               // emite cuando llega a 100 → game over
    void playerWon();

private slots:
    void tick();

private:
    QTimer* timer;
    float   dnaAccumulator;
    int     ticksPerDay;
    int     tickCounter;
};