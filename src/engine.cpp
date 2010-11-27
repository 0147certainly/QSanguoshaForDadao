#include "engine.h"
#include "card.h"
#include "client.h"
#include "ai.h"
#include "settings.h"
#include "scenario.h"
#include "challengemode.h"

extern audiere::AudioDevicePtr Device;

class StopCallback: public audiere::StopCallback{
public:
    ADR_METHOD(void) streamStopped(audiere::StopEvent *event){
        Sanguosha->removeFromPlaying(event->getOutputStream());
    }

    ADR_METHOD(void) ref(){

    }

    ADR_METHOD(void) unref(){

    }
};

#include <QFile>
#include <QStringList>
#include <QMessageBox>
#include <QDir>
#include <QLibrary>
#include <QApplication>

Engine *Sanguosha = NULL;

extern "C" {
    Package *NewStandard();
    Package *NewWind();
    Package *NewFire();
    Package *NewThicket();
    Package *NewManeuvering();
    Package *NewGod();
    Package *NewYitian();

    Scenario *NewGuanduScenario();
    Scenario *NewFanchengScenario();
}

Engine::Engine()
{
    addPackage(NewStandard());
    addPackage(NewWind());
    addPackage(NewFire());
    addPackage(NewThicket());
    addPackage(NewManeuvering());
    addPackage(NewGod());
    addPackage(NewYitian());

    addScenario(NewGuanduScenario());
    addScenario(NewFanchengScenario());

    if(Device)
        Device->registerCallback(new StopCallback);

    // available game modes
    modes["02p"] = tr("2 players");
    modes["03p"] = tr("3 players");
    modes["04p"] = tr("4 players");
    modes["05p"] = tr("5 players");
    modes["06p"] = tr("6 players");
    modes["06pd"] = tr("6 players (2 renegades)");
    modes["07p"] = tr("7 players");
    modes["08p"] = tr("8 players");
    modes["08pd"] = tr("8 players (2 renegades)");
    modes["08boss"] = tr("8 players (boss mode)");
    modes["09p"] = tr("9 players");
    modes["10p"] = tr("10 players");

    challenge_mode_set = new ChallengeModeSet(this);
    addPackage(challenge_mode_set);
}

QStringList Engine::getScenarioNames() const{
    return scenarios.keys();
}

void Engine::addScenario(Scenario *scenario){
    scenario->setParent(this);
    scenarios.insert(scenario->objectName(), scenario);

    addPackage(scenario);
}

const Scenario *Engine::getScenario(const QString &name) const{
    return scenarios.value(name, NULL);
}

const ChallengeModeSet *Engine::getChallengeModeSet() const{
    return challenge_mode_set;
}

const ChallengeMode *Engine::getChallengeMode(const QString &name) const{
    return challenge_mode_set->getMode(name);
}

void Engine::addPackage(Package *package){
    package->setParent(this);

    QList<Card *> all_cards = package->findChildren<Card *>();
    foreach(Card *card, all_cards){
        card->setId(cards.length());
        cards << card;

        QString card_name = card->objectName();
        metaobjects.insert(card_name, card->metaObject());
    }

    QList<General *> all_generals = package->findChildren<General *>();
    foreach(General *general, all_generals){
        if(general->isLord())
            lord_list << general->objectName();
        else
            nonlord_list << general->objectName();

        generals.insert(general->objectName(), general);

        QList<const Skill *> all_skills = general->findChildren<const Skill *>();
        foreach(const Skill *skill, all_skills)
            skills.insert(skill->objectName(), skill);
    }

    QList<const QMetaObject *> metas = package->getMetaObjects();
    foreach(const QMetaObject *meta, metas)
        metaobjects.insert(meta->className(), meta);

    translations.unite(package->getTranslation());

    QList<const Skill *> extra_skills = package->getSkills();
    foreach(const Skill *skill, extra_skills)
        skills.insert(skill->objectName(), skill);
}

void Engine::addBanPackage(const QString &package_name){
    ban_package.insert(package_name);
}

QStringList Engine::getBanPackages() const{
    return ban_package.toList();
}

QString Engine::translate(const QString &to_translate) const{
    return translations.value(to_translate, to_translate);
}

