#include "roomthread.h"
#include "room.h"
#include "gamerule.h"

bool PassiveSkillSorter::operator ()(const TriggerSkill *a, const TriggerSkill *b){
    int x = a->getPriority(target);
    int y = b->getPriority(target);

    return x > y;
}

void PassiveSkillSorter::sort(QList<const TriggerSkill *> &skills){
    qSort(skills.begin(), skills.end(), *this);
}

DamageStruct::DamageStruct()
    :from(NULL), to(NULL), card(NULL), damage(0), nature(Normal)
{
}

RoomThread::RoomThread(Room *room)
    :QThread(room), room(room)
{
    foreach(ServerPlayer *player, room->players){
        const General *general = player->getGeneral();

        QList<const TriggerSkill *> skills = general->findChildren<const TriggerSkill *>();
        foreach(const TriggerSkill *skill, skills){
            trigger_skills.insert(skill->objectName(), skill);
        }
    }
    GameRule *game_rule = new GameRule;
    trigger_skills.insert(game_rule->objectName(), game_rule);

    // construct trigger_table
    foreach(const TriggerSkill *skill, trigger_skills)
        addPassiveSkill(skill);
}

void RoomThread::run(){
    // start game, draw initial 4 cards
    foreach(ServerPlayer *player, room->players){
        invokePassiveSkills(GameStart, player);
    }

    room->changePhase(room->players.first());
}

bool RoomThread::invokePassiveSkills(TriggerEvent event, ServerPlayer *target, const QVariant &data){
    Q_ASSERT(QThread::currentThread() == this);

    QList<const TriggerSkill *> skills = skill_table[event];
    QMutableListIterator<const TriggerSkill *> itor(skills);
    while(itor.hasNext()){
        const TriggerSkill *skill = itor.next();
        if(!skill->triggerable(target))
            itor.remove();
    }

    static PassiveSkillSorter sorter;

    sorter.target = target;
    sorter.sort(skills);

    foreach(const TriggerSkill *skill, skills){
        if(skill->trigger(event, target, data))
            return true;
    }

    return false;
}

void RoomThread::addPassiveSkill(const TriggerSkill *skill){
    QList<TriggerEvent> events;
    skill->getTriggerEvents(events);
    foreach(TriggerEvent event, events){
        skill_table[event] << skill;
    }
}

void RoomThread::removePassiveSkill(const TriggerSkill *skill){
    QList<TriggerEvent> events;
    skill->getTriggerEvents(events);
    foreach(TriggerEvent event, events){
        skill_table[event].removeOne(skill);
    }
}
