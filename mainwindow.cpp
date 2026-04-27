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
#include <QVector>
#include <QCursor>
#include <QMessageBox>

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
        mainWin->showNodeTooltip(data, QCursor::pos());
        update();
    }
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override {
        setPen(QPen(data->unlocked ? QColor("#ff4444") : QColor("#5a0000"), 2));
        mainWin->hideNodeTooltip();
        update();
    }
};

static QPolygonF makeHexagon(qreal cx, qreal cy, qreal radius) {
    QPolygonF hex;
    for (int i = 0; i < 6; ++i) {
        qreal angle = qDegreesToRadians(60.0 * i);
        hex << QPointF(cx + radius * qCos(angle), cy + radius * qSin(angle));
    }
    return hex;
}

static int subtreeWidth(SkillNode* node) {
    if (!node || node->children.isEmpty()) return 1;
    int total = 0;
    for (auto* child : node->children)
        total += subtreeWidth(child);
    return total;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    tree       = new SkillTree();
    population = new Population(tree, this);

    buildUI();
    redrawTree();

    population->start();

    connect(population, &Population::populationUpdated, this, &MainWindow::onPopulationUpdated);
    connect(population, &Population::dnaEarned,         this, &MainWindow::onDnaEarned);
    connect(population, &Population::dayChanged,        this, &MainWindow::onDayChanged);
    connect(population, &Population::cureUpdated,       this, &MainWindow::onCureUpdated);
    connect(population, &Population::cureCompleted,     this, &MainWindow::onCureCompleted);
    connect(population, &Population::playerWon,         this, &MainWindow::onPlayerWon);
}

MainWindow::~MainWindow() { delete tree; }

