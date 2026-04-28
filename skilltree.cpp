#include "skilltree.h"
#include <QQueue>

// ── Construction helpers ─────────────────────────────────────────────────────

static SkillNode* makeNode(const QString& id, const QString& name,
                           const QString& desc, int cost,
                           SkillNode* parent, NodeEffect effect)
{
    return new SkillNode(id, name, desc, cost, parent, effect);
}

void SkillTree::clear(SkillNode* node) {
    if (!node) return;
    for (auto* child : node->m_children)
        clear(child);
    delete node;
}

SkillTree::SkillTree() : m_dnaPoints(INITIAL_DNA) {
    m_root = makeNode("bacteria", "Bacteria", "Pathogen base", 0, nullptr,
                      NodeEffect(0, 0, 0, 0));
    m_root->m_unlocked = true;

    // ── Branch 1: Transmission ───────────────────────────────────────────────
    auto* trans   = makeNode("trans",    "Transmision",   "Improves spread",         2, m_root,  NodeEffect(8,  0,  0,  0));
    auto* aire    = makeNode("aire",     "Via Aerea",     "+12% air infection",       3, trans,   NodeEffect(12, 0,  0, -3));
    auto* agua    = makeNode("agua",     "Via Acuatica",  "+8% water infection",      3, trans,   NodeEffect(8,  0,  0, -2));
    auto* vector  = makeNode("vector",   "Vectores",      "Insects and rodents",      3, trans,   NodeEffect(6,  2,  0, -2));
    auto* aerosol = makeNode("aerosol",  "Aerosol",       "+10% aerosol spread",      4, aire,    NodeEffect(10, 0,  0, -4));
    auto* viento  = makeNode("viento",   "Viento",        "+8% wind spread",          4, aire,    NodeEffect(8,  0,  0, -3));
    auto* lluvia  = makeNode("lluvia",   "Lluvia Acida",  "+6% rain spread",          4, agua,    NodeEffect(6,  3,  0, -3));
    auto* insect  = makeNode("insect",   "Insectos",      "+7% insect spread",        4, vector,  NodeEffect(7,  1,  0, -2));
    auto* roed    = makeNode("roed",     "Roedores",      "+5% rodent spread",        4, vector,  NodeEffect(5,  2,  1, -2));
    auto* pandemia= makeNode("pandemia", "Pandemia",      "+20% global spread",       5, aerosol, NodeEffect(20, 5,  2, -8));

    addChild(m_root,   trans);
    addChild(trans,    aire);    addChild(trans,  agua);    addChild(trans,   vector);
    addChild(aire,     aerosol); addChild(aire,   viento);
    addChild(agua,     lluvia);
    addChild(vector,   insect);  addChild(vector, roed);
    addChild(aerosol,  pandemia);

    // ── Branch 2: Symptoms ───────────────────────────────────────────────────
    auto* symp     = makeNode("symp",     "Sintomas",   "Activates visible symptoms",  2, m_root, NodeEffect(0,  8,  3, -5));
    auto* fiebre   = makeNode("fiebre",   "Fiebre",     "+10% severity",               3, symp,   NodeEffect(3, 10,  2, -5));
    auto* fatiga   = makeNode("fatiga",   "Fatiga",     "+8% severity",                3, symp,   NodeEffect(0,  8,  1, -3));
    auto* necrosis = makeNode("necrosis", "Necrosis",   "+15% lethality",              4, symp,   NodeEffect(0, 12, 15, -8));
    auto* tos      = makeNode("tos",      "Tos",        "+8% infection via cough",     4, fiebre, NodeEffect(8,  4,  1, -6));
    auto* vomitos  = makeNode("vomitos",  "Vomitos",    "+5% severity",                4, fiebre, NodeEffect(2,  5,  2, -4));
    auto* paralisis= makeNode("par",      "Paralisis",  "+10% lethality",              4, fatiga, NodeEffect(0,  5, 10, -5));
    auto* hemor    = makeNode("hemor",    "Hemorragia", "+25% lethality",              5, tos,    NodeEffect(0, 15, 25,-10));

    addChild(m_root,  symp);
    addChild(symp,    fiebre);   addChild(symp,   fatiga); addChild(symp, necrosis);
    addChild(fiebre,  tos);      addChild(fiebre, vomitos);
    addChild(fatiga,  paralisis);
    addChild(tos,     hemor);

    // ── Branch 3: Abilities ──────────────────────────────────────────────────
    auto* abil   = makeNode("abil",   "Habilidades",     "Adaptive resistances",    2, m_root, NodeEffect(0,  0,  0, 10));
    auto* drug1  = makeNode("drug1",  "Drug Resist. I",  "+15% drug resistance",    4, abil,   NodeEffect(3,  0,  4,  8));
    auto* camuf  = makeNode("camuf",  "Camuflaje",       "Reduces detection",       5, abil,   NodeEffect(0,  0,  0, 20));
    auto* drug2  = makeNode("drug2",  "Drug Resist. II", "+25% drug resistance",    6, drug1,  NodeEffect(6,  0,  8, 10));
    auto* mutac  = makeNode("mutac",  "Mutacion",        "+10% all stats",          5, drug1,  NodeEffect(5,  5,  5,  5));
    auto* inmun  = makeNode("inmun",  "Inmunidad",       "Immune to all drugs",     7, drug2,  NodeEffect(10, 5, 10, 15));
    auto* sigext = makeNode("sigext", "Sigilo Ext.",     "+30% stealth",            5, camuf,  NodeEffect(0,  0,  0, 30));

    addChild(m_root, abil);
    addChild(abil,   drug1);  addChild(abil,  camuf);
    addChild(drug1,  drug2);  addChild(drug1, mutac);
    addChild(drug2,  inmun);
    addChild(camuf,  sigext);

    recalcStats();
}

