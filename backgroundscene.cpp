#include "backgroundscene.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QRandomGenerator>
#include <QPainterPath>
#include <QtMath>

BackgroundScene::BackgroundScene(QObject* parent)
    : QGraphicsScene(parent)
{
    setBackgroundBrush(QBrush(QColor("#060608")));
    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(TICK_INTERVAL_MS);
    connect(m_animTimer, &QTimer::timeout, this, &BackgroundScene::animateTick);
    spawnParticles(PARTICLE_COUNT);
}

void BackgroundScene::startAnimation() { m_animTimer->start(); }

QGraphicsItem* BackgroundScene::makeBloodCell(float x, float y, float size, int layer) const {
    const int alpha = layer == 0 ? 40 : layer == 1 ? 85 : 140;

    auto* outer = new QGraphicsEllipseItem(0, 0, size, size * 0.7f);
    outer->setPen(QPen(QColor(210, 15, 15, alpha + 40), 1.5f));
    outer->setBrush(QBrush(QColor(160, 0, 0, alpha)));

    auto* mid = new QGraphicsEllipseItem(size*0.15f, size*0.08f, size*0.70f, size*0.54f);
    mid->setPen(QPen(QColor(120, 0, 0, alpha - 10), 1));
    mid->setBrush(QBrush(QColor(100, 0, 0, alpha - 20)));
    mid->setParentItem(outer);

    auto* inner = new QGraphicsEllipseItem(size*0.30f, size*0.18f, size*0.40f, size*0.34f);
    inner->setPen(Qt::NoPen);
    inner->setBrush(QBrush(QColor(55, 0, 0, alpha + 25)));
    inner->setParentItem(outer);

    outer->setPos(x, y);
    return outer;
}

QGraphicsItem* BackgroundScene::makeCoccus(float x, float y, float size, int layer) const {
    const int alpha = layer == 0 ? 35 : layer == 1 ? 75 : 120;

    auto* body = new QGraphicsEllipseItem(0, 0, size, size);
    body->setPen(QPen(QColor(0, 220, 90, alpha + 50), 1.5f));
    body->setBrush(QBrush(QColor(0, 150, 55, alpha)));

    auto* nucleus = new QGraphicsEllipseItem(size*0.28f, size*0.28f, size*0.44f, size*0.44f);
    nucleus->setPen(Qt::NoPen);
    nucleus->setBrush(QBrush(QColor(0, 255, 110, alpha + 35)));
    nucleus->setParentItem(body);

    auto* membrane = new QGraphicsEllipseItem(-size*0.1f, -size*0.1f, size*1.2f, size*1.2f);
    membrane->setPen(QPen(QColor(0, 255, 80, alpha - 15), 0.6f, Qt::DotLine));
    membrane->setBrush(Qt::NoBrush);
    membrane->setParentItem(body);

    body->setPos(x, y);
    return body;
}

QGraphicsItem* BackgroundScene::makeBacillus(float x, float y, float size, int layer) const {
    const int alpha = layer == 0 ? 35 : layer == 1 ? 75 : 120;

    auto* body = new QGraphicsEllipseItem(0, 0, size * 2.8f, size);
    body->setPen(QPen(QColor(0, 205, 125, alpha + 50), 1.5f));
    body->setBrush(QBrush(QColor(0, 135, 75, alpha)));

    for (int i = 0; i < 4; ++i) {
        auto* dot = new QGraphicsEllipseItem(
            size*0.25f + i*size*0.55f, size*0.22f, size*0.22f, size*0.22f);
        dot->setPen(Qt::NoPen);
        dot->setBrush(QBrush(QColor(0, 255, 155, alpha + 25)));
        dot->setParentItem(body);
    }

    QPainterPath flagPath;
    flagPath.moveTo(size * 2.8f, size * 0.5f);
    flagPath.cubicTo(size*3.2f, size*0.0f,  size*3.6f, size*1.0f, size*4.0f, size*0.3f);
    flagPath.cubicTo(size*4.4f, size*-0.3f, size*4.8f, size*0.8f, size*5.2f, size*0.2f);

    auto* flagelo = new QGraphicsPathItem(flagPath);
    flagelo->setPen(QPen(QColor(0, 200, 100, alpha), 0.8f));
    flagelo->setParentItem(body);

    body->setPos(x, y);
    return body;
}

