#include "roomscene.h"
#include "settings.h"
#include "carditem.h"
#include "engine.h"
#include "optionbutton.h"
#include "cardoverview.h"
#include "distanceviewdialog.h"

extern audiere::AudioDevicePtr Device;

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QStatusBar>
#include <QListWidget>
#include <QHBoxLayout>
#include <QSignalMapper>
#include <QKeyEvent>
#include <QCheckBox>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QMenu>
#include <QGroupBox>
#include <QLineEdit>
#include <QInputDialog>
#include <QLabel>

static const QPointF DiscardedPos(-494, -115);
static const QPointF DrawPilePos(893, -235);
static QSize GeneralSize(200 * 0.8, 290 * 0.8);

RoomScene::RoomScene(int player_count, QMainWindow *main_window)
    :focused(NULL), main_window(main_window)
{
    ClientInstance->setParent(this);
    setBackgroundBrush(Config.BackgroundBrush);

    // create pile
    pile = new Pixmap(":/pile.png");
    addItem(pile);
    pile->setPos(387, -132);

    pile_number_item = addText("", Config.SmallFont);
    pile_number_item->setPos(pile->pos());

    // create photos
    int i;
    for(i=0;i<player_count-1;i++){
        Photo *photo = new Photo(i);
        photos << photo;
        addItem(photo);
    }   

    // create dashboard
    dashboard = new Dashboard;
    addItem(dashboard);
    dashboard->setPos(Config.Rect.x(), Config.Rect.bottom() - dashboard->boundingRect().height());
    dashboard->setPlayer(Self);
    connect(Self, SIGNAL(general_changed()), dashboard, SLOT(updateAvatar()));
    connect(Self, SIGNAL(general_changed()), this, SLOT(updateSkillButtons()));
    connect(dashboard, SIGNAL(card_selected(const Card*)), this, SLOT(enableTargets(const Card*)));
    connect(dashboard, SIGNAL(card_to_use()), this, SLOT(doOkButton()));

    // add role combobox
    role_combobox = new QComboBox;
    role_combobox->addItem(tr("Your role"));
    role_combobox->addItem(tr("Unknown"));
    connect(Self, SIGNAL(role_changed(QString)), this, SLOT(updateRoleComboBox(QString)));    

    QGraphicsLinearLayout *button_layout = new QGraphicsLinearLayout(Qt::Horizontal);

    // add trust buttons
    trust_button = new QPushButton(tr("Trust"));
    trust_button->setEnabled(false);    
    QGraphicsProxyWidget *trust_widget = addWidget(trust_button);
    trust_widget->setPos(dashboard->pos());
    trust_widget->moveBy(0, -30);

    connect(trust_button, SIGNAL(clicked()), ClientInstance, SLOT(trust()));
    connect(Self, SIGNAL(state_changed()), this, SLOT(updateTrustButton()));

    // add other buttons
    ok_button = new QPushButton(tr("OK"));
    cancel_button = new QPushButton(tr("Cancel"));
    discard_button = new QPushButton(tr("Discard cards"));

    ok_button->setEnabled(false);
    cancel_button->setEnabled(false);
    discard_button->setEnabled(false);

    connect(ok_button, SIGNAL(clicked()), this, SLOT(doOkButton()));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(doCancelButton()));
    connect(discard_button, SIGNAL(clicked()), this, SLOT(doDiscardButton()));

    button_layout->addItem(addWidget(ok_button));
    button_layout->addItem(addWidget(cancel_button));
    button_layout->addItem(addWidget(discard_button));

    QGraphicsWidget *form = new QGraphicsWidget(dashboard);
    form->setLayout(button_layout);
    form->setPos(dashboard->boundingRect().width() - button_layout->preferredWidth(), -25);

    discard_skill = new DiscardSkill;
    yiji_skill = new YijiViewAsSkill;
    choose_skill = new ChoosePlayerSkill;

    known_cards_menu = new QMenu(main_window);

    // get dashboard's avatar
    avatar = dashboard->getAvatar();

    // do signal-slot connections
    connect(ClientInstance, SIGNAL(player_added(ClientPlayer*)), SLOT(addPlayer(ClientPlayer*)));
    connect(ClientInstance, SIGNAL(player_removed(QString)), SLOT(removePlayer(QString)));    
    connect(ClientInstance, SIGNAL(generals_got(QList<const General*>)), this, SLOT(chooseGeneral(QList<const General*>)));
    connect(ClientInstance, SIGNAL(seats_arranged(QList<const ClientPlayer*>)), SLOT(arrangeSeats(QList<const ClientPlayer*>)));
    connect(ClientInstance, SIGNAL(status_changed(Client::Status)), this, SLOT(updateStatus(Client::Status)));
    connect(ClientInstance, SIGNAL(avatars_hiden()), this, SLOT(hideAvatars()));
    connect(ClientInstance, SIGNAL(hp_changed(QString,int)), this, SLOT(changeHp(QString,int)));
    connect(ClientInstance, SIGNAL(message_changed(QString)), this, SLOT(changeMessage(QString)));
    connect(ClientInstance, SIGNAL(pile_cleared()), this, SLOT(clearPile()));
    connect(ClientInstance, SIGNAL(pile_num_set(int)), this, SLOT(setPileNumber(int)));
    connect(ClientInstance, SIGNAL(player_killed(QString)), this, SLOT(killPlayer(QString)));
    connect(ClientInstance, SIGNAL(card_shown(QString,int)), this, SLOT(showCard(QString,int)));
    connect(ClientInstance, SIGNAL(guanxing(QList<int>)), this, SLOT(doGuanxing(QList<int>)));
    connect(ClientInstance, SIGNAL(gongxin(QList<int>)), this, SLOT(doGongxin(QList<int>)));
    connect(ClientInstance, SIGNAL(words_spoken(QString,QString)), this, SLOT(speak(QString,QString)));
    connect(ClientInstance, SIGNAL(focus_moved(QString)), this, SLOT(moveFocus(QString)));

    connect(ClientInstance, SIGNAL(game_started()), this, SLOT(onGameStart()));
    connect(ClientInstance, SIGNAL(game_over(bool,QList<bool>)), this, SLOT(onGameOver(bool,QList<bool>)));
    connect(ClientInstance, SIGNAL(standoff()), this, SLOT(onStandoff()));

    connect(ClientInstance, SIGNAL(card_moved(CardMoveStructForClient)), this, SLOT(moveCard(CardMoveStructForClient)));
    connect(ClientInstance, SIGNAL(n_cards_moved(int,QString,QString)), this, SLOT(moveNCards(int,QString,QString)));

    connect(ClientInstance, SIGNAL(cards_drawed(QList<const Card*>)), this, SLOT(drawCards(QList<const Card*>)));
    connect(ClientInstance, SIGNAL(n_cards_drawed(ClientPlayer*,int)), SLOT(drawNCards(ClientPlayer*,int)));

    connect(ClientInstance, SIGNAL(ag_filled(QList<int>)), this, SLOT(fillAmazingGrace(QList<int>)));
    connect(ClientInstance, SIGNAL(ag_taken(const ClientPlayer*,int)), this, SLOT(takeAmazingGrace(const ClientPlayer*,int)));
    connect(ClientInstance, SIGNAL(ag_cleared()), this, SLOT(clearAmazingGrace()));

    connect(ClientInstance, SIGNAL(skill_attached(QString)), this, SLOT(attachSkill(QString)));
    connect(ClientInstance, SIGNAL(skill_detached(QString)), this, SLOT(detachSkill(QString)));

    daqiao = new Daqiao;
    daqiao->shift();
    daqiao->moveBy(-100, 0);
    daqiao->hide();
    addItem(daqiao);

    connect(ClientInstance, SIGNAL(prompt_changed(QString)), daqiao, SLOT(setContent(QString)));

    // log box
    log_box = new ClientLogBox;
    QGraphicsProxyWidget *log_box_widget = addWidget(log_box);
    log_box_widget->setPos(80, DiscardedPos.y() - 10);
    log_box_widget->setZValue(-2.0);
    connect(ClientInstance, SIGNAL(log_received(QString)), log_box, SLOT(appendLog(QString)));

    // chat box
    chat_box = new QTextEdit;
    QGraphicsProxyWidget *chat_box_widget = addWidget(chat_box);
    chat_box_widget->setPos(-300, DiscardedPos.y() - 10);
    chat_box_widget->setZValue(-2.0);
    QPalette palette;
    palette.setBrush(QPalette::Base, Config.BackgroundBrush);
    chat_box->resize(300, 200);
    chat_box->setPalette(palette);
    chat_box->setReadOnly(true);

    // chat edit
    chat_edit = new QLineEdit;
    chat_edit->setPlaceholderText(tr("Please enter text to chat ... "));
    QGraphicsProxyWidget *chat_edit_widget = addWidget(chat_edit);
    chat_edit_widget->setPos(dashboard->pos());
    connect(chat_edit, SIGNAL(returnPressed()), this, SLOT(speak()));

    const qreal photo_width = photos.first()->boundingRect().width();
    const qreal start_x = (Config.Rect.width() - photo_width*photos.length())/2 + Config.Rect.x();

    for(i=0;i<photos.length();i++){
        Photo *photo = photos[i];
        qreal x = i * photo_width + start_x;
        qreal y =  Config.Rect.y();
        photo->setPos(x, y);
    }

    memory = new QSharedMemory("QSanguosha", this);

    {
        progress_bar = new QProgressBar;
        progress_bar->setOrientation(Qt::Vertical);
        progress_bar->setMinimum(0);
        progress_bar->setMaximum(100);
        timer_id = 0;

        QGraphicsProxyWidget *widget = addWidget(progress_bar);
        widget->setParentItem(dashboard);
        widget->setPos(avatar->pos());
        widget->moveBy(-25, 0);

        progress_bar->hide();
    }
}

