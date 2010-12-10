#include "card.h"
#include "settings.h"
#include "engine.h"
#include "client.h"
#include "room.h"
#include "carditem.h"

const Card::Suit Card::AllSuits[4] = {
    Card::Spade,
    Card::Club,
    Card::Heart,
    Card::Diamond
};

Card::Card(Suit suit, int number, bool target_fixed)
    :target_fixed(target_fixed), suit(suit), number(number), id(-1)
{
    if(number < 1 || number > 13)
        number = 0;
}

QString Card::getSuitString() const{
    return Suit2String(suit);
}

QString Card::Suit2String(Suit suit){
    switch(suit){
    case Spade: return "spade";
    case Heart: return "heart";
    case Club: return "club";
    case Diamond: return "diamond";
    default: return "no_suit";
    }
}

bool Card::isRed() const{
    return suit == Heart || suit == Diamond;
}

bool Card::isBlack() const{
    return suit == Spade || suit == Club;
}

int Card::getId() const{
    return id;
}

void Card::setId(int id){
    this->id = id;
}

int Card::getNumber() const{
    return number;
}

void Card::setNumber(int number){
    this->number = number;
}

QString Card::getNumberString() const{
    if(number == 10)
        return "10";
    else{
        static const char *number_string = "-A23456789-JQK";
        return QString(number_string[number]);
    }
}

Card::Suit Card::getSuit() const{
    return suit;
}

void Card::setSuit(Suit suit){
    this->suit = suit;
}

bool Card::sameColorWith(const Card *other) const{
    return isBlack() == other->isBlack();
}

bool Card::match(const QString &pattern) const{
    return objectName() == pattern || getType() == pattern || getSubtype() == pattern;
}

bool Card::CompareBySuitNumber(const Card *a, const Card *b){
    if(a->suit != b->suit)
        return a->suit < b->suit;
    else
        return a->number < b->number;
}

bool Card::CompareByType(const Card *a, const Card *b){
    int order1 = a->getTypeId() * 10000 + a->id;
    int order2 = b->getTypeId() * 10000 + b->id;
    if(order1 != order2)
        return order1 < order2;
    else
        return CompareBySuitNumber(a,b);
}

QString Card::getPixmapPath() const{
    //return QString("%1/cards/card/%2.png").arg(parent()->objectName()).arg(objectName());
    return QString("image/card/%1.jpg").arg(objectName());
}

QString Card::getIconPath() const{
    return QString("image/icon/%1.png").arg(objectName());
}

QString Card::getPackage() const{
    if(parent())
        return parent()->objectName();
    else
        return "";
}

QString Card::getEffectPath(bool is_male) const{
    QString gender = is_male ? "male" : "female";
    return QString("audio/card/%1/%2.ogg").arg(gender).arg(objectName());
}

QString Card::getEffectPath() const{
    return QString("audio/card/common/%1.ogg").arg(objectName());
}

QIcon Card::getSuitIcon() const{
    return QIcon(QString(":/suit/%1.png").arg(getSuitString()));
}

QString Card::getFullName(bool include_suit) const{
    QString name = getName();
    if(include_suit){
        QString suit_name = Sanguosha->translate(getSuitString());
        return QString("%1%2 %3").arg(suit_name).arg(getNumberString()).arg(name);
    }else
        return QString("%1 %2").arg(getNumberString()).arg(name);
}

QString Card::getLogName() const{
    QString suit_char;
    QString number_string;

    if(suit != Card::NoSuit)
        suit_char = QString("<img src=':/suit/%1.png' width='15' height='15' />").arg(getSuitString());

    if(number != 0)
        number_string = getNumberString();

    return QString("%1[%2%3]").arg(getName()).arg(suit_char).arg(number_string);
}

QString Card::getName() const{
    return Sanguosha->translate(objectName());
}

QString Card::getSkillName() const{
    return skill_name;
}

void Card::setSkillName(const QString &name){
    this->skill_name = name;
}

QString Card::getDescription() const{
    QString desc = Sanguosha->translate(":" + objectName());
    return tr("<b>[%1]</b> %2").arg(getName()).arg(desc);
}

QString Card::toString() const{
    if(!isVirtualCard())
        return QString::number(id);
    else
        return QString("%1:%2[%3:%4]=%5")
                .arg(objectName()).arg(skill_name)
                .arg(getSuitString()).arg(getNumberString()).arg(subcardString());
}

QString Card::subcardString() const{
    if(subcards.isEmpty())
        return ".";

    QStringList str;
    foreach(int subcard, subcards)
        str << QString::number(subcard);

    return str.join("+");
}

void Card::addSubcards(const QList<CardItem *> &card_items){
    foreach(CardItem *card_item, card_items)
        subcards << card_item->getCard()->getId();
}

int Card::subcardsLength() const{
    return subcards.length();
}

bool Card::isVirtualCard() const{
    return id < 0;
}

