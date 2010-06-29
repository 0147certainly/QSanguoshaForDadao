#include "general.h"
#include "engine.h"

General *General::getInstance(const QString &name){
    General *copy = Sanguosha->getGeneral(name);

    if(copy){
        General *general = new General;
        general->setObjectName(name);
        general->kingdom = copy->kingdom;
        general->hp = general->max_hp = copy->max_hp;
        general->male = copy->max_hp;
        general->is_lord = false;
        return general;
    }else
        return NULL;
}

General::General(const QString &name, const QString &kingdom, int max_hp, bool male)
    :kingdom(kingdom), max_hp(max_hp), hp(max_hp), male(male), is_lord(false)
{
    setObjectName(name);

    // initialize related pixmaps
}

int General::getMaxHp() const{
    return max_hp;
}

int General::getHp() const{
    return hp;
}

void General::setHp(int hp){
    if(hp > 0 && hp <= max_hp)
        this->hp = hp;
}

QString General::getKingdom() const{
    return kingdom;
}

bool General::isMale() const{
    return male;
}

bool General::isFemale() const{
    return !male;
}

bool General::isWounded() const{
    return hp < max_hp;
}

void General::enthrone(){
    max_hp ++;
    hp = max_hp;
    is_lord = true;
}