void RoomScene::addPlayer(ClientPlayer *player){
    int i;
    for(i=0; i<photos.length(); i++){
        Photo *photo = photos[i];
        if(photo->getPlayer() == NULL){
            photo->setPlayer(player);

            name2photo[player->objectName()] = photo;

            Sanguosha->playEffect("audio/add-player.wav");

            return;
        }
    }
}

void RoomScene::removePlayer(const QString &player_name){
    Photo *photo = name2photo[player_name];
    if(photo){
        photo->setPlayer(NULL);
        name2photo.remove(player_name);
    }
}

void RoomScene::arrangeSeats(const QList<const ClientPlayer*> &seats){
    // rearrange the photos
    Q_ASSERT(seats.length() == photos.length());

    int i, j;
    for(i=0; i<seats.length(); i++){
        const Player *player = seats.at(i);
        for(j=0; j<photos.length(); j++){
            if(photos.at(j)->getPlayer() == player){
                photos.swap(i, j);
                break;
            }
        }
    }

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);

    const qreal photo_width = photos.first()->boundingRect().width();
    const qreal start_x = (Config.Rect.width() - photo_width*photos.length())/2 + Config.Rect.x();

    for(i=0;i<photos.length();i++){
        Photo *photo = photos[i];
        qreal x = i * photo_width + start_x;

        QPropertyAnimation *translation = new QPropertyAnimation(photo, "x");
        translation->setEndValue(x);
        translation->setEasingCurve(QEasingCurve::OutBounce);

        group->addAnimation(translation);
    }

    group->start(QAbstractAnimation::DeleteWhenStopped);

    // set item to player mapping
    item2player.insert(avatar, Self);
    connect(avatar, SIGNAL(selected_changed()), this, SLOT(updateSelectedTargets()));
    foreach(Photo *photo, photos){
        item2player.insert(photo, photo->getPlayer());
        connect(photo, SIGNAL(selected_changed()), this, SLOT(updateSelectedTargets()));
    }
}

void RoomScene::drawCards(const QList<const Card *> &cards){
    foreach(const Card * card, cards){
        CardItem *item = new CardItem(card);
        item->setPos(DrawPilePos);
        dashboard->addCardItem(item);
    }

    setPileNumber(pile_number - cards.length());

    log_box->appendLog("#DrawNCards", Self->getGeneralName(), QStringList(), QString(), QString::number(cards.length()));
}

void RoomScene::drawNCards(ClientPlayer *player, int n){
    QSequentialAnimationGroup *group =  new QSequentialAnimationGroup;
    QParallelAnimationGroup *moving = new QParallelAnimationGroup;
    QParallelAnimationGroup *disappering = new QParallelAnimationGroup;

    Photo *photo = name2photo[player->objectName()];
    int i;
    for(i=0; i<n; i++){
        Pixmap *pixmap = new Pixmap(":/card-back.png");
        addItem(pixmap);

        QPropertyAnimation *ugoku = new QPropertyAnimation(pixmap, "pos");
        ugoku->setStartValue(QPointF(387, -162));
        ugoku->setDuration(500);
        ugoku->setEasingCurve(QEasingCurve::OutBounce);
        ugoku->setEndValue(photo->pos() + QPointF(10 *i, 0));

        QPropertyAnimation *kieru = new QPropertyAnimation(pixmap, "opacity");
        kieru->setDuration(900);
        kieru->setKeyValueAt(0.8, 1.0);
        kieru->setEndValue(0.0);

        moving->addAnimation(ugoku);
        disappering->addAnimation(kieru);

        connect(kieru, SIGNAL(finished()), pixmap, SLOT(deleteLater()));
    }

    group->addAnimation(moving);
    group->addAnimation(disappering);

    group->start(QAbstractAnimation::DeleteWhenStopped);

    photo->update();

    setPileNumber(pile_number - n);

    // log_box->appendLog(player, tr("%from drawed %arg cards"), QString::number(n));
    log_box->appendLog(
            "#DrawNCards",
            player->getGeneralName(),
            QStringList(),
            QString(),
            QString::number(n)
            );
}

void RoomScene::mousePressEvent(QGraphicsSceneMouseEvent *event){
    QGraphicsItem* item = itemAt(event->scenePos().x(), event->scenePos().y());

    if(item && item2player.contains(item) && item->isEnabled() && item->flags() & QGraphicsItem::ItemIsSelectable)
        item->setSelected(!item->isSelected());
    else
        QGraphicsScene::mousePressEvent(event);
}

void RoomScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    QGraphicsScene::mouseMoveEvent(event);

    QGraphicsObject *obj = static_cast<QGraphicsObject*>(focusItem());
    CardItem *card_item = qobject_cast<CardItem*>(obj);
    if(!card_item || !card_item->isUnderMouse())
        return;

    foreach(Photo *photo, photos){
        if(photo->isUnderMouse()){
            photo->setSelected(true);
            break;
        }
    }

    if(avatar->isUnderMouse()){
        avatar->setSelected(true);
    }
}

