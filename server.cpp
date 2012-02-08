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
#include "debug.h"

Server::Server( quint16 port, QObject* parent )
    : QTcpServer( parent ), disabled( false )
{
    Debug::print( "server", "Constructing" );
    listen( QHostAddress::Any, port );
}

void Server::incomingConnection( int socket )
{
    if ( disabled )
    {
        return;
    }

    QTcpSocket* s = new QTcpSocket( this );
    
    connect( s, SIGNAL( readyRead() ), 
             this, SLOT( readClient() ) );
    
    connect( s, SIGNAL( disconnected() ), 
             this, SLOT( discardClient() ) );
    
    s->setSocketDescriptor( socket );
}

void Server::pause()
{
    Debug::print( "server", "Paused" );
    disabled = true;
}

void Server::resume()
{
    Debug::print( "server", "Resumed" );
    disabled = false;
}

void Server::readClient()
{
    if ( disabled )
    {
        return;
    }
    
    QTcpSocket* socket = ( QTcpSocket* )sender();
    QString dm;
    dm += socket->peerAddress().toString();
    
    if ( socket->canReadLine() )
    {
        QString r = socket->readLine();
        
        while ( socket->canReadLine() ) 
        {
            QString tmp = socket->readLine();
            
            if ( tmp.startsWith( "User-Agent" ) )
            {
                dm += " - " + tmp.remove( "User-Agent: " ).trimmed() + " - ";
            }
        }
        
        if ( dm.isEmpty() )
        {
            Debug::print( "server", " - " + r.trimmed() );
        }
        else
        {
            Debug::print( "server", dm + r.trimmed() );
        }

        QStringList tokens = r.split( QRegExp( "[ \r\n][ \r\n]*" ) );
        
        QTextStream os( socket );    
        os.setAutoDetectUnicode( true );
        
        QString req = tokens[ 0 ];
        QString cmd = tokens[ 1 ];
                       
        os << "HTTP/1.1 200 OK\r\n";
        os << "Server: kueued (Linux)\r\n";     
        
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
                    os << "Content-Type: text/xml; charset=\"utf-8\"\r\n";
                    os << "\r\n";
                    os << "<?xml version='1.0'?>\n\n";
                    os << "<qmon>\n";
                
                for ( int i = 0; i < l.size(); ++i ) 
                {
                    os << XML::sr( l.at( i ) );
                    delete l.at( i );
                }
                
                os << "</qmon>";
            }
            else if ( cmd.startsWith( "/bug" ) )
            {
                QString q = cmd.remove( "/bug/" );
                
                os << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
                os << "\r\n";
                                
                os << Database::getBugForSr( q );
            }
            else if ( cmd.startsWith( "/critsit" ) )
            {
                QString q = cmd.remove( "/critsit/" );
                
                os << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
                os << "\r\n";
                                
                os << Database::critSitFlagForSr( q );
            }
            else if ( cmd.startsWith( "/highvaluecritsit" ) )
            {
                QString q = cmd.remove( "/highvaluecritsit/" );
                
                os << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
                os << "\r\n";
                                
                os << Database::highValueCritSitFlagForSr( q );
            }
            else if ( ( cmd.startsWith( "/highvalue" ) ) && !( cmd.startsWith( "/highvaluecritsit" ) ) )
            {
                QString q = cmd.remove( "/highvalue/" );
                
                os << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
                os << "\r\n";
                
                os << Database::highValueFlagForSr( q );
            }
            else if ( cmd.startsWith( "/chat" ) )
            {
                QStringList l = Database::getCurrentBomgars();
              
                os << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
                os << "\r\n"; 
                os << "<chat>\n>";
                
                for ( int i = 0; i < l.size(); ++i )
                {
                    os << "  <bomgar>\n";
                    os << "    <bomgarQ>" + l.at(i).split("|||").at(1) + "</bomgarQ>\n";
                    os << "    <sr>" + l.at(i).split("|||").at(0) + "</sr>\n";
                    os << "  </bomgar>\n";
                }
                
                os << "</chat>";
            }
	    else if ( cmd.startsWith( "/test" ) )
	    {
		QStringList l = Database::getOracleSrList();
		
                for ( int i = 0; i < l.size(); ++i )
		{
			os << l.at(i) << "\n";
		}
	    }
            else
            {
                os << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
                os << "\r\n";
                
                os << "Welcome to kueue.hwlab.suse.de!\n\n";
                os << "Usage:\n\n";
                os << "  * http://kueue.hwlab.suse.de:8080/qmon\n    Get a list of all SRs in all pseudo queues\n\n";
                os << "  * http://kueue.hwlab.suse.de:8080/qmon/$QUEUE-NAME\n    Get a list of all SRs in $QUEUE-NAME\n\n";
                os << "    The output will contain the following xml elements:\n\n";
                os << "      id              SR Number\n";
                os << "      queue           Current Queue\n";
                os << "      severity        Severity\n";
                os << "      geo             GEO\n";
                os << "      type            Type (SR/CR)\n";
                os << "      bomgarQ         Current Bomgar Queue (only available if the customer is in chat)\n";
                os << "      age             SR age in seconds\n";
                os << "      timeinqueue     Time in queue in seconds\n";
                os << "      sla             SLA left in seconds (only available if there is SLA left)\n";
                os << "      lastupdate      Last activity in SR (only available if SR is not new)\n";
                os << "      description     Brief description\n";
                os << "      status          Status\n";
                os << "      contract        Customer's contract\n";
                os << "      contact         Preferred contact method\n\n";
                os << "  * http://kueue.hwlab.suse.de:8080/critsit/$SRNR\n    Is there a critsit with this customer? (Y/N)\n\n";
                os << "  * http://kueue.hwlab.suse.de:8080/highvalue/$SRNR\n    Is $SRNR considered high value? (Y/N)\n\n";
                os << "  * http://kueue.hwlab.suse.de:8080/highvaluecritsit/$SRNR\n    The last two queries combined (sample output: YN)\n\n";
                os << "  * http://kueue.hwlab.suse.de:8080/bug/$SRNR\n    Get the bugreport for $SRNR (if any)\n\n";
                os << "Stay tuned for more features!";
            }
        }

        socket->close();
        socket->waitForDisconnected();
        
        if ( socket->state() == QTcpSocket::UnconnectedState ) 
        {
            delete socket;
        }
    }
}

void Server::discardClient()
{
    QTcpSocket* socket = ( QTcpSocket* )sender();
    socket->deleteLater();
}

#include "server.moc"
