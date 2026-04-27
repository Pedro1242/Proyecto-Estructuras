#pragma once
#include <QGraphicsScene>
#include <QTimer>
#include <QList>
#include <QGraphicsItem>

struct Particle {
    QGraphicsItem* item;
    float vx, vy;
    float rotSpeed;
    float scale;
    float pulsePhase;   // ✅ fase actual del pulso
    float pulseSpeed;   // ✅ velocidad del pulso
    int   layer;        // ✅ 0=fondo, 1=medio, 2=frente
    int   type;         // ✅ 0=sangre, 1=coco, 2=bacilo, 3=espirilo
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

    // ✅ Todas las funciones con parámetro layer
    QGraphicsItem* makeBloodCell(float x, float y, float size, int layer);
    QGraphicsItem* makeCoccus(float x, float y, float size, int layer);
    QGraphicsItem* makeBacillus(float x, float y, float size, int layer);
    QGraphicsItem* makeSpirillum(float x, float y, float size, int layer);
};