void RoomScene::keyReleaseEvent(QKeyEvent *event){
    if(!Config.EnableHotKey)
        return;

    if(chat_edit->hasFocus())
        return;

    bool control_is_down = event->modifiers() & Qt::ControlModifier;

    switch(event->key()){        
    case Qt::Key_F1:
    case Qt::Key_F2:
    case Qt::Key_F3:
    case Qt::Key_F4: dashboard->sort(event->key() - Qt::Key_F1); break;

    case Qt::Key_F5:
    case Qt::Key_F6:
    case Qt::Key_F7:
    case Qt::Key_F8: break;

    case Qt::Key_F9:
    case Qt::Key_F10:
    case Qt::Key_F11:
    case Qt::Key_F12: clickSkillButton(event->key() - Qt::Key_F9); break;

    case Qt::Key_S: dashboard->selectCard("slash");  break;
    case Qt::Key_J: dashboard->selectCard("jink"); break;
    case Qt::Key_P: dashboard->selectCard("peach"); break;

    case Qt::Key_E: dashboard->selectCard("equip"); break;
    case Qt::Key_W: dashboard->selectCard("weapon"); break;
    case Qt::Key_H: dashboard->selectCard("horse"); break;

    case Qt::Key_T: dashboard->selectCard("trick"); break;
    case Qt::Key_A: dashboard->selectCard("aoe"); break;
    case Qt::Key_N: dashboard->selectCard("nullification"); break;    
    case Qt::Key_Q: dashboard->selectCard("snatch"); break;
    case Qt::Key_C: dashboard->selectCard("dismantlement"); break;
    case Qt::Key_U: dashboard->selectCard("duel"); break;
    case Qt::Key_L: dashboard->selectCard("lightning"); break;
    case Qt::Key_I: dashboard->selectCard("indulgence"); break;
    case Qt::Key_R: dashboard->selectCard("collateral"); break;
    case Qt::Key_Y: dashboard->selectCard("god_salvation"); break;

    case Qt::Key_Left: dashboard->selectCard(".", false); break;
    case Qt::Key_Right:
    case Qt::Key_Space:  dashboard->selectCard("."); break; // iterate all cards
    case Qt::Key_F:  break; // fix the selected

    case Qt::Key_G: selectNextTarget(control_is_down); break; // iterate generals

    case Qt::Key_Return : {
            if(ok_button->isEnabled())
                doOkButton();
            break;
        }

    case Qt::Key_Escape : {
            if(!discarded_queue.isEmpty() && discarded_queue.first()->rotation() == 0.0)
                hideDiscards();
            else if(ClientInstance->getStatus() == Client::Playing){
                dashboard->unselectAll();
                enableTargets(NULL);
            }
            break;
        }

    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7: selectTarget(event->key() - Qt::Key_0, control_is_down); break;

#ifndef QT_NO_DEBUG
    case Qt::Key_D:{
            int max = Sanguosha->getCardCount();
            bool ok;
            int card_id = QInputDialog::getInteger(main_window, tr("Get card"), tr("Plase input the card's id"), 1, 1, max, 1, &ok);
            if(ok)
                ClientInstance->requestCard(card_id - 1);

            break;
        }
#endif

    }
}

void RoomScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event){
    QGraphicsScene::contextMenuEvent(event);

    QGraphicsItem *item = itemAt(event->scenePos());
    if(!item)
        return;

    QGraphicsObject *item_obj = static_cast<QGraphicsObject *>(item);
    Photo *photo = qobject_cast<Photo *>(item_obj);

    if(!photo)
        return;

    const ClientPlayer *player = photo->getPlayer();
    if(player){
        QList<const Card *> cards = player->getCards();
        QMenu *menu = known_cards_menu;
        menu->clear();
        menu->setTitle(player->objectName());

        if(cards.isEmpty()){
            menu->addAction(tr("There is no known cards"));
        }else{
            foreach(const Card *card, cards)
                menu->addAction(card->getSuitIcon(), card->getFullName());
        }

        menu->popup(event->screenPos());
    }
}

void RoomScene::timerEvent(QTimerEvent *event){
    int step = 100 / double(Config.OperationTimeout * 5);
    int new_value = progress_bar->value() + step;
    new_value = qMin(progress_bar->maximum(), new_value);
    progress_bar->setValue(new_value);

    if(new_value >= progress_bar->maximum()){
        killTimer(event->timerId());
        timer_id = 0;
        doTimeout();
    }else
        progress_bar->setValue(new_value);
}

void RoomScene::chooseGeneral(const QList<const General *> &generals){
    if(photos.length()>1)
        changeMessage(tr("Please wait for other players choosing their generals"));

    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(tr("Choose general"));

    QSignalMapper *mapper = new QSignalMapper(dialog);
    QList<OptionButton *> buttons;
    foreach(const General *general, generals){
        QString icon_path = general->getPixmapPath("card");
        QString caption = Sanguosha->translate(general->objectName());
        OptionButton *button = new OptionButton(icon_path, caption);
        button->setToolTip(general->getSkillDescription());
        button->setIconSize(GeneralSize);
        buttons << button;

        mapper->setMapping(button, general->objectName());
        connect(button, SIGNAL(double_clicked()), mapper, SLOT(map()));
        connect(button, SIGNAL(double_clicked()), dialog, SLOT(accept()));

        // special case

        if(Self->getRole() == "lord" && general->objectName() == "shencaocao"){
            button->setEnabled(false);
        }
    }

    QLayout *layout = NULL;
    const static int columns = 5;
    if(generals.length() <= columns){
        layout = new QHBoxLayout;
        foreach(OptionButton *button, buttons)
            layout->addWidget(button);
    }else{
        QGridLayout *grid_layout = new QGridLayout;
        layout = grid_layout;

        int i;
        for(i=0; i<buttons.length(); i++){
            int row = i / columns;
            int column = i % columns;
            grid_layout->addWidget(buttons.at(i), row, column);
        }
    }

    mapper->setMapping(dialog, generals.first()->objectName());
    connect(dialog, SIGNAL(rejected()), mapper, SLOT(map()));

    connect(mapper, SIGNAL(mapped(QString)), ClientInstance, SLOT(itemChosen(QString)));

#ifndef QT_NO_DEBUG

    QLineEdit *cheat_edit = new QLineEdit;
    layout->addWidget(cheat_edit);

    connect(cheat_edit, SIGNAL(returnPressed()), ClientInstance, SLOT(cheatChoose()));
    connect(cheat_edit, SIGNAL(returnPressed()), dialog, SLOT(accept()));

#endif

    QVBoxLayout *dialog_layout = new QVBoxLayout;
    dialog_layout->addLayout(layout);
    QLabel *role_label = new QLabel(tr("Your role is %1").arg(Sanguosha->translate(Self->getRole())));
    dialog_layout->addWidget(role_label);

    dialog->setLayout(dialog_layout);
    dialog->exec();
}

void RoomScene::changeMessage(const QString &message){
    if(message.isNull())
        main_window->statusBar()->clearMessage();
    else
        main_window->statusBar()->showMessage(message);
}

void RoomScene::viewDiscards(){
    if(ClientInstance->discarded_list.isEmpty()){
        QMessageBox::information(NULL, tr("No discarded cards"), tr("There are no discarded cards yet"));
        return;
    }

    if(sender()->inherits("CardItem")){
        int i;
        for(i=0; i< discarded_queue.length(); i++){
            CardItem *card_item = discarded_queue.at(i);
            card_item->setRotation(0);
            card_item->setHomePos(QPointF(card_item->x() + i*card_item->boundingRect().width(), card_item->y()));
            card_item->goBack();
        }
    }else{
        CardOverview *overview = new CardOverview;
        overview->loadFromList(ClientInstance->discarded_list);
        overview->show();
    }
}

void RoomScene::hideDiscards(){
    foreach(CardItem *card_item, discarded_queue){
        card_item->setRotation(qrand() % 359 + 1);
        card_item->setHomePos(DiscardedPos);
        card_item->goBack();
    }
}

CardItem *RoomScene::takeCardItem(ClientPlayer *src, Player::Place src_place, int card_id){
    if(src){
        // from players

        if(src == Self){
            CardItem *card_item = dashboard->takeCardItem(card_id, src_place);
            card_item->setOpacity(1.0);
            card_item->setParentItem(NULL);
            card_item->setPos(dashboard->mapToScene(card_item->pos()));
            return card_item;
        }else{
            Photo *photo = name2photo.value(src->objectName(), NULL);
            if(photo)
                return photo->takeCardItem(card_id, src_place);
            else
                return NULL;
        }
    }

    // from system, i.e. from draw pile or discard pile
    CardItem *card_item = NULL;

    // from draw pile
    if(src_place == Player::DrawPile){
        card_item = new CardItem(Sanguosha->getCard(card_id));
        card_item->setPos(DrawPilePos);
        setPileNumber(pile_number - 1);
        return card_item;
    }

    // from discard pile
    int i;
    for(i=0; i<discarded_queue.length(); i++){
        if(discarded_queue.at(i)->getCard()->getId() == card_id){
            card_item = discarded_queue.takeAt(i);
        }
    }

    if(card_item == NULL){
        card_item = new CardItem(Sanguosha->getCard(card_id));
        card_item->setPos(DiscardedPos);
    }

    card_item->disconnect(this);

    return card_item;
}

