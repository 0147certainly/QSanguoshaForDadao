#include "standard.h"
#include "skill.h"
#include "wind.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"

// skill cards

GuidaoCard::GuidaoCard(){
    target_fixed = true;
}

void GuidaoCard::use(Room *room, ServerPlayer *zhangjiao, const QList<ServerPlayer *> &targets) const{
    room->obtainCard(zhangjiao, room->throwSpecialCard());

    int card_id = subcards.first();
    room->moveCardTo(card_id, NULL, Player::Special, true);
    room->setEmotion(zhangjiao, Room::Normal);
}

LeijiCard::LeijiCard(){

}

bool LeijiCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return targets.isEmpty();
}

void LeijiCard::use(Room *room, ServerPlayer *zhangjiao, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    room->setEmotion(zhangjiao, Room::Normal);
    room->setEmotion(target, Room::Bad);

    const Card *card = room->getJudgeCard(target);
    if(card->getSuit() == Card::Spade){
        DamageStruct damage;
        damage.card = this;
        damage.damage = 2;
        damage.from = zhangjiao;
        damage.to = target;
        damage.nature = DamageStruct::Thunder;

        room->damage(damage);
    }
}

HuangtianCard::HuangtianCard(){
    once = true;
}

void HuangtianCard::use(Room *room, ServerPlayer *, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *zhangjiao = targets.first();
    if(zhangjiao->hasSkill("huangtian")){
        zhangjiao->obtainCard(this);
        room->setEmotion(zhangjiao, Room::Good);
    }
}

bool HuangtianCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return to_select->hasSkill("huangtian");
}

class Guidao:public ViewAsSkill{
public:
    Guidao():ViewAsSkill("guidao"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@guidao";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && to_select->getFilteredCard()->isBlack();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        GuidaoCard *card = new GuidaoCard;
        card->addSubcard(cards.first()->getFilteredCard());

        return card;
    }
};

class HuangtianViewAsSkill: public OneCardViewAsSkill{
public:
    HuangtianViewAsSkill():OneCardViewAsSkill("huangtianv"){

    }

    virtual bool isEnabledAtPlay() const{
        return !ClientInstance->hasUsed("HuangtianCard") && Self->getKingdom() == "qun";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getCard();
        return card->objectName() == "jink" || card->objectName() == "lightning";
    }


    virtual const Card *viewAs(CardItem *card_item) const{
        HuangtianCard *card = new HuangtianCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Huangtian: public GameStartSkill{
public:
    Huangtian():GameStartSkill("huangtian$"){

    }

    virtual void onGameStart(ServerPlayer *zhangjiao) const{
        Room *room = zhangjiao->getRoom();
        QList<ServerPlayer *> players = room->getOtherPlayers(zhangjiao);
        foreach(ServerPlayer *player, players){
            room->attachSkillToPlayer(player, "huangtianv");
        }
    }
};

class LeijiViewAsSkill: public ZeroCardViewAsSkill{
public:
    LeijiViewAsSkill():ZeroCardViewAsSkill("leiji"){

    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern == "@@leiji";
    }

    virtual const Card *viewAs() const{
        return new LeijiCard;
    }
};

class Leiji: public TriggerSkill{
public:
    Leiji():TriggerSkill("leiji"){
        events << CardResponsed;
        view_as_skill = new LeijiViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhangjiao, QVariant &data) const{
        CardStar card_star = data.value<CardStar>();
        if(!card_star->inherits("Jink"))
            return false;

        Room *room = zhangjiao->getRoom();
        room->askForUseCard(zhangjiao, "@@leiji", "@leiji");

        return false;
    }
};

ShensuCard::ShensuCard(){
}

bool ShensuCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;

    return true;
}

void ShensuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    room->cardEffect(this, source, targets.first());
}

void ShensuCard::onEffect(const CardEffectStruct &card_effect) const{
    SlashEffectStruct effect;
    effect.slash = new Slash(Card::NoSuit, 0);
    effect.from = card_effect.from;
    effect.to = card_effect.to;
    effect.nature = DamageStruct::Normal;

    card_effect.from->getRoom()->slashEffect(effect);
}

class ShensuViewAsSkill: public ViewAsSkill{
public:
    ShensuViewAsSkill():ViewAsSkill("shensu"){
    }

    virtual bool isEnabledAtPlay() const{
        return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(ClientInstance->card_pattern.endsWith("1"))
            return false;
        else
            return selected.isEmpty() && to_select->getCard()->inherits("EquipCard");
    }

