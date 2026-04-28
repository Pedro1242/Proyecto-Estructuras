#include "menuwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QKeyEvent>

MenuWindow::MenuWindow(QWidget* parent)
    : QWidget(parent), titleFrame(0)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setFixedSize(900, 600);

    QScreen* screen = QApplication::primaryScreen();
    move(screen->geometry().center() - rect().center());

    buildUI();

    titleTimer = new QTimer(this);
    titleTimer->setInterval(80);
    connect(titleTimer, &QTimer::timeout, this, &MenuWindow::tickTitle);
    titleTimer->start();
}

QString MenuWindow::pathogenName() const {
    QString n = inputName->text().trimmed();
    return n.isEmpty() ? "PANDEMIA-X" : n.toUpper();
}

QPushButton* MenuWindow::makeMenuBtn(const QString& text, const QString& color) {
    QPushButton* btn = new QPushButton(text);
    btn->setFixedHeight(46);
    btn->setFixedWidth(280);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(QString(
                           "QPushButton {"
                           "  background: transparent;"
                           "  color: %1;"
                           "  border: 1px solid %1;"
                           "  border-radius: 4px;"
                           "  font-family: 'Courier New';"
                           "  font-size: 14px;"
                           "  font-weight: bold;"
                           "  letter-spacing: 2px;"
                           "  text-align: left;"
                           "  padding-left: 18px;"
                           "}"
                           "QPushButton:hover {"
                           "  background: rgba(180,0,0,0.15);"
                           "  border-color: #ffffff;"
                           "  color: #ffffff;"
                           "}"
                           "QPushButton:pressed { background: rgba(180,0,0,0.3); }"
                           ).arg(color));
    return btn;
}

