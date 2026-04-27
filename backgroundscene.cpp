#include "backgroundscene.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QRandomGenerator>
#include <QPainterPath>
#include <QtMath>

BackgroundScene::BackgroundScene(QObject* parent) : QGraphicsScene(parent) {
    setBackgroundBrush(QBrush(QColor("#060608")));
    animTimer = new QTimer(this);
    animTimer->setInterval(16); // 60fps
    connect(animTimer, &QTimer::timeout, this, &BackgroundScene::animateTick);
    spawnParticles(70);
}

void BackgroundScene::startAnimation() { animTimer->start(); }

// ── Glóbulo rojo bicóncavo ───────────────────────────────────────────────────
QGraphicsItem* BackgroundScene::makeBloodCell(float x, float y, float size, int layer) {
    int alpha = layer == 0 ? 40 : layer == 1 ? 85 : 140;

    QGraphicsEllipseItem* outer = new QGraphicsEllipseItem(0, 0, size, size * 0.7f);
    outer->setPen(QPen(QColor(210, 15, 15, alpha + 40), 1.5f));
    outer->setBrush(QBrush(QColor(160, 0, 0, alpha)));

    QGraphicsEllipseItem* mid = new QGraphicsEllipseItem(
        size * 0.15f, size * 0.08f, size * 0.70f, size * 0.54f);
    mid->setPen(QPen(QColor(120, 0, 0, alpha - 10), 1));
    mid->setBrush(QBrush(QColor(100, 0, 0, alpha - 20)));
    mid->setParentItem(outer);

    QGraphicsEllipseItem* inner = new QGraphicsEllipseItem(
        size * 0.30f, size * 0.18f, size * 0.40f, size * 0.34f);
    inner->setPen(Qt::NoPen);
    inner->setBrush(QBrush(QColor(55, 0, 0, alpha + 25)));
    inner->setParentItem(outer);

    outer->setPos(x, y);
    return outer;
}

// ── Coco: bacteria esférica con membrana ─────────────────────────────────────
QGraphicsItem* BackgroundScene::makeCoccus(float x, float y, float size, int layer) {
    int alpha = layer == 0 ? 35 : layer == 1 ? 75 : 120;

    QGraphicsEllipseItem* body = new QGraphicsEllipseItem(0, 0, size, size);
    body->setPen(QPen(QColor(0, 220, 90, alpha + 50), 1.5f));
    body->setBrush(QBrush(QColor(0, 150, 55, alpha)));

    QGraphicsEllipseItem* nucleus = new QGraphicsEllipseItem(
        size * 0.28f, size * 0.28f, size * 0.44f, size * 0.44f);
    nucleus->setPen(Qt::NoPen);
    nucleus->setBrush(QBrush(QColor(0, 255, 110, alpha + 35)));
    nucleus->setParentItem(body);

    QGraphicsEllipseItem* membrane = new QGraphicsEllipseItem(
        -size * 0.1f, -size * 0.1f, size * 1.2f, size * 1.2f);
    membrane->setPen(QPen(QColor(0, 255, 80, alpha - 15), 0.6f, Qt::DotLine));
    membrane->setBrush(Qt::NoBrush);
    membrane->setParentItem(body);

    body->setPos(x, y);
    return body;
}

// ── Bacilo: bacteria alargada con flagelo ondulado ───────────────────────────
QGraphicsItem* BackgroundScene::makeBacillus(float x, float y, float size, int layer) {
    int alpha = layer == 0 ? 35 : layer == 1 ? 75 : 120;

    QGraphicsEllipseItem* body = new QGraphicsEllipseItem(0, 0, size * 2.8f, size);
    body->setPen(QPen(QColor(0, 205, 125, alpha + 50), 1.5f));
    body->setBrush(QBrush(QColor(0, 135, 75, alpha)));

    for (int i = 0; i < 4; ++i) {
        QGraphicsEllipseItem* dot = new QGraphicsEllipseItem(
            size * 0.25f + i * size * 0.55f, size * 0.22f,
            size * 0.22f, size * 0.22f);
        dot->setPen(Qt::NoPen);
        dot->setBrush(QBrush(QColor(0, 255, 155, alpha + 25)));
        dot->setParentItem(body);
    }

    // Flagelo ondulado
    QPainterPath flagPath;
    flagPath.moveTo(size * 2.8f, size * 0.5f);
    flagPath.cubicTo(size * 3.2f, size * 0.0f,
                     size * 3.6f, size * 1.0f,
                     size * 4.0f, size * 0.3f);
    flagPath.cubicTo(size * 4.4f, size * -0.3f,
                     size * 4.8f, size * 0.8f,
                     size * 5.2f, size * 0.2f);
    QGraphicsPathItem* flagelo = new QGraphicsPathItem(flagPath);
    flagelo->setPen(QPen(QColor(0, 200, 100, alpha), 0.8f));
    flagelo->setParentItem(body);

    body->setPos(x, y);
    return body;
}

