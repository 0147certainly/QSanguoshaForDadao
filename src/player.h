#ifndef PLAYER_H
#define PLAYER_H

#include "general.h"

#include <QObject>

class Player : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int hp READ getHp WRITE setHp)
    Q_PROPERTY(bool wounded READ isWounded STORED false)
    // name is just an alias to objectName
    Q_PROPERTY(QString name READ objectName WRITE setObjectName STORED false)

public:
    explicit Player(QObject *parent = 0);

    // property setters/getters
    int getHp() const;
    void setHp(int hp);
    bool isWounded() const;
    void setGeneral(const General *general);
    const General *getGeneral() const;

private:
    const General *general;
    int hp;
};

#endif // PLAYER_H