QString Engine::getRoleString(const QString &role) const{
    if(ServerInfo.GameMode == "08boss"){
        if(role == "lord")
            return tr("boss");
        else if(role == "renegade")
            return tr("guardian");
        else if(role == "loyalist")
            return tr("citizen");
        else
            return tr("hero");
    }else if(ServerInfo.GameMode == "06_3v3"){
        if(role == "lord" || role == "renegade")
            return tr("marshal");
        else
            return tr("vanguard");
    }else
        return translate(role);
}

const General *Engine::getGeneral(const QString &name) const{
    return generals.value(name, NULL);
}

int Engine::getGeneralCount(bool include_banned) const{
    if(include_banned)
        return generals.size();

    int total = generals.size();
    QHashIterator<QString, const General *> itor(generals);
    while(itor.hasNext()){
        itor.next();
        const General *general = itor.value();
        if(ban_package.contains(general->getPackage()))
            total--;
    }

    return total;
}

const Card *Engine::getCard(int index) const{
    if(index < 0 || index >= cards.length())
        return NULL;
    else
        return cards.at(index);
}

Card *Engine::cloneCard(const QString &name, Card::Suit suit, int number) const{
    const QMetaObject *meta = metaobjects.value(name, NULL);
    if(meta){
        QObject *card_obj = meta->newInstance(Q_ARG(Card::Suit, suit), Q_ARG(int, number));
        return qobject_cast<Card *>(card_obj);
    }else
        return NULL;    
}

SkillCard *Engine::cloneSkillCard(const QString &name) const{
    const QMetaObject *meta = metaobjects.value(name, NULL);
    if(meta){
        QObject *card_obj = meta->newInstance();
        SkillCard *card = qobject_cast<SkillCard *>(card_obj);
        return card;
    }else
        return NULL;
}

AI *Engine::cloneAI(ServerPlayer *player) const{
    QString general_name = player->getGeneralName();
    general_name[0] = general_name[0].toUpper();
    QString ai_name = general_name + "AI";
    const QMetaObject *meta = metaobjects.value(ai_name, NULL);
    if(meta){
        QObject *ai_object = meta->newInstance(Q_ARG(ServerPlayer *, player));
        AI *ai = qobject_cast<AI *>(ai_object);
        return ai;
    }else
        return NULL;
}

QString Engine::getVersion() const{
    return "20101115";
}

QStringList Engine::getExtensions() const{
    static QStringList extensions;
    if(extensions.isEmpty())
        extensions << "wind" << "fire" << "thicket" << "maneuvering" << "god" << "yitian";

    return extensions;
}

QStringList Engine::getKingdoms() const{
    static QStringList kingdoms;
    if(kingdoms.isEmpty())
        kingdoms << "wei" << "shu" << "wu" << "qun" << "god";

    return kingdoms;
}

QString Engine::getSetupString() const{
    int timeout = Config.OperationNoLimit ? 0 : Config.OperationTimeout;
    QString flags;
    if(Config.FreeChoose)
        flags.append("F");
    if(Config.Enable2ndGeneral)
        flags.append("S");

    QString server_name = Config.ServerName.toUtf8().toBase64();
    QStringList setup_items;
    setup_items << server_name
            << Config.GameMode
            << QString::number(timeout)
            << Sanguosha->getBanPackages().join("+")
            << flags;

    return setup_items.join(":");
}

QMap<QString, QString> Engine::getAvailableModes() const{
    return modes;
}

QString Engine::getModeName(const QString &mode) const{
    if(modes.contains(mode))
        return modes.value(mode);
    else if(mode.startsWith("@"))
        return tr("%1 [Challenge mode]").arg(translate(mode));
    else
        return tr("%1 [Scenario mode]").arg(translate(mode));

    return QString();
}

int Engine::getPlayerCount(const QString &mode) const{
    if(modes.contains(mode)){
        QRegExp rx("(\\d+)");
        int index = rx.indexIn(mode);
        if(index != -1)
            return rx.capturedTexts().first().toInt();
    }else if(mode.startsWith("@")){
        // challenge mode
        const ChallengeMode *cmode = challenge_mode_set->getMode(mode);
        if(cmode)
            return cmode->getGenerals().length() * 2;
    }else{
        // scenario mode
        const Scenario *scenario = scenarios.value(mode, NULL);
        if(scenario)
            return scenario->getPlayerCount();
    }

    return -1;
}

