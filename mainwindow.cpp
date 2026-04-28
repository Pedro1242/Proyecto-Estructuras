#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QWheelEvent>
#include <QMessageBox>
#include <QPainter>

// ── Construction ──────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    m_tree       = new SkillTree();
    m_population = new Population(m_tree, this);

    buildUI();

    m_treeRenderer = new TreeRenderer(m_scene, m_view, m_tree, this);
    connect(m_treeRenderer, &TreeRenderer::nodeClicked, this, &MainWindow::onNodeClicked);
    connect(m_treeRenderer, &TreeRenderer::nodeHovered, this, &MainWindow::showNodeTooltip);
    connect(m_treeRenderer, &TreeRenderer::nodeLeft,    this, &MainWindow::hideNodeTooltip);

    m_treeRenderer->redraw();
    m_population->start();

    connect(m_population, &Population::populationUpdated, this, &MainWindow::onPopulationUpdated);
    connect(m_population, &Population::dnaEarned,         this, &MainWindow::onDnaEarned);
    connect(m_population, &Population::dayChanged,        this, &MainWindow::onDayChanged);
    connect(m_population, &Population::cureUpdated,       this, &MainWindow::onCureUpdated);
    connect(m_population, &Population::cureCompleted,     this, &MainWindow::onCureCompleted);
    connect(m_population, &Population::playerWon,         this, &MainWindow::onPlayerWon);
}

MainWindow::~MainWindow() { delete m_tree; }

// ── News ticker ───────────────────────────────────────────────────────────────

QString MainWindow::pickNews(int day) const {
    const auto&     s    = m_tree->m_stats;
    const long long inf  = m_population->m_infected;
    const long long dead = m_population->m_dead;

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
    m_lblDay->setText(QString("Dia %1  |  Paciente 0").arg(day));
    m_pendingNews = QString(60, ' ')
                    + QString("  >>> DIA %1 <<<  ").arg(day) + pickNews(day)
                    + QString(30, ' ');
}

void MainWindow::tickNews() {
    if (m_currentNews.isEmpty()) return;
    ++m_newsScrollPos;

    if (m_newsScrollPos >= m_currentNews.length()) {
        m_newsScrollPos = 0;
        if (!m_pendingNews.isEmpty()) {
            m_currentNews = m_pendingNews;
            m_pendingNews.clear();
        }
    }

    static constexpr int NEWS_WINDOW { 90 };
    QString visible = m_currentNews.mid(m_newsScrollPos, NEWS_WINDOW);
    if (visible.length() < NEWS_WINDOW)
        visible += m_currentNews.left(NEWS_WINDOW - visible.length());
    m_lblNewsTicker->setText(visible);
}

// ── UI Construction ───────────────────────────────────────────────────────────

