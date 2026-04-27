#pragma once
#include <QString>
#include <QList>

// Efectos que aplica cada nodo al desbloquearse
struct NodeEffect {
    float infectivity;   // % cambio en infectividad
    float severity;      // % cambio en severidad
    float lethality;     // % cambio en letalidad
    float stealth;       // % cambio en sigilo (- = más difícil detectar)

    NodeEffect(float i=0, float s=0, float l=0, float st=0)
        : infectivity(i), severity(s), lethality(l), stealth(st) {}
};

struct SkillNode {
    QString id;
    QString name;
    QString description;
    int     dnaCost;
    bool    unlocked;
    SkillNode* parent;
    QList<SkillNode*> children;
    NodeEffect effect;   // ✅ NUEVO

    SkillNode(QString id, QString name, QString desc, int cost,
              SkillNode* parent = nullptr,
              NodeEffect effect = NodeEffect())
        : id(id), name(name), description(desc),
        dnaCost(cost), unlocked(false), parent(parent), effect(effect) {}
};