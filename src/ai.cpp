#include "ai.h"
#include "serverplayer.h"
#include "engine.h"
#include "standard.h"
#include "settings.h"
#include "maneuvering.h"
#include "lua.hpp"

AI::AI(ServerPlayer *player)
    :self(player)
{
    room = player->getRoom();
}

typedef QPair<QString, QString> RolePair;

struct RoleMapping: public QMap<RolePair, AI::Relation> {
    void set(const QString &role1, const QString &role2, AI::Relation relation, bool reverse = false){
        insert(qMakePair(role1, role2), relation);
        if(reverse)
            insert(qMakePair(role2, role1), relation);
    }

    AI::Relation get(const QString &role1, const QString &role2){
        return value(qMakePair(role1, role2), AI::Neutrality);
    }
};

AI::Relation AI::relationTo(const ServerPlayer *other) const{
    if(self == other)
        return Friend;

    RoleMapping map, map_good, map_bad;
    if(map.isEmpty()){
        map.set("lord", "lord", Friend);
        map.set("lord", "rebel", Enemy);
        map.set("lord", "loyalist", Friend);
        map.set("lord", "renegade", Neutrality);

        map.set("loyalist", "loyalist", Friend);
        map.set("loyalist", "lord", Friend);
        map.set("loyalist", "rebel", Enemy);
        map.set("loyalist", "renegade", Neutrality);

        map.set("rebel", "rebel", Friend);
        map.set("rebel", "lord", Enemy);
        map.set("rebel", "loyalist", Enemy);
        map.set("rebel", "renegade", Neutrality);

        map.set("renegade", "lord", Friend);
        map.set("renegade", "loyalist", Neutrality);
        map.set("renegade", "rebel", Neutrality);
        map.set("renegade", "renegade", Neutrality);

        map_good = map;
        map_good.set("renegade", "loyalist", Enemy, true);
        map_good.set("renegade", "rebel", Friend, true);

        map_bad = map;
        map_bad.set("renegade", "loyalist", Friend, true);
        map_bad.set("renegade", "rebel", Enemy, true);
    }

    if(room->alivePlayerCount() == 2){
        return Enemy;
    }

    QString process = room->getTag("GameProcess").toString();
    if(process == "Balance")
        return map.get(self->getRole(), other->getRole());
    else if(process == "LordSuperior")
        return map_good.get(self->getRole(), other->getRole());
    else
        return map_bad.get(self->getRole(), other->getRole());
}

bool AI::isFriend(const ServerPlayer *other) const{
    return relationTo(other) == Friend;
}

bool AI::isEnemy(const ServerPlayer *other) const{
    return relationTo(other) == Enemy;
}

QList<ServerPlayer *> AI::getEnemies() const{
    QList<ServerPlayer *> players = room->getOtherPlayers(self);
    QList<ServerPlayer *> enemies;
    foreach(ServerPlayer *p, players){
        if(isEnemy(p))
            enemies << p;
    }

    return enemies;
}

QList<ServerPlayer *> AI::getFriends() const{
    QList<ServerPlayer *> players = room->getOtherPlayers(self);
    QList<ServerPlayer *> friends;
    foreach(ServerPlayer *p, players){
        if(isFriend(p))
            friends << p;
    }

    return friends;
}

void AI::filterEvent(TriggerEvent event, ServerPlayer *player, const QVariant &data){
    // dummy
}

TrustAI::TrustAI(ServerPlayer *player)
    :AI(player)
{
}

void TrustAI::activate(CardUseStruct &card_use){
    QList<const Card *> cards = self->getHandcards();
    foreach(const Card *card, cards){
        if(card->targetFixed()){
            if(useCard(card)){
                card_use.card = card;
                card_use.from = self;

                return;
            }
        }
    }
}

bool TrustAI::useCard(const Card *card){
    if(card->inherits("Peach"))
        return self->isWounded();
    else if(card->inherits("EquipCard")){
        const EquipCard *equip = qobject_cast<const EquipCard *>(card);
        switch(equip->location()){
        case EquipCard::WeaponLocation:{
                const Weapon *weapon = self->getWeapon();
                if(weapon == NULL)
                    return true;

                const Weapon *new_weapon = qobject_cast<const Weapon *>(equip);
                return new_weapon->getRange() > weapon->getRange();
            }
        case EquipCard::ArmorLocation: return !self->getArmor();
        case EquipCard::OffensiveHorseLocation: return !self->getOffensiveHorse();
        case EquipCard::DefensiveHorseLocation: return !self->getDefensiveHorse();
        default:
            return true;
        }

    }else if(card->inherits("TrickCard"))
        return true;
    else
        return false;
}