void MenuWindow::buildUI() {
    setStyleSheet("background-color: #050505;");

    QHBoxLayout* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ---- Panel izquierdo ----
    QWidget* leftPanel = new QWidget();
    leftPanel->setFixedWidth(420);
    leftPanel->setStyleSheet(
        "background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
        "stop:0 #0a0000, stop:0.5 #1a0000, stop:1 #050505);"
        "border-right: 1px solid #3a0000;"
        );

    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setAlignment(Qt::AlignCenter);
    leftLayout->setSpacing(6);

    QLabel* ascii = new QLabel(
        "<pre style='color:#3a0000; font-size:9px; font-family:Courier New; line-height:1.1;'>"
        "    \u2588\u2588\u2588\u2588\u2588\u2588\u2557 \u2588\u2588\u2557      \u2588\u2588\u2588\u2588\u2588\u2557  \u2588\u2588\u2588\u2588\u2588\u2588\u2557 \u2588\u2588\u2557   \u2588\u2588\u2557\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557\n"
        "    \u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2557\u2588\u2588\u2551     \u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2557\u2588\u2588\u2554\u2550\u2550\u2550\u2550\u255d \u2588\u2588\u2551   \u2588\u2588\u2551\u2588\u2588\u2554\u2550\u2550\u2550\u2550\u255d\n"
        "    \u2588\u2588\u2588\u2588\u2588\u2588\u2554\u255d\u2588\u2588\u2551     \u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2551\u2588\u2588\u2551  \u2588\u2588\u2588\u2557\u2588\u2588\u2551   \u2588\u2588\u2551\u2588\u2588\u2588\u2588\u2588\u2557  \n"
        "    \u2588\u2588\u2554\u2550\u2550\u2550\u255d \u2588\u2588\u2551     \u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2551\u2588\u2588\u2551   \u2588\u2588\u2551\u2588\u2588\u2551   \u2588\u2588\u2551\u2588\u2588\u2554\u2550\u2550\u255d  \n"
        "    \u2588\u2588\u2551     \u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557\u2588\u2588\u2551  \u2588\u2588\u2551\u255a\u2588\u2588\u2588\u2588\u2588\u2588\u2554\u255d\u255a\u2588\u2588\u2588\u2588\u2588\u2588\u2554\u255d\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557\n"
        "    \u255a\u2550\u255d     \u255a\u2550\u2550\u2550\u2550\u2550\u2550\u255d\u255a\u2550\u255d  \u255a\u2550\u255d \u255a\u2550\u2550\u2550\u2550\u2550\u255d  \u255a\u2550\u2550\u2550\u2550\u2550\u255d \u255a\u2550\u2550\u2550\u2550\u2550\u2550\u255d"
        "</pre>"
        );
    ascii->setTextFormat(Qt::RichText);
    ascii->setAlignment(Qt::AlignCenter);

    lblTitle = new QLabel();
    lblTitle->setAlignment(Qt::AlignCenter);
    lblTitle->setTextFormat(Qt::RichText);

    QLabel* tagline = new QLabel("ERRADICA. EVOLUCIONA. EXTERMINA.");
    tagline->setAlignment(Qt::AlignCenter);
    tagline->setStyleSheet(
        "color: #4a0000; font-family: 'Courier New'; font-size: 10px; letter-spacing: 3px;");

    lblVersion = new QLabel("v1.0.0  |  Estructuras de Datos  |  2026");
    lblVersion->setAlignment(Qt::AlignCenter);
    lblVersion->setStyleSheet("color: #2a2a2a; font-family:'Courier New'; font-size:9px;");

    QLabel* bio = new QLabel("\u2623");
    bio->setAlignment(Qt::AlignCenter);
    bio->setStyleSheet("color: #3a0000; font-size: 64px;");

    leftLayout->addStretch();
    leftLayout->addWidget(bio);
    leftLayout->addWidget(ascii);
    leftLayout->addWidget(lblTitle);
    leftLayout->addWidget(tagline);
    leftLayout->addStretch();
    leftLayout->addWidget(lblVersion);

    // ---- Panel derecho ----
    QWidget* rightPanel = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    rightLayout->setContentsMargins(60, 40, 40, 40);
    rightLayout->setSpacing(14);

    QLabel* lblMenu = new QLabel("> MENU PRINCIPAL");
    lblMenu->setStyleSheet(
        "color: #8b0000; font-family: 'Courier New'; font-size: 11px;"
        "letter-spacing: 2px; margin-bottom: 10px;");
    rightLayout->addWidget(lblMenu);

    QLabel* lblNombre = new QLabel("[ NOMBRE DEL PATOGENO ]");
    lblNombre->setStyleSheet(
        "color: #555555; font-family: 'Courier New'; font-size: 10px; letter-spacing: 1px;");

    inputName = new QLineEdit();
    inputName->setPlaceholderText("Ej: PANDEMIA-X, VIRUS-Z ...");
    inputName->setMaxLength(24);
    inputName->setFixedHeight(36);
    inputName->setFixedWidth(280);
    inputName->setStyleSheet(
        "QLineEdit {"
        "  background: #0d0000; color: #ff4444;"
        "  border: 1px solid #3a0000; border-radius: 4px;"
        "  font-family: 'Courier New'; font-size: 13px; padding: 4px 10px;"
        "}"
        "QLineEdit:focus { border-color: #ff4444; background: #150000; }"
        );

    rightLayout->addWidget(lblNombre);
    rightLayout->addWidget(inputName);

    QFrame* sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setFixedWidth(280);
    sep->setStyleSheet("color: #1a0000; margin: 6px 0;");
    rightLayout->addWidget(sep);

    QPushButton* btnPlay         = makeMenuBtn("\u25BA  NUEVA PARTIDA",  "#ff4444");
    QPushButton* btnInstructions = makeMenuBtn("\u2139  COMO JUGAR",     "#ff8800");
    QPushButton* btnSettings     = makeMenuBtn("\u2699  CONFIGURACION",  "#555555");
    QPushButton* btnCredits      = makeMenuBtn("\u2605  CREDITOS",       "#555555");
    QPushButton* btnExit         = makeMenuBtn("\u2716  SALIR",          "#3a0000");

    btnSettings->setEnabled(false);
    btnSettings->setToolTip("Proximamente");
    btnSettings->setStyleSheet(btnSettings->styleSheet() +
                               "QPushButton:disabled { color:#2a2a2a; border-color:#1a1a1a; }");

    rightLayout->addWidget(btnPlay);
    rightLayout->addWidget(btnInstructions);
    rightLayout->addWidget(btnSettings);
    rightLayout->addWidget(btnCredits);
    rightLayout->addWidget(btnExit);
    rightLayout->addStretch();

    QLabel* lblTip = new QLabel("[ ENTER ] para iniciar  |  ESC para salir");
    lblTip->setStyleSheet("color:#1e1e1e; font-family:'Courier New'; font-size:9px;");
    rightLayout->addWidget(lblTip);

    root->addWidget(leftPanel);
    root->addWidget(rightPanel);

    connect(btnPlay,         &QPushButton::clicked, this, &MenuWindow::onPlay);
    connect(btnInstructions, &QPushButton::clicked, this, &MenuWindow::onInstructions);
    connect(btnCredits,      &QPushButton::clicked, this, &MenuWindow::onCredits);
    connect(btnExit,         &QPushButton::clicked, this, &MenuWindow::exitRequested);
    connect(inputName,       &QLineEdit::returnPressed, this, &MenuWindow::onPlay);
}

void MenuWindow::tickTitle() {
    static const QStringList colors = {
        "#ff0000","#ff1a00","#ff3300","#ff4400",
        "#ff3300","#ff1a00","#ff0000","#cc0000",
        "#990000","#cc0000"
    };
    titleFrame = (titleFrame + 1) % colors.size();
    lblTitle->setText(
        QString("<p style='font-family:\"Courier New\"; font-size:22px; "
                "font-weight:bold; color:%1; letter-spacing:6px;'>"
                "SKILL TREE</p>")
            .arg(colors[titleFrame])
        );
}

