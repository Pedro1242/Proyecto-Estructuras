#include "backgroundscene.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QRandomGenerator>
#include <QPolygonF>
#include <QtMath>

BackgroundScene::BackgroundScene(QObject* parent) : QGraphicsScene(parent) {
    setBackgroundBrush(QBrush(QColor("#0d0d0d")));
    animTimer = new QTimer(this);
    animTimer->setInterval(30); // ~33fps
    connect(animTimer, &QTimer::timeout, this, &BackgroundScene::animateTick);
    spawnParticles(18);
}

void BackgroundScene::startAnimation() {
    animTimer->start();
}

QGraphicsItem* BackgroundScene::makeBloodCell(float x, float y, float size) {
    // Glóbulo rojo: elipse exterior + elipse interior (forma de dona)
    QGraphicsEllipseItem* outer = new QGraphicsEllipseItem(x, y, size, size * 0.65f);
    outer->setPen(QPen(QColor(150, 0, 0, 60), 1));
    outer->setBrush(QBrush(QColor(120, 0, 0, 35)));

    QGraphicsEllipseItem* inner = new QGraphicsEllipseItem(
        x + size*0.3f, y + size*0.15f, size*0.4f, size*0.35f);
    inner->setPen(Qt::NoPen);
    inner->setBrush(QBrush(QColor(80, 0, 0, 40)));
    inner->setParentItem(outer);

    return outer;
}

QGraphicsItem* BackgroundScene::makeBacteria(float x, float y, float size) {
    // Bacteria: elipse alargada con flagelo (línea)
    QGraphicsEllipseItem* body = new QGraphicsEllipseItem(x, y, size * 2.2f, size);
    body->setPen(QPen(QColor(0, 180, 0, 50), 1));
    body->setBrush(QBrush(QColor(0, 120, 0, 25)));

    // Pequeños puntos internos
    for (int i = 0; i < 3; ++i) {
        QGraphicsEllipseItem* dot = new QGraphicsEllipseItem(
            size * 0.3f + i * size * 0.5f, size * 0.25f,
            size * 0.2f, size * 0.2f);
        dot->setPen(Qt::NoPen);
        dot->setBrush(QBrush(QColor(0, 200, 0, 40)));
        dot->setParentItem(body);
    }
    return body;
}

void BackgroundScene::spawnParticles(int count) {
    auto* rng = QRandomGenerator::global();
    float W = 1400, H = 800;

    for (int i = 0; i < count; ++i) {
        float x    = rng->bounded((int)W);
        float y    = rng->bounded((int)H);
        float size = 15 + rng->bounded(25);
        bool  isCell = rng->bounded(2) == 0;

        QGraphicsItem* item = isCell
                                  ? makeBloodCell(x, y, size)
                                  : makeBacteria(x, y, size);

        item->setZValue(-10); // detrás de todo
        addItem(item);

        Particle p;
        p.item      = item;
        p.vx        = (rng->generateDouble() - 0.5f) * 0.4f;
        p.vy        = (rng->generateDouble() - 0.5f) * 0.4f;
        p.rotSpeed  = (rng->generateDouble() - 0.5f) * 0.3f;
        p.scale     = 1.0f;
        particles.append(p);
    }
}

void BackgroundScene::animateTick() {
    QRectF bounds = sceneRect();
    float W = bounds.width()  + 100;
    float H = bounds.height() + 100;

    for (auto& p : particles) {
        QPointF pos = p.item->pos();
        float nx = pos.x() + p.vx;
        float ny = pos.y() + p.vy;

        // Wrap around: si sale por un lado entra por el otro
        if (nx >  W / 2) nx = -W / 2;
        if (nx < -W / 2) nx =  W / 2;
        if (ny >  H / 2) ny = -H / 2;
        if (ny < -H / 2) ny =  H / 2;

        p.item->setPos(nx, ny);
        p.item->setRotation(p.item->rotation() + p.rotSpeed);
    }
}