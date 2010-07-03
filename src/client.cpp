#include "client.h"
#include "settings.h"

Client::Client(QObject *parent)
    :QTcpSocket(parent), room(new QObject(this)), self(NULL)
{
    connectToHost(Config.HostAddress, Config.Port);

    connect(this, SIGNAL(readyRead()), this, SLOT(update()));
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(raiseError(QAbstractSocket::SocketError)));
}

void Client::setSelf(Player *self){
    this->self = self;
}

void Client::request(const QString &message){
    write(message.toAscii());
    write("\n");
}

void Client::sendField(const QString &field){
    request(QString("set %1 %2").arg(field).arg(property(field.toAscii()).toString()));
}

void Client::signup(){
    setProperty("name", Config.UserName);
    setProperty("avatar", Config.UserAvatar);

    request(QString("signup %1 %2").arg(Config.UserName).arg(Config.UserAvatar));
}

Player *Client::getPlayer(const QString &name){
    return parent()->findChild<Player*>(name);
}

void Client::update(){
    static QChar self_prefix('.');
    static QChar room_prefix('$');
    static QChar other_prefix('#');
    static QChar method_prefix('!');

    while(canReadLine()){
        QString update_str = readLine(1024);
        QStringList words = update_str.split(QChar(' '));

        QString object = words[0];        
        const char *field = words[1].toAscii();
        QString value = words[2];

        if(object.startsWith(self_prefix)){
            // client it self
            if(self)
                self->setProperty(field, value);
        }else if(object.startsWith(room_prefix)){
            // room
            room->setProperty(field, value);
        }else if(object.startsWith(other_prefix)){
            // others
            object.remove(other_prefix);
            Player *player = getPlayer(object);
            player->setProperty(field, value);
        }else if(object.startsWith(method_prefix)){
            // invoke parent methods
            if(parent()){
                const QMetaObject *meta = parent()->metaObject();
                meta->invokeMethod(parent(), field, Qt::DirectConnection, Q_ARG(QString, value));
            }
        }
    }
}

void Client::raiseError(QAbstractSocket::SocketError socket_error){
    // translate error message
    QString reason;
    switch(socket_error){
    case ConnectionRefusedError: reason = tr("Connection was refused or timeout"); break;
    case RemoteHostClosedError: reason = tr("Remote host close this connection"); break;
        // FIXME
    default: reason = tr("Unknow error"); break;
    }

    emit error_message(tr("Connection failed, error code = %1\n reason:\n %2").arg(socket_error).arg(reason));
}
