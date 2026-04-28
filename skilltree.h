#pragma once
#include "skillnode.h"
#include <QList>

/*!
 * \brief Aggregated stats of the pathogen based on unlocked nodes.
 */
struct BacteriaStats {
    float infectivity { 5.0f  };
    float severity    { 2.0f  };
    float lethality   { 1.0f  };
    float stealth     { 15.0f };
};

/*!
 * \brief N-ary skill tree that manages pathogen upgrades and DNA points.
 *
 * Owns all SkillNode instances. Non-copyable.
 */
class SkillTree {
public:
    static constexpr int MAX_CHILDREN    { 3   };
    static constexpr int INITIAL_DNA     { 20  };

    SkillNode*    m_root     { nullptr };
    int           m_dnaPoints{ 0       };
    BacteriaStats m_baseStats;
    BacteriaStats m_stats;

    SkillTree();
    ~SkillTree();

    // Non-copyable
    SkillTree(const SkillTree&)            = delete;
    SkillTree& operator=(const SkillTree&) = delete;

    bool addChild(SkillNode* parent, SkillNode* child);
    bool unlock(SkillNode* node);
    void recalcStats();
    void clear(SkillNode* node);

    // Tree traversals
    void preOrder (SkillNode* node, QList<SkillNode*>& result) const;
    void inOrder  (SkillNode* node, QList<SkillNode*>& result) const;
    void postOrder(SkillNode* node, QList<SkillNode*>& result) const;
    void bfs      (QList<SkillNode*>& result) const;

    int        height   (SkillNode* node) const;
    SkillNode* findById (SkillNode* node, const QString& id) const;
};