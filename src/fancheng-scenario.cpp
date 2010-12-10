#include "fancheng-scenario.h"
#include "scenario.h"
#include "skill.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "standard.h"

class Guagu: public TriggerSkill{
public:
    Guagu():TriggerSkill("guagu"){
        events << Damage;

        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to->isLord()){
            int x = damage.damage;
            Room *room = player->getRoom();
            room->recover(damage.to, x*2);
            player->drawCards(x);
        }

        return false;
    }
};

DujiangCard::DujiangCard(){
    target_fixed = true;
}

void DujiangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);

    LogMessage log;
    log.type = "#InvokeSkill";
    log.from = source;
    log.arg = "dujiang";
    room->sendLog(log);

    // transfiguration
    room->setPlayerProperty(source, "general", "shenlumeng");
    room->getThread()->removeTriggerSkill("keji");
    room->acquireSkill(source, "shelie", false);

    source->setMaxHP(3);
    room->broadcastProperty(source, "maxhp");
    room->broadcastProperty(source, "hp");

    room->setTag("Dujiang", true);
}

class DujiangViewAsSkill: public ViewAsSkill{
public:
    DujiangViewAsSkill():ViewAsSkill("dujiang"){
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@dujiang-card";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 2 && to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return false;

        DujiangCard *card = new DujiangCard;
        card->addSubcards(cards);

        return card;
    }
};

class Dujiang: public PhaseChangeSkill{
public:
    Dujiang():PhaseChangeSkill("dujiang"){
        view_as_skill = new DujiangViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        if(!PhaseChangeSkill::triggerable(target))
            return false;

        return target->getGeneralName() != "shenlumeng";
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Start){
            int equip_num = 0;
            if(target->getWeapon())
                equip_num ++;

            if(target->getArmor())
                equip_num ++;

            if(target->getDefensiveHorse())
                equip_num ++;

            if(target->getOffensiveHorse())
                equip_num ++;

            if(equip_num < 2)
                return false;

            Room *room = target->getRoom();
            room->askForUseCard(target, "@dujiang-card", "@@dujiang");
        }

        return false;  
    }
};

FloodCard::FloodCard(){
    target_fixed = true;
}

void FloodCard::use(const QList<const ClientPlayer *> &targets) const{
    ClientInstance->tag.insert("flood_used", true);
}

void FloodCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    room->setTag("Flood", true);

    QList<ServerPlayer *> players = room->getOtherPlayers(source);
    foreach(ServerPlayer *player, players){
        if(player->getRoleEnum() == Player::Rebel){
            room->cardEffect(this, source, player);
        }
    }
}

void FloodCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->throwAllEquips();

    Room *room = effect.to->getRoom();
    if(!room->askForDiscard(effect.to, 2, true)){
        DamageStruct damage;
        damage.from = effect.from;
        damage.to = effect.to;

        room->damage(damage);
    }
}

class Flood: public ViewAsSkill{
public:
    Flood():ViewAsSkill("flood"){
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay() const{
        return !ClientInstance->tag.value("flood_used", false).toBool();
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 3 && !to_select->isEquipped() && to_select->getCard()->isBlack();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 3)
            return NULL;

        FloodCard *card = new FloodCard;
        card->addSubcards(cards);

        return card;
    }
};

TaichenCard::TaichenCard(){
    target_fixed = true;
}

void TaichenCard::use(const QList<const ClientPlayer *> &targets) const{
    ClientInstance->turn_tag.insert("taichen_used", true);
}

void TaichenCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->loseHp(source);

    if(source->isAlive()){
        Duel *duel = new Duel(Card::NoSuit, 0);
        duel->setCancelable(false);

        CardEffectStruct effect;
        effect.card = duel;
        effect.from = source;
        effect.to = room->getLord();

        room->cardEffect(effect);
    }
}

class Taichen: public ZeroCardViewAsSkill{
public:
    Taichen():ZeroCardViewAsSkill("taichen"){

    }

    virtual bool isEnabledAtPlay() const{
        return ! ClientInstance->turn_tag.value("taichen_used", false).toBool();
    }

    virtual const Card *viewAs() const{
        return new TaichenCard;
    }
};

class Xiansheng: public PhaseChangeSkill{
public:
    Xiansheng():PhaseChangeSkill("xiansheng"){
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getGeneralName() == "guanyu" && target->getHp() <= 2;
    }

