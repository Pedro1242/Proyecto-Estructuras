#pragma once
#include <QDialog>
#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>

class SplashScreen : public QWidget {
    Q_OBJECT
public:
    explicit SplashScreen(QWidget* parent = nullptr);

signals:
    void finished();

private slots:
    void nextSlide();
    void fadeOut();

private:
    QLabel*            imgLabel;
    QLabel*            textLabel;
    QTimer*            timer;
    QPropertyAnimation* anim;
    int                currentSlide;

    void showSlide(int index);
    void fadeIn();
};