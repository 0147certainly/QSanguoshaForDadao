#include "maneuvering.h"
#include "client.h"
#include "engine.h"

NatureSlash::NatureSlash(Suit suit, int number, DamageStruct::Nature nature)
    :Slash(suit, number)
{
    this->nature = nature;
}

bool NatureSlash::match(const QString &pattern) const{
    if(pattern == "slash")
        return true;
    else
        return Slash::match(pattern);
}

ThunderSlash::ThunderSlash(Suit suit, int number)
    :NatureSlash(suit, number, DamageStruct::Thunder)
{    
    setObjectName("thunder_slash");
}

FireSlash::FireSlash(Suit suit, int number)
    :NatureSlash(suit, number, DamageStruct::Fire)
{
    setObjectName("fire_slash");
    nature = DamageStruct::Fire;
}

Analeptic::Analeptic(Card::Suit suit, int number)
    :BasicCard(suit, number)
{
    setObjectName("analeptic");
    target_fixed = true;
}

QString Analeptic::getSubtype() const{
    return "buff_card";
}

bool Analeptic::isAvailable() const{
    return !Self->hasFlag("drank");
}

void Analeptic::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    source->playCardEffect(this);

    CardEffectStruct effect;
    effect.card = this;
    effect.from = source;
    effect.to = source;

    room->cardEffect(effect);
}

void Analeptic::onEffect(const CardEffectStruct &effect) const{
    effect.to->getRoom()->setPlayerFlag(effect.to, "drank");
}

bool Analeptic::match(const QString &pattern) const{
    if(pattern == "peach+analeptic")
        return true;
    else
        return BasicCard::match(pattern);
}

Fan::Fan(Suit suit, int number):Weapon(suit, number, 4){
    setObjectName("fan");
}

