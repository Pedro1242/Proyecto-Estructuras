#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGraphicsPolygonItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsSceneMouseEvent>
#include <QWidget>
#include <QFont>
#include <QPolygonF>
#include <QtMath>
#include <QProgressBar>
#include <QTimer>
#include <QWheelEvent>

// ── Hexágono clickeable ──────────────────────────────────────────────────────
class ClickableHex : public QGraphicsPolygonItem {
public:
    SkillNode* data;
    MainWindow* mainWin;

    ClickableHex(const QPolygonF& poly, SkillNode* node, MainWindow* win,
                 QGraphicsItem* parent = nullptr)
        : QGraphicsPolygonItem(poly, parent), data(node), mainWin(win) {
        setAcceptHoverEvents(true);
        setCursor(Qt::PointingHandCursor);
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent*) override {
        mainWin->onNodeClicked(data);
    }
    void hoverEnterEvent(QGraphicsSceneHoverEvent*) override {
        setPen(QPen(QColor("#ffffff"), 2));
        update();
    }
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override {
        setPen(QPen(data->unlocked ? QColor("#ff4444") : QColor("#5a0000"), 2));
        update();
    }
};

// ── Helper hexágono ──────────────────────────────────────────────────────────
static QPolygonF makeHexagon(qreal cx, qreal cy, qreal radius) {
    QPolygonF hex;
    for (int i = 0; i < 6; ++i) {
        qreal angle = qDegreesToRadians(60.0 * i);
        hex << QPointF(cx + radius * qCos(angle), cy + radius * qSin(angle));
    }
    return hex;
}

// ── MainWindow ───────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    tree       = new SkillTree();
    population = new Population(tree, this);

    buildUI();
    redrawTree();

    population->start();

    connect(population, &Population::populationUpdated,
            this, &MainWindow::onPopulationUpdated);
    connect(population, &Population::dnaEarned,
            this, &MainWindow::onDnaEarned);
}

MainWindow::~MainWindow() { delete tree; }