int Engine::getCardCount() const{
    return cards.length();
}

QStringList Engine::getLords() const{
    QStringList lords;

    // add intrinsic lord
    foreach(QString lord, lord_list){
        const General *general = generals.value(lord);
        if(!ban_package.contains(general->getPackage()))
            lords << lord;
    }

    return lords;
}

QStringList Engine::getRandomLords() const{
    QStringList lords = getLords();

    QStringList nonlord_list;
    foreach(QString nonlord, this->nonlord_list){
        const General *general = generals.value(nonlord);
        if(!ban_package.contains(general->getPackage()))
            nonlord_list << nonlord;
    }

    qShuffle(nonlord_list);

    int i;
    const static int extra = 2;
    for(i=0; i< extra; i++)
        lords << nonlord_list.at(i);

    return lords;
}

QStringList Engine::getLimitedGeneralNames() const{
    QStringList general_names;
    QHashIterator<QString, const General *> itor(generals);
    while(itor.hasNext()){
        itor.next();
        if(!ban_package.contains(itor.value()->getPackage())){
            general_names << itor.key();
        }
    }

    return general_names;
}

QStringList Engine::getRandomGenerals(int count, const QSet<QString> &ban_set) const{
    QStringList all_generals = getLimitedGeneralNames();

    Q_ASSERT(all_generals.count() >= count);

    if(!ban_set.isEmpty()){
        QSet<QString> general_set = all_generals.toSet();
        all_generals = general_set.subtract(ban_set).toList();
    }

    // shuffle them
    qShuffle(all_generals);

    QStringList general_list = all_generals.mid(0, count);
    Q_ASSERT(general_list.count() == count);

    return general_list;
}

QList<int> Engine::getRandomCards() const{
    QList<int> list;
    foreach(Card *card, cards){
        if(!ban_package.contains(card->getPackage()))
            list << card->getId();
    }

    qShuffle(list);

    return list;
}

void Engine::playAudio(const QString &audio, const QString &suffix){
    playEffect(QString("audio/%1.%2").arg(audio).arg(suffix.isEmpty() ? "wav" : suffix));
}

void Engine::playEffect(const QString &filename){
    if(!Config.EnableEffects)
        return;

    audiere::OutputStreamPtr effect;
    if(playing.contains(filename))
        return;

    effect = effects.value(filename, NULL);
    if(effect == NULL){
        effect = audiere::OpenSound(Device, filename.toAscii());
        effects.insert(filename, effect);
    }

    if(effect){
        playing.insert(filename, effect);
        effect->setVolume(Config.Volume);
        effect->play();
    }
}

void Engine::playSkillEffect(const QString &skill_name, int index){
    const Skill *skill = skills.value(skill_name, NULL);
    if(skill)
        skill->playEffect(index);
}

void Engine::playCardEffect(const QString &card_name, bool is_male){
    const Card *card = findChild<const Card *>(card_name);
    if(card)
        playEffect(card->getEffectPath(is_male));
}

void Engine::playCardEffect(const QString &card_name, const QString &package, bool is_male){
    QString gender = is_male ? "male" : "female";
    QString path = QString("%1/cards/effect/%2/%3.mp3").arg(package).arg(gender).arg(card_name);
    playEffect(path);
}

void Engine::removeFromPlaying(audiere::OutputStreamPtr stream){
    mutex.lock();

    QString key = playing.key(stream);
    if(!key.isEmpty())
        playing.remove(key);    

    mutex.unlock();
}

const Skill *Engine::getSkill(const QString &skill_name) const{
    return skills.value(skill_name, NULL);
}

const TriggerSkill *Engine::getTriggerSkill(const QString &skill_name) const{
    const Skill *skill = getSkill(skill_name);
    if(skill)
        return qobject_cast<const TriggerSkill *>(skill);
    else
        return NULL;
}