class GudingBladeSkill: public TriggerSkill{
public:
    GudingBladeSkill():TriggerSkill("guding_blade"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card->inherits("Slash") && damage.to->isKongcheng()){
            damage.damage ++;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

GudingBlade::GudingBlade(Suit suit, int number):Weapon(suit, number, 2){
    setObjectName("guding_blade");

    skill = new GudingBladeSkill;
}

class VineSkill: public TriggerSkill{
public:
    VineSkill():TriggerSkill("vine"){
        events << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Fire){
            damage.damage ++;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

Vine::Vine(Suit suit, int number):Armor(suit, number){
    setObjectName("vine");
    set_flag = true;
}

class SilverLionSkill: public TriggerSkill{
public:
    SilverLionSkill():TriggerSkill("silver_lion"){
        events << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasFlag("silver_lion");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.damage > 1){
            damage.damage = 1;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

SilverLion::SilverLion(Suit suit, int number):Armor(suit, number){
    setObjectName("silver_lion");

    skill = new SilverLionSkill;
    set_flag = true;
}

void SilverLion::onUninstall(ServerPlayer *player) const{
    player->getRoom()->recover(player, 1);
}

FireAttack::FireAttack(Card::Suit suit, int number)
    :SingleTargetTrick(suit, number)
{
    setObjectName("fire_attack");
}

bool FireAttack::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && !to_select->isKongcheng();
}

void FireAttack::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if(effect.to->isKongcheng())
        return;

    int card_id = room->askForCardShow(effect.to);
    //FIXME: broadcard players that effect.to has shown his/her card
    const Card *card = Sanguosha->getCard(card_id);
    QString suit_str = card->getSuitString();


}

IronChain::IronChain(Card::Suit suit, int number)
    :TrickCard(suit, number)
{
    setObjectName("iron_chain");
}

QString IronChain::getSubtype() const{
    return "damage_spread";
}

bool IronChain::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(isBlack() && to_select->hasSkill("weimu"))
        return false;

    if(targets.length() > 2)
        return false;

    return true;
}

bool IronChain::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    return targets.length() <= 2;
}

void IronChain::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{   
    room->throwCard(this);
    source->playCardEffect(this); // play another effect if it is recasted

    if(targets.isEmpty()){       
        source->drawCards(1);
    }else{
        foreach(ServerPlayer *target, targets){
            CardEffectStruct effect;
            effect.card = this;
            effect.from = source;
            effect.to = target;

            room->cardEffect(effect);
        }
    }
}

void IronChain::onEffect(const CardEffectStruct &effect) const{
    bool chained = ! effect.to->isChained();
    effect.to->setChained(chained);

    effect.to->getRoom()->broadcastProperty(effect.to, "chained");
}

SupplyShortage::SupplyShortage(Card::Suit suit, int number)
    :DelayedTrick(suit, number)
{
    setObjectName("supply_shortage");
}

bool SupplyShortage::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select){
    if(!targets.isEmpty())
        return false;

    if(isBlack() && to_select->hasSkill("weimu"))
        return false;

    if(Self->hasSkill("qicai"))
        return true;

    int distance = Self->distanceTo(to_select);
    if(Self->hasSkill("duanliang"))
        return distance <= 2;
    else
        return distance <= 1;
}

ManeuveringPackage::ManeuveringPackage()
    :Package("maneuvering")
{
    t["maneuvering"] = tr("maneuvering");
    t["buff_card"] = tr("buff_card");
    t["damage_spread"] = tr("damage_spread");
    t["fire_attack"] = tr("fire_attack");
    t["thunder_slash"] = tr("thunder_slash");
    t["fire_slash"] = tr("fire_slash");
    t["analeptic"] = tr("analeptic");
    t["fan"] = tr("fan");
    t["hualiu"] = tr("hualiu");
    t["guding_blade"] = tr("guding_blade");
    t["vine"] = tr("vine");
    t["silver_lion"] = tr("silver_lion");
    t["iron_chain"] = tr("iron_chain");    
    t["supply_shortage"] = tr("supply_shortage");

    QList<Card *> cards;

    // spade
    cards << new GudingBlade(Card::Spade, 1)
            << new Vine(Card::Spade, 2)
            << new Analeptic(Card::Spade, 3)
            << new ThunderSlash(Card::Spade, 4)
            << new ThunderSlash(Card::Spade, 5)
            << new ThunderSlash(Card::Spade, 6)
            << new ThunderSlash(Card::Spade, 7)
            << new ThunderSlash(Card::Spade, 8)
            << new Analeptic(Card::Spade, 9)
            << new SupplyShortage(Card::Spade,10)
            << new IronChain(Card::Spade, 11)
            << new IronChain(Card::Spade, 12)
            << new Nullification(Card::Spade, 13);

    // club
    cards << new SilverLion(Card::Club, 1)
            << new Vine(Card::Club, 2)
            << new Analeptic(Card::Club, 3)
            << new SupplyShortage(Card::Club, 4)
            << new ThunderSlash(Card::Club, 5)
            << new ThunderSlash(Card::Club, 6)
            << new ThunderSlash(Card::Club, 7)
            << new ThunderSlash(Card::Club, 8)
            << new Analeptic(Card::Club, 9)
            << new IronChain(Card::Club, 10)
            << new IronChain(Card::Club, 11)
            << new IronChain(Card::Club, 12)
            << new IronChain(Card::Club, 13);

    // heart
    cards << new Nullification(Card::Heart, 1)
            << new FireAttack(Card::Heart, 2)
            << new FireSlash(Card::Heart, 3)
            << new FireAttack(Card::Heart, 4)
            << new Peach(Card::Heart, 5)
            << new Peach(Card::Heart, 6)
            << new FireSlash(Card::Heart, 7)
            << new Jink(Card::Heart, 8)
            << new Jink(Card::Heart, 9)
            << new FireSlash(Card::Heart, 10)
            << new Jink(Card::Heart, 11)
            << new Jink(Card::Heart, 12)
            << new Nullification(Card::Heart, 13);

    // diamond
    cards << new Fan(Card::Diamond, 1)
            << new Peach(Card::Diamond, 2)
            << new Peach(Card::Diamond, 3)
            << new FireSlash(Card::Diamond, 4)
            << new FireSlash(Card::Diamond, 5)
            << new Jink(Card::Diamond, 6)
            << new Jink(Card::Diamond, 7)
            << new Jink(Card::Diamond, 8)
            << new Analeptic(Card::Diamond, 9)
            << new Jink(Card::Diamond, 10)
            << new Jink(Card::Diamond, 11)
            << new FireSlash(Card::Diamond, 12)
            << new Horse("hualiu", Card::Diamond, 13, +1);

    foreach(Card *card, cards)
        card->setParent(this);
}

ADD_PACKAGE(Maneuvering)
