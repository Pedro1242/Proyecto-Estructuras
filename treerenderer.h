#pragma once
#include <QObject>
#include <QMap>
#include <QPointF>
#include "skilltree.h"
#include "backgroundscene.h"

class QGraphicsView;

class TreeRenderer : public QObject {
    Q_OBJECT

public:
    explicit TreeRenderer(BackgroundScene* scene, QGraphicsView* view,
                          SkillTree* tree, QObject* parent = nullptr);

    void setTree(SkillTree* tree);
    void redraw();

    // ── Llamados por ClickableHex ─────────────────────────────────────────────
    void onHexClicked(SkillNode* node)                    { emit nodeClicked(node); }
    void onHexHovered(SkillNode* node, const QPoint& pos) { emit nodeHovered(node, pos); }
    void onHexLeft()                                       { emit nodeLeft(); }

signals:
    void nodeClicked(SkillNode* node);
    void nodeHovered(SkillNode* node, const QPoint& globalPos);
    void nodeLeft();

private:
    BackgroundScene* m_scene { nullptr };
    QGraphicsView*   m_view  { nullptr };
    SkillTree*       m_tree  { nullptr };

    QMap<SkillNode*, QPointF> m_nodePositions;

    void drawTree(SkillNode* node, qreal x, qreal y, qreal hSpacing, int level);
    void drawNode(SkillNode* node, qreal x, qreal y);
};