    virtual bool onPhaseChange(ServerPlayer *guanyu) const{
        if(guanyu->getPhase() == Player::Start){
            Room *room = guanyu->getRoom();

            if(guanyu->askForSkillInvoke("xiansheng")){
                guanyu->throwAllEquips();
                guanyu->throwAllHandCards();

                room->setPlayerProperty(guanyu, "general", "shenguanyu");

                guanyu->setMaxHP(6);
                guanyu->setHp(6);

                room->broadcastProperty(guanyu, "maxhp");
                room->broadcastProperty(guanyu, "hp");

                room->drawCards(guanyu, 3);
            }
        }

        return false;
    }
};

ZhiyuanCard::ZhiyuanCard(){

}

bool ZhiyuanCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && to_select != Self && to_select->getRoleEnum() == Player::Rebel;
}

void ZhiyuanCard::use(const QList<const ClientPlayer *> &) const{
    int count = ClientInstance->turn_tag.value("zhiyuan_count", 0).toInt();
    count ++;
    ClientInstance->turn_tag.insert("zhiyuan_count", count);
}

void ZhiyuanCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    targets.first()->obtainCard(this);
}

class Zhiyuan: public OneCardViewAsSkill{
public:
    Zhiyuan():OneCardViewAsSkill("zhiyuan"){

    }

    virtual bool isEnabledAtPlay() const{
        return ClientInstance->turn_tag.value("zhiyuan_count", 0).toInt() < 2;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("BasicCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ZhiyuanCard *card = new ZhiyuanCard;
        card->addSubcard(card_item->getCard()->getId());

        return card;
    }
};

class FanchengRule: public ScenarioRule{
public:
    FanchengRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart << Death;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        switch(event){
        case GameStart:{
                if(player->isLord()){
                    room->installEquip(player, "chitu");
                    room->installEquip(player, "blade");
                    room->acquireSkill(player, "flood");
                    room->acquireSkill(player, "xiansheng");

                    ServerPlayer *pangde = room->findPlayer("pangde");
                    room->acquireSkill(pangde, "taichen");

                    ServerPlayer *huatuo = room->findPlayer("huatuo");
                    room->installEquip(huatuo, "hualiu");
                    room->acquireSkill(huatuo, "guagu");

                    ServerPlayer *lumeng = room->findPlayer("lumeng");
                    room->acquireSkill(lumeng, "dujiang");

                    ServerPlayer *caoren = room->findPlayer("caoren");
                    room->installEquip(caoren, "renwang_shield");
                    room->acquireSkill(caoren, "zhiyuan");
                }

                break;
            }

        case Death:{
                QString killer_name = data.toString();
                if(player->getGeneralName() == "pangde" &&
                   killer_name == room->getLord()->objectName())
                {
                    data = QString();
                }

                break;
            }

        default:
            break;
        }

        return false;
    }
};

FanchengScenario::FanchengScenario()
    :Scenario("fancheng")
{
    lord = "guanyu";
    loyalists << "huatuo";
    rebels << "caoren" << "pangde" << "xuhuang";
    renegades << "lumeng";

    rule = new FanchengRule(this);

    skills << new Guagu
            << new Dujiang
            << new Flood
            << new Taichen
            << new Xiansheng
            << new Skill("changqu", Skill::Compulsory)
            << new Zhiyuan;

    addMetaObject<DujiangCard>();
    addMetaObject<FloodCard>();
    addMetaObject<TaichenCard>();
    addMetaObject<ZhiyuanCard>();

    t["fancheng"] = tr("fancheng");

    t["guagu"] = tr("guagu");
    t["dujiang"] = tr("dujiang");
    t["flood"] = tr("flood");
    t["taichen"] = tr("taichen");
    t["changqu"] = tr("changqu");
    t["xiansheng"] = tr("xiansheng");
    t["zhiyuan"] = tr("zhiyuan");

    t[":guagu"] = tr(":guagu");
    t[":dujiang"] = tr(":dujiang");
    t[":flood"] = tr(":flood");
    t[":taichen"] = tr(":taichen");
    t[":changqu"] = tr(":changqu");
    t[":xiansheng"] = tr(":xiansheng");
    t[":zhiyuan"] = tr(":zhiyuan");

    t["@@dujiang"] = tr("@@dujiang");
}

void FanchengScenario::onTagSet(Room *room, const QString &key) const{
    if(key == "Flood"){
        ServerPlayer *xuhuang = room->findPlayer("xuhuang");
        if(xuhuang)
            room->acquireSkill(xuhuang, "changqu");

        ServerPlayer *caoren = room->findPlayer("caoren");
        if(caoren)
            room->setPlayerProperty(caoren, "xueyi", -1);
    }else if(key == "Dujiang"){
        ServerPlayer *caoren = room->findPlayer("caoren");
        if(caoren)
            room->setPlayerProperty(caoren, "xueyi", 0);
    }
}

ADD_SCENARIO(Fancheng);
