#pragma once
#include <QGraphicsScene>
#include <QTimer>
#include <QList>
#include <QGraphicsItem>

struct Particle {
    QGraphicsItem* item;
    float vx, vy;       // velocidad
    float rotSpeed;     // velocidad de rotación
    float scale;
};

class BackgroundScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit BackgroundScene(QObject* parent = nullptr);
    void startAnimation();

private slots:
    void animateTick();

private:
    QTimer*         animTimer;
    QList<Particle> particles;

    void spawnParticles(int count);
    QGraphicsItem* makeBloodCell(float x, float y, float size);
    QGraphicsItem* makeBacteria(float x, float y, float size);
};