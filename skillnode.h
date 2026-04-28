#pragma once
#include <QString>
#include <QList>

/*!
 * \brief Represents the stat changes applied when a node is unlocked.
 */
struct NodeEffect {
    float infectivity { 0.0f };
    float severity    { 0.0f };
    float lethality   { 0.0f };
    float stealth     { 0.0f };

    constexpr NodeEffect() = default;
    constexpr NodeEffect(float i, float s, float l, float st)
        : infectivity(i), severity(s), lethality(l), stealth(st) {}
};

/*!
 * \brief A single node in the skill tree.
 *
 * Each node represents an upgradable pathogen trait.
 * Ownership of children belongs to the SkillTree.
 */
struct SkillNode {
    QString            m_id;
    QString            m_name;
    QString            m_description;
    int                m_dnaCost  { 0 };
    bool               m_unlocked { false };
    SkillNode*         m_parent   { nullptr };
    QList<SkillNode*>  m_children;
    NodeEffect         m_effect;

    explicit SkillNode(const QString& id,
                       const QString& name,
                       const QString& description,
                       int            dnaCost,
                       SkillNode*     parent = nullptr,
                       NodeEffect     effect = NodeEffect())
        : m_id(id)
        , m_name(name)
        , m_description(description)
        , m_dnaCost(dnaCost)
        , m_parent(parent)
        , m_effect(effect)
    {}

    ~SkillNode() = default;

    // Non-copyable — tree owns all nodes
    SkillNode(const SkillNode&)            = delete;
    SkillNode& operator=(const SkillNode&) = delete;
};