    virtual bool isEnabledAtResponse() const{
        return ClientInstance->card_pattern.startsWith("@@shensu");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(ClientInstance->card_pattern.endsWith("1")){
            if(cards.isEmpty())
                return new ShensuCard;
            else
                return NULL;
        }else{
            if(cards.length() != 1)
                return NULL;

            ShensuCard *card = new ShensuCard;
            card->addSubcards(cards);

            return card;
        }
    }
};

class Shensu: public PhaseChangeSkill{
public:
    Shensu():PhaseChangeSkill("shensu"){
        view_as_skill = new ShensuViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *xiahouyuan) const{
        Room *room = xiahouyuan->getRoom();

        if(xiahouyuan->getPhase() == Player::Start){
            if(room->askForUseCard(xiahouyuan, "@@shensu1", "@shensu1")){
                room->skip(Player::Judge);
                room->skip(Player::Draw);
            }
        }else if(xiahouyuan->getPhase() == Player::Play){
            if(room->askForUseCard(xiahouyuan, "@@shensu2", "@shensu2")){
                return true;
            }
        }

        return false;
    }
};

class Jushou: public PhaseChangeSkill{
public:
    Jushou():PhaseChangeSkill("jushou"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Finish){
            Room *room = target->getRoom();
            if(room->askForSkillInvoke(target, objectName())){
                target->drawCards(3);
                target->turnOver();

                room->broadcastProperty(target, "faceup");
                room->playSkillEffect(objectName());
            }
        }

        return false;
    }
};

class Liegong: public SlashBuffSkill{
public:
    Liegong():SlashBuffSkill("liegong"){

    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *huangzhong = effect.from;
        Room *room = huangzhong->getRoom();
        if(room->getCurrent() != huangzhong)
            return false;

        int num = effect.to->getHandcardNum();
        if(num >= huangzhong->getHp() || num <= huangzhong->getAttackRange()){
            if(room->askForSkillInvoke(huangzhong, "liegong")){
                room->playSkillEffect(objectName());
                room->slashResult(effect, true);

                return true;
            }
        }

        return false;
    }
};

class Kuanggu: public TriggerSkill{
public:
    Kuanggu():TriggerSkill("kuanggu"){
        frequency = Compulsory;
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        if(data.canConvert<DamageStruct>()){
            DamageStruct damage = data.value<DamageStruct>();

            if(player->distanceTo(damage.to) <= 1){
                Room *room = player->getRoom();
                room->recover(player, damage.damage);
            }
        }

        return false;
    }
};

class Buqu: public TriggerSkill{
public:
    Buqu():TriggerSkill("buqu"){
        events << Dying << HpRecover;
    }

    bool HasDuplicated(ServerPlayer *zhoutai) const{
        const QList<int> &buqu = zhoutai->getPile("buqu");
        return buqu.toSet().size() < buqu.size();
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhoutai, QVariant &data) const{
        Room *room = zhoutai->getRoom();
        if(event == Dying){
            DyingStruct dying = data.value<DyingStruct>();
            if(room->askForSkillInvoke(zhoutai, objectName(), data)){
                room->playSkillEffect(objectName());

                return true;
            }else
                return false;
        }else if(event == HpRecover){
            if(zhoutai->getHp() > 0)
                return false;
        }

        return false;
    }
};

WindPackage::WindPackage()
    :Package("wind")
{
    // xiaoqiao, zhoutai and yuji is omitted
    General *xiahouyuan, *caoren, *huangzhong, *weiyan, *zhangjiao; // *zhoutai;

    xiahouyuan = new General(this, "xiahouyuan", "wei");
    xiahouyuan->addSkill(new Shensu);

    caoren = new General(this, "caoren", "wei");
    caoren->addSkill(new Jushou);

    huangzhong = new General(this, "huangzhong", "shu");
    huangzhong->addSkill(new Liegong);

    weiyan = new General(this, "weiyan", "shu");
    weiyan->addSkill(new Kuanggu);

    zhangjiao = new General(this, "zhangjiao$", "qun", 3);
    zhangjiao->addSkill(new Guidao);
    zhangjiao->addSkill(new Leiji);
    zhangjiao->addSkill(new Huangtian);

    //zhoutai = new General(this, "zhoutai", "wu");
    //zhoutai->addSkill(new Buqu);

    addMetaObject<GuidaoCard>();
    addMetaObject<HuangtianCard>();
    addMetaObject<LeijiCard>();
    addMetaObject<ShensuCard>();

    skills << new HuangtianViewAsSkill;
}

ADD_PACKAGE(Wind)


