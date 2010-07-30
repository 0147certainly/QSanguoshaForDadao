#include "general.h"
#include "engine.h"
#include "skill.h"
#include "package.h"
#include "client.h"

General::General(Package *package, const QString &name, const QString &kingdom, int max_hp, bool male)
    :QObject(package), kingdom(kingdom), max_hp(max_hp), male(male)
{
    static QChar lord_symbol('$');
    if(name.contains(lord_symbol)){
        QString copy = name;
        copy.remove(lord_symbol);
        lord = true;
        setObjectName(copy);
    }else{
        lord = false;
        setObjectName(name);
    }
}

int General::getMaxHp() const{
    return max_hp;
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

bool General::isLord() const{
    return lord;
}

QString General::getPixmapPath(const QString &category) const{
    return QString("%1/generals/%2/%3.png").arg(getPackage()).arg(category).arg(objectName());
}

QString General::getKingdomPath() const{
    return QString("%1/generals/kingdom/%2.png").arg(getPackage()).arg(kingdom);
}

void General::addSkill(Skill *skill){    
    skill->setParent(this);
    skill->initMediaSource();
}

QString General::getPackage() const{
    return parent()->objectName();
}

void General::playEffect() const
{
    QList<const Skill *> skills = findChildren<const Skill *>();
    if(!skills.isEmpty()){
        int r = qrand() % skills.length();
        skills.at(r)->playEffect();
    }
}
