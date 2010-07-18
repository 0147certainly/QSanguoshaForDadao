#ifndef ENGINE_H
#define ENGINE_H

#include "general.h"
#include "cardclass.h"
#include "skill.h"

#include <QScriptEngine>
#include <QHash>
#include <QEvent>
#include <QStringList>

class Engine : public QScriptEngine
{
    Q_OBJECT
    Q_PROPERTY(QString pixmap_dir READ getPixmapDir WRITE setPixmapDir)
public:
    explicit Engine(QObject *parent);

    // invokable methods, functions that marked this flags can be called by scripts enironment
    Q_INVOKABLE QObject *addGeneral(const QString &name, const QString &kingdom, int max_hp = 4, bool male = true);
    Q_INVOKABLE QObject *addCard(const QString &name, const QScriptValue &suit_value, const QScriptValue &number_value);
    Q_INVOKABLE QObject *addCardClass(const QString &class_name, const QString &type, const QString &subtype = "");
    Q_INVOKABLE QObject *addSkill(const QString &name, const QScriptValue &obj);

    Q_INVOKABLE void addTranslationTable(const QScriptValue &table);
    Q_INVOKABLE QString translate(const QString &to_translate);

    Q_INVOKABLE QScriptValue doScript(const QString &filename);
    Q_INVOKABLE void alert(const QString &message);
    Q_INVOKABLE void quit(const QString &reason = "");

    const General *getGeneral(const QString &name);
    int getGeneralCount() const;
    CardClass *getCardClass(const QString &name);
    int getCardCount() const;
    Card *getCard(int index);
    void setPixmapDir(const QString &pixmap_dir);
    QString getPixmapDir() const;
    QEvent::Type getEventType() const;
    Skill *getSkill(const QString &name);
    void getRandomLords(QStringList &lord_list, int lord_count = 5);
    void getRandomGenerals(QStringList &general_list, int count);
    void getRandomCards(QList<int> &list);

private:
    QObject *generals;    
    QObject *card_classes;
    QObject *skills;

    QHash<QString,QString> translations;
    QList<Card*> cards;
    QString pixmap_dir;
    QEvent::Type event_type;
    QStringList lord_names;
};

extern Engine *Sanguosha;

#endif // ENGINE_H
