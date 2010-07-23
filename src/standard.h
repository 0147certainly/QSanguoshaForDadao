#ifndef STANDARD_H
#define STANDARD_H

#include "package.h"
#include "card.h"
#include "event.h"

class StandardPackage:public Package{
    Q_OBJECT

public:
    StandardPackage();
    void addCards();
    void addGenerals();
};

class BasicCard:public Card{
    Q_OBJECT

public:
    BasicCard(Suit suit, int number):Card(suit, number){}
    virtual QString getType() const;
    virtual int getTypeId() const;
};

class TrickCard:public Card{
    Q_OBJECT

public:
    TrickCard(Suit suit, int number):Card(suit, number){}
    virtual QString getType() const;
    virtual int getTypeId() const;
};

class EquipCard:public Card{
    Q_OBJECT

public:
    EquipCard(Suit suit, int number):Card(suit, number){}
    virtual QString getType() const;
    virtual int getTypeId() const;
};

class GlobalEffect:public TrickCard{
    Q_OBJECT

public:
    GlobalEffect(Suit suit, int number):TrickCard(suit, number){}
    virtual QString getSubtype() const;
};

class AOE:public TrickCard{
    Q_OBJECT

public:
    AOE(Suit suit, int number):TrickCard(suit, number){}
    virtual QString getSubtype() const;
};

class SingleTargetTrick: public TrickCard{
    Q_OBJECT

public:
    SingleTargetTrick(Suit suit, int number):TrickCard(suit, number){}
    virtual QString getSubtype() const;
};

class DelayedTrick:public TrickCard{
    Q_OBJECT

public:
    DelayedTrick(Suit suit, int number):TrickCard(suit, number){ }
    virtual QString getSubtype() const;
};

class Weapon:public EquipCard{
    Q_OBJECT

public:
    Weapon(Suit suit, int number, int range)
        :EquipCard(suit, number), range(range){}
    virtual QString getSubtype() const;

protected:
    int range;
};

class Armor:public EquipCard{
    Q_OBJECT

public:
    Armor(Suit suit, int number):EquipCard(suit, number){}
    virtual QString getSubtype() const;
};

class Horse:public EquipCard{
    Q_OBJECT

public:
    Horse(const QString &name, Suit suit, int number, int correct);
    virtual QString getSubtype() const;
    virtual Card *clone(Suit suit, int number) const;

private:
    int correct;
};

#endif // STANDARD_H