const Card *Card::Parse(const QString &str){
    if(str.startsWith(QChar('@'))){
        // skill card
        static QRegExp pattern("@(\\w+)=(.+)");
        pattern.indexIn(str);
        QStringList texts = pattern.capturedTexts();
        QString card_name = texts.at(1);
        QStringList subcard_ids;
        QString subcard_str = texts.at(2);
        if(subcard_str != ".")
           subcard_ids = subcard_str.split("+");
        SkillCard *card = Sanguosha->cloneSkillCard(card_name);

        if(card == NULL)
            return NULL;

        foreach(QString subcard_id, subcard_ids)
            card->addSubcard(subcard_id.toInt());

        // skill name
        QString skill_name = card_name.remove("Card").toLower();
        card->setSkillName(skill_name);

        return card;        
    }else if(str.startsWith(QChar('$'))){
        QString copy = str;
        copy.remove(QChar('$'));
        QStringList card_strs = copy.split("+");
        DummyCard *dummy = new DummyCard;
        foreach(QString card_str, card_strs){
            dummy->addSubcard(card_str.toInt());
        }

        return dummy;
    }else if(str.contains(QChar('='))){
        static QRegExp pattern("(\\w+):(\\w*)\\[(\\w+):(.+)\\]=(.+)");
        if(!pattern.exactMatch(str))
            return NULL;

        QStringList texts = pattern.capturedTexts();
        QString name = texts.at(1);
        QString skill_name = texts.at(2);
        QString suit_string = texts.at(3);
        QString number_string = texts.at(4);
        QString subcard_str = texts.at(5);
        QStringList subcard_ids;
        if(subcard_str != ".")
            subcard_ids = subcard_str.split("+");

        static QMap<QString, Card::Suit> suit_map;
        if(suit_map.isEmpty()){
            suit_map.insert("spade", Card::Spade);
            suit_map.insert("club", Card::Club);
            suit_map.insert("heart", Card::Heart);
            suit_map.insert("diamond", Card::Diamond);
            suit_map.insert("no_suit", Card::NoSuit);
        }

        Suit suit = suit_map.value(suit_string, Card::NoSuit);

        int number = 0;
        if(number_string == "A")
            number = 1;
        else if(number_string == "J")
            number = 11;
        else if(number_string == "Q")
            number = 12;
        else if(number_string == "K")
            number = 13;
        else
            number = number_string.toInt();

        Card *card = Sanguosha->cloneCard(name, suit, number);
        if(card == NULL)
            return NULL;        

        foreach(QString subcard_id, subcard_ids)
            card->addSubcard(subcard_id.toInt());

        card->setSkillName(skill_name);
        return card;
    }else{
        bool ok;
        int card_id = str.toInt(&ok);
        if(ok)
            return Sanguosha->getCard(card_id);
        else
            return NULL;
    }
}

Card *Card::Clone(const Card *card){
    const QMetaObject *meta = card->metaObject();
    Card::Suit suit = card->getSuit();
    int number = card->getNumber();

    QObject *card_obj = meta->newInstance(Q_ARG(Card::Suit, suit), Q_ARG(int, number));
    if(card_obj){
        Card *new_card = qobject_cast<Card *>(card_obj);
        new_card->setObjectName(card->objectName());
        new_card->addSubcard(card->getId());
        return new_card;
    }else
        return NULL;
}

bool Card::targetFixed() const{
    return target_fixed;
}

bool Card::targetsFeasible(const QList<const ClientPlayer *> &targets) const{
    if(target_fixed)
        return true;
    else
        return !targets.isEmpty();
}

bool Card::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{    
    return targets.isEmpty() && to_select != Self;
}

void Card::use(const QList<const ClientPlayer *> &targets) const{    
}

static bool CompareByActionOrder(ServerPlayer *a, ServerPlayer *b){
    Room *room = a->getRoom();
    int current = room->getCurrent()->getSeat();

    int seat1 = a->getSeat();
    if(seat1 < current)
        seat1 += room->alivePlayerCount();

    int seat2 = b->getSeat();
    if(seat2 < current)
        seat2 += room->alivePlayerCount();

    return seat1 < seat2;
}

void Card::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    if(targets.length() == 1){
        room->cardEffect(this, source, targets.first());
    }else{
        QList<ServerPlayer *> players = targets;
        qSort(players.begin(), players.end(), CompareByActionOrder);

        foreach(ServerPlayer *target, players){            
            CardEffectStruct effect;
            effect.card = this;
            effect.from = source;
            effect.to = target;
            effect.multiple = true;

            room->cardEffect(effect);
        }
    }
}

void Card::onEffect(const CardEffectStruct &effect) const{

}

bool Card::isCancelable(const CardEffectStruct &effect) const{
    return false;
}

void Card::onMove(const CardMoveStruct &move) const{
    // usually dummy
}

void Card::addSubcard(int card_id){
    if(card_id < 0)
        qWarning(qPrintable(tr("Subcard must not be virtual card!")));
    else
        subcards << card_id;
}

QList<int> Card::getSubcards() const{
    return subcards;
}

void Card::clearSubcards(){
    subcards.clear();
}

bool Card::isAvailable() const{
    return true;
}

// ---------   Skill card     ------------------

SkillCard::SkillCard()
    :Card(NoSuit, 0)
{
}

QString SkillCard::getType() const{
    return "skill_card";
}

QString SkillCard::getSubtype() const{
    return "skill_card";
}

int SkillCard::getTypeId() const{
    return 0;
}

QString SkillCard::toString() const{
    return QString("@%1=%2").arg(metaObject()->className()).arg(subcardString());
}

// ---------- Dummy card      -------------------

DummyCard::DummyCard()
    :Card(NoSuit, 0)
{
    setObjectName("dummy");
    target_fixed = true;
}

QString DummyCard::getType() const{
    return "dummy_card";
}

QString DummyCard::getSubtype() const{
    return "dummy_card";
}

int DummyCard::getTypeId() const{
    return 0;
}

QString DummyCard::toString() const{
    return "$" + subcardString();
}