void MainWindow::buildUI() {
    setWindowTitle("Plague Skill Tree");
    setMinimumSize(1150, 720);
    setStyleSheet("background-color: #0a0a0a; color: #ffffff;");

    m_nodeTooltip = new QLabel(this);
    m_nodeTooltip->setStyleSheet(
        "background-color:#1a0000; color:#ffffff;"
        "border:1px solid #8b0000; border-radius:5px;"
        "padding:8px 12px; font-family:'Courier New'; font-size:11px;");
    m_nodeTooltip->setTextFormat(Qt::RichText);
    m_nodeTooltip->setWordWrap(true);
    m_nodeTooltip->setFixedWidth(220);
    m_nodeTooltip->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_nodeTooltip->hide();

    m_scene = new BackgroundScene(this);
    m_scene->startAnimation();

    m_view = new QGraphicsView(m_scene);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setStyleSheet("border:1px solid #3a0000;");
    m_view->setMinimumWidth(750);
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_view->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_lblNewsTicker = new QLabel();
    m_lblNewsTicker->setStyleSheet(
        "background-color:#0d0d0d; color:#ff4444;"
        "font-family:'Courier New'; font-size:11px; font-weight:bold;"
        "padding:4px 8px; border-bottom:1px solid #3a0000;");
    m_lblNewsTicker->setFixedHeight(24);

    m_currentNews = QString(60, ' ')
                    + "  >>> SISTEMA ACTIVO <<<  Vigilancia epidemiologica iniciada.  Paciente 0 bajo observacion.  "
                    + QString(30, ' ');
    m_newsScrollPos = 0;

    m_newsTimer = new QTimer(this);
    m_newsTimer->setInterval(120);
    connect(m_newsTimer, &QTimer::timeout, this, &MainWindow::tickNews);
    m_newsTimer->start();

    // ── Side panel ───────────────────────────────────────────────────────────
    QWidget* panel = new QWidget();
    panel->setStyleSheet("background-color:#111111; border-left:1px solid #3a0000;");
    panel->setFixedWidth(300);

    QVBoxLayout* panelLayout = new QVBoxLayout(panel);
    panelLayout->setSpacing(8);
    panelLayout->setContentsMargins(10, 10, 10, 10);

    m_lblDay = new QLabel("Dia 0  |  Paciente 0");
    m_lblDay->setStyleSheet("color:#ff6600; font-size:11px; font-weight:bold;"
                            "background:#1a0800; padding:4px 6px; border-radius:3px;");
    panelLayout->addWidget(m_lblDay);

    m_dnaLabel = new QLabel("ADN disponible: 20");
    m_dnaLabel->setStyleSheet("color:#00ff41; font-size:14px; font-weight:bold;");
    panelLayout->addWidget(m_dnaLabel);

    QHBoxLayout* actionRow = new QHBoxLayout();
    actionRow->setSpacing(6);
    auto makeActionBtn = [](const QString& text, const QString& color) -> QPushButton* {
        QPushButton* btn = new QPushButton(text);
        btn->setStyleSheet(QString(
                               "QPushButton{background:#0a1a0a;color:%1;border:1px solid %1;"
                               "border-radius:4px;padding:5px;font-size:11px;}"
                               "QPushButton:hover{background:#1a2a1a;}").arg(color));
        return btn;
    };
    QPushButton* btnAddDna = makeActionBtn("+10 ADN", "#00ff41");
    QPushButton* btnReset  = makeActionBtn("Reset",   "#ff6600");
    actionRow->addWidget(btnAddDna);
    actionRow->addWidget(btnReset);
    panelLayout->addLayout(actionRow);

    m_statsLabel = new QLabel("Selecciona un nodo\npara ver sus estadisticas.");
    m_statsLabel->setStyleSheet("color:#cccccc; font-size:12px;"
                                "background:#1a0000; padding:8px; border-radius:4px;");
    m_statsLabel->setWordWrap(true);
    m_statsLabel->setFixedHeight(100);
    panelLayout->addWidget(m_statsLabel);

    QLabel* statsTitle = new QLabel("---- Estadisticas ----");
    statsTitle->setAlignment(Qt::AlignCenter);
    statsTitle->setStyleSheet("color:#8b0000; font-size:12px; margin-top:4px;");
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
                               "QProgressBar{background:#1a0000;border-radius:4px;}"
                               "QProgressBar::chunk{background:%1;border-radius:4px;}").arg(color));
        valLabel = new QLabel("0%");
        valLabel->setStyleSheet(QString("color:%1;font-size:10px;font-weight:bold;").arg(color));
        valLabel->setFixedWidth(32);
        rl->addWidget(lbl);
        rl->addWidget(bar);
        rl->addWidget(valLabel);
        panelLayout->addWidget(row);
    };

    makeStatRow("Infectiv.",  "#ff4444", m_barInfectivity, m_lblInfVal);
    makeStatRow("Severidad",  "#ff8800", m_barSeverity,    m_lblSevVal);
    makeStatRow("Letalidad",  "#cc0000", m_barLethality,   m_lblLetVal);
    makeStatRow("Sigilo",     "#00cc88", m_barStealth,     m_lblStlVal);
    updateStatsPanel();

    QLabel* popTitle = new QLabel("---- Poblacion Global ----");
    popTitle->setAlignment(Qt::AlignCenter);
    popTitle->setStyleSheet("color:#8b0000; font-size:12px;");
    panelLayout->addWidget(popTitle);

    auto makePopLabel = [&](const QString& prefix, const QString& color) -> QLabel* {
        QLabel* lbl = new QLabel(prefix + "0");
        lbl->setStyleSheet(QString("color:%1;font-size:10px;font-weight:bold;").arg(color));
        return lbl;
    };
    m_lblPopHealthy  = makePopLabel("Sanos:      ", "#00cc44");
    m_lblPopInfected = makePopLabel("Infectados: ", "#ff4444");
    m_lblPopDead     = makePopLabel("Muertos:    ", "#888888");

    m_barInfectedPop = new QProgressBar();
    m_barInfectedPop->setRange(0, 100);
    m_barInfectedPop->setTextVisible(false);
    m_barInfectedPop->setFixedHeight(8);
    m_barInfectedPop->setStyleSheet(
        "QProgressBar{background:#003300;border-radius:3px;}"
        "QProgressBar::chunk{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #ff4444,stop:1 #cc0000);border-radius:3px;}");

    panelLayout->addWidget(m_lblPopHealthy);
    panelLayout->addWidget(m_lblPopInfected);
    panelLayout->addWidget(m_lblPopDead);
    panelLayout->addWidget(m_barInfectedPop);

    // ── Cure bar ─────────────────────────────────────────────────────────────
    m_lblCureTitle = new QLabel("---- Investigacion Cura ----");
    m_lblCureTitle->setAlignment(Qt::AlignCenter);
    m_lblCureTitle->setStyleSheet("color:#00cc88; font-size:12px; margin-top:4px;");
    panelLayout->addWidget(m_lblCureTitle);

    QWidget* cureRow = new QWidget();
    QHBoxLayout* cureLayout = new QHBoxLayout(cureRow);
    cureLayout->setContentsMargins(0, 0, 0, 0);
    cureLayout->setSpacing(4);
    QLabel* cureLbl = new QLabel("Vacuna:");
    cureLbl->setStyleSheet("color:#aaaaaa; font-size:10px;");
    cureLbl->setFixedWidth(78);
    m_barCure = new QProgressBar();
    m_barCure->setRange(0, 100);
    m_barCure->setValue(0);
    m_barCure->setTextVisible(false);
    m_barCure->setFixedHeight(10);
    m_barCure->setStyleSheet(
        "QProgressBar{background:#001a00;border-radius:4px;}"
        "QProgressBar::chunk{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #00cc88,stop:1 #00ffaa);border-radius:4px;}");
    m_lblCureVal = new QLabel("0%");
    m_lblCureVal->setStyleSheet("color:#00cc88;font-size:10px;font-weight:bold;");
    m_lblCureVal->setFixedWidth(32);
    cureLayout->addWidget(cureLbl);
    cureLayout->addWidget(m_barCure);
    cureLayout->addWidget(m_lblCureVal);
    panelLayout->addWidget(cureRow);

    // ── Traversal buttons ─────────────────────────────────────────────────────
    QLabel* recLbl = new QLabel("---- Recorridos ----");
    recLbl->setAlignment(Qt::AlignCenter);
    recLbl->setStyleSheet("color:#8b0000; font-size:12px;");
    panelLayout->addWidget(recLbl);

    auto makeBtn = [](const QString& text) -> QPushButton* {
        QPushButton* btn = new QPushButton(text);
        btn->setStyleSheet(
            "QPushButton{background:#1e0000;color:#ff4444;"
            "border:1px solid #8b0000;border-radius:4px;padding:6px;}"
            "QPushButton:hover{background:#3a0000;}");
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

    m_traversalList = new QListWidget();
    m_traversalList->setStyleSheet("background:#0a0a0a; color:#00ff41;"
                                   "border:1px solid #3a0000; font-size:11px;");
    panelLayout->addWidget(m_traversalList);
    panelLayout->addStretch();

    connect(btnPre,    &QPushButton::clicked, this, &MainWindow::onPreOrder);
    connect(btnIn,     &QPushButton::clicked, this, &MainWindow::onInOrder);
    connect(btnPost,   &QPushButton::clicked, this, &MainWindow::onPostOrder);
    connect(btnBFS,    &QPushButton::clicked, this, &MainWindow::onBFS);
    connect(btnAddDna, &QPushButton::clicked, this, &MainWindow::onAddDna);
    connect(btnReset,  &QPushButton::clicked, this, &MainWindow::onReset);

    // ── Root layout ───────────────────────────────────────────────────────────
    QWidget* central = new QWidget();
    QVBoxLayout* rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    rootLayout->addWidget(m_lblNewsTicker);

    QWidget* content = new QWidget();
    QHBoxLayout* mainLayout = new QHBoxLayout(content);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_view);
    mainLayout->addWidget(panel);

    rootLayout->addWidget(content);
    setCentralWidget(central);
}

