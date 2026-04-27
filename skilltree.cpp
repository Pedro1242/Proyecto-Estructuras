#include "skilltree.h"
#include <QQueue>

SkillTree::SkillTree() : dnaPoints(20) {
    root = new SkillNode("bacteria", "Bacteria", "Patógeno base", 0, nullptr,
                         NodeEffect(0, 0, 0, 0));
    root->unlocked = true;

    // Rama 1: Transmisión — mejora infectividad
    auto* trans = new SkillNode("trans", "Transmisión", "Mejora de propagación", 2, root,
                                NodeEffect(10, 0, 0, -5));
    auto* aire  = new SkillNode("aire", "Vía Aérea", "+15% infección aérea", 3, trans,
                               NodeEffect(15, 0, 0, -8));
    auto* agua  = new SkillNode("agua", "Vía Acuática", "+10% infección acuática", 3, trans,
                               NodeEffect(10, 0, 0, -5));
    auto* pluma = new SkillNode("pluma", "Zoonosis", "+20% en aves, +5% lethal", 4, aire,
                                NodeEffect(20, 5, 5, -10));
    addChild(root, trans); addChild(trans, aire);
    addChild(trans, agua); addChild(aire, pluma);

    // Rama 2: Síntomas — mejora severidad y letalidad
    auto* symp   = new SkillNode("symp", "Síntomas", "Activa síntomas visibles", 2, root,
                               NodeEffect(0, 10, 5, -15));
    auto* fiebre = new SkillNode("fiebre", "Fiebre", "+10% severidad", 3, symp,
                                 NodeEffect(5, 10, 3, -10));
    auto* tos    = new SkillNode("tos", "Tos", "+8% infección por tos", 3, fiebre,
                              NodeEffect(8, 5, 2, -12));
    auto* hemor  = new SkillNode("hemor", "Hemorragia", "+25% letalidad", 5, fiebre,
                                NodeEffect(0, 15, 25, -20));
    addChild(root, symp); addChild(symp, fiebre);
    addChild(fiebre, tos); addChild(fiebre, hemor);

    // Rama 3: Habilidades — mejora sigilo y resistencia
    auto* abil  = new SkillNode("abil", "Habilidades", "Mejoras adaptativas", 2, root,
                               NodeEffect(0, 0, 0, 15));
    auto* drug1 = new SkillNode("drug1", "Drug Resist. I", "+20% resist. fármacos", 4, abil,
                                NodeEffect(5, 0, 5, 10));
    auto* drug2 = new SkillNode("drug2", "Drug Resist. II", "+40% resist. fármacos", 6, drug1,
                                NodeEffect(10, 0, 10, 15));
    auto* camuf = new SkillNode("camuf", "Camuflaje", "Reduce detección", 5, abil,
                                NodeEffect(0, 0, 0, 30));
    addChild(root, abil); addChild(abil, drug1);
    addChild(drug1, drug2); addChild(abil, camuf);

    recalcStats();
}

SkillTree::~SkillTree() {
    clear(root);
}

void SkillTree::clear(SkillNode* node) {
    if (!node) return;
    for (auto* child : node->children)
        clear(child);
    delete node;
}

bool SkillTree::addChild(SkillNode* parent, SkillNode* child) {
    if (!parent || parent->children.size() >= 3) return false;
    parent->children.append(child);
    child->parent = parent;
    return true;
}

bool SkillTree::unlock(SkillNode* node) {
    if (!node || node->unlocked) return false;
    if (node->parent && !node->parent->unlocked) return false;
    if (dnaPoints < node->dnaCost) return false;
    dnaPoints -= node->dnaCost;
    node->unlocked = true;
    recalcStats(); // ✅ recalcula stats inmediatamente al desbloquear
    return true;
}

void SkillTree::recalcStats() {
    stats = baseStats;
    QList<SkillNode*> all;
    preOrder(root, all);
    for (auto* n : all) {
        if (n->unlocked) {
            stats.infectivity += n->effect.infectivity;
            stats.severity    += n->effect.severity;
            stats.lethality   += n->effect.lethality;
            stats.stealth     += n->effect.stealth;
        }
    }
    auto clamp = [](float v){ return qBound(0.0f, v, 100.0f); };
    stats.infectivity = clamp(stats.infectivity);
    stats.severity    = clamp(stats.severity);
    stats.lethality   = clamp(stats.lethality);
    stats.stealth     = clamp(stats.stealth);
}

void SkillTree::preOrder(SkillNode* node, QList<SkillNode*>& result) {
    if (!node) return;
    result.append(node);
    for (auto* child : node->children) preOrder(child, result);
}

void SkillTree::inOrder(SkillNode* node, QList<SkillNode*>& result) {
    if (!node) return;
    if (!node->children.isEmpty()) inOrder(node->children[0], result);
    result.append(node);
    for (int i = 1; i < node->children.size(); ++i) inOrder(node->children[i], result);
}

void SkillTree::postOrder(SkillNode* node, QList<SkillNode*>& result) {
    if (!node) return;
    for (auto* child : node->children) postOrder(child, result);
    result.append(node);
}

void SkillTree::bfs(QList<SkillNode*>& result) {
    if (!root) return;
    QQueue<SkillNode*> queue;
    queue.enqueue(root);
    while (!queue.isEmpty()) {
        auto* curr = queue.dequeue();
        result.append(curr);
        for (auto* child : curr->children) queue.enqueue(child);
    }
}