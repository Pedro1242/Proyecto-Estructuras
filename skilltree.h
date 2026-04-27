#pragma once
#include "skillnode.h"
#include <QList>

// Estadísticas globales de la bacteria
struct BacteriaStats {
    float infectivity = 25.0f;  // base
    float severity    = 10.0f;
    float lethality   = 5.0f;
    float stealth     = 80.0f;  // 100 = invisible, 0 = muy detectada
};

class SkillTree {
public:
    SkillNode*    root;
    int           dnaPoints;
    BacteriaStats baseStats;   // stats fijas base
    BacteriaStats stats;       // stats calculadas con bonuses

    SkillTree();
    ~SkillTree();

    bool addChild(SkillNode* parent, SkillNode* child);
    bool unlock(SkillNode* node);
    void clear(SkillNode* node);
    void recalcStats();   // ✅ recalcula stats sumando efectos desbloqueados

    void preOrder(SkillNode* node, QList<SkillNode*>& result);
    void inOrder(SkillNode* node, QList<SkillNode*>& result);
    void postOrder(SkillNode* node, QList<SkillNode*>& result);
    void bfs(QList<SkillNode*>& result);

    int height(SkillNode* node);
    SkillNode* findById(SkillNode* node, const QString& id);
};