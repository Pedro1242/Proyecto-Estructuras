#include "treerenderer.h"
#include "clickablehex.h"
#include <QGraphicsView>
#include <QGraphicsLineItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsTextItem>
#include <QGraphicsDropShadowEffect>
#include <QPolygonF>
#include <QVector>
#include <QFont>
#include <QPen>
#include <QBrush>
#include <QtMath>

// ── Static helpers ────────────────────────────────────────────────────────────

static QPolygonF makeHexagon(qreal cx, qreal cy, qreal radius) {
    QPolygonF hex;
    hex.reserve(6);
    for (int i = 0; i < 6; ++i) {
        const qreal angle = qDegreesToRadians(60.0 * i);
        hex << QPointF(cx + radius * qCos(angle), cy + radius * qSin(angle));
    }
    return hex;
}

static int subtreeWidth(SkillNode* node) {
    if (!node || node->m_children.isEmpty()) return 1;
    int total = 0;
    for (auto* child : node->m_children)
        total += subtreeWidth(child);
    return total;
}

// ── Construction ──────────────────────────────────────────────────────────────

TreeRenderer::TreeRenderer(BackgroundScene* scene, QGraphicsView* view,
                           SkillTree* tree, QObject* parent)
    : QObject(parent), m_scene(scene), m_view(view), m_tree(tree)
{}

void TreeRenderer::setTree(SkillTree* tree) {
    m_tree = tree;
}

// ── Public API ────────────────────────────────────────────────────────────────

void TreeRenderer::redraw() {
    const auto items = m_scene->items();
    for (auto* item : items) {
        if (item->zValue() >= 0)
            m_scene->removeItem(item);
    }
    m_nodePositions.clear();
    drawTree(m_tree->m_root, 0, 0, 110.0, 0);

    QRectF treeBounds;
    for (auto* item : m_scene->items()) {
        if (item->zValue() >= 0)
            treeBounds = treeBounds.united(
                item->mapToScene(item->boundingRect()).boundingRect());
    }

    m_scene->setSceneRect(treeBounds.adjusted(-80, -80, 80, 80));
    m_view->setTransform(QTransform());
    m_view->centerOn(treeBounds.center());
}

// ── Private: layout ───────────────────────────────────────────────────────────

void TreeRenderer::drawTree(SkillNode* node, qreal x, qreal y,
                            qreal hSpacing, int level) {
    if (!node) return;
    m_nodePositions[node] = QPointF(x, y);

    const int n = node->m_children.size();
    if (n == 0) { drawNode(node, x, y); return; }

    QVector<int> widths;
    widths.reserve(n);
    int totalWidth = 0;
    for (auto* child : node->m_children) {
        const int w = subtreeWidth(child);
        widths.append(w);
        totalWidth += w;
    }

    const qreal totalPixels = (totalWidth - 1) * hSpacing;
    const qreal startX      = x - totalPixels / 2.0;
    qreal       cursor      = 0;

    for (int i = 0; i < n; ++i) {
        const qreal childX = startX + (cursor + (widths[i] - 1) / 2.0) * hSpacing;
        const qreal childY = y + 130;

        QGraphicsLineItem* line = m_scene->addLine(x, y + 32, childX, childY - 32);
        line->setPen(QPen(node->m_unlocked ? QColor("#8b0000") : QColor("#2a2a2a"), 2));

        drawTree(node->m_children[i], childX, childY, hSpacing, level + 1);
        cursor += widths[i];
    }
    drawNode(node, x, y);
}

// ── Private: visual de un nodo ────────────────────────────────────────────────

