QT += widgets

CONFIG += c++17

SOURCES += \
    backgroundscene.cpp \
    clickablehex.cpp \
    main.cpp \
    mainwindow.cpp \
    menuwindow.cpp \
    population.cpp \
    skilltree.cpp \
    splashscreen.cpp \
    treerenderer.cpp \
    worldmapwidget.cpp

HEADERS += \
    backgroundscene.h \
    clickablehex.h \
    mainwindow.h \
    menuwindow.h \
    population.h \
    skillnode.h \
    skilltree.h \
    splashscreen.h \
    treerenderer.h \
    worldmapwidget.h \
    worldregion.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target