void MenuWindow::onPlay() {
    QGraphicsOpacityEffect* eff = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(eff);
    QPropertyAnimation* anim = new QPropertyAnimation(eff, "opacity", this);
    anim->setDuration(400);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    connect(anim, &QPropertyAnimation::finished, this, [this](){
        emit playRequested(pathogenName());
    });
    anim->start();
}

void MenuWindow::onInstructions() {
    QMessageBox* box = new QMessageBox(this);
    box->setWindowTitle("COMO JUGAR");
    box->setText(
        "<div style='font-family:Courier New; font-size:12px; color:#cccccc; min-width:400px;'>"
        "<b style='color:#ff4444; font-size:14px;'>OBJETIVO</b><br>"
        "Extermina al 95% de la humanidad antes de que la vacuna llegue al 100%.<br><br>"
        "<b style='color:#ff4444;'>MECANICA</b><br>"
        "&#9658; Desbloquea habilidades en el arbol gastando <b style='color:#00ff41;'>ADN</b><br>"
        "&#9658; Cada habilidad sube Infectividad, Severidad, Letalidad o Sigilo<br>"
        "&#9658; El ADN se gana automaticamente al infectar poblacion<br>"
        "&#9658; Debes desbloquear el nodo padre antes del hijo<br><br>"
        "<b style='color:#ff4444;'>ESTADISTICAS</b><br>"
        "<span style='color:#ff4444;'>&#9763; Infectividad</span> — velocidad de contagio<br>"
        "<span style='color:#ff8800;'>&#9889; Severidad</span>    — dano a los infectados<br>"
        "<span style='color:#cc0000;'>&#9760; Letalidad</span>    — tasa de muerte<br>"
        "<span style='color:#00cc88;'>&#9679; Sigilo</span>       — retrasa el avance de la cura<br><br>"
        "<b style='color:#ff4444;'>RECORRIDOS</b><br>"
        "Usa los botones del panel para recorrer el arbol en<br>"
        "PreOrden, InOrden, PostOrden o BFS.<br><br>"
        "<b style='color:#ff8800;'>CONSEJO</b><br>"
        "Sube Sigilo primero para ganar tiempo. Despues Infectividad."
        "</div>"
        );
    box->setStandardButtons(QMessageBox::Ok);
    box->button(QMessageBox::Ok)->setText("Entendido");
    box->setStyleSheet(
        "QMessageBox { background:#0a0a0a; }"
        "QLabel { color:#cccccc; font-family:'Courier New'; }"
        "QPushButton { background:#1e0000; color:#ff4444; "
        "border:1px solid #8b0000; border-radius:4px; padding:6px 20px; }"
        "QPushButton:hover { background:#3a0000; }"
        );
    box->exec();
}

void MenuWindow::onCredits() {
    QMessageBox* box = new QMessageBox(this);
    box->setWindowTitle("CREDITOS");
    box->setText(
        "<div style='font-family:Courier New; font-size:12px; color:#cccccc; text-align:center;'>"
        "<b style='color:#ff4444; font-size:15px;'>PLAGUE SKILL TREE</b><br>"
        "<span style='color:#555;'>v1.0.0 \u2014 2026</span><br><br>"
        "<b style='color:#ff8800;'>DESARROLLO</b><br>"
        "Estructura de Datos Dev Team<br><br>"
        "<b style='color:#ff8800;'>INSTITUCION</b><br>"
        "Universidad de los Llanos<br>"
        "Semillero City Games \u2014 Unillanos<br><br>"
        "<b style='color:#ff8800;'>TECNOLOGIAS</b><br>"
        "C++17  |  Qt 6.11  |  QGraphicsScene<br>"
        "Arbol N-ario  |  BFS  |  DFS<br><br>"
        "<span style='color:#3a3a3a;'>Inspirado en Plague Inc.</span>"
        "</div>"
        );
    box->setStandardButtons(QMessageBox::Ok);
    box->button(QMessageBox::Ok)->setText("Cerrar");
    box->setStyleSheet(
        "QMessageBox { background:#0a0a0a; }"
        "QLabel { color:#cccccc; font-family:'Courier New'; }"
        "QPushButton { background:#1e0000; color:#ff4444; "
        "border:1px solid #8b0000; border-radius:4px; padding:6px 20px; }"
        "QPushButton:hover { background:#3a0000; }"
        );
    box->exec();
}

void MenuWindow::onSettings() {
    // Reservado para futuras mejoras:
    // - Velocidad de simulacion
    // - Poblacion inicial
    // - Dificultad de la cura
    // - Tema visual
}

void MenuWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        onPlay();
    else if (event->key() == Qt::Key_Escape)
        emit exitRequested();
    else
        QWidget::keyPressEvent(event);
}