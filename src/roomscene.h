#ifndef ROOMSCENE_H
#define ROOMSCENE_H

#include "photo.h"
#include "dashboard.h"
#include "card.h"
#include "client.h"

#include <QGraphicsScene>
#include <MediaObject>
#include <QQueue>

class RoomScene : public QGraphicsScene
{
    Q_OBJECT
public:
    RoomScene(Client *client, int player_count);    

public slots:
    void showBust(const QString &name);
    void addPlayer(ClientPlayer *player);
    void removePlayer(const QString &player_name);
    void drawCards(const QList<Card *> &cards);
    void drawNCards(ClientPlayer *player, int n);
    void discardCard(CardItem *card_item);
    void chooseLord(const QList<const General *> &lords);
    void chooseGeneral(const General *lord, const QList<const General*> &generals);
    void changePrompt(const QString &prompt_str);
    void updatePhotos(const QList<const Player*> &seats);
    void viewDiscarded();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

private:
    Client *client;
    QList<Photo*> photos;
    QMap<QString, Photo*> name2photo;
    Dashboard *dashboard;
    Pixmap *pile;
    QGraphicsSimpleTextItem *prompt_label;
    Pixmap *avatar;
    Pixmap *bust;
    QList<const Card*> discarded_list;
    QQueue<CardItem*> discarded_queue;
    Phonon::MediaObject *effect;

    void startEnterAnimation();
};

#endif // ROOMSCENE_H
