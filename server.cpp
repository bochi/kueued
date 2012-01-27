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


#include "server.h"
#include "database.h"

Server::Server( quint16 port, QObject* parent )
    : QTcpServer(parent), disabled(false)
{
    listen(QHostAddress::Any, port);
}

void Server::incomingConnection(int socket)
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

void Server::pause()
{
    disabled = true;
}

void Server::resume()
{
    disabled = false;
}

void Server::readClient()
{
    if ( disabled )
    {
        return;
    }

    QTcpSocket* socket = (QTcpSocket*)sender();

    if ( socket->canReadLine() ) 
    {
        QStringList tokens = QString(socket->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));
        QTextStream os(socket);    
        os.setAutoDetectUnicode(true);
        
        QString req = tokens[0];
        QString cmd = tokens[1];
        
        if ( req == "GET" )
        {
            if ( cmd.startsWith( "/qmon" ) )
            {
                QList< SiebelItem* > l;
                QString q = cmd.remove( "/qmon" );
                
                if ( !q.remove( "/" ).isEmpty() )
                {
                    l = Database::getSrsForQueue( q.remove( "/" ) );
                }
                else
                {
                    l = Database::getSrsForQueue();
                }
             
                os << "<?xml version='1.0'?>\n\n<qmon>\n";
                
                for ( int i = 0; i < l.size(); ++i ) 
                {
                    os << XML::sr(l.at(i));
                    delete l.at( i );
                }
                
                os << "</qmon>";
            }
            else
            {
                os << "Welcome to kueue.hwlab.suse.de!\n\n";
                os << "Usage:\n\n";
                os << "http://kueue.hwlab.suse.de:8080/qmon\nGet a list of all SRs in all pseudo queues\n\n";
                os << "http://kueue.hwlab.suse.de:8080/qmon/$QUEUE-NAME\nGet a list of all SRs in $QUEUE-NAME\n\n";
                os << "The output will contain the following xml elements:\n\n";
                os << "id              SR Number\n";
                os << "queue           Current Queue\n";
                os << "severity        Severity\n";
                os << "geo             GEO\n";
                os << "type            Type (SR/CR)\n";
                os << "bomgarQ         Current Bomgar Queue (only available if the customer is in chat)\n";
                os << "age             SR age in seconds\n";
                os << "timeinqueue     Time in queue in seconds\n";
                os << "sla             SLA left in seconds (only available if there is SLA left)\n";
                os << "lastupdate      Last activity in SR (only available if SR is not new)\n";
                os << "description     Brief description\n";
                os << "status          Status\n";
                os << "contract        Customer's contract\n";
                os << "contact         Preferred contact method\n\n";
                os << "Stay tuned for more features!";
            }
        }

        socket->close();
        
        if (socket->state() == QTcpSocket::UnconnectedState) 
        {
            delete socket;
        }
    }
}

void Server::discardClient()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    socket->deleteLater();

    qDebug() << "Connection closed";
}

#include "server.moc"