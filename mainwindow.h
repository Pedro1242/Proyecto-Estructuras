#pragma once
#include <QMainWindow>
#include <QGraphicsView>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QMap>
#include <QWheelEvent>
#include <QTimer>
#include "skilltree.h"
#include "population.h"
#include "backgroundscene.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onNodeClicked(SkillNode* node);
    void onAddDna();
    void onReset();
    void onPopulationUpdated();
    void onDnaEarned(int amount);

    // tooltip persistente para nodos
    void showNodeTooltip(SkillNode* node, const QPoint& globalPos);
    void hideNodeTooltip();

    // barra de cura
    void onCureUpdated(float progress);
    void onCureCompleted();
    void onPlayerWon();

private slots:
    void onPreOrder();
    void onInOrder();
    void onPostOrder();
    void onBFS();
    void onDayChanged(int day);
    void tickNews();

private:

    SkillTree*       tree;
    Population*      population;
    BackgroundScene* scene;
    QGraphicsView*   view;
    QListWidget*     traversalList;
    QLabel*          dnaLabel;
    QLabel*          statsLabel;

    // tooltip de nodo
    QLabel*          nodeTooltip;

    // contador de dias
    QLabel*          lblDay;

    // ticker de noticias
    QLabel*          lblNewsTicker;
    QTimer*          newsTimer;
    QString          currentNews;
    QString          pendingNews;
    int              newsScrollPos;

    QProgressBar* barInfectivity;
    QProgressBar* barSeverity;
    QProgressBar* barLethality;
    QProgressBar* barStealth;
    QLabel*       lblInfVal;
    QLabel*       lblSevVal;
    QLabel*       lblLetVal;
    QLabel*       lblStlVal;

    QLabel*       lblPopHealthy;
    QLabel*       lblPopInfected;
    QLabel*       lblPopDead;
    QProgressBar* barInfectedPop;

    // barra de cura
    QLabel*       lblCureTitle;
    QProgressBar* barCure;
    QLabel*       lblCureVal;

    QMap<SkillNode*, QPointF> nodePositions;

    void buildUI();
    void redrawTree();
    void drawTree(SkillNode* node, qreal x, qreal y, qreal hSpacing, int level);
    void drawNode(SkillNode* node, qreal x, qreal y);
    void showTraversal(QList<SkillNode*>& nodes, const QString& title);
    void updateDnaLabel();
    void updateStatsPanel();
    void wheelEvent(QWheelEvent* event) override;

    QString pickNews(int day);
};