void TreeRenderer::drawNode(SkillNode* node, qreal x, qreal y) {
    const bool canUnlock = !node->m_unlocked
                           && node->m_parent
                           && node->m_parent->m_unlocked
                           && m_tree->m_dnaPoints >= node->m_dnaCost;

    const QColor fillColor   = node->m_unlocked ? QColor("#7a0000")
                             : canUnlock         ? QColor("#2a0a0a")
                                              : QColor("#150000");
    const QColor borderColor = node->m_unlocked ? QColor("#ff4444")
                               : canUnlock         ? QColor("#cc3300")
                                                : QColor("#4a0000");

    // ── Hexágono exterior (clickeable) ────────────────────────────────────────
    QPolygonF hex = makeHexagon(x, y, 34);
    ClickableHex* hexItem = new ClickableHex(hex, node, this);
    hexItem->setPen(QPen(borderColor, 2));
    hexItem->setBrush(QBrush(fillColor));
    m_scene->addItem(hexItem);

    // ── Hexágono interior decorativo ──────────────────────────────────────────
    QPolygonF hexInner = makeHexagon(x, y, 28);
    QGraphicsPolygonItem* inner = m_scene->addPolygon(
        hexInner, QPen(borderColor.darker(150), 1), QBrush(Qt::NoBrush));
    inner->setZValue(0.5);

    // ── Glow / pulse ──────────────────────────────────────────────────────────
    if (node->m_unlocked) {
        QGraphicsDropShadowEffect* glow = new QGraphicsDropShadowEffect();
        glow->setColor(QColor("#ff0000"));
        glow->setBlurRadius(25);
        glow->setOffset(0, 0);
        hexItem->setGraphicsEffect(glow);
    } else if (canUnlock) {
        QGraphicsDropShadowEffect* pulse = new QGraphicsDropShadowEffect();
        pulse->setColor(QColor("#ff4400"));
        pulse->setBlurRadius(12);
        pulse->setOffset(0, 0);
        hexItem->setGraphicsEffect(pulse);
    }

    // ── Icono por efecto dominante ────────────────────────────────────────────
    const auto& e = node->m_effect;
    QString icon;
    QColor  iconColor;

    if (node->m_parent == nullptr) {
        icon      = QString(QChar(0x2B22));
        iconColor = QColor("#ff6600");
    } else {
        const float maxVal = qMax(qMax(e.infectivity, e.severity),
                                  qMax(e.lethality,   e.stealth));
        if (maxVal <= 0.0f) {
            icon      = QString(QChar(0x2605));
            iconColor = QColor("#555555");
        } else if (e.infectivity >= maxVal) {
            icon      = QString(QChar(0x2623));
            iconColor = node->m_unlocked ? QColor("#ff4444") : QColor("#4a0000");
        } else if (e.lethality >= maxVal) {
            icon      = QString(QChar(0x2620));
            iconColor = node->m_unlocked ? QColor("#cc0000") : QColor("#3a0000");
        } else if (e.stealth >= maxVal) {
            icon      = QString(QChar(0x25CF));
            iconColor = node->m_unlocked ? QColor("#00cc88") : QColor("#004433");
        } else {
            icon      = QString(QChar(0x26A1));
            iconColor = node->m_unlocked ? QColor("#ff8800") : QColor("#3a2000");
        }
    }

    QGraphicsTextItem* iconItem = m_scene->addText(icon);
    iconItem->setDefaultTextColor(iconColor);
    iconItem->setFont(QFont("Segoe UI Symbol", 11));
    const QRectF ib = iconItem->boundingRect();
    iconItem->setPos(x - ib.width() / 2, y - ib.height() / 2 - 10);
    iconItem->setZValue(2);

    // ── Nombre del nodo ───────────────────────────────────────────────────────
    QGraphicsTextItem* text = m_scene->addText(node->m_name);
    text->setDefaultTextColor(node->m_unlocked ? QColor("#ffffff")
                              : canUnlock       ? QColor("#bbbbbb")
                                               : QColor("#555555"));
    text->setFont(QFont("Segoe UI", 6,
                        node->m_unlocked ? QFont::Bold : QFont::Normal));
    const QRectF tb = text->boundingRect();
    text->setPos(x - tb.width() / 2, y + 2);
    text->setZValue(2);

    // ── Costo ADN ─────────────────────────────────────────────────────────────
    const QString costStr = node->m_dnaCost > 0
                                ? QString("%1 ADN").arg(node->m_dnaCost)
                                : "ROOT";
    QGraphicsTextItem* cost = m_scene->addText(costStr);
    cost->setDefaultTextColor(node->m_unlocked ? QColor("#00ff41") : QColor("#2a5a2a"));
    cost->setFont(QFont("Segoe UI", 6));
    const QRectF cb = cost->boundingRect();
    cost->setPos(x - cb.width() / 2, y + 16);
    cost->setZValue(2);
}