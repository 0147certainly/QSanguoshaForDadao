#ifndef ROOMSCENE_H
#define ROOMSCENE_H

#include "photo.h"
#include "dashboard.h"
#include "card.h"
#include "client.h"

#include <QGraphicsScene>

class RoomScene : public QGraphicsScene
{
    Q_OBJECT
public:
    RoomScene(Client *client, int player_count = 8);
    void updatePhotos();
    void setPlayer(int index, const Player *player);

public slots:
    void showBust(const QString &name);
    void drawCards(const QString &cards_str);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
    Client *client;
    QList<Photo*> photos;
    Dashboard *dashboard;
    Pixmap *pile;
    QGraphicsSimpleTextItem *skill_label;
    Pixmap *avatar;
    Pixmap *bust;

    void startEnterAnimation();
};

#endif // ROOMSCENE_H
