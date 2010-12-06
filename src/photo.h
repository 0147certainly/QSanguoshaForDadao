#ifndef PHOTO_H
#define PHOTO_H

#include "pixmap.h"
#include "player.h"
#include "carditem.h"

#include <QGraphicsObject>
#include <QPixmap>
#include <QComboBox>
#include <QProgressBar>

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
    void installDelayedTrick(CardItem *trick);
    void addCardItem(CardItem *card_item);
    void hideAvatar();
    void showCard(int card_id);
    void showProcessBar();
    void hideProcessBar();    
    void setEmotion(const QString &emotion, bool permanent = false);
    void tremble();
    void showSkillName(const QString &skill_name);

    enum FrameType{
        Playing,
        Responsing,
        SOS,
        NoFrame
    };

    void setFrame(FrameType type);

public slots:
    void separateRoleCombobox();
    void updateAvatar();
    void updateSmallAvatar();
    void updateRoleCombobox(const QString &new_role);
    void updateMarks(const QString &mark);
    void updatePhase();
    void refresh();
    void hideEmotion();
    void hideSkillName();

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void timerEvent(QTimerEvent *);

private:
    const ClientPlayer *player;
    QPixmap avatar, small_avatar;
    QGraphicsPixmapItem *kingdom_item;
    QPixmap kingdom_frame;
    QPixmap handcard;
    QPixmap chain;
    QComboBox *role_combobox;
    QGraphicsProxyWidget *role_combobox_widget;

    CardItem *weapon, *armor, *defensive_horse, *offensive_horse;
    QList<CardItem **> equips;
    QGraphicsRectItem *equip_rects[4];

    QStack<QGraphicsPixmapItem *> judging_pixmaps;    
    QStack<CardItem *> judging_area;

    QMap<QString, QGraphicsPixmapItem *> mark_items;
    QMap<QString, QGraphicsSimpleTextItem *> mark_texts;

    QGraphicsPixmapItem *order_item;
    bool hide_avatar;
    QPixmap death_pixmap;
    Pixmap *back_icon;
    QProgressBar *progress_bar;
    int timer_id;
    QGraphicsPixmapItem *emotion_item, *frame_item;
    QGraphicsSimpleTextItem *skill_name_item;
    QGraphicsRectItem *avatar_area, *small_avatar_area;

    void drawEquip(QPainter *painter, CardItem *equip, int order);
    void drawHp(QPainter *painter);
    void drawMagatama(QPainter *painter, int index, const QPixmap &pixmap);
};

#endif // PHOTOBACK_H