void MainWindow::buildUI() {
    setWindowTitle("🦠 Plague Skill Tree");
    setMinimumSize(1150, 720);
    setStyleSheet("background-color: #0a0a0a; color: #ffffff;");

    scene = new BackgroundScene(this);
    scene->startAnimation();

    view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet("border: 1px solid #3a0000;");
    view->setMinimumWidth(750);
    // ✅ Fix 1: zoom y arrastre con mouse
    view->setDragMode(QGraphicsView::ScrollHandDrag);
    view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    view->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QWidget* panel = new QWidget();
    panel->setStyleSheet("background-color: #111111; border-left: 1px solid #3a0000;");
    panel->setFixedWidth(300);

    QVBoxLayout* panelLayout = new QVBoxLayout(panel);
    panelLayout->setSpacing(8);
    panelLayout->setContentsMargins(10, 10, 10, 10);

    // ── ADN label ─────────────────────────────────────────────────────────
    dnaLabel = new QLabel("🧬 ADN disponible: 20");
    dnaLabel->setStyleSheet("color: #00ff41; font-size: 14px; font-weight: bold;");
    panelLayout->addWidget(dnaLabel);

    // ── Botones ADN y Reset ───────────────────────────────────────────────
    QHBoxLayout* actionRow = new QHBoxLayout();
    actionRow->setSpacing(6);

    auto makeActionBtn = [](const QString& text, const QString& color) {
        QPushButton* btn = new QPushButton(text);
        btn->setStyleSheet(QString(
                               "QPushButton { background:#0a1a0a; color:%1; "
                               "border:1px solid %1; border-radius:4px; padding:5px; font-size:11px; }"
                               "QPushButton:hover { background:#1a2a1a; }").arg(color));
        return btn;
    };

    QPushButton* btnAddDna = makeActionBtn("🧬 +10 ADN", "#00ff41");
    QPushButton* btnReset  = makeActionBtn("🔄 Reset",   "#ff6600");
    actionRow->addWidget(btnAddDna);
    actionRow->addWidget(btnReset);
    panelLayout->addLayout(actionRow);

    // ── Info nodo seleccionado ────────────────────────────────────────────
    statsLabel = new QLabel("Selecciona un nodo\npara ver sus estadísticas.");
    statsLabel->setStyleSheet("color: #cccccc; font-size: 12px; "
                              "background: #1a0000; padding: 8px; border-radius: 4px;");
    statsLabel->setWordWrap(true);
    statsLabel->setFixedHeight(100);
    panelLayout->addWidget(statsLabel);

    // ── Estadísticas bacteria ─────────────────────────────────────────────
    QLabel* statsTitle = new QLabel("── Estadísticas ──");
    statsTitle->setAlignment(Qt::AlignCenter);
    statsTitle->setStyleSheet("color: #8b0000; font-size: 12px; margin-top: 4px;");
    panelLayout->addWidget(statsTitle);

    auto makeStatRow = [&](const QString& label, const QString& color,
                           QProgressBar*& bar, QLabel*& valLabel) {
        QWidget* row = new QWidget();
        QHBoxLayout* rl = new QHBoxLayout(row);
        rl->setContentsMargins(0, 0, 0, 0);
        rl->setSpacing(4);

        QLabel* lbl = new QLabel(label);
        lbl->setStyleSheet("color:#aaaaaa; font-size:10px;");
        lbl->setFixedWidth(78);

        bar = new QProgressBar();
        bar->setRange(0, 100);
        bar->setTextVisible(false);
        bar->setFixedHeight(10);
        bar->setStyleSheet(QString(
                               "QProgressBar { background:#1a0000; border-radius:4px; }"
                               "QProgressBar::chunk { background:%1; border-radius:4px; }").arg(color));

        valLabel = new QLabel("0%");
        valLabel->setStyleSheet(QString("color:%1; font-size:10px; font-weight:bold;").arg(color));
        valLabel->setFixedWidth(32);

        rl->addWidget(lbl);
        rl->addWidget(bar);
        rl->addWidget(valLabel);
        panelLayout->addWidget(row);
    };

    makeStatRow("🦠 Infectiv.",  "#ff4444", barInfectivity, lblInfVal);
    makeStatRow("🤒 Severidad", "#ff8800", barSeverity,    lblSevVal);
    makeStatRow("💀 Letalidad", "#cc0000", barLethality,   lblLetVal);
    makeStatRow("👁 Sigilo",    "#00cc88", barStealth,     lblStlVal);

    updateStatsPanel();

    // ── Población global ──────────────────────────────────────────────────
    QLabel* popTitle = new QLabel("── Población Global ──");
    popTitle->setAlignment(Qt::AlignCenter);
    popTitle->setStyleSheet("color: #8b0000; font-size: 12px;");
    panelLayout->addWidget(popTitle);

    auto makePopLabel = [&](const QString& icon, const QString& color) {
        QLabel* lbl = new QLabel(icon + "0");
        lbl->setStyleSheet(QString("color:%1; font-size:10px; font-weight:bold;").arg(color));
        return lbl;
    };

    lblPopHealthy  = makePopLabel("💚 Sanos:      ", "#00cc44");
    lblPopInfected = makePopLabel("🔴 Infectados: ", "#ff4444");
    lblPopDead     = makePopLabel("💀 Muertos:    ", "#888888");

    barInfectedPop = new QProgressBar();
    barInfectedPop->setRange(0, 100);
    barInfectedPop->setTextVisible(false);
    barInfectedPop->setFixedHeight(8);
    barInfectedPop->setStyleSheet(
        "QProgressBar { background:#003300; border-radius:3px; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #ff4444, stop:1 #cc0000); border-radius:3px; }");

    panelLayout->addWidget(lblPopHealthy);
    panelLayout->addWidget(lblPopInfected);
    panelLayout->addWidget(lblPopDead);
    panelLayout->addWidget(barInfectedPop);

    // ── Recorridos ────────────────────────────────────────────────────────
    QLabel* recLbl = new QLabel("── Recorridos ──");
    recLbl->setAlignment(Qt::AlignCenter);
    recLbl->setStyleSheet("color: #8b0000; font-size: 12px;");
    panelLayout->addWidget(recLbl);

    auto makeBtn = [](const QString& text) {
        QPushButton* btn = new QPushButton(text);
        btn->setStyleSheet("QPushButton { background:#1e0000; color:#ff4444; "
                           "border:1px solid #8b0000; border-radius:4px; padding:6px; }"
                           "QPushButton:hover { background:#3a0000; }");
        return btn;
    };

    QPushButton* btnPre  = makeBtn("▶ PreOrden (DFS)");
    QPushButton* btnIn   = makeBtn("▶ InOrden");
    QPushButton* btnPost = makeBtn("▶ PostOrden");
    QPushButton* btnBFS  = makeBtn("▶ BFS (por niveles)");

    panelLayout->addWidget(btnPre);
    panelLayout->addWidget(btnIn);
    panelLayout->addWidget(btnPost);
    panelLayout->addWidget(btnBFS);

    traversalList = new QListWidget();
    traversalList->setStyleSheet("background:#0a0a0a; color:#00ff41; "
                                 "border:1px solid #3a0000; font-size: 11px;");
    panelLayout->addWidget(traversalList);
    panelLayout->addStretch();

    connect(btnPre,    &QPushButton::clicked, this, &MainWindow::onPreOrder);
    connect(btnIn,     &QPushButton::clicked, this, &MainWindow::onInOrder);
    connect(btnPost,   &QPushButton::clicked, this, &MainWindow::onPostOrder);
    connect(btnBFS,    &QPushButton::clicked, this, &MainWindow::onBFS);
    connect(btnAddDna, &QPushButton::clicked, this, &MainWindow::onAddDna);
    connect(btnReset,  &QPushButton::clicked, this, &MainWindow::onReset);

    QWidget* central = new QWidget();
    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(view);
    mainLayout->addWidget(panel);
    setCentralWidget(central);
}