void RoomScene::moveNCards(int n, const QString &from, const QString &to){
    Photo *src = name2photo.value(from, NULL);
    Photo *dest = name2photo.value(to, NULL);

    if(src == NULL || dest == NULL){
        QMessageBox::warning(main_window, tr("Warning"), tr("Can not find moving targets!"));
        return;
    }

    QParallelAnimationGroup *group = new QParallelAnimationGroup;

    int i;
    for(i=0; i<n; i++){
        Pixmap *card_pixmap = new Pixmap(":/card-back.png");
        addItem(card_pixmap);

        QPropertyAnimation *ugoku = new QPropertyAnimation(card_pixmap, "pos");
        ugoku->setStartValue(src->pos());
        ugoku->setEndValue(dest->pos() + QPointF(i * 10, 0));
        ugoku->setDuration(1000);

        QPropertyAnimation *kieru = new QPropertyAnimation(card_pixmap, "opacity");
        kieru->setStartValue(1.0);
        kieru->setKeyValueAt(0.8, 1.0);
        kieru->setEndValue(0.0);
        kieru->setDuration(1000);

        group->addAnimation(ugoku);
        group->addAnimation(kieru);

        connect(group, SIGNAL(finished()), card_pixmap, SLOT(deleteLater()));       
    }

    group->start(QAbstractAnimation::DeleteWhenStopped);

    src->update();
    dest->update();

    QString type = "#MoveNCards";
    QString from_general = src->getPlayer()->getGeneralName();
    QStringList tos;
    tos << dest->getPlayer()->getGeneralName();
    QString n_str = QString::number(n);
    log_box->appendLog(type, from_general, tos, QString(), n_str);
}

void RoomScene::moveCardToDrawPile(const QString &from){
    Photo *src = name2photo.value(from, NULL);

    Q_ASSERT(src != NULL);

    pile_number ++;
    setPileNumber(pile_number);

    QParallelAnimationGroup *group = new QParallelAnimationGroup;

    Pixmap *card_pixmap = new Pixmap(":/card-back.png");
    addItem(card_pixmap);

    QPropertyAnimation *ugoku = new QPropertyAnimation(card_pixmap, "pos");
    ugoku->setStartValue(src->pos());
    ugoku->setEndValue(DrawPilePos + dashboard->pos());
    ugoku->setDuration(1000);

    QPropertyAnimation *kieru = new QPropertyAnimation(card_pixmap, "opacity");
    kieru->setStartValue(1.0);
    kieru->setKeyValueAt(0.8, 1.0);
    kieru->setEndValue(0.0);
    kieru->setDuration(1000);

    group->addAnimation(ugoku);
    group->addAnimation(kieru);

    connect(group, SIGNAL(finished()), card_pixmap, SLOT(deleteLater()));

    group->start(QAbstractAnimation::DeleteWhenStopped);

    src->update();
}

void RoomScene::moveCard(const CardMoveStructForClient &move){
    ClientPlayer *src = move.from;
    ClientPlayer *dest = move.to;
    Player::Place src_place = move.from_place;
    Player::Place dest_place = move.to_place;
    int card_id = move.card_id;    

    CardItem *card_item = takeCardItem(src, src_place, card_id);
    if(card_item->scene() == NULL)
        addItem(card_item);

    putCardItem(dest, dest_place, card_item);

    QString card_str = QString::number(card_id);
    if(src && dest){
        if(src == dest)
            return;

        // both src and dest are player
        QString type;
        if(dest_place == Player::Judging){
            const Card *trick = Sanguosha->getCard(move.card_id);
            if(trick->objectName() == "lightning")
                type = "$LightningMove";
            else
                type = "$PasteCard";
        }else if(dest_place == Player::Hand)
            type = "$MoveCard";

        QString from_general = src->getGeneralName();
        QStringList tos;
        tos << dest->getGeneralName();
        log_box->appendLog(type, from_general, tos, card_str);
    }else if(src){
        // src throw card
        if(src->getPhase() == Player::Discard){
            QString type = "$DiscardCard";
            QString from_general = src->getGeneralName();
            log_box->appendLog(type, from_general, QStringList(), card_str);
        }
    }else if(dest){
        if(src_place == Player::DiscardedPile){
            QString type = "$RecycleCard";
            QString from_general = dest->getGeneralName();
            log_box->appendLog(type, from_general, QStringList(), card_str);
        }
    }
}

void RoomScene::putCardItem(const ClientPlayer *dest, Player::Place dest_place, CardItem *card_item){
    static QString install_equip_source("audio/install-equip.wav");

    if(dest == NULL){
        if(dest_place == Player::DiscardedPile){
            card_item->setHomePos(DiscardedPos);
            card_item->setRotation(qrand() % 359 + 1);
            card_item->goBack();
            card_item->setEnabled(true);

            card_item->setFlag(QGraphicsItem::ItemIsFocusable, false);

            card_item->setZValue(0.1*ClientInstance->discarded_list.length());
            discarded_queue.enqueue(card_item);

            if(discarded_queue.length() > 8){
                CardItem *first = discarded_queue.dequeue();
                delete first;
            }

            connect(card_item, SIGNAL(show_discards()), this, SLOT(viewDiscards()));
            connect(card_item, SIGNAL(hide_discards()), this, SLOT(hideDiscards()));
        }else if(dest_place == Player::DrawPile){
            QPointF pos = DrawPilePos;
            pos += dashboard->pos();
            card_item->setHomePos(pos);
            card_item->goBack(true);

            pile_number ++;
            setPileNumber(pile_number);
        }

    }else if(dest->objectName() == Self->objectName()){
        switch(dest_place){
        case Player::Equip:{
                dashboard->installEquip(card_item);
                Sanguosha->playEffect(install_equip_source);
                break;
            }

        case Player::Hand:{
                dashboard->addCardItem(card_item);
                break;
            }

        case Player::Judging:{
                dashboard->installDelayedTrick(card_item);
                break;
            }

        default:
            ;
            // FIXME
        }
    }else{
        Photo *photo = name2photo.value(dest->objectName(), NULL);
        if(photo){
            switch(dest_place){
            case Player::Equip:
                photo->installEquip(card_item);
                Sanguosha->playEffect(install_equip_source);
                break;
            case Player::Hand:
                photo->addCardItem(card_item);
                break;
            case Player::Judging:
                photo->installDelayedTrick(card_item);
                break;
            default:
                ;
            }
        }

        photo->update();
    }
}

void RoomScene::updateSkillButtons(){
    const Player *player = qobject_cast<const Player *>(sender());
    const General *general = player->getGeneral();
    main_window->setStatusBar(NULL);
    skill_buttons.clear();
    button2skill.clear();
    QStatusBar *status_bar = main_window->statusBar();

    const QList<const Skill*> &skills = general->findChildren<const Skill *>();
    foreach(const Skill* skill, skills){
        if(skill->objectName().startsWith("#"))
            continue;

        QAbstractButton *button = NULL;
        QString skill_name = Sanguosha->translate(skill->objectName());
        if(skill->inherits("TriggerSkill")){
            const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
            switch(trigger_skill->getFrequency()){
            case TriggerSkill::Frequent:{
                    QCheckBox *checkbox = new QCheckBox(skill_name);

                    checkbox->setObjectName(skill->objectName());
                    checkbox->setChecked(false);
                    connect(checkbox, SIGNAL(stateChanged(int)), ClientInstance, SLOT(updateFrequentFlags(int)));
                    checkbox->setChecked(true);

                    button = checkbox;
                    break;
            }
            case TriggerSkill::NotFrequent:{
                    const ViewAsSkill *view_as_skill = trigger_skill->getViewAsSkill();
                    button = new QPushButton(skill_name);
                    if(view_as_skill){
                        button2skill.insert(button, view_as_skill);
                        connect(button, SIGNAL(clicked()), this, SLOT(doSkillButton()));
                    }

                    break;
            }

            case TriggerSkill::Compulsory:{
                    button = new QPushButton(skill_name + tr(" [Compulsory]"));
                    button->setEnabled(false);
                    break;
                }
            }
        }else if(skill->inherits("ViewAsSkill")){
            button = new QPushButton(skill_name);
            button2skill.insert(button, qobject_cast<const ViewAsSkill *>(skill));
            connect(button, SIGNAL(clicked()), this, SLOT(doSkillButton()));
        }else{
            button = new QPushButton(skill_name);
            if(skill->getFrequency() == Skill::Compulsory){
                button->setText(skill_name + tr(" [Compulsory]"));
                button->setEnabled(false);
            }
        }

        button->setObjectName(skill->objectName());
        if(skill->isLordSkill())
            button->setText(button->text() + tr(" [Lord Skill]"));

        button->setToolTip(skill->getDescription());

        status_bar->addPermanentWidget(button);
        skill_buttons << button;
    }

    status_bar->addPermanentWidget(role_combobox);

    // disable all skill buttons
    foreach(QAbstractButton *button, skill_buttons)
        button->setDisabled(true);
}

