#pragma once
#include <QGraphicsPolygonItem>
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QCursor>
#include "skillnode.h"

class TreeRenderer;   // forward declaration — igual que antes pero con TreeRenderer

class ClickableHex : public QGraphicsPolygonItem {
public:
    SkillNode*    m_data;
    TreeRenderer* m_renderer;

    ClickableHex(const QPolygonF& poly, SkillNode* node, TreeRenderer* renderer,
                 QGraphicsItem* parent = nullptr)
        : QGraphicsPolygonItem(poly, parent), m_data(node), m_renderer(renderer)
    {
        setAcceptHoverEvents(true);
        setCursor(Qt::PointingHandCursor);
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent*) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
};