#pragma once
#include <QMainWindow>
#include <QGraphicsView>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QWheelEvent>
#include <QTimer>
#include "skilltree.h"
#include "population.h"
#include "backgroundscene.h"
#include "treerenderer.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void setPathogenName(const QString& name);

public slots:
    void onNodeClicked(SkillNode* node);
    void onAddDna();
    void onReset();
    void onPopulationUpdated();
    void onDnaEarned(int amount);

private slots:
    void onPreOrder();
    void onInOrder();
    void onPostOrder();
    void onBFS();
    void onDayChanged(int day);
    void tickNews();
    void onCureUpdated(float progress);
    void onCureCompleted();
    void onPlayerWon();

private:
    // Core
    SkillTree*       m_tree        { nullptr };
    Population*      m_population  { nullptr };
    BackgroundScene* m_scene       { nullptr };
    TreeRenderer*    m_treeRenderer{ nullptr };

    // View
    QGraphicsView*   m_view          { nullptr };
    QListWidget*     m_traversalList { nullptr };
    QLabel*          m_nodeTooltip   { nullptr };

    // Info labels
    QLabel*          m_dnaLabel   { nullptr };
    QLabel*          m_statsLabel { nullptr };
    QLabel*          m_lblDay     { nullptr };

    // News ticker
    QLabel*          m_lblNewsTicker { nullptr };
    QTimer*          m_newsTimer     { nullptr };
    QString          m_currentNews;
    QString          m_pendingNews;
    int              m_newsScrollPos { 0 };

    // Stat bars
    QProgressBar*    m_barInfectivity { nullptr };
    QProgressBar*    m_barSeverity    { nullptr };
    QProgressBar*    m_barLethality   { nullptr };
    QProgressBar*    m_barStealth     { nullptr };
    QLabel*          m_lblInfVal      { nullptr };
    QLabel*          m_lblSevVal      { nullptr };
    QLabel*          m_lblLetVal      { nullptr };
    QLabel*          m_lblStlVal      { nullptr };

    // Population display
    QLabel*          m_lblPopHealthy  { nullptr };
    QLabel*          m_lblPopInfected { nullptr };
    QLabel*          m_lblPopDead     { nullptr };
    QProgressBar*    m_barInfectedPop { nullptr };

    // Cure bar
    QLabel*          m_lblCureTitle { nullptr };
    QProgressBar*    m_barCure      { nullptr };
    QLabel*          m_lblCureVal   { nullptr };

    void    buildUI();
    void    showTraversal(QList<SkillNode*>& nodes, const QString& title);
    void    updateDnaLabel();
    void    updateStatsPanel();
    void    showNodeTooltip(SkillNode* node, const QPoint& globalPos);
    void    hideNodeTooltip();
    QString pickNews(int day) const;

    void wheelEvent(QWheelEvent* event) override;
};