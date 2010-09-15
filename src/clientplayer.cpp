#include "clientplayer.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "standard.h"

ClientPlayer *Self = NULL;

ClientPlayer::ClientPlayer(Client *client)
    :Player(client), handcard_num(0)
{
}

void ClientPlayer::handCardChange(int delta){
    handcard_num += delta;
}

int ClientPlayer::aliveCount() const{
    return ClientInstance->alivePlayerCount();
}

int ClientPlayer::getHandcardNum() const{
    return handcard_num;
}

void ClientPlayer::addCard(const Card *card, Place place){
    switch(place){
    case Hand: {
            if(card)
                known_cards << card;
            handcard_num++;
            break;
        }
    case Equip: {
            const EquipCard *equip = qobject_cast<const EquipCard*>(card);
            setEquip(equip);
            break;
        }
    case Judging:{
            addDelayedTrick(card);
            break;
        }
    default:
        // FIXME
        ;
    }
}

void ClientPlayer::addKnownHandCard(const Card *card){
    if(!known_cards.contains(card))
        known_cards << card;
}

void ClientPlayer::removeCard(const Card *card, Place place){
    switch(place){
    case Hand: {
            handcard_num--;
            if(card)
                known_cards.removeOne(card);
            break;
        }
    case Equip:{
            const EquipCard *equip = qobject_cast<const EquipCard*>(card);
            removeEquip(equip);
            break;
        }
    case Judging:{
            removeDelayedTrick(card);
            break;
        }

    default:
        // FIXME
        ;
    }
}

QList<const Card *> ClientPlayer::getCards() const{
    return known_cards;
}

QList<int> ClientPlayer::nullifications() const{
    QList<int> card_ids;

    if(Self->hasSkill("kanpo")){
        foreach(const Card *card, known_cards){
            if(card->isBlack())
                card_ids << card->getId();
        }
    }else{
        foreach(const Card *card, known_cards){
            if(card->objectName() == "nullification")
                card_ids << card->getId();
        }
    }

    return card_ids;
}

void ClientPlayer::MoveCard(const CardMoveStructForClient &move){
    if(move.card_id == -1)
        return;

    const Card *card = Sanguosha->getCard(move.card_id);
    if(move.from)
        move.from->removeCard(card, move.from_place);
    else{
        Q_ASSERT(card != NULL);
        ClientInstance->discarded_list.removeOne(card);
    }

    if(move.to)
        move.to->addCard(card, move.to_place);
    else{
        Q_ASSERT(card != NULL);
        ClientInstance->discarded_list.prepend(card);
    }
}
