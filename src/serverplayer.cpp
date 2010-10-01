#include "serverplayer.h"
#include "skill.h"
#include "engine.h"
#include "standard.h"
#include "ai.h"

#include <QHostAddress>

ServerPlayer::ServerPlayer(Room *room)
    : Player(room), socket(NULL), room(room), ai(NULL)
{
}

void ServerPlayer::drawCard(const Card *card){
    handcards << card;
}

Room *ServerPlayer::getRoom() const{
    return room;
}

void ServerPlayer::playCardEffect(const Card *card){
    if(card->isVirtualCard())
        room->playSkillEffect(card->getSkillName());
    else
        room->playCardEffect(card->objectName(), getGeneral()->isMale());
}

int ServerPlayer::getRandomHandCard() const{
    int index = qrand() % handcards.length();
    return handcards.at(index)->getId();
}

void ServerPlayer::leaveTo(ServerPlayer *legatee){
    if(legatee){
        legatee->obtainCard(getWeapon());
        legatee->obtainCard(getArmor());
        legatee->obtainCard(getDefensiveHorse());
        legatee->obtainCard(getOffensiveHorse());

        foreach(const Card *card, handcards)
            legatee->obtainCard(card);

        QStack<const Card *> tricks = getJudgingArea();
        foreach(const Card *trick, tricks)
            room->throwCard(trick);
    }else{
        throwAllCards();
    }

    QStack<const Card *> tricks = getJudgingArea();
    foreach(const Card *trick, tricks)
        room->throwCard(trick);
}

void ServerPlayer::obtainCard(const Card *card){
    room->obtainCard(this, card);
}

void ServerPlayer::throwAllEquips(){
    room->throwCard(getWeapon());
    room->throwCard(getArmor());
    room->throwCard(getDefensiveHorse());
    room->throwCard(getOffensiveHorse());
}

void ServerPlayer::throwAllHandCards(){
    foreach(const Card *card, handcards)
        room->throwCard(card);
}

void ServerPlayer::throwAllCards(){
    throwAllEquips();
    throwAllHandCards();

    QStack<const Card *> tricks = getJudgingArea();
    foreach(const Card *trick, tricks)
        room->throwCard(trick);
}

void ServerPlayer::drawCards(int n){
    room->drawCards(this, n);
}

int ServerPlayer::aliveCount() const{
    return room->alivePlayerCount();
}

int ServerPlayer::getHandcardNum() const{
    return handcards.length();
}

void ServerPlayer::setSocket(QTcpSocket *socket){    
    this->socket = socket;

    if(socket){
        connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
        connect(socket, SIGNAL(readyRead()), this, SLOT(getRequest()));

        connect(this, SIGNAL(message_cast(QString)), this, SLOT(castMessage(QString)));
    }
}

void ServerPlayer::unicast(const QString &message){
    emit message_cast(message);
}

void ServerPlayer::castMessage(const QString &message){
    if(socket){
        socket->write(message.toAscii());
        socket->write("\n");

#ifndef QT_NO_DEBUG
        qDebug("%s: %s", qPrintable(objectName()), qPrintable(message));
#endif
    }
}

void ServerPlayer::invoke(const char *method, const QString &arg){
    unicast(QString("! %1 %2").arg(method).arg(arg));
}

QString ServerPlayer::reportHeader() const{
    QString name = objectName();
    return QString("%1 ").arg(name.isEmpty() ? tr("Anonymous") : name);
}

void ServerPlayer::sendProperty(const char *property_name){
    QString value = property(property_name).toString();
    unicast(QString(". %1 %2").arg(property_name).arg(value));
}

void ServerPlayer::getRequest(){
    while(socket->canReadLine()){
        QString request = socket->readLine(1024);
        request.chop(1); // remove the ending '\n' character
        emit request_got(request);
    }
}

void ServerPlayer::removeCard(const Card *card, Place place){
    switch(place){
    case Hand: handcards.removeOne(card); break;
    case Equip: {
            const EquipCard *equip = qobject_cast<const EquipCard *>(card);
            removeEquip(equip);

            LogMessage log;
            log.type = "$Uninstall";
            log.card_str = card->toString();
            log.from = this;
            room->sendLog(log);

            equip->onUninstall(this);
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

void ServerPlayer::addCard(const Card *card, Place place){
    switch(place){
    case Hand: handcards << card; break;
    case Equip: {
            const EquipCard *equip = qobject_cast<const EquipCard *>(card);
            setEquip(equip);
            equip->onInstall(this);
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

QList<int> ServerPlayer::handCards() const{
    QList<int> card_ids;
    foreach(const Card *card, handcards)
        card_ids << card->getId();

    return card_ids;
}

QList<const Card *> ServerPlayer::getHandcards() const{
    return handcards;
}

QList<const Card *> ServerPlayer::getCards(const QString &flags) const{
    QList<const Card *> cards;
    if(flags.contains("h"))
        cards << handcards;

    cards << getEquips();

    if(flags.contains("j")){
        QStack<const Card *> tricks = getJudgingArea();
        foreach(const Card *trick, tricks)
            cards << trick;
    }

    return cards;
}

DummyCard *ServerPlayer::wholeHandCards() const{
    DummyCard *dummy_card = new DummyCard;
    foreach(const Card *card, handcards)
        dummy_card->addSubcard(card->getId());

    return dummy_card;
}

bool ServerPlayer::isLord() const{
    return room->getLord() == this;
}

bool ServerPlayer::hasNullification() const{
    if(hasSkill("kanpo")){
        foreach(const Card *card, handcards){
            if(card->isBlack() || card->objectName() == "nullification")
                return true;
        }
    }else{
        foreach(const Card *card, handcards){
            if(card->objectName() == "nullification")
                return true;
        }
    }

    return false;
}

void ServerPlayer::setAIByGeneral(){
    if(getGeneral()){
        delete ai;
        ai = Sanguosha->cloneAI(this);
        if(ai == NULL)
            ai = new TrustAI(this);
    }
}

void ServerPlayer::setAI(AI *ai) {
    this->ai = ai;
}

AI *ServerPlayer::getAI() const{
    if(getState() == "online")
        return NULL;
    else
        return ai;
}