// ✅ Fix 1: zoom con rueda del mouse sobre la vista
void MainWindow::wheelEvent(QWheelEvent* event) {
    if (view->underMouse()) {
        double factor = event->angleDelta().y() > 0 ? 1.15 : 0.87;
        view->scale(factor, factor);
        event->accept();
    } else {
        QMainWindow::wheelEvent(event);
    }
}

void MainWindow::redrawTree() {
    // ✅ Fix 1: no borra partículas del fondo (zValue < 0)
    const auto items = scene->items();
    for (auto* item : items) {
        if (item->zValue() >= 0)
            scene->removeItem(item);
    }
    nodePositions.clear();
    drawTree(tree->root, 0, 0, 220.0, 0);

    QRectF treeBounds;
    for (auto* item : scene->items()) {
        if (item->zValue() >= 0)
            treeBounds = treeBounds.united(
                item->mapToScene(item->boundingRect()).boundingRect());
    }

    scene->setSceneRect(treeBounds.adjusted(-80, -80, 80, 80));
    view->setTransform(QTransform()); // reset zoom
    view->centerOn(treeBounds.center());
}

void MainWindow::drawTree(SkillNode* node, qreal x, qreal y, qreal hSpacing, int level) {
    if (!node) return;
    nodePositions[node] = QPointF(x, y);

    int n = node->children.size();
    qreal startX = x - (hSpacing * (n - 1)) / 2.0;

    for (int i = 0; i < n; ++i) {
        qreal childX = startX + i * hSpacing;
        qreal childY = y + 130;

        QGraphicsLineItem* line = scene->addLine(x, y + 32, childX, childY - 32);
        QPen pen(node->unlocked ? QColor("#8b0000") : QColor("#2a2a2a"), 2);
        line->setPen(pen);

        drawTree(node->children[i], childX, childY, hSpacing * 0.55, level + 1);
    }
    drawNode(node, x, y);
}