// ── Zoom ──────────────────────────────────────────────────────────────────────

void MainWindow::wheelEvent(QWheelEvent* event) {
    if (m_view->underMouse()) {
        const double factor = event->angleDelta().y() > 0 ? 1.15 : 0.87;
        m_view->scale(factor, factor);
        event->accept();
    } else {
        QMainWindow::wheelEvent(event);
    }
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void MainWindow::onNodeClicked(SkillNode* node) {
    const QString estado = node->m_unlocked ? "DESBLOQUEADO" : "BLOQUEADO";
    const QString padre  = node->m_parent   ? node->m_parent->m_name : "-";

    m_statsLabel->setText(
        QString("<b style='color:#ff4444'>%1</b><br>"
                "<span style='color:#888'>%2</span><br><br>"
                "Estado: %3<br>"
                "Costo: <b style='color:#00ff41'>%4 ADN</b><br>"
                "Padre: %5 | Hijos: %6")
            .arg(node->m_name).arg(node->m_description)
            .arg(estado).arg(node->m_dnaCost)
            .arg(padre).arg(node->m_children.size()));
    m_statsLabel->setTextFormat(Qt::RichText);

    if (!node->m_unlocked) {
        const bool ok = m_tree->unlock(node);
        if (ok) {
            updateDnaLabel();
            updateStatsPanel();
            m_treeRenderer->redraw();
            m_statsLabel->setText(
                QString("<b style='color:#00ff41'>%1 DESBLOQUEADO</b><br>"
                        "<span style='color:#888'>%2</span><br><br>"
                        "ADN gastado: <b style='color:#ff4444'>%3</b>")
                    .arg(node->m_name).arg(node->m_description).arg(node->m_dnaCost));
            m_statsLabel->setTextFormat(Qt::RichText);
        } else {
            QString razon;
            if (node->m_parent && !node->m_parent->m_unlocked)
                razon = "Desbloquea primero:<br><b>" + node->m_parent->m_name + "</b>";
            else if (m_tree->m_dnaPoints < node->m_dnaCost)
                razon = QString("ADN insuficiente.<br>Necesitas <b>%1</b>, tienes <b>%2</b>.")
                            .arg(node->m_dnaCost).arg(m_tree->m_dnaPoints);
            else
                razon = "Ya esta desbloqueado.";
            m_statsLabel->setText(
                QString("<b style='color:#ff4444'>%1</b><br><br>%2")
                    .arg(node->m_name).arg(razon));
            m_statsLabel->setTextFormat(Qt::RichText);
        }
    }
}

void MainWindow::onAddDna() {
    m_tree->m_dnaPoints += 10;
    updateDnaLabel();
    updateStatsPanel();
    m_treeRenderer->redraw();
    m_statsLabel->setText("<b style='color:#00ff41'>+10 ADN agregado!</b>");
    m_statsLabel->setTextFormat(Qt::RichText);
}

void MainWindow::onReset() {
    m_population->stop();
    m_newsTimer->stop();
    delete m_tree;
    m_tree                    = new SkillTree();
    m_population->m_skillTree = m_tree;
    m_population->reset();
    m_treeRenderer->setTree(m_tree);  // ← notificar al renderer del nuevo árbol

    m_lblDay->setText("Dia 0  |  Paciente 0");
    m_currentNews   = QString(60, ' ')
                    + "  >>> RESET <<<  Nuevo patogeno detectado.  Protocolo reiniciado.  "
                    + QString(30, ' ');
    m_pendingNews   = QString();
    m_newsScrollPos = 0;

    m_barCure->setValue(0);
    m_lblCureVal->setText("0%");
    m_lblCureTitle->setStyleSheet("color:#00cc88; font-size:12px; margin-top:4px;");
    m_lblCureVal->setStyleSheet("color:#00cc88; font-size:10px; font-weight:bold;");
    m_barCure->setStyleSheet(
        "QProgressBar{background:#001a00;border-radius:4px;}"
        "QProgressBar::chunk{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #00cc88,stop:1 #00ffaa);border-radius:4px;}");

    m_population->start();
    m_newsTimer->start();
    updateDnaLabel();
    updateStatsPanel();
    m_treeRenderer->redraw();
    m_traversalList->clear();
    m_statsLabel->setText("Arbol reiniciado.<br>Selecciona un nodo para comenzar.");
    m_statsLabel->setTextFormat(Qt::RichText);
}

void MainWindow::updateDnaLabel() {
    m_dnaLabel->setText(QString("ADN disponible: %1").arg(m_tree->m_dnaPoints));
}

void MainWindow::updateStatsPanel() {
    m_tree->recalcStats();
    const auto& s = m_tree->m_stats;
    m_barInfectivity->setValue(static_cast<int>(s.infectivity));
    m_barSeverity   ->setValue(static_cast<int>(s.severity));
    m_barLethality  ->setValue(static_cast<int>(s.lethality));
    m_barStealth    ->setValue(static_cast<int>(s.stealth));
    m_lblInfVal->setText(QString("%1%").arg(static_cast<int>(s.infectivity)));
    m_lblSevVal->setText(QString("%1%").arg(static_cast<int>(s.severity)));
    m_lblLetVal->setText(QString("%1%").arg(static_cast<int>(s.lethality)));
    m_lblStlVal->setText(QString("%1%").arg(static_cast<int>(s.stealth)));
}

void MainWindow::onPopulationUpdated() {
    auto fmt = [](long long n) -> QString {
        if (n >= 1'000'000'000) return QString("%1B").arg(n / 1'000'000'000.0, 0, 'f', 2);
        if (n >= 1'000'000)     return QString("%1M").arg(n / 1'000'000.0,     0, 'f', 1);
        if (n >= 1'000)         return QString("%1K").arg(n / 1'000.0,         0, 'f', 1);
        return QString::number(n);
    };
    m_lblPopHealthy ->setText(QString("Sanos:      %1").arg(fmt(m_population->m_healthy)));
    m_lblPopInfected->setText(QString("Infectados: %1").arg(fmt(m_population->m_infected)));
    m_lblPopDead    ->setText(QString("Muertos:    %1").arg(fmt(m_population->m_dead)));

    const int pct = static_cast<int>(
        ((m_population->m_infected + m_population->m_dead)
         / static_cast<float>(m_population->m_total)) * 100.0f);
    m_barInfectedPop->setValue(qBound(0, pct, 100));
    updateDnaLabel();
}

void MainWindow::onDnaEarned(int amount) {
    Q_UNUSED(amount);
    m_dnaLabel->setStyleSheet("color:#ffffff; font-size:14px; font-weight:bold;");
    QTimer::singleShot(300, this, [this]() {
        m_dnaLabel->setStyleSheet("color:#00ff41; font-size:14px; font-weight:bold;");
    });
}

void MainWindow::onCureUpdated(float progress) {
    const int pct = static_cast<int>(progress);
    m_barCure->setValue(pct);
    m_lblCureVal->setText(QString("%1%").arg(pct));

    if (progress >= 80.0f) {
        m_barCure->setStyleSheet(
            "QProgressBar{background:#1a0000;border-radius:4px;}"
            "QProgressBar::chunk{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
            "stop:0 #ff4444,stop:1 #ff0000);border-radius:4px;}");
        m_lblCureVal  ->setStyleSheet("color:#ff4444;font-size:10px;font-weight:bold;");
        m_lblCureTitle->setStyleSheet("color:#ff4444;font-size:12px;font-weight:bold;margin-top:4px;");
        if (pct == 80) {
            m_pendingNews = QString(60, ' ')
            + "  !!! ALERTA !!!  Cientificos anuncian vacuna al 80%.  El tiempo se agota.  "
                + QString(30, ' ');
        }
    } else if (progress >= 50.0f) {
        m_barCure->setStyleSheet(
            "QProgressBar{background:#1a1000;border-radius:4px;}"
            "QProgressBar::chunk{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
            "stop:0 #ffaa00,stop:1 #ffdd00);border-radius:4px;}");
        m_lblCureVal  ->setStyleSheet("color:#ffaa00;font-size:10px;font-weight:bold;");
        m_lblCureTitle->setStyleSheet("color:#ffaa00;font-size:12px;font-weight:bold;margin-top:4px;");
        if (pct == 50) {
            m_pendingNews = QString(60, ' ')
            + "  ATENCION:  Vacuna experimental al 50% de desarrollo.  Los laboratorios trabajan sin descanso.  "
                + QString(30, ' ');
        }
    }
}

void MainWindow::onCureCompleted() {
    m_newsTimer->stop();
    m_currentNews = QString(60, ' ')
                    + "  >>> VACUNA COMPLETADA <<<  "
                      "La humanidad ha desarrollado la cura.  El patogeno ha sido erradicado.  DERROTA.  "
                    + QString(30, ' ');
    m_newsScrollPos = 0;
    m_newsTimer->start();

    QMessageBox* box = new QMessageBox(this);
    box->setWindowTitle("DERROTA");
    box->setText(
        "<div style='font-family:Courier New;color:#00cc88;font-size:13px;'>"
        "<b style='font-size:16px;'>LA HUMANIDAD HA GANADO</b><br><br>"
        "La vacuna fue desarrollada antes de que el patogeno<br>"
        "pudiera exterminar a la especie.<br><br>"
        "<span style='color:#888888;'>Infectados: " + QString::number(m_population->m_infected) +
        "<br>Muertos: "  + QString::number(m_population->m_dead) +
        "<br>Dia: "      + QString::number(m_population->m_day) + "</span></div>");
    box->setStandardButtons(QMessageBox::Ok);
    box->button(QMessageBox::Ok)->setText("Reintentar");
    box->setStyleSheet(
        "QMessageBox{background:#0a0a0a;color:#ffffff;}"
        "QLabel{color:#ffffff;font-family:'Courier New';}"
        "QPushButton{background:#1e0000;color:#ff4444;"
        "border:1px solid #8b0000;border-radius:4px;padding:6px 16px;}"
        "QPushButton:hover{background:#3a0000;}");
    if (box->exec() == QMessageBox::Ok)
        onReset();
}

void MainWindow::onPlayerWon() {
    m_newsTimer->stop();
    m_currentNews = QString(60, ' ')
                    + "  >>> PATOGENO VICTORIOSO <<<  "
                      "El 95% de la humanidad ha sido eliminada.  La civilizacion ha colapsado.  VICTORIA.  "
                    + QString(30, ' ');
    m_newsScrollPos = 0;
    m_newsTimer->start();

    auto fmt = [](long long n) -> QString {
        if (n >= 1'000'000'000) return QString("%1B").arg(n / 1'000'000'000.0, 0, 'f', 2);
        if (n >= 1'000'000)     return QString("%1M").arg(n / 1'000'000.0,     0, 'f', 1);
        if (n >= 1'000)         return QString("%1K").arg(n / 1'000.0,         0, 'f', 1);
        return QString::number(n);
    };

    QMessageBox* box = new QMessageBox(this);
    box->setWindowTitle("VICTORIA");
    box->setText(
        "<div style='font-family:Courier New;color:#ff4444;font-size:13px;'>"
        "<b style='font-size:16px;color:#ff0000;'>EL PATOGENO HA TRIUNFADO</b><br><br>"
        "El 95% de la humanidad ha sido erradicada.<br>"
        "La civilizacion humana ha dejado de existir.<br><br>"
        "<span style='color:#888888;'>Muertos: " + fmt(m_population->m_dead) +
        "<br>Supervivientes: " + fmt(m_population->m_healthy + m_population->m_infected) +
        "<br>Dia: "            + QString::number(m_population->m_day) +
        "<br>Cura detenida en: " + QString::number(static_cast<int>(m_population->m_cureProgress))
        + "%</span></div>");
    box->setStandardButtons(QMessageBox::Ok);
    box->button(QMessageBox::Ok)->setText("Jugar de nuevo");
    box->setStyleSheet(
        "QMessageBox{background:#0a0a0a;color:#ffffff;}"
        "QLabel{color:#ffffff;font-family:'Courier New';}"
        "QPushButton{background:#3a0000;color:#ff4444;"
        "border:1px solid #ff0000;border-radius:4px;padding:6px 16px;}"
        "QPushButton:hover{background:#5a0000;}");
    if (box->exec() == QMessageBox::Ok)
        onReset();
}

// ── Traversal display ─────────────────────────────────────────────────────────

void MainWindow::showTraversal(QList<SkillNode*>& nodes, const QString& title) {
    m_traversalList->clear();
    m_traversalList->addItem("-- " + title + " --");
    for (int i = 0; i < nodes.size(); ++i) {
        const QString state = nodes[i]->m_unlocked ? "[OK]" : "[--]";
        m_traversalList->addItem(
            QString("%1. %2 %3").arg(i + 1).arg(state).arg(nodes[i]->m_name));
    }
}

void MainWindow::onPreOrder()  { QList<SkillNode*> r; m_tree->preOrder (m_tree->m_root, r); showTraversal(r, "PreOrden");       }
void MainWindow::onInOrder()   { QList<SkillNode*> r; m_tree->inOrder  (m_tree->m_root, r); showTraversal(r, "InOrden");        }
void MainWindow::onPostOrder() { QList<SkillNode*> r; m_tree->postOrder(m_tree->m_root, r); showTraversal(r, "PostOrden");      }
void MainWindow::onBFS()       { QList<SkillNode*> r; m_tree->bfs(r);                       showTraversal(r, "BFS por niveles"); }

// ── Tooltip ───────────────────────────────────────────────────────────────────

void MainWindow::showNodeTooltip(SkillNode* node, const QPoint& globalPos) {
    const QString estado = node->m_unlocked
                               ? "<span style='color:#00ff41;'>&#10003; DESBLOQUEADO</span>"
                               : "<span style='color:#ff4444;'>&#10007; BLOQUEADO</span>";
    const QString costo = node->m_dnaCost == 0
                              ? "<span style='color:#ffaa00;'>ROOT</span>"
                              : QString("<span style='color:#00ff41;'><b>%1 ADN</b></span>").arg(node->m_dnaCost);

    m_nodeTooltip->setText(
        QString("<b style='color:#ff4444;font-size:12px;'>%1</b><br>"
                "<span style='color:#999999;'>%2</span><br><br>"
                "&#9654; Costo: %3<br>%4")
            .arg(node->m_name).arg(node->m_description).arg(costo).arg(estado));

    m_nodeTooltip->adjustSize();
    QPoint localPos = mapFromGlobal(globalPos) + QPoint(20, 10);
    if (localPos.x() + m_nodeTooltip->width()  > width())
        localPos.setX(width()  - m_nodeTooltip->width()  - 8);
    if (localPos.y() + m_nodeTooltip->height() > height())
        localPos.setY(height() - m_nodeTooltip->height() - 8);
    m_nodeTooltip->move(localPos);
    m_nodeTooltip->show();
    m_nodeTooltip->raise();
}

void MainWindow::hideNodeTooltip() { m_nodeTooltip->hide(); }

// ── Pathogen name ─────────────────────────────────────────────────────────────

void MainWindow::setPathogenName(const QString& name) {
    setWindowTitle(QString("Plague Skill Tree — %1").arg(name));
    m_currentNews = QString(60, ' ')
                    + QString("  >>> ALERTA BIOLOGICA <<<  Nuevo patogeno identificado: [ %1 ]"
                              "  Origen desconocido.  Autoridades investigan.  ").arg(name)
                    + QString(30, ' ');
    m_newsScrollPos = 0;
    m_lblDay->setText(QString("Dia 0  |  %1").arg(name));
}