void RoomScene::updateRoleComboBox(const QString &new_role){
    role_combobox->setItemText(1, Sanguosha->translate(new_role));
    role_combobox->setItemIcon(1, QIcon(QString(":/roles/%1.png").arg(new_role)));
    role_combobox->setCurrentIndex(1);

    if(new_role != "lord"){
        foreach(QAbstractButton *button, skill_buttons){
            const Skill *skill = Sanguosha->getSkill(button->objectName());

            Q_ASSERT(skill != NULL);
            if(skill->isLordSkill())
                button->hide();
        }
    }
}

void RoomScene::clickSkillButton(int order){
    if(order >= 0 && order < skill_buttons.length())
        skill_buttons.at(order)->click();
}

void RoomScene::enableTargets(const Card *card){
    selected_targets.clear();

    // unset avatar and all photo
    foreach(QGraphicsItem *item, item2player.keys()){
        item->setSelected(false);
    }

    if(card == NULL){
        foreach(QGraphicsItem *item, item2player.keys()){
            item->setEnabled(false);
            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        }

        changeMessage();
        ok_button->setEnabled(false);
        return;
    }

    changeMessage(tr("You choosed card [%1]").arg(card->getName()));

    if(card->targetFixed() || ClientInstance->noTargetResponsing()){
        foreach(QGraphicsItem *item, item2player.keys()){
            item->setEnabled(true);
            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        }

        ok_button->setEnabled(true);
        return;
    }

    updateTargetsEnablity(card);

    if(Config.EnableAutoTarget)
        selectNextTarget(false);

    ok_button->setEnabled(card->targetsFeasible(selected_targets));
}

void RoomScene::updateTargetsEnablity(const Card *card){
    QMapIterator<QGraphicsItem *, const ClientPlayer *> itor(item2player);
    while(itor.hasNext()){
        itor.next();

        QGraphicsItem *item = itor.key();
        const ClientPlayer *player = itor.value();

        if(item->isSelected())
            continue;

        if(card->targetFilter(selected_targets, player)){
            item->setEnabled(true);
            item->setFlag(QGraphicsItem::ItemIsSelectable, true);
        }else{
            item->setEnabled(false);
            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        }
    }
}

void RoomScene::updateSelectedTargets(){
    Pixmap *item = qobject_cast<Pixmap *>(sender());

    if(item == NULL)
        return;

    const Card *card = dashboard->getSelected();
    if(card){
        const ClientPlayer *player = item2player[item];
        if(item->isSelected()){
            selected_targets.append(player);
        }else{
            selected_targets.removeOne(player);
        }

        updateTargetsEnablity(card);
        ok_button->setEnabled(card->targetsFeasible(selected_targets));
    }else{
        selected_targets.clear();
        changeMessage();
    }
}

void RoomScene::useSelectedCard(){
    const Card *card = dashboard->getSelected();
    if(card)
        useCard(card);
    else
        changeMessage(tr("You didn't choose any card to use yet!"));
}

void RoomScene::useCard(const Card *card){
    if(card->targetFixed() || card->targetsFeasible(selected_targets))
        ClientInstance->useCard(card, selected_targets);
    else
        changeMessage(tr("Not enough targets"));

    enableTargets(NULL);
}

void RoomScene::callViewAsSkill(){
    const Card *card = dashboard->pendingCard();

    if(card == NULL){
        changeMessage(tr("Not enough cards to call skill"));
        return;
    }

    if(card->isAvailable()){
        // use card
        dashboard->stopPending();
        useCard(card);
    }else{
        changeMessage(tr("Card [%1] can not be used right now").arg(card->getName()));
    }
}

void RoomScene::cancelViewAsSkill(){
    const ViewAsSkill *skill = dashboard->currentSkill();
    dashboard->stopPending();
    QAbstractButton *button = button2skill.key(skill, NULL);

    if(button)
        updateStatus(ClientInstance->getStatus());
}

void RoomScene::selectTarget(int order, bool multiple){
    QGraphicsItem *to_select = NULL;

    if(order == 0)
        to_select = avatar;
    else if(order > 0 && order <= photos.length())
        to_select = photos.at(order - 1);

    if(!multiple)
        unselectAllTargets(to_select);

    if(to_select)
        to_select->setSelected(! to_select->isSelected());
}

void RoomScene::selectNextTarget(bool multiple){
    if(!multiple)
        unselectAllTargets();

    QList<QGraphicsItem *> targets;
    foreach(Photo *photo, photos){
        if(photo->flags() & QGraphicsItem::ItemIsSelectable)
            targets << photo;
    }

    if(avatar->flags() & QGraphicsItem::ItemIsSelectable)
        targets << avatar;

    int i, j;
    for(i=0; i<targets.length(); i++){
        if(targets.at(i)->isSelected()){
            for(j=i+1; j<targets.length(); j++){
                if(!targets.at(j)->isSelected()){
                    targets.at(j)->setSelected(true);
                    return;
                }
            }
        }
    }

    foreach(QGraphicsItem *target, targets){
        if(!target->isSelected()){
            target->setSelected(true);
            break;
        }
    }
}

void RoomScene::unselectAllTargets(const QGraphicsItem *except){
    if(avatar != except)
        avatar->setSelected(false);

    foreach(Photo *photo, photos){
        if(photo != except)
            photo->setSelected(false);
    }
}

void RoomScene::doTimeout(){
    switch(ClientInstance->getStatus()){
    case Client::Responsing:{
            doCancelButton();
            break;
        }

    case Client::Playing:{
            discard_button->click();
            break;
        }

    case Client::Discarding:{
            doCancelButton();

            break;
        }

    case Client::ExecDialog:{
            doCancelButton();

            break;
        }

    case Client::AskForSkillInvoke:{
            cancel_button->click();
            break;
        }

    case Client::AskForChoices:{
            ClientInstance->selectChoice(0);
            break;
        }

    case Client::AskForPlayerChoose:{
            ClientInstance->choosePlayer(NULL);
            dashboard->stopPending();
            daqiao->hide();
            break;
        }

    case Client::AskForAG:{
            foreach(CardItem *item, amazing_grace){
                if(item->isEnabled()){
                    ClientInstance->chooseAG(item->getCard()->getId());
                    break;
                }
            }

            break;
        }

    case Client::AskForYiji:{
            cancel_button->click();
            break;
        }

    case Client::AskForGuanxing:
    case Client::AskForGongxin:{
            ok_button->click();
            break;
        }

    default:
        break;
    }
}