SkillTree::~SkillTree() { clear(m_root); }

bool SkillTree::addChild(SkillNode* parent, SkillNode* child) {
    if (!parent || parent->m_children.size() >= MAX_CHILDREN) return false;
    parent->m_children.append(child);
    child->m_parent = parent;
    return true;
}

bool SkillTree::unlock(SkillNode* node) {
    if (!node || node->m_unlocked)                           return false;
    if (node->m_parent && !node->m_parent->m_unlocked)       return false;
    if (m_dnaPoints < node->m_dnaCost)                       return false;
    m_dnaPoints -= node->m_dnaCost;
    node->m_unlocked = true;
    recalcStats();
    return true;
}

void SkillTree::recalcStats() {
    m_stats = m_baseStats;
    QList<SkillNode*> all;
    preOrder(m_root, all);
    for (const auto* n : all) {
        if (!n->m_unlocked) continue;
        m_stats.infectivity += n->m_effect.infectivity;
        m_stats.severity    += n->m_effect.severity;
        m_stats.lethality   += n->m_effect.lethality;
        m_stats.stealth     += n->m_effect.stealth;
    }
    const auto clamp = [](float v) { return qBound(0.0f, v, 100.0f); };
    m_stats.infectivity = clamp(m_stats.infectivity);
    m_stats.severity    = clamp(m_stats.severity);
    m_stats.lethality   = clamp(m_stats.lethality);
    m_stats.stealth     = clamp(m_stats.stealth);
}

void SkillTree::preOrder(SkillNode* node, QList<SkillNode*>& result) const {
    if (!node) return;
    result.append(node);
    for (auto* child : node->m_children)
        preOrder(child, result);
}

void SkillTree::inOrder(SkillNode* node, QList<SkillNode*>& result) const {
    if (!node) return;
    if (!node->m_children.isEmpty())
        inOrder(node->m_children[0], result);
    result.append(node);
    for (int i = 1; i < node->m_children.size(); ++i)
        inOrder(node->m_children[i], result);
}

void SkillTree::postOrder(SkillNode* node, QList<SkillNode*>& result) const {
    if (!node) return;
    for (auto* child : node->m_children)
        postOrder(child, result);
    result.append(node);
}

void SkillTree::bfs(QList<SkillNode*>& result) const {
    if (!m_root) return;
    QQueue<SkillNode*> queue;
    queue.enqueue(m_root);
    while (!queue.isEmpty()) {
        auto* curr = queue.dequeue();
        result.append(curr);
        for (auto* child : curr->m_children)
            queue.enqueue(child);
    }
}

int SkillTree::height(SkillNode* node) const {
    if (!node || node->m_children.isEmpty()) return 0;
    int maxH = 0;
    for (const auto* child : node->m_children)
        maxH = qMax(maxH, height(const_cast<SkillNode*>(child)));
    return maxH + 1;
}

SkillNode* SkillTree::findById(SkillNode* node, const QString& id) const {
    if (!node) return nullptr;
    if (node->m_id == id) return node;
    for (auto* child : node->m_children) {
        if (auto* found = findById(child, id))
            return found;
    }
    return nullptr;
}