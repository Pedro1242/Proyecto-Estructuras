#pragma once
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QMap>
#include "skilltree.h"
#include "population.h"        // ✅
#include "backgroundscene.h"   // ✅

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onNodeClicked(SkillNode* node);
    void onAddDna();
    void onReset();
    void onPopulationUpdated();   // ✅
    void onDnaEarned(int amount); // ✅

private slots:
    void onPreOrder();
    void onInOrder();
    void onPostOrder();
    void onBFS();

private:
    SkillTree*       tree;
    Population*      population;       // ✅
    BackgroundScene* scene;            // ✅ cambia de QGraphicsScene a BackgroundScene
    QGraphicsView*   view;
    QListWidget*     traversalList;
    QLabel*          dnaLabel;
    QLabel*          statsLabel;

    // Barras de stats bacteria
    QProgressBar* barInfectivity;
    QProgressBar* barSeverity;
    QProgressBar* barLethality;
    QProgressBar* barStealth;
    QLabel*       lblInfVal;
    QLabel*       lblSevVal;
    QLabel*       lblLetVal;
    QLabel*       lblStlVal;

    // ✅ Labels de población
    QLabel* lblPopHealthy;
    QLabel* lblPopInfected;
    QLabel* lblPopDead;
    QProgressBar* barInfectedPop;

    QMap<SkillNode*, QPointF> nodePositions;

    void buildUI();
    void redrawTree();
    void drawTree(SkillNode* node, qreal x, qreal y, qreal hSpacing, int level);
    void drawNode(SkillNode* node, qreal x, qreal y);
    void showTraversal(QList<SkillNode*>& nodes, const QString& title);
    void updateDnaLabel();
    void updateStatsPanel();
    // en mainwindow.h — dentro de private:
    void wheelEvent(QWheelEvent* event) override;
};