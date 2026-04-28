#include "clickablehex.h"
#include "treerenderer.h"   // full include aquí, no en el .h — evita includes circulares

void ClickableHex::mousePressEvent(QGraphicsSceneMouseEvent*) {
    m_renderer->onHexClicked(m_data);
}

void ClickableHex::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
    setPen(QPen(QColor("#ffffff"), 2));
    m_renderer->onHexHovered(m_data, QCursor::pos());
    update();
}

void ClickableHex::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
    setPen(QPen(m_data->m_unlocked ? QColor("#ff4444") : QColor("#5a0000"), 2));
    m_renderer->onHexLeft();
    update();
}