#include "servingthread.h"
#include "roomthread.h"

ServingThread::ServingThread(QObject *parent, QTcpSocket *socket)
    :QThread(parent), socket(socket), room_thread(NULL)
{

}

void ServingThread::setRoomThread(RoomThread *room_thread){
    this->room_thread = room_thread;
}

void ServingThread::run()
{
    QString request;
    while(true){
        request = socket->readLine(1024);
        // limit the request line length, I think 1024 is enough for communication

        if(request.isEmpty())
            break;

        room_thread->processRequest(request);
    }
}
