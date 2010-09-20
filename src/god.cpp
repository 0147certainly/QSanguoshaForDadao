#include "god.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"

GongxinCard::GongxinCard(){

}

bool GongxinCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

class Wuhun: public TriggerSkill{
public:
    Wuhun():TriggerSkill("wuhun"){
        events << Death;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        // FIXME:

        return false;
    }
};

static bool CompareBySuit(int card1, int card2){
    const Card *c1 = Sanguosha->getCard(card1);
    const Card *c2 = Sanguosha->getCard(card2);

    int a = static_cast<int>(c1->getSuit());
    int b = static_cast<int>(c2->getSuit());

    return a < b;
}

class Shelie: public PhaseChangeSkill{
public:
    Shelie():PhaseChangeSkill("shelie"){

    }

    virtual bool onPhaseChange(ServerPlayer *shenlumeng) const{
        if(shenlumeng->getPhase() != Player::Draw)
            return false;

        Room *room = shenlumeng->getRoom();
        if(!room->askForSkillInvoke(shenlumeng, objectName()))
            return false;

        QList<int> card_ids = room->getNCards(5);
        qSort(card_ids.begin(), card_ids.end(), CompareBySuit);
        QStringList card_str;
        foreach(int card_id, card_ids)
            card_str << QString::number(card_id);
        room->broadcastInvoke("fillAG", card_str.join("+"));

        while(!card_ids.isEmpty()){
            int card_id = room->askForAG(shenlumeng);
            room->takeAG(shenlumeng, card_id);

            const Card *card = Sanguosha->getCard(card_id);

            // quick-and-dirty
            shenlumeng->addCard(card, Player::Hand);
            room->setCardMapping(card_id, shenlumeng, Player::Hand);

            Card::Suit suit = card->getSuit();
            card_ids.removeOne(card_id);
            QMutableListIterator<int> itor(card_ids);
            while(itor.hasNext()){
                const Card *c = Sanguosha->getCard(itor.next());
                if(c->getSuit() == suit){
                    itor.remove();

                    room->setCardMapping(c->getId(), NULL, Player::DiscardedPile);
                    room->takeAG(NULL, c->getId());
                }
            }
        }

        room->broadcastInvoke("clearAG");

        return true;
    }
};

class Gongxin: ZeroCardViewAsSkill{
public:
    Gongxin():ZeroCardViewAsSkill("gongxin"){

    }

    virtual const Card *viewAs() const{
        return new GongxinCard;
    }
};

GreatYeyanCard::GreatYeyanCard(){

}

class GreatYeyan: public ViewAsSkill{
public:
    GreatYeyan(): ViewAsSkill("great_yeyan"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 4)
            return false;

        foreach(CardItem *item, selected){
            if(to_select->getCard()->getSuit() == item->getCard()->getSuit())
                return false;
        }

        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 4)
            return NULL;

        GreatYeyanCard *card = new GreatYeyanCard;
        card->addSubcards(cards);
    }
};

class Qinyin: public TriggerSkill{
public:
    Qinyin():TriggerSkill("qinyin"){
        events << CardDiscarded;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        int discard_num = data.toInt();

        if(player->getPhase() == Player::Discard && discard_num >= 2){
            Room *room = player->getRoom();
            QString result = room->askForChoice(player, objectName(), "up+down+no");
            if(result == "no"){
                return false;
            }

            QList<ServerPlayer *> all_players = room->getAllPlayers();
            if(result == "up"){
                foreach(ServerPlayer *player, all_players){
                    room->recover(player);
                }
            }else if(result == "down"){
                foreach(ServerPlayer *player, all_players){
                    room->loseHp(player);
                }
            }
        }

        return false;
    }
};

class Guixin: public MasochismSkill{
public:
    Guixin():MasochismSkill("guixin"){

    }

    virtual void onDamaged(ServerPlayer *shencc, const DamageStruct &damage) const{
        Room *room = shencc->getRoom();
        int i, x = damage.damage;
        for(i=0; i<x; i++){
            if(room->askForSkillInvoke(shencc, objectName())){
                QList<ServerPlayer *> players = room->getOtherPlayers(shencc);
                foreach(ServerPlayer *player, players){
                    if(!player->isAllNude()){
                        int card_id = room->askForCardChosen(shencc, player, "hej", objectName());
                        room->obtainCard(shencc, card_id);
                    }
                }

                shencc->turnOver();
                room->broadcastProperty(shencc, "faceup");
            }else
                break;
        }
    }
};

class Feiying: public GameStartSkill{
public:
    Feiying():GameStartSkill("feiying"){

    }

    virtual void onGameStart(ServerPlayer *player) const{
        player->getRoom()->setPlayerCorrect(player, "skill_dest", +1);
    }
};

