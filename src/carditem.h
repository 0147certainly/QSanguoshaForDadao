#ifndef CARDITEM_H
#define CARDITEM_H

#include "card.h"
#include "pixmap.h"

#include <QSize>
#include <QPropertyAnimation>

class CardItem : public Pixmap
{
    Q_OBJECT
public:
    CardItem(const Card *card);

    const Card *getCard() const;
    void setHomePos(QPointF home_pos);
    void goBack(bool kieru = false);
//    void viewAs(const QString &name);
    const QPixmap &getSuitPixmap() const;
    void select();
    void unselect();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    const Card *card;
    QPixmap suit_pixmap;
    QPointF home_pos;
    QGraphicsPixmapItem *view_card_item;

signals:
    void show_discards();
    void hide_discards();
    void card_selected(CardItem *card_item);
    void pending(CardItem *item, bool add_to_pendings);
};

#endif // CARDITEM_H