Card::Suit TrustAI::askForSuit(){
    return Card::AllSuits[qrand() % 4];
}

QString TrustAI::askForKingdom(){
    QString role;
    switch(self->getRoleEnum()){
    case Player::Lord:
    case Player::Rebel: role = "wei"; break;
    case Player::Loyalist:
    case Player::Renegade:
        role = self->getRoom()->getLord()->getRole(); break;
    }

    return role;
}

bool TrustAI::askForSkillInvoke(const QString &skill_name, const QVariant &data){
    return false;
}

QString TrustAI::askForChoice(const QString &skill_name, const QString &choice){
    const Skill *skill = Sanguosha->getSkill(skill_name);
    if(skill)
        return skill->getDefaultChoice(self);
    else{
        QStringList choices = choice.split("+");
        return choices.at(qrand() % choices.length());
    }
}

QList<int> TrustAI::askForDiscard(const QString &reason, int discard_num, bool optional, bool include_equip){
    QList<int> to_discard;

    if(optional)
        return to_discard;
    else
        return self->forceToDiscard(discard_num, include_equip);
}

const Card *TrustAI::askForNullification(const QString &trick_name, ServerPlayer *from, ServerPlayer *to) {
    const TrickCard *card = Sanguosha->findChild<const TrickCard *>(trick_name);
    if(to == self && card->isAggressive()){
        QList<const Card *> cards = self->getHandcards();

        foreach(const Card *card, cards){
            if(card->objectName() == "nullification")
                return card;
        }

        if(self->hasSkill("kanpo")){
            foreach(const Card *card, cards){
                if(card->isBlack()){
                    Nullification *ncard = new Nullification(card->getSuit(), card->getNumber());
                    ncard->addSubcard(card);
                    ncard->setSkillName("kanpo");

                    return ncard;
                }
            }
        }
    }

    return NULL;
}

int TrustAI::askForCardChosen(ServerPlayer *who, const QString &flags, const QString &) {
    QList<const Card *> cards = who->getCards(flags);
    int r = qrand() % cards.length();
    return cards.at(r)->getId();
}

const Card *TrustAI::askForCard(const QString &pattern, const QString &prompt){
    static QRegExp id_rx("\\d+");

    if(pattern.contains("+")){
        QStringList subpatterns = pattern.split("+");
        foreach(QString subpattern, subpatterns){
            const Card *result = askForCard(subpattern, prompt);
            if(result)
                return result;
        }
    }

    QList<const Card *> cards = self->getHandcards();
    if(id_rx.exactMatch(pattern)){
        int card_id = pattern.toInt();
        foreach(const Card *card, cards)
            if(card->getId() == card_id)
                return card;
    }else if(pattern.startsWith(".")){
        if(pattern == ".")
            return cards.isEmpty() ? NULL : cards.first();

        QChar end = pattern.at(1).toLower();
        foreach(const Card *card, cards){
            if(card->getSuitString().startsWith(end, Qt::CaseInsensitive))
                return card;
        }
    }else{
        foreach(const Card *card, cards)
            if(card->match(pattern))
                return card;
    }

    return NULL;
}

QString TrustAI::askForUseCard(const QString &, const QString &) {
    return ".";
}

int TrustAI::askForAG(const QList<int> &card_ids, bool refsuable){
    if(refsuable)
        return -1;

    int r = qrand() % card_ids.length();
    return card_ids.at(r);
}

const Card *TrustAI::askForCardShow(ServerPlayer *) {
    return self->getRandomHandCard();
}

const Card *TrustAI::askForPindian() {
    QList<const Card *> cards = self->getHandcards();
    const Card *highest = cards.first();
    foreach(const Card *card, cards){
        if(card->getNumber() > highest->getNumber())
            highest = card;
    }

    return highest;
}

ServerPlayer *TrustAI::askForPlayerChosen(const QList<ServerPlayer *> &targets, const QString &reason){
    int r = qrand() % targets.length();
    return targets.at(r);
}

