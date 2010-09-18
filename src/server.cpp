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

Server::Server(QObject *parent)
    :QTcpServer(parent)
{
    quint16 port = Config.Port;

    connect(this, SIGNAL(newConnection()), SLOT(processNewConnection()));

    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();

    QStringList items;
    foreach(QHostAddress address, addresses){
        quint32 ipv4 = address.toIPv4Address();
        if(ipv4)
            items << QHostAddress(ipv4).toString();
    }

    int current = items.indexOf(Config.ListenAddress);
    if(current == -1)
        current = 0;

    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(tr("Select network address"));

    QComboBox *combobox = new QComboBox;
    combobox->addItems(items);
    combobox->setCurrentIndex(current);

    QLineEdit *port_edit = new QLineEdit;
    port_edit->setValidator(new QIntValidator);
    port_edit->setText(QString::number(port));

    QSpinBox *spinbox = new QSpinBox;
    spinbox->setMinimum(2);
    spinbox->setMaximum(8);
    spinbox->setValue(Config.PlayerCount);

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
    extensions << "wind" << "fire" << "thicket" << "maneuvering" << "yitian";
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
    layout->addRow(tr("Network address"), combobox);
    layout->addRow(tr("Port"), port_edit);
    layout->addRow(tr("Player count"), spinbox);
    layout->addRow(box);
    layout->addRow(button_layout);

    dialog->setLayout(layout);
    dialog->exec();

    if(dialog->result() != QDialog::Accepted)
        return;

    Config.ListenAddress = combobox->currentText();
    Config.Port = port_edit->text().toInt();
    Config.PlayerCount = spinbox->value();

    Config.setValue("ListenAddress", Config.ListenAddress);
    Config.setValue("Port", Config.Port);
    Config.setValue("PlayerCount", Config.PlayerCount);

    QList<QAbstractButton *> checkboxes = extension_group->buttons();
    foreach(QAbstractButton *checkbox, checkboxes){
        if(!checkbox->isChecked()){
            Sanguosha->addBanPackage(checkbox->objectName());
        }
    }

    listen(QHostAddress(Config.ListenAddress), Config.Port);
    if(!isListening())
        QMessageBox::warning(NULL, tr("Warning"), tr("Can not start server on address %1 !").arg(Config.ListenAddress));
}

void Server::processNewConnection(){
    QTcpSocket *socket = nextPendingConnection();

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
