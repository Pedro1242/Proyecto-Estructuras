#include "splashscreen.h"
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QApplication>
#include <QScreen>
#include <QFont>

SplashScreen::SplashScreen(QWidget* parent)
    : QWidget(parent), currentSlide(0)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(700, 400);

    // Centrar en pantalla
    QScreen* screen = QApplication::primaryScreen();
    QRect sg = screen->geometry();
    move(sg.center() - rect().center());

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);

    imgLabel = new QLabel();
    imgLabel->setAlignment(Qt::AlignCenter);
    imgLabel->setFixedSize(160, 160);
    imgLabel->setScaledContents(true);

    textLabel = new QLabel();
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setWordWrap(true);

    layout->addStretch();
    layout->addWidget(imgLabel,  0, Qt::AlignCenter);
    layout->addWidget(textLabel, 0, Qt::AlignCenter);
    layout->addStretch();

    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, &SplashScreen::fadeOut);

    anim = new QPropertyAnimation(this);
    connect(anim, &QPropertyAnimation::finished, this, [this]() {
        if (anim->endValue().toReal() < 0.1)
            nextSlide();
    });

    showSlide(0);
}

struct Slide {
    QString title;
    QString subtitle;
    QString color;
    QString bg;
};

static const Slide slides[] = {
    {
        "UNILLANOS GAMES",
        "Universidad de los Llanos",
        "#ffffff",
        "rgba(10,0,0,220)"
    },
    {
        "CITY GAMES",
        "Semillero de Investigacion\nUnillanos",
        "#00ff41",
        "rgba(0,10,0,220)"
    },
    {
        "ESTRUCTURA DE DATOS\nDEV TEAM",
        "Proyecto Final · 2026",
        "#ff4444",
        "rgba(20,0,0,220)"
    }
};

void SplashScreen::showSlide(int index) {
    if (index >= 3) { emit finished(); return; }

    const Slide& s = slides[index];

    imgLabel->hide();  // sin imagen por ahora, solo texto

    textLabel->setText(
        QString("<div style='text-align:center;'>"
                "<p style='font-family:\"Courier New\"; font-size:28px; "
                "font-weight:bold; color:%1; letter-spacing:4px;'>%2</p>"
                "<p style='font-family:\"Courier New\"; font-size:14px; "
                "color:#aaaaaa; margin-top:8px;'>%3</p>"
                "</div>")
            .arg(s.color).arg(s.title).arg(s.subtitle)
        );
    textLabel->setTextFormat(Qt::RichText);

    setStyleSheet(QString(
                      "SplashScreen { background: %1; border: 1px solid #3a0000; border-radius: 8px; }"
                      ).arg(s.bg));

    fadeIn();
    timer->start(2200);  // cuánto dura cada slide visible
}

void SplashScreen::fadeIn() {
    QGraphicsOpacityEffect* eff = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(eff);
    anim->setTargetObject(eff);
    anim->setPropertyName("opacity");
    anim->setDuration(700);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start();
}

void SplashScreen::fadeOut() {
    QGraphicsOpacityEffect* eff = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
    if (!eff) { eff = new QGraphicsOpacityEffect(this); setGraphicsEffect(eff); }
    anim->setTargetObject(eff);
    anim->setPropertyName("opacity");
    anim->setDuration(600);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    anim->start();
}

void SplashScreen::nextSlide() {
    currentSlide++;
    showSlide(currentSlide);
}