const Card *TrustAI::askForSinglePeach(ServerPlayer *dying) {
    if(isFriend(dying)){
        QList<const Card *> cards = self->getHandcards();
        foreach(const Card *card, cards){
            if(card->inherits("Peach"))
                return card;

            if(card->inherits("Analeptic") && dying == self)
                return card;
        }

        if(dying == self){
            if(self->hasSkill("jiuchi")){
                foreach(const Card *card, cards){
                    if(card->getSuit() == Card::Spade){
                        Analeptic *analeptic = new Analeptic(Card::Spade, card->getNumber());
                        analeptic->addSubcard(card);
                        analeptic->setSkillName("jiuchi");
                        return analeptic;
                    }
                }
            }

            if(self->hasSkill("jiushi") && self->faceUp()){
                Analeptic *analeptic = new Analeptic(Card::NoSuit, 0);
                analeptic->setSkillName("jiushi");
                return analeptic;
            }
        }

        if(self->hasSkill("jijiu") && self->getPhase() == Player::NotActive){
            cards = self->getCards("he");
            foreach(const Card *card, cards){
                if(card->isRed()){
                    Peach *peach = new Peach(card->getSuit(), card->getNumber());
                    peach->addSubcard(card);
                    peach->setSkillName("jijiu");
                    return peach;
                }
            }
        }
    }

    return NULL;
}

ServerPlayer *TrustAI::askForYiji(const QList<int> &, int &){
    return NULL;
}

LuaAI::LuaAI(ServerPlayer *player)
    :TrustAI(player), callback(0)
{

}

const Card *LuaAI::askForCardShow(ServerPlayer *requestor) {
    QList<const Card *> cards = requestor->getHandcards();
    Card::Suit lack = Card::NoSuit;
    int i;
    for(i=0; i<4; i++){
        Card::Suit suit = Card::AllSuits[i];
        bool found = false;
        foreach(const Card *card, cards){
            if(card->getSuit() == suit){
                found = true;
                break;
            }
        }

        if(!found){
            lack = suit;
            break;
        }
    }

    cards = self->getHandcards();
    if(lack != Card::NoSuit){
        foreach(const Card *card, cards){
            if(card->getSuit() == lack)
                return card;
        }
    }

    return TrustAI::askForCardShow(requestor);
}

QString LuaAI::askForUseCard(const QString &pattern, const QString &prompt){
    if(callback == 0)
        return TrustAI::askForUseCard(pattern, prompt);

    lua_State *L = room->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);

    lua_pushstring(L, __func__);

    lua_pushstring(L, pattern.toAscii());

    lua_pushstring(L, prompt.toAscii());

    int error = lua_pcall(L, 3, 1, 0);
    const char *result = lua_tostring(L, -1);
    lua_pop(L, 1);

    if(error){
        const char *error_msg = result;
        room->output(error_msg);
        return ".";
    }

    return result;
}

QList<int> LuaAI::askForDiscard(const QString &reason, int discard_num, bool optional, bool include_equip){
    if(callback == 0)
        return TrustAI::askForDiscard(reason, discard_num, optional, include_equip);

    lua_State *L = room->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);

    lua_pushstring(L, __func__);

    lua_pushstring(L, reason.toAscii());

    lua_pushinteger(L, discard_num);

    lua_pushboolean(L, optional);

    lua_pushboolean(L, include_equip);

    int error = lua_pcall(L, 5, 1, 0);
    if(error){
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        room->output(error_msg);
        return TrustAI::askForDiscard(reason, discard_num, optional, include_equip);
    }

    QList<int> result;

    if(lua_istable(L, -1)){
        size_t len = lua_objlen(L, -1);
        size_t i;
        for(i=0; i<len; i++){
            lua_rawgeti(L, -1, i+1);
            result << lua_tointeger(L, -1);
            lua_pop(L, 1);
        }
    }else
        result = TrustAI::askForDiscard(reason, discard_num, optional, include_equip);

    lua_pop(L, 1);

    return result;
}

QString LuaAI::askForChoice(const QString &skill_name, const QString &choices){
    Q_ASSERT(callback);

    lua_State *L = room->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);

    lua_pushstring(L, __func__);

    lua_pushstring(L, skill_name.toAscii());

    lua_pushstring(L, choices.toAscii());

    int error = lua_pcall(L, 3, 1, 0);
    const char *result = lua_tostring(L, -1);
    lua_pop(L, 1);
    if(error){
        room->output(result);

        return TrustAI::askForChoice(skill_name, choices);
    }

    return result;
}

const Card *LuaAI::askForCard(const QString &pattern, const QString &prompt){
    Q_ASSERT(callback);

    lua_State *L = room->getLuaState();

    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);

    lua_pushstring(L, __func__);

    lua_pushstring(L, pattern.toAscii());

    lua_pushstring(L, prompt.toAscii());

    int error = lua_pcall(L, 3, 1, 0);
    const char *result = lua_tostring(L, -1);
    lua_pop(L, 1);
    if(error){
        room->output(result);
        return TrustAI::askForCard(pattern, prompt);
    }

    if(result == NULL)
        return TrustAI::askForCard(pattern, prompt);

    return Card::Parse(result);
}
