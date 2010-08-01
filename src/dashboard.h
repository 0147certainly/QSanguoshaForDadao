#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "pixmap.h"
#include "carditem.h"
#include "player.h"
#include "skill.h"

#include <QPushButton>
#include <QComboBox>
#include <QStack>

class Dashboard : public Pixmap
{
    Q_OBJECT

public:
    Dashboard();
    void addCardItem(CardItem *card_item);
    CardItem *takeCardItem(int card_id, Player::Place place);
    void setPlayer(const Player *player);
    Pixmap *getAvatar();
    void selectCard(const QString &pattern = "", bool forward = true);
    void useSelected();
    CardItem *getSelected() const;
    void unselectAll();
    void sort(int order);
    void disableAllCards();
    void enableCards();
    void installEquip(CardItem *equip);

    // pending operations
    void startPending(const ViewAsSkill *skill);
    void stopPending();
    const ViewAsSkill *currentSkill() const;    
    const Card *pendingCard() const;

public slots:
    void updateAvatar();
    void setSelectedItem(CardItem *card_item);
    void enableCards(const QString &pattern);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QList<CardItem*> card_items;
    CardItem *selected;
    const Player *player;
    QPixmap magatamas[5];
    Pixmap *avatar;
    QGraphicsPixmapItem *kingdom;
    bool use_skill;
    QComboBox *sort_combobox;
    CardItem *weapon, *armor, *defensive_horse, *offensive_horse;
    QStack<CardItem *> judging_area;

    // for pendings
    QList<CardItem *> pendings;
    const ViewAsSkill *view_as_skill;

    void adjustCards();
    void adjustCards(const QList<CardItem *> &list, int y);
    void installDelayedTrick(CardItem *card);
    void drawEquip(QPainter *painter, CardItem *equip, int order);

private slots:
    void sortCards();
    void doPending(CardItem *card_item, bool add_to_pendings);

signals:
    void card_selected(const Card *card);
    void card_to_use();
};

#endif // DASHBOARD_H
