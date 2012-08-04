#ifndef RECORDANALYSIS_H
#define RECORDANALYSIS_H

#include "client.h"
#include "engine.h"
#include "serverplayer.h"

struct PlayerRecordStruct;

class RecAnalysis{
public:
    explicit RecAnalysis(QString dir = QString());

    void initialize(QString dir = QString());
    PlayerRecordStruct *getPlayerRecord(const Player *player) const;
    QMap<QString, PlayerRecordStruct *> getRecordMap() const;
    QList<QString> getRecordPackages() const;
    QList<QString> getRecordWinners() const;

private:
    QMap<QString, PlayerRecordStruct *> m_recordMap;
    QList<QString> m_recordPackages, m_recordWinners;
};

struct PlayerRecordStruct{
    PlayerRecordStruct();

    QString m_generalName;
    QString m_screenName;
    QString m_statue;
    QString m_role;
    int m_recover;
    int m_damage;
    int m_kill;
    bool m_isAlive;
    QList<QString> m_designation;
};

#endif // RECORDANALYSIS_H