class Baonu: public TriggerSkill{
public:
    Baonu():TriggerSkill("baonu"){
        events << GameStart<< Damage << Damaged;
    }

    virtual int getPriority(ServerPlayer *target) const{
        return -1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == GameStart){
            player->setMark("anger", 2);
        }else{
            if(player->isAlive()){
                DamageStruct damage = data.value<DamageStruct>();
                int value = player->getMark("anger");
                value += damage.damage;
                player->getRoom()->setPlayerMark(player, "anger", value);
            }
        }

        return false;
    }
};

class Wumo: public TriggerSkill{
public:
    Wumo():TriggerSkill("wumo"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("TrickCard") && !use.card->inherits("DelayedTrick")){
            Room *room = player->getRoom();
            room->loseHp(player);
        }

        return false;
    }
};

class Shenfen:public ZeroCardViewAsSkill{
public:
    Shenfen():ZeroCardViewAsSkill("shenfen"){
    }

    virtual bool isEnabledAtPlay() const{
        return Self->getMark("baonu") >= 6;
    }

    virtual const Card *viewAs() const{
        return new ShenfenCard;
    }
};

ShenfenCard::ShenfenCard(){
    target_fixed = true;
}

void ShenfenCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    source->turnOver();
    room->broadcastProperty(source, "faceup");

    int value = source->getMark("baonu");
    value -= 6;
    room->setPlayerMark(source, "baonu", value);

    QList<ServerPlayer *> players = room->getOtherPlayers(source);

    foreach(ServerPlayer *player, players){
        DamageStruct damage;
        damage.card = this;
        damage.damage = 1;
        damage.from = source;
        damage.to = player;
        damage.nature = DamageStruct::Normal;

        room->damage(damage);
    }

    foreach(ServerPlayer *player, players){
        player->throwAllEquips();
    }

    foreach(ServerPlayer *player, players){
        int discard_num = qMin(player->getHandcardNum(), 4);
        room->askForDiscard(player, discard_num);
    }
}

GodPackage::GodPackage()
    :Package("god")
{
    t["god"] = tr("god");

    General *shenguanyu, *shenlumeng;

    shenguanyu = new General(this, "shenguanyu", "shu", 5);
    shenguanyu->addSkill(new Wuhun);

    shenlumeng = new General(this, "shenlumeng", "wu", 3);
    shenlumeng->addSkill(new Shelie);

    t["shenguanyu"] = tr("shenguanyu");
    t["shenlumeng"] = tr("shenlumeng");

    t["wuhun"] = tr("wuhun");
    t["shelie"] = tr("shelie");
    t["gongxin"] = tr("gongxin");

    t[":wuhun"] = tr(":wuhun");
    t[":shelie"] = tr(":shelie");
    t[":gongxin"] = tr(":gongxin");

    General *shenzhouyu, *shenzhugeliang;

    shenzhouyu = new General(this, "shenzhouyu", "wu");
    shenzhouyu->addSkill(new Qinyin);

    shenzhugeliang = new General(this, "shenzhugeliang", "shu", 3);

    t["shenzhouyu"] = tr("shenzhouyu");
    t["shenzhugeliang"] = tr("shenzhugeliang");

    t[":qinyin"] = tr(":qinyin");
    t[":yeyan"] = tr(":yeyan");
    t[":qixing"] = tr(":qixing");
    t[":kuangfeng"] = tr(":kuangfeng");
    t[":dawu"] = tr(":dawu");

    t[":qinyin:"] = tr(":qinyin:");
    t["qinyin:up"] = tr("qinyin:up");
    t["qinyin:down"] = tr("qinyin:down");
    t["qinyin:no"] = tr("qinyin:no");

    General *shencaocao, *shenlubu;

    shencaocao = new General(this, "shencaocao$", "wei", 3);
    shencaocao->addSkill(new Guixin);
    shencaocao->addSkill(new Feiying);

    shenlubu = new General(this, "shenlubu", "qun", 5);
    shenlubu->addSkill(new Baonu);
    shenlubu->addSkill(new Shenfen);

    t["shencaocao"] = tr("shencaocao");
    t["shenlubu"] = tr("shenlubu");

    t["guixin"] = tr("guixin");
    t["feiying"] = tr("feiying");
    t["baonu"] = tr("baonu");
    t["wumou"] = tr("wumou");
    t["wuqian"] = tr("wuqian");
    t["shenfen"] = tr("shenfen");

    t[":guixin"] = tr(":guixin");
    t[":feiying"] = tr(":feiying");
    t[":baonu"] = tr(":baonu");
    t[":wumou"] = tr(":wumou");
    t[":wuqian"] = tr(":wuqian");
    t[":shenfen"] = tr(":shenfen");

    addMetaObject<GongxinCard>();
    addMetaObject<GreatYeyanCard>();
    addMetaObject<ShenfenCard>();
}

ADD_PACKAGE(God)
