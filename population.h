#pragma once
#include <QObject>
#include <QTimer>

class SkillTree;

class Population : public QObject {
    Q_OBJECT
public:
    explicit Population(SkillTree* tree, QObject* parent = nullptr);

    // Estado de la población
    long long total;
    long long healthy;
    long long infected;
    long long dead;
    int       pendingDna;

    SkillTree* skillTree;  // ✅ movido a public

    void start();
    void stop();
    void reset();

signals:
    void populationUpdated();
    void dnaEarned(int amount);

private slots:
    void tick();

private:
    QTimer* timer;
    float   dnaAccumulator;
};