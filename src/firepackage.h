#ifndef FIREPACKAGE_H
#define FIREPACKAGE_H

#include "package.h"
#include "card.h"

class FirePackage : public Package{
    Q_OBJECT

public:
    FirePackage();
};

class QuhuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QuhuCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void use(const QList<const ClientPlayer *> &targets) const;
};

class JiemingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JiemingCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class QiangxiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QiangxiCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual void use(const QList<const ClientPlayer *> &targets) const;
};

class TianyiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TianyiCard();

    virtual bool targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void use(const QList<const ClientPlayer *> &targets) const;
};

#endif // FIREPACKAGE_H