void RoomScene::updateStatus(Client::Status status){
    switch(status){
    case Client::NotActive:{
            daqiao->hide();
            dashboard->disableAllCards();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            // enable all photos for mark roles
            foreach(Photo *photo, photos)
                photo->setEnabled(true);

            break;
        }

    case Client::Responsing: {
            if(ClientInstance->card_pattern.startsWith("@"))
                dashboard->disableAllCards();
            else
                dashboard->enableCards(ClientInstance->card_pattern);

            ok_button->setEnabled(false);

            if(ClientInstance->card_pattern.endsWith("!")){
                QRegExp rx("@@(\\w+)!");
                if(rx.exactMatch(ClientInstance->card_pattern)){
                    QString skill_name = rx.capturedTexts().at(1);
                    const Skill *skill = Sanguosha->getSkill(skill_name);
                    const ViewAsSkill *view_as_skill = qobject_cast<const ViewAsSkill *>(skill);
                    dashboard->startPending(view_as_skill);
                }
            }
            cancel_button->setEnabled(ClientInstance->refusable);
            discard_button->setEnabled(false);
            break;
        }

    case Client::Playing:{
            dashboard->enableCards();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(true);
            break;
        }

    case Client::Discarding:{
            ok_button->setEnabled(false);
            cancel_button->setEnabled(ClientInstance->refusable);
            discard_button->setEnabled(false);

            discard_skill->setNum(ClientInstance->discard_num);
            discard_skill->setIncludeEquip(ClientInstance->include_equip);
            discard_skill->setSuit(ClientInstance->discard_suit);
            dashboard->startPending(discard_skill);
            break;
        }

    case Client::ExecDialog:{
            ok_button->setEnabled(false);
            cancel_button->setEnabled(true);
            discard_button->setEnabled(false);

            break;
        }

    case Client::AskForSkillInvoke:{
            ok_button->setEnabled(true);
            cancel_button->setEnabled(true);
            discard_button->setEnabled(false);

            break;
        }

    case Client::AskForChoices:{
            ok_button->setEnabled(true);
            cancel_button->setEnabled(true);
            discard_button->setEnabled(false);

            break;
        }

    case Client::AskForPlayerChoose:{
            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            daqiao->setContent(tr("Please choose a player"));

            choose_skill->setPlayerNames(ClientInstance->players_to_choose);
            dashboard->startPending(choose_skill);

            break;
        }

    case Client::AskForAG:{
            dashboard->disableAllCards();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            foreach(CardItem *item, amazing_grace)
                connect(item, SIGNAL(double_clicked()), this, SLOT(chooseAmazingGrace()));

            break;
        }
    case Client::AskForYiji:{
            ok_button->setEnabled(false);
            cancel_button->setEnabled(true);
            discard_button->setEnabled(false);

            yiji_skill->setCards(ClientInstance->card_pattern);
            dashboard->startPending(yiji_skill);

            break;
        }
    case Client::AskForGuanxing:{
            ok_button->setEnabled(true);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            break;
        }

    case Client::AskForGongxin:{
            ok_button->setEnabled(true);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            break;
        }
    }

    foreach(QAbstractButton *button, skill_buttons){
        const ViewAsSkill *skill = button2skill.value(button, NULL);
        if(skill)
            button->setEnabled(skill->isAvailable());
        else if(button->inherits("QCheckBox"))
            button->setEnabled(true);
    }

    if(status != Client::NotActive && focused){
        focused->hideProcessBar();
    }

    if(Config.OperationNoLimit)
        return;

    // do timeout
    progress_bar->setValue(0);
    if(status == Client::NotActive){
        if(timer_id != 0){
            killTimer(timer_id);
            timer_id = 0;
        }
        progress_bar->hide();
    }else{
        timer_id = startTimer(200);
        progress_bar->show();
    }
}

void RoomScene::doSkillButton(){
    const ViewAsSkill *current = dashboard->currentSkill();
    if(current){
        dashboard->stopPending();
        QAbstractButton *button = button2skill.key(current);
        if(button)
            button->setEnabled(true);
    }

    QPushButton *button = qobject_cast<QPushButton *>(sender());
    const ViewAsSkill *skill = button2skill.value(button, NULL);

    if(skill){
        dashboard->startPending(skill);

        button->setEnabled(false);
        ok_button->setEnabled(false);
        cancel_button->setEnabled(true);

        const Card *card = dashboard->pendingCard();
        if(card && card->targetFixed()){
            useSelectedCard();
            dashboard->stopPending();
        }
    }
}

void RoomScene::updateTrustButton(){
    if(Self->getState() == "trust")
        trust_button->setText(tr("Cancel trust"));
    else
        trust_button->setText(tr("Trust"));
}

void RoomScene::doOkButton(){
    if(!ok_button->isEnabled())
        return;

    switch(ClientInstance->getStatus()){
    case Client::Playing:{
            useSelectedCard();
            break;
        }
    case Client::Responsing:{
            const Card *card = dashboard->getSelected();
            if(card){
                if(ClientInstance->noTargetResponsing())
                    ClientInstance->responseCard(card);
                else
                    ClientInstance->responseCard(card, selected_targets);
                daqiao->hide();
            }

            dashboard->unselectAll();
            break;
        }

    case Client::Discarding: {
            const Card *card = dashboard->pendingCard();
            if(card){
                ClientInstance->discardCards(card);
                dashboard->stopPending();
                daqiao->hide();
            }
            break;
        }        

    case Client::AskForSkillInvoke:{
            ClientInstance->invokeSkill(true);
            daqiao->hide();
            break;
        }

    case Client::AskForChoices:{
            ClientInstance->selectChoice(1);
            daqiao->hide();
            break;
        }

    case Client::NotActive: {
            QMessageBox::warning(main_window, tr("Warning"),
                                 tr("The OK button should be disabled when client is not active!"));
            return;
        }
    case Client::AskForAG:{
            QMessageBox::warning(main_window, tr("Warning"),
                                 tr("The OK button should be disabled when client is in asking for amazing grace status"));
            return;
        }

    case Client::ExecDialog:{
            QMessageBox::warning(main_window, tr("Warning"),
                                 tr("The OK button should be disabled when client is in executing dialog"));
            return;
        }

    case Client::AskForPlayerChoose:{
            ClientInstance->choosePlayer(selected_targets.first());
            daqiao->hide();

            break;
        }

    case Client::AskForYiji:{
            const Card *card = dashboard->pendingCard();
            if(card){
                ClientInstance->replyYiji(card, selected_targets.first());
                dashboard->stopPending();
                daqiao->hide();
            }

            break;
        }

    case Client::AskForGuanxing:{
            QList<int> up_cards, down_cards;
            foreach(CardItem *card_item, up_items){
                up_cards << card_item->getCard()->getId();
                removeItem(card_item);
                delete card_item;
            }

            foreach(CardItem *card_item, down_items){
                down_cards << card_item->getCard()->getId();
                removeItem(card_item);
                delete card_item;
            }

            up_items.clear();
            down_items.clear();

            ClientInstance->replyGuanxing(up_cards, down_cards);

            break;
        }

    case Client::AskForGongxin:{
            ClientInstance->replyGongxin();

            foreach(CardItem *card_item, gongxin_items)
                delete card_item;

            gongxin_items.clear();

            break;
        }
    }

    const ViewAsSkill *skill = dashboard->currentSkill();
    if(skill)
        dashboard->stopPending();
}

void RoomScene::doCancelButton(){
    switch(ClientInstance->getStatus()){
    case Client::Playing:{
            const ViewAsSkill *skill = dashboard->currentSkill();
            if(skill)
                cancelViewAsSkill();
            else
                dashboard->unselectAll();
            dashboard->stopPending();
            break;
        }

    case Client::Responsing:{
            if(!ClientInstance->card_pattern.startsWith("@")){
                const ViewAsSkill *skill = dashboard->currentSkill();
                if(skill){
                    cancelViewAsSkill();
                    break;
                }
            }

            if(ClientInstance->noTargetResponsing())
                ClientInstance->responseCard(NULL);
            else
                ClientInstance->responseCard(NULL, QList<const ClientPlayer *>());
            daqiao->hide();
            dashboard->stopPending();
            break;
        }

    case Client::Discarding:{
            dashboard->stopPending();
            ClientInstance->discardCards(NULL);
            daqiao->hide();
            break;
        }

    case Client::ExecDialog:{
            ClientInstance->ask_dialog->reject();
            break;
        }

    case Client::AskForSkillInvoke:{
            ClientInstance->invokeSkill(false);
            daqiao->hide();
            break;
        }

    case Client::AskForChoices:{
            ClientInstance->selectChoice(2);
            daqiao->hide();
            break;
        }

    case Client::AskForYiji:{
            dashboard->stopPending();
            ClientInstance->replyYiji(NULL, NULL);
            daqiao->hide();
            break;
        }

    default:{
            break;
        }
    }
}

void RoomScene::doDiscardButton(){
    dashboard->unselectAll();

    if(ClientInstance->getStatus() == Client::Playing){
        ClientInstance->useCard(NULL);
    }
}