QString MainWindow::pickNews(int day) {
    auto& s       = tree->stats;
    long long inf  = population->infected;
    long long dead = population->dead;

    if (day == 0)
        return "Reportan un caso inusual de fiebre en Asia Central...  Se investigan los origenes.";
    if (dead > 1'000'000'000LL)
        return "MAS DE MIL MILLONES DE MUERTOS.  Los gobiernos han caido.  No hay esperanza.";
    if (dead > 100'000'000LL)
        return "Colapso civilizatorio.  El ejercito no puede contener el avance.  Cifra de muertos supera los 100 millones.";
    if (inf > 3'000'000'000LL)
        return "PANDEMIA GLOBAL TOTAL.  Mas de la mitad de la humanidad infectada.  La ONU declara el fin del mundo moderno.";
    if (inf > 1'000'000'000LL)
        return "MIL MILLONES DE INFECTADOS.  Los aeropuertos cierran.  Las fronteras desaparecen.";
    if (s.lethality > 60 && inf > 10'000'000LL)
        return "Medicos reportan una tasa de mortalidad aterradora.  'Nunca habiamos visto algo asi' -- CDC.";
    if (s.stealth > 70 && inf > 1'000'000LL)
        return "Cientificos no logran identificar el agente causante.  El patogeno parece invisible a los test convencionales.";
    if (inf > 500'000'000LL)
        return "Europa en cuarentena total.  America Latina reporta colapso hospitalario.  Africa sin suministros medicos.";
    if (inf > 100'000'000LL)
        return "La OMS eleva la alerta al maximo nivel.  Cientos de millones afectados en 6 continentes.";
    if (inf > 10'000'000LL)
        return "Panico en los mercados globales.  Decenas de millones de casos confirmados.  Vacuna en desarrollo.";
    if (inf > 1'000'000LL)
        return "La OMS declara emergencia sanitaria internacional.  Casos reportados en 47 paises.";
    if (inf > 100'000LL)
        return "Brote se extiende a nuevas regiones.  Autoridades piden calma.  'Situacion bajo control', dice ministro.";
    if (inf > 10'000LL)
        return "Nuevos casos reportados en tres continentes.  Cientificos analizan la secuencia genetica del patogeno.";
    if (inf > 1'000LL)
        return "Cluster de enfermos en zona rural.  Autoridades locales no descartan nuevo brote.";
    return "Caso aislado bajo vigilancia epidemiologica.  'No hay motivo de alarma' -- ministerio de salud.";
}

void MainWindow::onDayChanged(int day) {
    lblDay->setText(QString("Dia %1  |  Paciente 0").arg(day));
    QString msg = QString("  >>> DIA %1 <<<  ").arg(day) + pickNews(day);
    pendingNews = QString(60, ' ') + msg + QString(30, ' ');
}

void MainWindow::tickNews() {
    if (currentNews.isEmpty()) return;
    newsScrollPos++;

    if (newsScrollPos >= currentNews.length()) {
        newsScrollPos = 0;
        if (!pendingNews.isEmpty()) {
            currentNews = pendingNews;
            pendingNews.clear();
        }
    }

    const int WINDOW = 90;
    QString visible = currentNews.mid(newsScrollPos, WINDOW);
    if (visible.length() < WINDOW)
        visible += currentNews.left(WINDOW - visible.length());
    lblNewsTicker->setText(visible);
}

void MainWindow::buildUI() {
    setWindowTitle("Plague Skill Tree");
    setMinimumSize(1150, 720);
    setStyleSheet("background-color: #0a0a0a; color: #ffffff;");

    nodeTooltip = new QLabel(this);
    nodeTooltip->setStyleSheet(
        "background-color: #1a0000;"
        "color: #ffffff;"
        "border: 1px solid #8b0000;"
        "border-radius: 5px;"
        "padding: 8px 12px;"
        "font-family: 'Courier New';"
        "font-size: 11px;"
        );
    nodeTooltip->setTextFormat(Qt::RichText);
    nodeTooltip->setWordWrap(true);
    nodeTooltip->setFixedWidth(220);
    nodeTooltip->setAttribute(Qt::WA_TransparentForMouseEvents);
    nodeTooltip->hide();

    scene = new BackgroundScene(this);
    scene->startAnimation();

    view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet("border: 1px solid #3a0000;");
    view->setMinimumWidth(750);
    view->setDragMode(QGraphicsView::ScrollHandDrag);
    view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    view->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    lblNewsTicker = new QLabel();
    lblNewsTicker->setStyleSheet(
        "background-color: #0d0d0d;"
        "color: #ff4444;"
        "font-family: 'Courier New';"
        "font-size: 11px;"
        "font-weight: bold;"
        "padding: 4px 8px;"
        "border-bottom: 1px solid #3a0000;");
    lblNewsTicker->setFixedHeight(24);

    currentNews   = QString(60, ' ')
                  + "  >>> SISTEMA ACTIVO <<<  Vigilancia epidemiologica iniciada.  Paciente 0 bajo observacion.  "
                  + QString(30, ' ');
    pendingNews   = QString();
    newsScrollPos = 0;

    newsTimer = new QTimer(this);
    newsTimer->setInterval(120);
    connect(newsTimer, &QTimer::timeout, this, &MainWindow::tickNews);
    newsTimer->start();

    QWidget* panel = new QWidget();
    panel->setStyleSheet("background-color: #111111; border-left: 1px solid #3a0000;");
    panel->setFixedWidth(300);

    QVBoxLayout* panelLayout = new QVBoxLayout(panel);
    panelLayout->setSpacing(8);
    panelLayout->setContentsMargins(10, 10, 10, 10);

    lblDay = new QLabel("Dia 0  |  Paciente 0");
    lblDay->setStyleSheet("color: #ff6600; font-size: 11px; font-weight: bold;"
                          "background: #1a0800; padding: 4px 6px; border-radius: 3px;");
    panelLayout->addWidget(lblDay);

    dnaLabel = new QLabel("ADN disponible: 20");
    dnaLabel->setStyleSheet("color: #00ff41; font-size: 14px; font-weight: bold;");
    panelLayout->addWidget(dnaLabel);

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

    QPushButton* btnAddDna = makeActionBtn("+10 ADN", "#00ff41");
    QPushButton* btnReset  = makeActionBtn("Reset",   "#ff6600");
    actionRow->addWidget(btnAddDna);
    actionRow->addWidget(btnReset);
    panelLayout->addLayout(actionRow);

    statsLabel = new QLabel("Selecciona un nodo\npara ver sus estadisticas.");
    statsLabel->setStyleSheet("color: #cccccc; font-size: 12px; "
                              "background: #1a0000; padding: 8px; border-radius: 4px;");
    statsLabel->setWordWrap(true);
    statsLabel->setFixedHeight(100);
    panelLayout->addWidget(statsLabel);

    QLabel* statsTitle = new QLabel("---- Estadisticas ----");
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

    makeStatRow("Infectiv.",  "#ff4444", barInfectivity, lblInfVal);
    makeStatRow("Severidad",  "#ff8800", barSeverity,    lblSevVal);
    makeStatRow("Letalidad",  "#cc0000", barLethality,   lblLetVal);
    makeStatRow("Sigilo",     "#00cc88", barStealth,     lblStlVal);

    updateStatsPanel();

    QLabel* popTitle = new QLabel("---- Poblacion Global ----");
    popTitle->setAlignment(Qt::AlignCenter);
    popTitle->setStyleSheet("color: #8b0000; font-size: 12px;");
    panelLayout->addWidget(popTitle);

    auto makePopLabel = [&](const QString& prefix, const QString& color) {
        QLabel* lbl = new QLabel(prefix + "0");
        lbl->setStyleSheet(QString("color:%1; font-size:10px; font-weight:bold;").arg(color));
        return lbl;
    };

    lblPopHealthy  = makePopLabel("Sanos:      ", "#00cc44");
    lblPopInfected = makePopLabel("Infectados: ", "#ff4444");
    lblPopDead     = makePopLabel("Muertos:    ", "#888888");

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

    // --- Barra de Cura ---
    lblCureTitle = new QLabel("---- Investigacion Cura ----");
    lblCureTitle->setAlignment(Qt::AlignCenter);
    lblCureTitle->setStyleSheet("color: #00cc88; font-size: 12px; margin-top: 4px;");
    panelLayout->addWidget(lblCureTitle);

    QWidget* cureRow = new QWidget();
    QHBoxLayout* cureLayout = new QHBoxLayout(cureRow);
    cureLayout->setContentsMargins(0, 0, 0, 0);
    cureLayout->setSpacing(4);

    QLabel* cureLbl = new QLabel("Vacuna:");
    cureLbl->setStyleSheet("color:#aaaaaa; font-size:10px;");
    cureLbl->setFixedWidth(78);

    barCure = new QProgressBar();
    barCure->setRange(0, 100);
    barCure->setValue(0);
    barCure->setTextVisible(false);
    barCure->setFixedHeight(10);
    barCure->setStyleSheet(
        "QProgressBar { background:#001a00; border-radius:4px; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #00cc88, stop:1 #00ffaa); border-radius:4px; }");

    lblCureVal = new QLabel("0%");
    lblCureVal->setStyleSheet("color:#00cc88; font-size:10px; font-weight:bold;");
    lblCureVal->setFixedWidth(32);

    cureLayout->addWidget(cureLbl);
    cureLayout->addWidget(barCure);
    cureLayout->addWidget(lblCureVal);
    panelLayout->addWidget(cureRow);

    QLabel* recLbl = new QLabel("---- Recorridos ----");
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

    QPushButton* btnPre  = makeBtn("> PreOrden (DFS)");
    QPushButton* btnIn   = makeBtn("> InOrden");
    QPushButton* btnPost = makeBtn("> PostOrden");
    QPushButton* btnBFS  = makeBtn("> BFS (por niveles)");

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
    QVBoxLayout* rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    rootLayout->addWidget(lblNewsTicker);

    QWidget* content = new QWidget();
    QHBoxLayout* mainLayout = new QHBoxLayout(content);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(view);
    mainLayout->addWidget(panel);

    rootLayout->addWidget(content);
    setCentralWidget(central);
}

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
    const auto items = scene->items();
    for (auto* item : items) {
        if (item->zValue() >= 0)
            scene->removeItem(item);
    }
    nodePositions.clear();
    drawTree(tree->root, 0, 0, 110.0, 0);

    QRectF treeBounds;
    for (auto* item : scene->items()) {
        if (item->zValue() >= 0)
            treeBounds = treeBounds.united(
                item->mapToScene(item->boundingRect()).boundingRect());
    }

    scene->setSceneRect(treeBounds.adjusted(-80, -80, 80, 80));
    view->setTransform(QTransform());
    view->centerOn(treeBounds.center());
}

void MainWindow::drawTree(SkillNode* node, qreal x, qreal y, qreal hSpacing, int level) {
    if (!node) return;
    nodePositions[node] = QPointF(x, y);

    int n = node->children.size();
    if (n == 0) { drawNode(node, x, y); return; }

    QVector<int> widths;
    int totalWidth = 0;
    for (auto* child : node->children) {
        int w = subtreeWidth(child);
        widths.append(w);
        totalWidth += w;
    }

    qreal totalPixels = (totalWidth - 1) * hSpacing;
    qreal startX      = x - totalPixels / 2.0;
    qreal cursor      = 0;

    for (int i = 0; i < n; ++i) {
        qreal childX = startX + (cursor + (widths[i] - 1) / 2.0) * hSpacing;
        qreal childY = y + 130;

        QGraphicsLineItem* line = scene->addLine(x, y + 32, childX, childY - 32);
        line->setPen(QPen(node->unlocked ? QColor("#8b0000") : QColor("#2a2a2a"), 2));

        drawTree(node->children[i], childX, childY, hSpacing, level + 1);
        cursor += widths[i];
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

    // --- Icono por efecto dominante ---
    auto& e = node->effect;
    QString icon;
    QColor  iconColor;

    if (node->parent == nullptr) {
        icon      = QString("\u2B22");   // ⬢ ROOT
        iconColor = QColor("#ff6600");
    } else {
        float maxVal = qMax(qMax(e.infectivity, e.severity), qMax(e.lethality, e.stealth));
        if (maxVal <= 0.0f) {
            icon      = QString("\u2605");  // ★
            iconColor = QColor("#555555");
        } else if (e.infectivity >= maxVal) {
            icon      = QString("\u2623");  // ☣ infectividad
            iconColor = node->unlocked ? QColor("#ff4444") : QColor("#4a0000");
        } else if (e.lethality >= maxVal) {
            icon      = QString("\u2620");  // ☠ letalidad
            iconColor = node->unlocked ? QColor("#cc0000") : QColor("#3a0000");
        } else if (e.stealth >= maxVal) {
            icon      = QString("\u25CF");  // ● sigilo
            iconColor = node->unlocked ? QColor("#00cc88") : QColor("#004433");
        } else {
            icon      = QString("\u26A1");  // ⚡ severidad
            iconColor = node->unlocked ? QColor("#ff8800") : QColor("#3a2000");
        }
    }

    QGraphicsTextItem* iconItem = scene->addText(icon);
    iconItem->setDefaultTextColor(iconColor);
    iconItem->setFont(QFont("Segoe UI Symbol", 11));
    QRectF ib = iconItem->boundingRect();
    iconItem->setPos(x - ib.width() / 2, y - ib.height() / 2 - 10);
    iconItem->setZValue(2);

    // --- Nombre ---
    QGraphicsTextItem* text = scene->addText(node->name);
    text->setDefaultTextColor(node->unlocked ? QColor("#ffffff")
                              : canUnlock    ? QColor("#bbbbbb")
                                             : QColor("#555555"));
    QFont f("Segoe UI", 6, node->unlocked ? QFont::Bold : QFont::Normal);
    text->setFont(f);
    QRectF tb = text->boundingRect();
    text->setPos(x - tb.width() / 2, y + 2);
    text->setZValue(2);

    // --- Costo ---
    QString costStr = node->dnaCost > 0 ? QString("%1 ADN").arg(node->dnaCost) : "ROOT";
    QGraphicsTextItem* cost = scene->addText(costStr);
    cost->setDefaultTextColor(node->unlocked ? QColor("#00ff41") : QColor("#2a5a2a"));
    cost->setFont(QFont("Segoe UI", 6));
    QRectF cb = cost->boundingRect();
    cost->setPos(x - cb.width() / 2, y + 16);
    cost->setZValue(2);
}

void MainWindow::onNodeClicked(SkillNode* node) {
    QString estado = node->unlocked ? "DESBLOQUEADO" : "BLOQUEADO";
    QString padre  = node->parent ? node->parent->name : "-";

    statsLabel->setText(
        QString("<b style='color:#ff4444'>%1</b><br>"
                "<span style='color:#888'>%2</span><br><br>"
                "Estado: %3<br>"
                "Costo: <b style='color:#00ff41'>%4 ADN</b><br>"
                "Padre: %5 | Hijos: %6")
            .arg(node->name).arg(node->description)
            .arg(estado).arg(node->dnaCost)
            .arg(padre).arg(node->children.size()));
    statsLabel->setTextFormat(Qt::RichText);

    if (!node->unlocked) {
        bool ok = tree->unlock(node);
        if (ok) {
            updateDnaLabel();
            updateStatsPanel();
            redrawTree();
            statsLabel->setText(
                QString("<b style='color:#00ff41'>%1 DESBLOQUEADO</b><br>"
                        "<span style='color:#888'>%2</span><br><br>"
                        "ADN gastado: <b style='color:#ff4444'>%3</b>")
                    .arg(node->name).arg(node->description).arg(node->dnaCost));
            statsLabel->setTextFormat(Qt::RichText);
        } else {
            QString razon;
            if (node->parent && !node->parent->unlocked)
                razon = "Desbloquea primero:<br><b>" + node->parent->name + "</b>";
            else if (tree->dnaPoints < node->dnaCost)
                razon = QString("ADN insuficiente.<br>Necesitas <b>%1</b>, tienes <b>%2</b>.")
                            .arg(node->dnaCost).arg(tree->dnaPoints);
            else
                razon = "Ya esta desbloqueado.";
            statsLabel->setText(
                QString("<b style='color:#ff4444'>%1</b><br><br>%2")
                    .arg(node->name).arg(razon));
            statsLabel->setTextFormat(Qt::RichText);
        }
    }
}

void MainWindow::onAddDna() {
    tree->dnaPoints += 10;
    updateDnaLabel();
    updateStatsPanel();
    redrawTree();
    statsLabel->setText("<b style='color:#00ff41'>+10 ADN agregado!</b>");
    statsLabel->setTextFormat(Qt::RichText);
}

void MainWindow::onReset() {
    population->stop();
    newsTimer->stop();
    delete tree;
    tree = new SkillTree();
    population->skillTree = tree;
    population->reset();
    lblDay->setText("Dia 0  |  Paciente 0");
    currentNews   = QString(60, ' ')
                  + "  >>> RESET <<<  Nuevo patogeno detectado.  Protocolo reiniciado.  "
                  + QString(30, ' ');
    pendingNews   = QString();
    newsScrollPos = 0;

    barCure->setValue(0);
    lblCureVal->setText("0%");
    lblCureTitle->setStyleSheet("color: #00cc88; font-size: 12px; margin-top: 4px;");
    lblCureVal->setStyleSheet("color:#00cc88; font-size:10px; font-weight:bold;");
    barCure->setStyleSheet(
        "QProgressBar { background:#001a00; border-radius:4px; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #00cc88, stop:1 #00ffaa); border-radius:4px; }");

    population->start();
    newsTimer->start();
    updateDnaLabel();
    updateStatsPanel();
    redrawTree();
    traversalList->clear();
    statsLabel->setText("Arbol reiniciado.<br>Selecciona un nodo para comenzar.");
    statsLabel->setTextFormat(Qt::RichText);
}

void MainWindow::updateDnaLabel() {
    dnaLabel->setText(QString("ADN disponible: %1").arg(tree->dnaPoints));
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

    lblPopHealthy->setText( QString("Sanos:      %1").arg(fmt(population->healthy)));
    lblPopInfected->setText(QString("Infectados: %1").arg(fmt(population->infected)));
    lblPopDead->setText(    QString("Muertos:    %1").arg(fmt(population->dead)));

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

void MainWindow::onCureUpdated(float progress) {
    int pct = (int)progress;
    barCure->setValue(pct);
    lblCureVal->setText(QString("%1%").arg(pct));

    if (progress >= 80.0f) {
        barCure->setStyleSheet(
            "QProgressBar { background:#1a0000; border-radius:4px; }"
            "QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
            "stop:0 #ff4444, stop:1 #ff0000); border-radius:4px; }");
        lblCureVal->setStyleSheet("color:#ff4444; font-size:10px; font-weight:bold;");
        lblCureTitle->setStyleSheet("color:#ff4444; font-size:12px; font-weight:bold; margin-top:4px;");
        if (pct == 80) {
            pendingNews = QString(60, ' ')
            + "  !!! ALERTA !!!  Cientificos anuncian vacuna al 80%.  El tiempo se agota.  "
                + QString(30, ' ');
        }
    } else if (progress >= 50.0f) {
        barCure->setStyleSheet(
            "QProgressBar { background:#1a1000; border-radius:4px; }"
            "QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
            "stop:0 #ffaa00, stop:1 #ffdd00); border-radius:4px; }");
        lblCureVal->setStyleSheet("color:#ffaa00; font-size:10px; font-weight:bold;");
        lblCureTitle->setStyleSheet("color:#ffaa00; font-size:12px; font-weight:bold; margin-top:4px;");
        if (pct == 50) {
            pendingNews = QString(60, ' ')
            + "  ATENCION:  Vacuna experimental al 50% de desarrollo.  Los laboratorios trabajan sin descanso.  "
                + QString(30, ' ');
        }
    }
}

void MainWindow::onCureCompleted() {
    newsTimer->stop();
    currentNews = QString(60, ' ')
                  + "  >>> VACUNA COMPLETADA <<<  "
                    "La humanidad ha desarrollado la cura.  El patogeno ha sido erradicado.  DERROTA.  "
                  + QString(30, ' ');
    newsScrollPos = 0;
    newsTimer->start();

    QMessageBox* box = new QMessageBox(this);
    box->setWindowTitle("DERROTA");
    box->setText(
        "<div style='font-family:Courier New; color:#00cc88; font-size:13px;'>"
        "<b style='font-size:16px;'>LA HUMANIDAD HA GANADO</b><br><br>"
        "La vacuna fue desarrollada antes de que el patogeno<br>"
        "pudiera exterminar a la especie.<br><br>"
        "<span style='color:#888888;'>Infectados: " + QString::number(population->infected) +
        "<br>Muertos: "    + QString::number(population->dead) +
        "<br>Dia: "        + QString::number(population->day) + "</span></div>"
        );
    box->setStandardButtons(QMessageBox::Ok);
    box->button(QMessageBox::Ok)->setText("Reintentar");
    box->setStyleSheet(
        "QMessageBox { background:#0a0a0a; color:#ffffff; }"
        "QLabel { color:#ffffff; font-family:'Courier New'; }"
        "QPushButton { background:#1e0000; color:#ff4444; "
        "border:1px solid #8b0000; border-radius:4px; padding:6px 16px; }"
        "QPushButton:hover { background:#3a0000; }"
        );
    if (box->exec() == QMessageBox::Ok)
        onReset();
}

void MainWindow::onPlayerWon() {
    newsTimer->stop();
    currentNews = QString(60, ' ')
                  + "  >>> PATOGENO VICTORIOSO <<<  "
                    "El 95% de la humanidad ha sido eliminada.  La civilizacion ha colapsado.  VICTORIA.  "
                  + QString(30, ' ');
    newsScrollPos = 0;
    newsTimer->start();

    auto fmt = [](long long n) -> QString {
        if (n >= 1'000'000'000) return QString("%1B").arg(n / 1'000'000'000.0, 0, 'f', 2);
        if (n >= 1'000'000)     return QString("%1M").arg(n / 1'000'000.0,     0, 'f', 1);
        if (n >= 1'000)         return QString("%1K").arg(n / 1'000.0,         0, 'f', 1);
        return QString::number(n);
    };

    QMessageBox* box = new QMessageBox(this);
    box->setWindowTitle("VICTORIA");
    box->setText(
        "<div style='font-family:Courier New; color:#ff4444; font-size:13px;'>"
        "<b style='font-size:16px; color:#ff0000;'>EL PATOGENO HA TRIUNFADO</b><br><br>"
        "El 95% de la humanidad ha sido erradicada.<br>"
        "La civilizacion humana ha dejado de existir.<br><br>"
        "<span style='color:#888888;'>Muertos: " + fmt(population->dead) +
        "<br>Supervivientes: " + fmt(population->healthy + population->infected) +
        "<br>Dia: " + QString::number(population->day) +
        "<br>Cura detenida en: " + QString::number((int)population->cureProgress) + "%</span></div>"
        );
    box->setStandardButtons(QMessageBox::Ok);
    box->button(QMessageBox::Ok)->setText("Jugar de nuevo");
    box->setStyleSheet(
        "QMessageBox { background:#0a0a0a; color:#ffffff; }"
        "QLabel { color:#ffffff; font-family:'Courier New'; }"
        "QPushButton { background:#3a0000; color:#ff4444; "
        "border:1px solid #ff0000; border-radius:4px; padding:6px 16px; }"
        "QPushButton:hover { background:#5a0000; }"
        );
    if (box->exec() == QMessageBox::Ok)
        onReset();
}

void MainWindow::showTraversal(QList<SkillNode*>& nodes, const QString& title) {
    traversalList->clear();
    traversalList->addItem("-- " + title + " --");
    for (int i = 0; i < nodes.size(); ++i) {
        QString state = nodes[i]->unlocked ? "[OK]" : "[--]";
        traversalList->addItem(QString("%1. %2 %3").arg(i+1).arg(state).arg(nodes[i]->name));
    }
}

void MainWindow::onPreOrder()  { QList<SkillNode*> r; tree->preOrder(tree->root, r);  showTraversal(r, "PreOrden"); }
void MainWindow::onInOrder()   { QList<SkillNode*> r; tree->inOrder(tree->root, r);   showTraversal(r, "InOrden"); }
void MainWindow::onPostOrder() { QList<SkillNode*> r; tree->postOrder(tree->root, r); showTraversal(r, "PostOrden"); }
void MainWindow::onBFS()       { QList<SkillNode*> r; tree->bfs(r);                   showTraversal(r, "BFS por niveles"); }

void MainWindow::showNodeTooltip(SkillNode* node, const QPoint& globalPos) {
    QString estado = node->unlocked
                         ? "<span style='color:#00ff41;'>&#10003; DESBLOQUEADO</span>"
                         : "<span style='color:#ff4444;'>&#10007; BLOQUEADO</span>";

    QString costo = node->dnaCost == 0
                        ? "<span style='color:#ffaa00;'>ROOT</span>"
                        : QString("<span style='color:#00ff41;'><b>%1 ADN</b></span>").arg(node->dnaCost);

    nodeTooltip->setText(
        QString("<b style='color:#ff4444; font-size:12px;'>%1</b><br>"
                "<span style='color:#999999;'>%2</span><br><br>"
                "&#9654; Costo: %3<br>%4")
            .arg(node->name).arg(node->description).arg(costo).arg(estado)
        );

    nodeTooltip->adjustSize();
    QPoint localPos = mapFromGlobal(globalPos) + QPoint(20, 10);

    if (localPos.x() + nodeTooltip->width() > width())
        localPos.setX(width() - nodeTooltip->width() - 8);
    if (localPos.y() + nodeTooltip->height() > height())
        localPos.setY(height() - nodeTooltip->height() - 8);

    nodeTooltip->move(localPos);
    nodeTooltip->show();
    nodeTooltip->raise();
}

void MainWindow::hideNodeTooltip() {
    nodeTooltip->hide();
}