// ── Espirilo: bacteria helicoidal ────────────────────────────────────────────
QGraphicsItem* BackgroundScene::makeSpirillum(float x, float y, float size, int layer) {
    int alpha = layer == 0 ? 30 : layer == 1 ? 65 : 110;

    QPainterPath path;
    int steps = 40;
    for (int i = 0; i < steps; ++i) {
        float t  = i / (float)(steps - 1);
        float px = t * size * 3.5f;
        float py = size * 0.5f + qSin(t * M_PI * 3.5f) * size * 0.45f;
        if (i == 0) path.moveTo(px, py);
        else        path.lineTo(px, py);
    }

    // Halo exterior más grueso
    QGraphicsPathItem* halo = new QGraphicsPathItem(path);
    halo->setPen(QPen(QColor(0, 170, 130, alpha - 10), 5.0f,
                      Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    // Línea central brillante
    QGraphicsPathItem* core = new QGraphicsPathItem(path);
    core->setPen(QPen(QColor(80, 230, 190, alpha + 50), 1.5f,
                      Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    core->setParentItem(halo);

    halo->setPos(x, y);
    return halo;
}

// ── Spawn con 3 capas de profundidad (parallax) ──────────────────────────────
void BackgroundScene::spawnParticles(int count) {
    auto* rng = QRandomGenerator::global();
    float W = 1600.0f, H = 900.0f;

    for (int i = 0; i < count; ++i) {
        int   layer = rng->bounded(3);
        float scale = layer == 0 ? 0.5f : layer == 1 ? 0.85f : 1.3f;
        float size  = (18 + rng->bounded(22)) * scale;
        float x     = rng->bounded((int)W) - W * 0.1f;
        float y     = rng->bounded((int)H) - H * 0.1f;

        // 40% sangre | 25% cocos | 20% bacilos | 15% espirilos
        int typeRoll = rng->bounded(100);
        QGraphicsItem* item = nullptr;
        int type;

        if (typeRoll < 40) {
            item = makeBloodCell(x, y, size, layer);  type = 0;
        } else if (typeRoll < 65) {
            item = makeCoccus(x, y, size, layer);     type = 1;
        } else if (typeRoll < 85) {
            item = makeBacillus(x, y, size, layer);   type = 2;
        } else {
            item = makeSpirillum(x, y, size, layer);  type = 3;
        }

        item->setZValue(-10 + layer); // parallax z
        addItem(item);

        float speedMult = layer == 0 ? 0.10f : layer == 1 ? 0.25f : 0.50f;

        Particle p;
        p.item       = item;
        p.vx         = (rng->generateDouble() - 0.5) * speedMult;
        p.vy         = (rng->generateDouble() - 0.5) * speedMult;
        p.rotSpeed   = (rng->generateDouble() - 0.5) * (layer + 1) * 0.20f;
        p.scale      = scale;
        p.pulsePhase = rng->generateDouble() * M_PI * 2.0;
        p.pulseSpeed = 0.02f + rng->generateDouble() * 0.03f;
        p.layer      = layer;
        p.type       = type;
        particles.append(p);
    }
}

// ── Animación: movimiento + pulso de escala ──────────────────────────────────
void BackgroundScene::animateTick() {
    float W = 1800.0f, H = 1100.0f;

    for (auto& p : particles) {
        QPointF pos = p.item->pos();
        float nx = pos.x() + p.vx;
        float ny = pos.y() + p.vy;

        if (nx >  W * 0.6f)  nx = -W * 0.4f;
        if (nx < -W * 0.4f)  nx =  W * 0.6f;
        if (ny >  H * 0.6f)  ny = -H * 0.4f;
        if (ny < -H * 0.4f)  ny =  H * 0.6f;

        p.item->setPos(nx, ny);
        p.item->setRotation(p.item->rotation() + p.rotSpeed);

        // Pulso suave solo en bacterias (no en glóbulos)
        if (p.type != 0) {
            p.pulsePhase += p.pulseSpeed;
            float pulse = 1.0f + qSin(p.pulsePhase) * 0.04f;
            p.item->setScale(p.scale * pulse);
        }
    }
}