void MainWindow::drawNode(SkillNode* node, qreal x, qreal y) {
    bool canUnlock = !node->unlocked
                     && node->parent
                     && node->parent->unlocked
                     && tree->dnaPoints >= node->dnaCost;

    QColor fillColor   = node->unlocked ? QColor("#7a0000")
                       : canUnlock      ? QColor("#2a0a0a")
                                      : QColor("#150000");
    QColor borderColor = node->unlocked ? QColor("#ff4444")
                         : canUnlock      ? QColor("#cc3300")
                                        : QColor("#4a0000");

    QPolygonF hex = makeHexagon(x, y, 34);
    ClickableHex* hexItem = new ClickableHex(hex, node, this);
    hexItem->setPen(QPen(borderColor, 2));
    hexItem->setBrush(QBrush(fillColor));
    scene->addItem(hexItem);

    QPolygonF hexInner = makeHexagon(x, y, 28);
    QGraphicsPolygonItem* inner = scene->addPolygon(hexInner,
                                                    QPen(borderColor.darker(150), 1), QBrush(Qt::NoBrush));
    inner->setZValue(0.5);

    if (node->unlocked) {
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

    QGraphicsTextItem* text = scene->addText(node->name);
    text->setDefaultTextColor(node->unlocked ? QColor("#ffffff")
                              : canUnlock    ? QColor("#bbbbbb")
                                             : QColor("#555555"));
    QFont f("Segoe UI", 7, node->unlocked ? QFont::Bold : QFont::Normal);
    text->setFont(f);
    QRectF tb = text->boundingRect();
    text->setPos(x - tb.width() / 2, y - tb.height() / 2 - 4);
    text->setZValue(2);

    QString costStr = node->dnaCost > 0 ? QString("🧬 %1").arg(node->dnaCost) : "⭐";
    QGraphicsTextItem* cost = scene->addText(costStr);
    cost->setDefaultTextColor(node->unlocked ? QColor("#00ff41") : QColor("#2a5a2a"));
    cost->setFont(QFont("Segoe UI", 6));
    QRectF cb = cost->boundingRect();
    cost->setPos(x - cb.width() / 2, y + 10);
    cost->setZValue(2);
}

void MainWindow::onNodeClicked(SkillNode* node) {
    QString estado = node->unlocked ? "✅ DESBLOQUEADO" : "🔒 BLOQUEADO";
    QString padre  = node->parent ? node->parent->name : "—";

    statsLabel->setText(
        QString("<b style='color:#ff4444'>%1</b><br>"
                "<span style='color:#888'>%2</span><br><br>"
                "Estado: %3<br>"
                "Costo: <b style='color:#00ff41'>🧬 %4</b><br>"
                "Padre: %5 | Hijos: %6")
            .arg(node->name).arg(node->description)
            .arg(estado).arg(node->dnaCost)
            .arg(padre).arg(node->children.size())
        );
    statsLabel->setTextFormat(Qt::RichText);

    if (!node->unlocked) {
        bool ok = tree->unlock(node);
        if (ok) {
            updateDnaLabel();
            updateStatsPanel();
            redrawTree();
            statsLabel->setText(
                QString("<b style='color:#00ff41'>✅ %1</b><br>"
                        "<span style='color:#888'>%2</span><br><br>"
                        "🧬 ADN gastado: <b style='color:#ff4444'>%3</b>")
                    .arg(node->name).arg(node->description).arg(node->dnaCost)
                );
            statsLabel->setTextFormat(Qt::RichText);
        } else {
            QString razon;
            if (node->parent && !node->parent->unlocked)
                razon = "⚠️ Desbloquea primero:<br><b>" + node->parent->name + "</b>";
            else if (tree->dnaPoints < node->dnaCost)
                razon = QString("⚠️ ADN insuficiente.<br>Necesitas <b>%1</b>, tienes <b>%2</b>.")
                            .arg(node->dnaCost).arg(tree->dnaPoints);
            else
                razon = "Ya está desbloqueado.";

            statsLabel->setText(
                QString("<b style='color:#ff4444'>%1</b><br><br>%2").arg(node->name).arg(razon)
                );
            statsLabel->setTextFormat(Qt::RichText);
        }
    }
}

void MainWindow::onAddDna() {
    tree->dnaPoints += 10;
    updateDnaLabel();
    updateStatsPanel();
    redrawTree();
    statsLabel->setText("<b style='color:#00ff41'>🧬 +10 ADN agregado!</b>");
    statsLabel->setTextFormat(Qt::RichText);
}

void MainWindow::onReset() {
    population->stop();
    delete tree;
    tree = new SkillTree();
    population->skillTree = tree;
    population->reset();
    population->start();
    updateDnaLabel();
    updateStatsPanel();
    redrawTree();
    traversalList->clear();
    statsLabel->setText("Árbol reiniciado.<br>Selecciona un nodo para comenzar.");
    statsLabel->setTextFormat(Qt::RichText);
}

void MainWindow::updateDnaLabel() {
    dnaLabel->setText(QString("🧬 ADN disponible: %1").arg(tree->dnaPoints));
}

void MainWindow::updateStatsPanel() {
    tree->recalcStats();
    auto& s = tree->stats;
    barInfectivity->setValue((int)s.infectivity);
    barSeverity->setValue((int)s.severity);
    barLethality->setValue((int)s.lethality);
    barStealth->setValue((int)s.stealth);
    lblInfVal->setText(QString("%1%").arg((int)s.infectivity));
    lblSevVal->setText(QString("%1%").arg((int)s.severity));
    lblLetVal->setText(QString("%1%").arg((int)s.lethality));
    lblStlVal->setText(QString("%1%").arg((int)s.stealth));
}

void MainWindow::onPopulationUpdated() {
    auto fmt = [](long long n) -> QString {
        if (n >= 1'000'000'000) return QString("%1B").arg(n / 1'000'000'000.0, 0, 'f', 2);
        if (n >= 1'000'000)     return QString("%1M").arg(n / 1'000'000.0,     0, 'f', 1);
        if (n >= 1'000)         return QString("%1K").arg(n / 1'000.0,         0, 'f', 1);
        return QString::number(n);
    };

    lblPopHealthy->setText( QString("💚 Sanos:      %1").arg(fmt(population->healthy)));
    lblPopInfected->setText(QString("🔴 Infectados: %1").arg(fmt(population->infected)));
    lblPopDead->setText(    QString("💀 Muertos:    %1").arg(fmt(population->dead)));

    int pct = (int)(((population->infected + population->dead)
                      / (float)population->total) * 100.0f);
    barInfectedPop->setValue(qBound(0, pct, 100));

    updateDnaLabel();
}

void MainWindow::onDnaEarned(int amount) {
    Q_UNUSED(amount);
    dnaLabel->setStyleSheet("color: #ffffff; font-size: 14px; font-weight: bold;");
    QTimer::singleShot(300, this, [this](){
        dnaLabel->setStyleSheet("color: #00ff41; font-size: 14px; font-weight: bold;");
    });
}

void MainWindow::showTraversal(QList<SkillNode*>& nodes, const QString& title) {
    traversalList->clear();
    traversalList->addItem("── " + title + " ──");
    for (int i = 0; i < nodes.size(); ++i) {
        QString state = nodes[i]->unlocked ? "✅" : "🔒";
        traversalList->addItem(QString("%1. %2 %3").arg(i+1).arg(state).arg(nodes[i]->name));
    }
}

void MainWindow::onPreOrder()  { QList<SkillNode*> r; tree->preOrder(tree->root, r);  showTraversal(r, "PreOrden"); }
void MainWindow::onInOrder()   { QList<SkillNode*> r; tree->inOrder(tree->root, r);   showTraversal(r, "InOrden"); }
void MainWindow::onPostOrder() { QList<SkillNode*> r; tree->postOrder(tree->root, r); showTraversal(r, "PostOrden"); }
void MainWindow::onBFS()       { QList<SkillNode*> r; tree->bfs(r);                   showTraversal(r, "BFS por niveles"); }