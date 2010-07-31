#ifndef CARD_H
#define CARD_H

#include <QObject>
#include <QMap>

class Room;
class ServerPlayer;
class Client;
class ClientPlayer;

class Card : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString suit READ getSuitString CONSTANT)
    Q_PROPERTY(bool red READ isRed STORED false CONSTANT)
    Q_PROPERTY(bool black READ isBlack STORED false CONSTANT)
    Q_PROPERTY(int id READ getID CONSTANT)
    Q_PROPERTY(int number READ getNumber CONSTANT)    
    Q_PROPERTY(QString number_string READ getNumberString CONSTANT)
    Q_PROPERTY(QString type READ getType CONSTANT)
    Q_PROPERTY(QString pixmap_path READ getPixmapPath)

    Q_ENUMS(Suit)

public:
    // enumeration type
    enum Suit {Spade, Club, Heart, Diamond, NoSuit};

    // constructor
    Card(Suit suit, int number);

    // property getters, as all properties of card is read only, no setter is defined
    QString getSuitString() const;
    bool isRed() const;
    bool isBlack() const;
    int getID() const;
    void setID(int id);
    int getNumber() const;
    QString getNumberString() const;
    Suit getSuit() const;
    QString getPixmapPath() const;
    QString getPackage() const;
    QString toString() const;
    bool isVirtualCard() const;

    bool match(const QString &pattern) const;
    bool isAvailable() const;

    virtual QString getType() const = 0;
    virtual QString getSubtype() const = 0;
    virtual int getTypeId() const = 0;

    // card target selection
    virtual bool targetFixed() const;
    virtual void targetRange(int *min, int *max, bool *include_self) const;
    virtual bool targetFilter(const QList<const ClientPlayer *> &targets) const;    

    // FIXME: should be pure virtual
    virtual void use(Room *room, ServerPlayer *source,  const QList<ServerPlayer *> &targets) const;
    virtual void use(const QList<const ClientPlayer *> &targets) const;

    void addSubcard(const Card *card);
    void addSubcards(const QList<const Card *> &cards);
    QString subcardString() const;

    // static functions
    static bool CompareBySuitNumber(const Card *a, const Card *b);
    static bool CompareByType(const Card *a, const Card *b);

    static const Card *Parse(const QString &str);

protected:
    virtual bool isAvailableAtPlay() const;

    QList<const Card *> subcards;

private:
    Suit suit;
    int number;
    int id;

};

class SkillCard: public Card{
    Q_OBJECT

public:
    SkillCard();
    virtual QString getSubtype() const;    
    virtual QString getType() const;
    virtual int getTypeId() const;
};

#endif // CARD_H
