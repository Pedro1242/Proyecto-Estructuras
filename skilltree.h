#pragma once
#include "skillnode.h"
#include <QList>

struct BacteriaStats {
    float infectivity =  5.0f;  // base muy bajo - crece con habilidades
    float severity    =  2.0f;
    float lethality   =  1.0f;
    float stealth     = 15.0f;  // 0=detectable, 100=invisible
};

class SkillTree {
public:
    SkillNode*    root;
    int           dnaPoints;
    BacteriaStats baseStats;
    BacteriaStats stats;

    SkillTree();
    ~SkillTree();

    bool addChild(SkillNode* parent, SkillNode* child);
    bool unlock(SkillNode* node);
    void clear(SkillNode* node);
    void recalcStats();

    void preOrder(SkillNode* node, QList<SkillNode*>& result);
    void inOrder(SkillNode* node, QList<SkillNode*>& result);
    void postOrder(SkillNode* node, QList<SkillNode*>& result);
    void bfs(QList<SkillNode*>& result);

    int height(SkillNode* node);
    SkillNode* findById(SkillNode* node, const QString& id);
};