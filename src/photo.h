#ifndef PHOTO_H
#define PHOTO_H

#include "pixmap.h"
#include "player.h"

#include <QGraphicsObject>
#include <QPixmap>

class Photo : public Pixmap
{
    Q_OBJECT
public:
    explicit Photo();
    void setPlayer(const Player *player);
    const Player *getPlayer() const;
    void speak(const QString &content);

public slots:
    void updateAvatar();

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);

private:
    const Player *player;
    QPixmap avatar;
    QPixmap avatar_frame;
    QPixmap kingdom;
};

#endif // PHOTOBACK_H
