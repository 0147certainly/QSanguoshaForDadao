#ifndef ENGINE_H
#define ENGINE_H

#include "general.h"

#include <QScriptEngine>

class Engine : public QScriptEngine
{
    Q_OBJECT
public:
    explicit Engine(QObject *parent);

    // invokable methods, functions that marked this flags can be called by scripts enironment
    Q_INVOKABLE QObject *addGeneral(const QString &name, const QString &kingdom, int max_hp, bool male);
    Q_INVOKABLE void addTranslationTable(QVariantMap table);

private:
    QObject *generals;
    QObject *translation;
};

#endif // ENGINE_H
