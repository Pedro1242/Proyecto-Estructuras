#include "mainwindow.h"
#include "splashscreen.h"
#include "menuwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setStyle("Fusion");

    SplashScreen* splash = new SplashScreen();
    MenuWindow*   menu   = new MenuWindow();
    MainWindow*   window = new MainWindow();

    // Splash → Menu
    QObject::connect(splash, &SplashScreen::finished, [=]() {
        splash->close();
        splash->deleteLater();
        menu->show();
    });

    // Menu → Juego
    QObject::connect(menu, &MenuWindow::playRequested, [=](const QString& name) {
        window->setPathogenName(name);
        menu->close();
        menu->deleteLater();
        window->show();
    });

    // Salir desde menu
    QObject::connect(menu, &MenuWindow::exitRequested, [&a]() {
        a.quit();
    });

    splash->show();
    return a.exec();
}