#pragma once
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>

class MenuWindow : public QWidget {
    Q_OBJECT
public:
    explicit MenuWindow(QWidget* parent = nullptr);
    QString pathogenName() const;

signals:
    void playRequested(const QString& name);
    void exitRequested();

private slots:
    void onPlay();
    void onInstructions();
    void onCredits();
    void onSettings();
    void tickTitle();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    QLineEdit*   inputName;
    QLabel*      lblTitle;
    QLabel*      lblVersion;
    QLabel*      lblParticles;
    QTimer*      titleTimer;
    int          titleFrame;

    void buildUI();
    QPushButton* makeMenuBtn(const QString& text, const QString& color = "#ff4444");
};