void RoomScene::hideAvatars(){
    foreach(Photo *photo, photos)
        photo->hideAvatar();

    dashboard->hideAvatar();
}

void RoomScene::changeHp(const QString &who, int delta){
    if(who == Self->objectName()){
        dashboard->update();
    }else{
        Photo *photo = name2photo.value(who, NULL);
        if(photo)
            photo->update();
    }

    if(delta < 0){
        if(delta <= -3){
            static QString lightning_effect("audio/lightning.wav");
            Sanguosha->playEffect(lightning_effect);
            return;
        }

        static QString male_damage_effect("audio/male-damage.mp3");
        static QString female_damage_effect("audio/female-damage.mp3");

        ClientPlayer *player = ClientInstance->findChild<ClientPlayer *>(who);

        if(player->getGeneral()->isMale())
            Sanguosha->playEffect(male_damage_effect);
        else
            Sanguosha->playEffect(female_damage_effect);
    }
}

void RoomScene::clearPile(){
    foreach(CardItem *item, discarded_queue){
        removeItem(item);
        delete item;
    }

    discarded_queue.clear();
}

void RoomScene::setPileNumber(int n){
    pile_number = n;
    pile_number_item->setPlainText(QString::number(pile_number));
}

void RoomScene::freeze(){
    dashboard->setEnabled(false);
    avatar->setEnabled(false);
    foreach(Photo *photo, photos)
        photo->setEnabled(false);
    item2player.clear();
    trust_button->setEnabled(false);
    chat_edit->setEnabled(false);
    if(bgmusic){
        bgmusic->stop();
        bgmusic = NULL;
    }
    progress_bar->hide();
}

void RoomScene::onStandoff(){
    freeze();

    QDialog *dialog = new QDialog(main_window);
    dialog->setWindowTitle(tr("Standoff"));

    QVBoxLayout *layout = new QVBoxLayout;

    QTableWidget *table = new QTableWidget;
    fillTable(table, ClientInstance->getPlayers());

    layout->addWidget(table);
    dialog->setLayout(layout);

    addRestartButton(dialog);

    dialog->exec();
}

void RoomScene::onGameOver(bool victory, const QList<bool> &result_list){
    freeze();

    QDialog *dialog = new QDialog(main_window);
    dialog->setWindowTitle(victory ? tr("Victory") : tr("Failure"));

    QGroupBox *winner_box = new QGroupBox(tr("Winner(s)"));
    QGroupBox *loser_box = new QGroupBox(tr("Loser(s)"));

    QTableWidget *winner_table = new QTableWidget;
    QTableWidget *loser_table = new QTableWidget;

    QVBoxLayout *winner_layout = new QVBoxLayout;
    winner_layout->addWidget(winner_table);
    winner_box->setLayout(winner_layout);

    QVBoxLayout *loser_layout = new QVBoxLayout;
    loser_layout->addWidget(loser_table);
    loser_box->setLayout(loser_layout);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(winner_box);
    layout->addWidget(loser_box);
    dialog->setLayout(layout);

    winner_table->setColumnCount(4);
    loser_table->setColumnCount(4);

    QList<ClientPlayer *> players = ClientInstance->getPlayers();
    QList<ClientPlayer *> winner_list, loser_list;
    int i;
    for(i=0; i<players.length(); i++){
        ClientPlayer *player = players.at(i);
        bool result = result_list.at(i);
        if(result)
            winner_list << player;
        else
            loser_list << player;
    }

    winner_table->setRowCount(winner_list.length());
    loser_table->setRowCount(loser_list.length());

    fillTable(winner_table, winner_list);
    fillTable(loser_table, loser_list);

    addRestartButton(dialog);

    dialog->exec();
}

