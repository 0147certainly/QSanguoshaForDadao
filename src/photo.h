#ifndef PHOTO_H
#define PHOTO_H

#include "pixmap.h"
#include "player.h"
#include "carditem.h"

#include <QGraphicsObject>
#include <QPixmap>
#include <QComboBox>

class ClientPlayer;

class Photo : public Pixmap
{
    Q_OBJECT
public:
    explicit Photo(int order);
    void setPlayer(const ClientPlayer *player);
    const ClientPlayer *getPlayer() const;
    void speak(const QString &content);
    CardItem *takeCardItem(int card_id, Player::Place place);
    void installEquip(CardItem *equip);
    void addCardItem(CardItem *card_item);

public slots:
    void updateAvatar();
    void updateRoleCombobox(const QString &new_role);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    const ClientPlayer *player;
    QPixmap avatar;
    QPixmap avatar_frame;
    QPixmap kingdom;
    QPixmap handcard;
    QPixmap magatamas[5];   
    QComboBox *role_combobox;
    CardItem *weapon, *armor, *defensive_horse, *offensive_horse;
    QGraphicsPixmapItem *order_item;

    void drawEquip(QPainter *painter, CardItem *equip, int order);
};

#endif // PHOTOBACK_H
