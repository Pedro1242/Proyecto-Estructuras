#include "skilltree.h"
#include <QQueue>

SkillTree::SkillTree() : dnaPoints(20) {
    root = new SkillNode("bacteria", "Bacteria", "Pathogen base", 0, nullptr,
                         NodeEffect(0, 0, 0, 0));
    root->unlocked = true;

    // --- Rama 1: Transmision (3 hijos) ---
    auto* trans  = new SkillNode("trans",   "Transmision",   "Improves spread",          2, root,  NodeEffect(8,  0,  0,  0));
    auto* aire   = new SkillNode("aire",    "Via Aerea",     "+12% air infection",        3, trans, NodeEffect(12, 0,  0, -3));
    auto* agua   = new SkillNode("agua",    "Via Acuatica",  "+8% water infection",       3, trans, NodeEffect(8,  0,  0, -2));
    auto* vector = new SkillNode("vector",  "Vectores",      "Insects and rodents",       3, trans, NodeEffect(6,  2,  0, -2));

    auto* aerosol = new SkillNode("aerosol", "Aerosol",      "+10% aerosol spread",       4, aire,   NodeEffect(10, 0,  0, -4));
    auto* viento  = new SkillNode("viento",  "Viento",       "+8% wind spread",           4, aire,   NodeEffect(8,  0,  0, -3));
    auto* lluvia  = new SkillNode("lluvia",  "Lluvia Acida", "+6% rain spread",           4, agua,   NodeEffect(6,  3,  0, -3));
    auto* insect  = new SkillNode("insect",  "Insectos",     "+7% insect spread",         4, vector, NodeEffect(7,  1,  0, -2));
    auto* roed    = new SkillNode("roed",    "Roedores",     "+5% rodent spread",         4, vector, NodeEffect(5,  2,  1, -2));

    // height 4: pandemia is child of aerosol
    auto* pandemia = new SkillNode("pandemia", "Pandemia",   "+20% global spread",        5, aerosol, NodeEffect(20, 5,  2, -8));

    addChild(root,    trans);
    addChild(trans,   aire);   addChild(trans,  agua);   addChild(trans,  vector);
    addChild(aire,    aerosol); addChild(aire,  viento);
    addChild(agua,    lluvia);
    addChild(vector,  insect);  addChild(vector, roed);
    addChild(aerosol, pandemia);

    // --- Rama 2: Sintomas (3 hijos) ---
    auto* symp    = new SkillNode("symp",    "Sintomas",     "Activates visible symptoms", 2, root,   NodeEffect(0,  8,  3, -5));
    auto* fiebre  = new SkillNode("fiebre",  "Fiebre",       "+10% severity",              3, symp,   NodeEffect(3, 10,  2, -5));
    auto* fatiga  = new SkillNode("fatiga",  "Fatiga",       "+8% severity",               3, symp,   NodeEffect(0,  8,  1, -3));
    auto* necrosis= new SkillNode("necrosis","Necrosis",     "+15% lethality",             4, symp,   NodeEffect(0, 12, 15, -8));

    auto* tos     = new SkillNode("tos",     "Tos",          "+8% infection via cough",    4, fiebre, NodeEffect(8,  4,  1, -6));
    auto* vomitos = new SkillNode("vomitos", "Vomitos",      "+5% severity",               4, fiebre, NodeEffect(2,  5,  2, -4));
    auto* paralisis=new SkillNode("par",     "Paralisis",    "+10% lethality",             4, fatiga, NodeEffect(0,  5, 10, -5));

    auto* hemor   = new SkillNode("hemor",   "Hemorragia",   "+25% lethality",             5, tos,    NodeEffect(0, 15, 25,-10));

    addChild(root,   symp);
    addChild(symp,   fiebre); addChild(symp,  fatiga); addChild(symp, necrosis);
    addChild(fiebre, tos);    addChild(fiebre, vomitos);
    addChild(fatiga, paralisis);
    addChild(tos,    hemor);

    // --- Rama 3: Habilidades (2 hijos - rama asimetrica) ---
    auto* abil   = new SkillNode("abil",   "Habilidades",    "Adaptive resistances",       2, root,   NodeEffect(0,  0,  0, 10));
    auto* drug1  = new SkillNode("drug1",  "Drug Resist. I", "+15% drug resistance",       4, abil,   NodeEffect(3,  0,  4,  8));
    auto* camuf  = new SkillNode("camuf",  "Camuflaje",      "Reduces detection",          5, abil,   NodeEffect(0,  0,  0, 20));

    auto* drug2  = new SkillNode("drug2",  "Drug Resist. II","+25% drug resistance",       6, drug1,  NodeEffect(6,  0,  8, 10));
    auto* mutac  = new SkillNode("mutac",  "Mutacion",       "+10% all stats",             5, drug1,  NodeEffect(5,  5,  5,  5));

    auto* inmun  = new SkillNode("inmun",  "Inmunidad",      "Immune to all drugs",        7, drug2,  NodeEffect(10, 5, 10, 15));
    auto* sigext = new SkillNode("sigext", "Sigilo Ext.",    "+30% stealth",               5, camuf,  NodeEffect(0,  0,  0, 30));

    addChild(root,  abil);
    addChild(abil,  drug1);  addChild(abil,  camuf);
    addChild(drug1, drug2);  addChild(drug1, mutac);
    addChild(drug2, inmun);
    addChild(camuf, sigext);

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
    recalcStats();
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