void RoomScene::addRestartButton(QDialog *dialog){
    dialog->resize(main_window->width()/2, dialog->height());

    QPushButton *restart_button = new QPushButton(tr("Restart Game"));
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addWidget(restart_button);

    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(dialog->layout());
    if(layout)
        layout->addLayout(hlayout);

    connect(restart_button, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(dialog, SIGNAL(accepted()), this, SIGNAL(restart()));
}

void RoomScene::fillTable(QTableWidget *table, const QList<ClientPlayer *> &players){
    static QStringList labels;
    if(labels.isEmpty())
        labels << tr("General") << tr("Name") << tr("Alive") << tr("Role");
    table->setHorizontalHeaderLabels(labels);

    table->setSelectionBehavior(QTableWidget::SelectRows);

    int i;
    for(i=0; i<players.length(); i++){
        ClientPlayer *player = players.at(i);

        QTableWidgetItem *item = new QTableWidgetItem;
        item->setText(Sanguosha->translate(player->getGeneralName()));
        table->setItem(i, 0, item);

        item = new QTableWidgetItem;
        item->setText(player->screenName());
        table->setItem(i, 1, item);

        item = new QTableWidgetItem;
        if(player->isAlive())
            item->setText(tr("Alive"));
        else
            item->setText(tr("Dead"));
        table->setItem(i, 2, item);

        item = new QTableWidgetItem;
        QIcon icon(QString(":/roles/%1.png").arg(player->getRole()));
        item->setIcon(icon);
        if(!player->isAlive())
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        item->setText(Sanguosha->translate(player->getRole()));
        table->setItem(i, 3, item);
    }
}

void RoomScene::killPlayer(const QString &who){
    const General *general = NULL;

    if(who == Self->objectName()){
        dashboard->update();

        general = Self->getGeneral();
    }else{
        Photo *photo = name2photo[who];
        photo->update();

        general = photo->getPlayer()->getGeneral();
    }

    QMutableMapIterator<QGraphicsItem *, const ClientPlayer *> itor(item2player);
    while(itor.hasNext()){
        itor.next();
        if(itor.value()->objectName() == who){
            itor.key()->setEnabled(false);
            itor.remove();
            break;
        }
    }

    general->lastWord();
}

void RoomScene::fillAmazingGrace(const QList<int> &card_ids){
    amazing_grace.clear();

    static const int columns = 4;
    static const qreal ratio = 0.8;

    int i;
    for(i=0; i<card_ids.length(); i++){
        int card_id = card_ids.at(i);
        CardItem *card_item = new CardItem(Sanguosha->getCard(card_id));
        card_item->setScale(ratio);

        QRectF rect = card_item->boundingRect();
        int row = i / columns;
        int column = i % columns;
        QPointF pos(column * rect.width() * ratio, row * rect.height() * ratio);

        card_item->setPos(pos);
        card_item->setHomePos(pos);
        card_item->setFlag(QGraphicsItem::ItemIsFocusable);
        amazing_grace << card_item;
        addItem(card_item);
    }

    int row_count = amazing_grace.length() > 4 ? 2 : 1;
    int column_count = amazing_grace.length() > 4 ? 4 : amazing_grace.length();
    QRectF rect = amazing_grace.first()->boundingRect();

    int width = rect.width() * column_count;
    int height = rect.height() * row_count;
    qreal dx = - width/2;
    qreal dy = - height/2;

    foreach(CardItem *card_item, amazing_grace){
        card_item->moveBy(dx, dy);
        card_item->setHomePos(card_item->pos());
    }
}

void RoomScene::clearAmazingGrace(){
    foreach(CardItem *item, amazing_grace){
        removeItem(item);
        delete item;
    }

    foreach(QGraphicsSimpleTextItem *item, taker_names){
        removeItem(item);
        delete item;
    }

    amazing_grace.clear();
    taker_names.clear();
}

void RoomScene::takeAmazingGrace(const ClientPlayer *taker, int card_id){
    foreach(CardItem *card_item, amazing_grace){
        if(card_item->getCard()->getId() == card_id){
            card_item->setEnabled(false);

            CardItem *item = new CardItem(card_item->getCard());
            item->setPos(card_item->pos());
            addItem(item);

            QString name;
            if(taker){
                putCardItem(taker, Player::Hand, item);
                name = Sanguosha->translate(taker->getGeneralName());
            }else{
                putCardItem(NULL, Player::DiscardedPile, item);
                name = tr("Discarded Pile");
            }

            QGraphicsSimpleTextItem *text_item = addSimpleText(name, Config.TinyFont);
            text_item->setPos(card_item->pos() + QPointF(20, 70));
            text_item->setZValue(card_item->zValue() + 1.0);
            taker_names << text_item;

            break;
        }
    }
}

void RoomScene::chooseAmazingGrace(){
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if(card_item){
        ClientInstance->chooseAG(card_item->getCard()->getId());
        foreach(CardItem *item, amazing_grace)
            item->disconnect(this);
    }
}

void RoomScene::showCard(const QString &player_name, int card_id){
    QString card_str = QString::number(card_id);

    if(player_name == Self->objectName())
        log_box->appendLog("$ShowCard", Self->getGeneralName(), QStringList(), card_str);
    else{
        Photo *photo = name2photo.value(player_name, NULL);
        photo->showCard(card_id);

        log_box->appendLog("$ShowCard", photo->getPlayer()->getGeneralName(),
                           QStringList(), card_str);
    }
}

const ViewAsSkill *RoomScene::getViewAsSkill(const QString &skill_name){
    const Skill *skill = Sanguosha->getSkill(skill_name);
    if(!skill){
        QMessageBox::warning(main_window, tr("Warning"), tr("No such skill named %1").arg(skill_name));
        return NULL;
    }

    const ViewAsSkill *view_as_skill = qobject_cast<const ViewAsSkill *>(skill);
    if(!view_as_skill){
        QMessageBox::warning(main_window, tr("Warning"), tr("The skill %1 must be view as skill!").arg(skill_name));
        return NULL;
    }

    return view_as_skill;
}

void RoomScene::attachSkill(const QString &skill_name){
    const ViewAsSkill *skill = getViewAsSkill(skill_name);

    QPushButton *button = new QPushButton(Sanguosha->translate(skill_name));

    skill_buttons << button;
    button2skill.insert(button, skill);

    button->setEnabled(skill->isAvailable());
    dashboard->addSkillButton(button);    

    connect(button, SIGNAL(clicked()), this, SLOT(doSkillButton()));
}

void RoomScene::detachSkill(const QString &skill_name){
    const ViewAsSkill *skill = getViewAsSkill(skill_name);

    QAbstractButton *button = button2skill.key(skill, NULL);
    if(button){
        skill_buttons.removeOne(button);
        button2skill.remove(button);
        QPushButton *push_button = qobject_cast<QPushButton *>(button);
        dashboard->removeSkillButton(push_button);

        delete button;
    }
}

static qreal CardWidth, CardHeight;

void RoomScene::doGuanxing(const QList<int> &card_ids){
    up_items.clear();
    static const qreal scale = 0.8;
    foreach(int card_id, card_ids){
        GuanxingCardItem *card_item = new GuanxingCardItem(Sanguosha->getCard(card_id));
        card_item->setFlag(QGraphicsItem::ItemIsFocusable);
        card_item->setScale(scale);
        connect(card_item, SIGNAL(released()), this, SLOT(adjustGuanxing()));

        up_items << card_item;
        addItem(card_item);
    }

    QRectF rect = up_items.first()->boundingRect();
    CardWidth = rect.width() * scale;
    CardHeight = rect.height() * scale;

    qreal width = CardWidth * up_items.length();
    qreal height = CardHeight * 2;

    qreal start_x = - width/2;
    qreal start_y = - height + 120;

    int i;
    for(i=0; i<up_items.length(); i++){
        CardItem *card_item = up_items.at(i);
        QPointF pos(start_x + i*CardWidth, start_y);
        card_item->setPos(pos);
        card_item->setHomePos(pos);
    }

    guanxing_origin.setX(start_x);
    guanxing_origin.setY(start_y);
}

void RoomScene::adjustGuanxing(){
    GuanxingCardItem *item = qobject_cast<GuanxingCardItem*>(sender());

    if(item == NULL)
        return;

    up_items.removeOne(item);
    down_items.removeOne(item);

    int r = (item->y() - guanxing_origin.y()) / CardHeight;
    r = qBound(0, r, 1);
    QList<GuanxingCardItem *> &items = r == 0 ? up_items : down_items;
    int c = (item->x() - guanxing_origin.x()) / CardWidth;

    c = qBound(0, c, items.length());
    items.insert(c, item);

    int i;
    for(i=0; i<up_items.length(); i++){
        QPointF pos = guanxing_origin;
        pos += QPointF(i * CardWidth, 0);
        up_items.at(i)->setHomePos(pos);
        up_items.at(i)->goBack();
    }

    for(i=0; i<down_items.length(); i++){
        QPointF pos = guanxing_origin;
        pos += QPointF(i * CardWidth, CardHeight);
        down_items.at(i)->setHomePos(pos);
        down_items.at(i)->goBack();
    }
}

void RoomScene::viewDistance(){
    DistanceViewDialog *dialog = new DistanceViewDialog(main_window);
    dialog->show();
}

void RoomScene::speak(const QString &who, const QString &text){
    const ClientPlayer *from = ClientInstance->findChild<const ClientPlayer *>(who);
    QString title;
    if(from){
        title = from->getGeneralName();
        title = Sanguosha->translate(title);
        title.append(QString("(%1)").arg(from->screenName()));
    }

    title = QString("<b>%1</b>").arg(title);

    QString line = QString(tr("[%1] said: %2").arg(title).arg(text));
    chat_box->append(line);
}

void RoomScene::speak(){
    ClientInstance->speakToServer(chat_edit->text());
    chat_edit->clear();
}

void RoomScene::doGongxin(const QList<int> &card_ids){
    gongxin_items.clear();

    foreach(int card_id, card_ids){
        const Card *card = Sanguosha->getCard(card_id);
        CardItem *card_item = new CardItem(card);
        if(card->getSuit() == Card::Heart){
            card_item->setEnabled(true);
            card_item->setFlag(QGraphicsItem::ItemIsFocusable);
            connect(card_item, SIGNAL(double_clicked()), this, SLOT(chooseGongxinCard()));
        }else
            card_item->setEnabled(false);

        addItem(card_item);
        gongxin_items << card_item;
    }

    QRectF rect = gongxin_items.first()->boundingRect();
    qreal card_width = rect.width();
    qreal card_height = rect.height();

    qreal width = card_width * gongxin_items.length();
    qreal start_x = - width / 2;
    qreal start_y = - card_height / 2;

    int i;
    for(i=0; i<gongxin_items.length(); i++){
        QPoint pos;
        pos.setX(start_x + i * card_width);
        pos.setY(start_y);

        gongxin_items.at(i)->setPos(pos);
        gongxin_items.at(i)->setHomePos(pos);
    }
}

void RoomScene::chooseGongxinCard(){
    CardItem *item = qobject_cast<CardItem *>(sender());

    if(item){
        ClientInstance->replyGongxin(item->getCard()->getId());

        foreach(CardItem *card_item, gongxin_items)
            delete card_item;

        gongxin_items.clear();
    }
}

void RoomScene::onGameStart(){
    trust_button->setEnabled(true);
    updateStatus(ClientInstance->getStatus());

    if(!Config.EnableBgMusic)
        return;

    bool play_music = false;
    if(memory->isAttached() || memory->attach()){
        memory->lock();

        char *username = static_cast<char *>(memory->data());
        const char *my_username = Config.UserName.toAscii();
        play_music = qstrcmp(username, my_username) == 0;

        memory->unlock();
    }else if(memory->create(255)){
        memory->lock();

        void *data = memory->data();
        const char *username = Config.UserName.toAscii();
        memcpy(data, username, qstrlen(username));
        play_music = true;

        memory->unlock();
    }

    if(!play_music)
        return;

    // start playing background music
    QString bgmusic_path = Config.value("BackgroundMusic", "audio/background.mp3").toString();
    bgmusic = audiere::OpenSound(Device, bgmusic_path.toAscii(), true);
    if(bgmusic){
        bgmusic->setRepeat(true);
        bgmusic->setVolume(Config.Volume);
        bgmusic->play();
    }
}

void RoomScene::moveFocus(const QString &who){
    Photo *photo = name2photo[who];
    if(photo){
        if(focused != photo && focused)
            focused->hideProcessBar();

        focused = photo;
        focused->showProcessBar();
    }
}
