#pragma once
#include <QGraphicsScene>
#include <QTimer>
#include <QList>
#include <QGraphicsItem>

/*!
 * \brief Animated background particle representing a micro-organism.
 */
struct Particle {
    QGraphicsItem* m_item       { nullptr };
    float          m_vx         { 0.0f   };
    float          m_vy         { 0.0f   };
    float          m_rotSpeed   { 0.0f   };
    float          m_scale      { 1.0f   };
    float          m_pulsePhase { 0.0f   };
    float          m_pulseSpeed { 0.0f   };
    int            m_layer      { 0      };
    int            m_type       { 0      };
};

/*!
 * \brief Animated QGraphicsScene with layered micro-organism particles.
 *
 * Renders blood cells, cocci, bacilli and spirilla in a parallax system.
 */
class BackgroundScene : public QGraphicsScene {
    Q_OBJECT

public:
    static constexpr int   PARTICLE_COUNT   { 70     };
    static constexpr float CANVAS_W         { 1600.0f};
    static constexpr float CANVAS_H         { 900.0f };
    static constexpr int   TICK_INTERVAL_MS { 16     };  // ~60 fps

    explicit BackgroundScene(QObject* parent = nullptr);

    BackgroundScene(const BackgroundScene&)            = delete;
    BackgroundScene& operator=(const BackgroundScene&) = delete;

    void startAnimation();

private slots:
    void animateTick();

private:
    QTimer*         m_animTimer;
    QList<Particle> m_particles;

    void spawnParticles(int count);

    QGraphicsItem* makeBloodCell  (float x, float y, float size, int layer) const;
    QGraphicsItem* makeCoccus     (float x, float y, float size, int layer) const;
    QGraphicsItem* makeBacillus   (float x, float y, float size, int layer) const;
    QGraphicsItem* makeSpirillum  (float x, float y, float size, int layer) const;
};