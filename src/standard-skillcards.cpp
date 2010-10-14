#include "standard.h"
#include "room.h"
#include "clientplayer.h"
#include "engine.h"
#include "client.h"

ZhihengCard::ZhihengCard(){
    target_fixed = true;
}

void ZhihengCard::use(const QList<const ClientPlayer *> &) const{
    ClientInstance->turn_tag.insert("zhiheng_used", true);
}

void ZhihengCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    room->drawCards(source, subcards.length());
}


RendeCard::RendeCard(){
}

void RendeCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(targets.isEmpty())
        return;

    ServerPlayer *target = targets.first();
    room->moveCardTo(this, target, Player::Hand, false);

    int old_value = source->getMark("rende");
    int new_value = old_value + subcards.length();
    source->setMark("rende", new_value);

    if(old_value < 2 && new_value >= 2)
        room->recover(source);
}

JieyinCard::JieyinCard(){

}

bool JieyinCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    return to_select->getGeneral()->isMale() && to_select->isWounded();
}

void JieyinCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    room->recover(source, 1, true);
    room->recover(targets.first(), 1, true);
}

void JieyinCard::use(const QList<const ClientPlayer *> &targets) const{
    ClientInstance->turn_tag.insert("jieyin_used", true);
}

TuxiCard::TuxiCard(){
}

bool TuxiCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(targets.length() >= 2)
        return false;

    if(to_select == Self)
        return false;

    return !to_select->isKongcheng();
}

void TuxiCard::onEffect(const CardEffectStruct &effect) const{
    int card_id = effect.to->getRandomHandCard();
    const Card *card = Sanguosha->getCard(card_id);
    Room *room = effect.from->getRoom();
    room->moveCardTo(card, effect.from, Player::Hand, false);

    room->setEmotion(effect.to, Room::Bad);
    room->setEmotion(effect.from, Room::Good);
}

FanjianCard::FanjianCard(){

}

void FanjianCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();

    int card_id = source->getRandomHandCard();
    const Card *card = Sanguosha->getCard(card_id);
    Card::Suit suit = room->askForSuit(target);

    LogMessage log;
    log.type = "#ChooseSuit";
    log.from = target;
    log.arg = Card::Suit2String(suit);
    room->sendLog(log);

    if(card->getSuit() != suit){
        DamageStruct damage;
        damage.card = this;
        damage.from = source;
        damage.to = target;
        damage.damage = 1;

        room->damage(damage);
    }

    if(target->isAlive()){
        target->obtainCard(card);
    }
}

void FanjianCard::use(const QList<const ClientPlayer *> &) const{
    ClientInstance->turn_tag.insert("fanjian_used", true);
}

KurouCard::KurouCard(){
    target_fixed = true;
}

void KurouCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->loseHp(source);
    if(source->isAlive())
        room->drawCards(source, 2);
}

LijianCard::LijianCard(){

}

bool LijianCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!to_select->getGeneral()->isMale())
        return false;

    if(targets.isEmpty() && to_select->hasSkill("kongcheng") && to_select->isKongcheng()){
        return false;
    }

    return true;
}

bool LijianCard::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    return targets.length() == 2;
}

void LijianCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    ServerPlayer *to = targets.at(0);
    ServerPlayer *from = targets.at(1);

    CardEffectStruct effect;
    effect.card = new Duel(Card::NoSuit, 0);
    effect.from = from;
    effect.to = to;

    room->directCardEffect(effect);
}

void LijianCard::use(const QList<const ClientPlayer *> &targets) const{
    ClientInstance->turn_tag.insert("lijian_used", true);
}

QingnangCard::QingnangCard(){
}

bool QingnangCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && to_select->isWounded();
}

bool QingnangCard::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    if(targets.length() > 1)
        return false;

    const ClientPlayer *to_cure = targets.isEmpty() ? Self : targets.first();
    return to_cure->isWounded();
}

void QingnangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    ServerPlayer *target;
    if(!targets.isEmpty())
        target = targets.first();
    else
        target = source;

    CardEffectStruct effect;
    effect.card = this;
    effect.from = source;
    effect.to = target;

    room->cardEffect(effect);
}

void QingnangCard::use(const QList<const ClientPlayer *> &targets) const{
    ClientInstance->turn_tag.insert("qingnang_used", true);
}

void QingnangCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->getRoom()->recover(effect.to, 1);
}

GuicaiCard::GuicaiCard(){
    target_fixed = true;
}

LiuliCard::LiuliCard(){
}

void LiuliCard::setSlashSource(const QString &slash_source){
    this->slash_source = slash_source;
}

bool LiuliCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;

    if(to_select->objectName() == slash_source)
        return false;

    int card_id = subcards.first();
    if(Self->getWeapon() && Self->getWeapon()->getId() == card_id)
        return Self->distanceTo(to_select) <= 1;
    else
        return Self->inMyAttackRange(to_select);
}

void LiuliCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->getRoom()->setPlayerFlag(effect.to, "liuli_target");
}

JijiangCard::JijiangCard(){

}

bool JijiangCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && Self->canSlash(to_select);
}

void JijiangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> lieges = room->getLieges(source);
    const Card *slash = NULL;
    foreach(ServerPlayer *liege, lieges){
        QString result = room->askForChoice(liege, "jijiang", "accept+ignore");
        if(result == "ignore")
            continue;

        slash = room->askForCard(liege, "slash", "@jijiang-slash");
        if(slash){
            source->invoke("increaseSlashCount");
            room->cardEffect(slash, source, targets.first());
            return;
        }
    }
}

#ifndef QT_NO_DEBUG

CheatCard::CheatCard(){
    target_fixed = true;
}

void CheatCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->obtainCard(source, subcards.first());
}

#endif

