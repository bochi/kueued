/*
              kueued - create xml data for kueue's qmon 
              (C) 2012 Stefan Bogner <sbogner@suse.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the

    Free Software Foundation, Inc.
    59 Temple Place - Suite 330
    Boston, MA  02111-1307, USA

    Have a lot of fun :-)

*/

#ifndef SERVER_H
#define SERVER_H
#include "qtservice/qtservice.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QtNetwork>

 // HttpDaemon is the the class that implements the simple HTTP server.
 class HttpDaemon : public QTcpServer
 {
     Q_OBJECT
 public:
     HttpDaemon(quint16 port, QObject* parent = 0)
         : QTcpServer(parent), disabled(false)
     {
         listen(QHostAddress::Any, port);
     }

     void incomingConnection(int socket)
     {
         if (disabled)
             return;

         // When a new client connects, the server constructs a QTcpSocket and all
         // communication with the client is done over this QTcpSocket. QTcpSocket
         // works asynchronously, this means that all the communication is done
         // in the two slots readClient() and discardClient().
         QTcpSocket* s = new QTcpSocket(this);
         connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
         connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
         s->setSocketDescriptor(socket);

         qDebug() << "New Connection";
     }

     void pause()
     {
         disabled = true;
     }

     void resume()
     {
         disabled = false;
     }

 private slots:
     void readClient()
     {
         if (disabled)
             return;

         // This slot is called when the client sent data to the server. The
         // server looks if it was a get request and sends a very simple HTML
         // document back.
         QTcpSocket* socket = (QTcpSocket*)sender();
         if (socket->canReadLine()) {
             QStringList tokens = QString(socket->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));
             if (tokens[0] == "GET") {
                 QTextStream os(socket);
                 os.setAutoDetectUnicode(true);
                 os << "HTTP/1.0 200 Ok\r\n"
                     "Content-Type: text/html; charset=\"utf-8\"\r\n"
                     "\r\n"
                     "<h1>Nothing to see here</h1>\n"
                     << QDateTime::currentDateTime().toString() << "\n";
                 socket->close();

                 qDebug() << "Wrote to client";

                 if (socket->state() == QTcpSocket::UnconnectedState) {
                     delete socket;
                     qDebug() << "Connection closed";
                 }
             }
         }
     }
     void discardClient()
     {
         QTcpSocket* socket = (QTcpSocket*)sender();
         socket->deleteLater();

         qDebug() << "Connection closed";
     }

 private:
     bool disabled;
 };

 class Server : public QTcpServer
 {
     Q_OBJECT
 public:
     Server(quint16 port, QObject* parent = 0)
         : QTcpServer(parent), disabled(false)
     {
         listen(QHostAddress::Any, port);
     }

     void incomingConnection(int socket)
     {
         if (disabled)
             return;

         // When a new client connects, the server constructs a QTcpSocket and all
         // communication with the client is done over this QTcpSocket. QTcpSocket
         // works asynchronously, this means that all the communication is done
         // in the two slots readClient() and discardClient().
         QTcpSocket* s = new QTcpSocket(this);
         connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
         connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
         s->setSocketDescriptor(socket);

         qDebug() << "New Connection";
     }

     void pause()
     {
         disabled = true;
     }

     void resume()
     {
         disabled = false;
     }

 private slots:
     void readClient()
     {
         if (disabled)
             return;

         // This slot is called when the client sent data to the server. The
         // server looks if it was a get request and sends a very simple HTML
         // document back.
         QTcpSocket* socket = (QTcpSocket*)sender();
         if (socket->canReadLine()) {
             QStringList tokens = QString(socket->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));
             if (tokens[0] == "GET") {
                 QTextStream os(socket);
                 os.setAutoDetectUnicode(true);
                 os << "HTTP/1.0 200 Ok\r\n"
                     "Content-Type: text/html; charset=\"utf-8\"\r\n"
                     "\r\n"
                     "<h1>Nothing to see here</h1>\n"
                     << QDateTime::currentDateTime().toString() << "\n";
                 socket->close();

                 qDebug() << "Wrote to client";

                 if (socket->state() == QTcpSocket::UnconnectedState) {
                     delete socket;
                     qDebug() << "Connection closed";
                 }
             }
         }
     }
     void discardClient()
     {
         QTcpSocket* socket = (QTcpSocket*)sender();
         socket->deleteLater();

         qDebug() << "Connection closed";
     }

 private:
     bool disabled;
};


 
#endif