QGraphicsItem* BackgroundScene::makeSpirillum(float x, float y, float size, int layer) const {
    const int alpha = layer == 0 ? 30 : layer == 1 ? 65 : 110;
    constexpr int STEPS = 40;

    QPainterPath path;
    for (int i = 0; i < STEPS; ++i) {
        const float t  = i / static_cast<float>(STEPS - 1);
        const float px = t * size * 3.5f;
        const float py = size * 0.5f + qSin(t * static_cast<float>(M_PI) * 3.5f) * size * 0.45f;
        i == 0 ? path.moveTo(px, py) : path.lineTo(px, py);
    }

    auto* halo = new QGraphicsPathItem(path);
    halo->setPen(QPen(QColor(0, 170, 130, alpha - 10), 5.0f,
                      Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    auto* core = new QGraphicsPathItem(path);
    core->setPen(QPen(QColor(80, 230, 190, alpha + 50), 1.5f,
                      Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    core->setParentItem(halo);

    halo->setPos(x, y);
    return halo;
}

void BackgroundScene::spawnParticles(int count) {
    auto* rng = QRandomGenerator::global();

    for (int i = 0; i < count; ++i) {
        const int   layer = rng->bounded(3);
        const float scale = layer == 0 ? 0.5f : layer == 1 ? 0.85f : 1.3f;
        const float size  = (18 + rng->bounded(22)) * scale;
        const float x     = rng->bounded(static_cast<int>(CANVAS_W)) - CANVAS_W * 0.1f;
        const float y     = rng->bounded(static_cast<int>(CANVAS_H)) - CANVAS_H * 0.1f;

        // Distribution: 40% blood | 25% cocci | 20% bacilli | 15% spirilla
        const int typeRoll = rng->bounded(100);
        QGraphicsItem* item = nullptr;
        int type = 0;

        if      (typeRoll < 40) { item = makeBloodCell (x, y, size, layer); type = 0; }
        else if (typeRoll < 65) { item = makeCoccus    (x, y, size, layer); type = 1; }
        else if (typeRoll < 85) { item = makeBacillus  (x, y, size, layer); type = 2; }
        else                    { item = makeSpirillum (x, y, size, layer); type = 3; }

        item->setZValue(-10 + layer);
        addItem(item);

        const float speedMult = layer == 0 ? 0.10f : layer == 1 ? 0.25f : 0.50f;

        Particle p;
        p.m_item       = item;
        p.m_vx         = static_cast<float>(rng->generateDouble() - 0.5) * speedMult;
        p.m_vy         = static_cast<float>(rng->generateDouble() - 0.5) * speedMult;
        p.m_rotSpeed   = static_cast<float>(rng->generateDouble() - 0.5) * (layer + 1) * 0.20f;
        p.m_scale      = scale;
        p.m_pulsePhase = static_cast<float>(rng->generateDouble() * M_PI * 2.0);
        p.m_pulseSpeed = 0.02f + static_cast<float>(rng->generateDouble()) * 0.03f;
        p.m_layer      = layer;
        p.m_type       = type;
        m_particles.append(p);
    }
}

void BackgroundScene::animateTick() {
    static constexpr float WRAP_W = 1800.0f;
    static constexpr float WRAP_H = 1100.0f;

    for (auto& p : m_particles) {
        float nx = static_cast<float>(p.m_item->pos().x()) + p.m_vx;
        float ny = static_cast<float>(p.m_item->pos().y()) + p.m_vy;

        if (nx >  WRAP_W * 0.6f) nx = -WRAP_W * 0.4f;
        if (nx < -WRAP_W * 0.4f) nx =  WRAP_W * 0.6f;
        if (ny >  WRAP_H * 0.6f) ny = -WRAP_H * 0.4f;
        if (ny < -WRAP_H * 0.4f) ny =  WRAP_H * 0.6f;

        p.m_item->setPos(nx, ny);
        p.m_item->setRotation(p.m_item->rotation() + p.m_rotSpeed);

        if (p.m_type != 0) {
            p.m_pulsePhase += p.m_pulseSpeed;
            const float pulse = 1.0f + qSin(p.m_pulsePhase) * 0.04f;
            p.m_item->setScale(p.m_scale * pulse);
        }
    }
}