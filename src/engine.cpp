#include "engine.h"
#include "card.h"

#include <QFile>
#include <QStringList>
#include <QMessageBox>

Engine *Sanguosha = NULL;

Engine::Engine(QObject *parent) :
    QScriptEngine(parent)
{
    globalObject().setProperty("sgs", newQObject(this));

    generals = new QObject(this);
    generals->setObjectName("generals");

    translation = new QObject(this);
    translation->setObjectName("translation");

    QStringList script_files;
    script_files << "init.js" << "cards.js" << "generals.js";
    foreach(QString filename, script_files){
        QFile file("scripts/" + filename);
        if(file.open(QIODevice::ReadOnly)){
            evaluate(file.readAll(), filename);
            if(hasUncaughtException()){
                QString error_msg = tr("%1\n\n Stack trace:\n %2")
                                    .arg(uncaughtException().toString())
                                    .arg(uncaughtExceptionBacktrace().join("\n"));
                QMessageBox::warning(NULL, tr("Script exception!"), error_msg);
                exit(1);
                return;
            }
        }
    }
}

QObject *Engine::addGeneral(const QString &name, const QString &kingdom, int max_hp, bool male){
    General *general = new General(name, kingdom, max_hp, male);
    general->setParent(generals);
    return general;
}

QObject *Engine::addCard(const QString &name, const QString &suit_str, int number){
    Card::Suit suit;
    if(suit_str == "spade")
        suit = Card::Spade;
    else if(suit_str == "club")
        suit = Card::Club;
    else if(suit_str == "heart")
        suit = Card::Heart;
    else if(suit_str == "diamond")
        suit = Card::Diamond;
    else
        suit = Card::NoSuit;

    Card *card = new Card(name, suit, number);
    return card;
}

void Engine::addTranslationTable(QVariantMap table)
{
    QMapIterator<QString,QVariant> itor(table);
    while(itor.hasNext()){
        itor.next();
        translation->setProperty(itor.key().toAscii(), itor.value());
    }
}

QString Engine::translate(const QString &to_translate){
    return translation->property(to_translate.toAscii()).toString();
}

General *Engine::getGeneral(const QString &name){
    return generals->findChild<General*>(name);
}


