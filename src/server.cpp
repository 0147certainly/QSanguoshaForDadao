#include "server.h"
#include "settings.h"
#include "room.h"
#include "engine.h"

#include <QInputDialog>
#include <QNetworkInterface>
#include <QMessageBox>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QButtonGroup>
#include <QLabel>

Server::Server(QObject *parent)
    :QTcpServer(parent)
{
    quint16 port = Config.Port;

    connect(this, SIGNAL(newConnection()), SLOT(processNewConnection()));

    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(tr("Start server"));

    QLineEdit *port_edit = new QLineEdit;
    port_edit->setValidator(new QIntValidator);
    port_edit->setText(QString::number(port));
    port_edit->setToolTip(tr("Change the port number is not necessary for most cases"));

    QSpinBox *player_count_spinbox = new QSpinBox;
    player_count_spinbox->setMinimum(2);
    player_count_spinbox->setMaximum(8);
    player_count_spinbox->setValue(Config.PlayerCount);

    QSpinBox *timeout_spinbox = new QSpinBox;
    timeout_spinbox->setMinimum(5);
    timeout_spinbox->setMaximum(30);
    timeout_spinbox->setValue(Config.OperationTimeout);
    timeout_spinbox->setSuffix(tr(" seconds"));

    QHBoxLayout *button_layout = new QHBoxLayout;
    button_layout->addStretch();

    QPushButton *ok_button = new QPushButton(tr("OK"));
    QPushButton *cancel_button = new QPushButton(tr("Cancel"));

    button_layout->addWidget(ok_button);
    button_layout->addWidget(cancel_button);

    connect(ok_button, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(cancel_button, SIGNAL(clicked()), dialog, SLOT(reject()));

    QGroupBox *box = new QGroupBox;
    box->setTitle(tr("Extension package selection"));
    QVBoxLayout *vlayout = new QVBoxLayout;
    QButtonGroup *extension_group = new QButtonGroup;
    extension_group->setExclusive(false);

    QStringList extensions;
    extensions << "wind" << "fire" << "thicket" << "maneuvering" << "god" << "yitian";
    foreach(QString extension, extensions){
        QCheckBox *checkbox = new QCheckBox;
        checkbox->setObjectName(extension);
        checkbox->setText(Sanguosha->translate(extension));
        checkbox->setChecked(true);

        vlayout->addWidget(checkbox);
        extension_group->addButton(checkbox);
    }

    box->setLayout(vlayout);

    QFormLayout *layout = new QFormLayout;
    layout->addRow(tr("Port"), port_edit);
    layout->addRow(tr("Player count"), player_count_spinbox);
    layout->addRow(tr("Operation timeout"), timeout_spinbox);
    layout->addRow(box);
    layout->addRow(button_layout);

    dialog->setLayout(layout);
    dialog->exec();

    if(dialog->result() != QDialog::Accepted)
        return;

    Config.Port = port_edit->text().toInt();
    Config.PlayerCount = player_count_spinbox->value();
    Config.OperationTimeout = timeout_spinbox->value();

    Config.setValue("Port", Config.Port);
    Config.setValue("PlayerCount", Config.PlayerCount);
    Config.setValue("OperationTimeout", Config.OperationTimeout);

    QList<QAbstractButton *> checkboxes = extension_group->buttons();
    foreach(QAbstractButton *checkbox, checkboxes){
        if(!checkbox->isChecked()){
            Sanguosha->addBanPackage(checkbox->objectName());
        }
    }

    listen(QHostAddress::Any, Config.Port);
    if(!isListening())
        QMessageBox::warning(NULL, tr("Warning"), tr("Can not start server on address %1 !").arg(serverAddress().toString()));
}

void Server::processNewConnection(){
    QTcpSocket *socket = nextPendingConnection();

    QMutableListIterator<Room *> itor(rooms);
    while(itor.hasNext()){
        Room *room = itor.next();
        if(room->isFinished()){
            delete room;
            itor.remove();
        }
    }

    Room *free_room = NULL;
    foreach(Room *room, rooms){
        if(!room->isFull()){
            free_room = room;
            break;
        }
    }

    if(free_room == NULL){
        free_room = new Room(this, Config.PlayerCount);
        rooms << free_room;
        connect(free_room, SIGNAL(room_message(QString)), this, SIGNAL(server_message(QString)));
    }

    free_room->addSocket(socket);

    emit server_message(tr("%1 connected, port = %2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort()));
}

