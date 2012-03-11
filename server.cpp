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
#include "settings.h"
#include "debug.h"
#include "queuethread.h"
#include <iostream>

Server::Server( quint16 port, QObject* parent )
    : QTcpServer( parent ), disabled( false )
{
    Debug::log( "server", "Constructing " + QString::number( thread()->currentThreadId() ) );
    
    mUpdateThread = new UpdateThread( this );
    mUpdateThread->start();
    
    listen( QHostAddress::Any, port );
    
    char hostname[ 1024 ];
    gethostname( hostname, sizeof( hostname ) );
    mHostName = hostname;
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
    Debug::log( "server", "Paused" );
    disabled = true;
}

void Server::resume()
{
    Debug::log( "server", "Resumed" );
    disabled = false;
}

void Server::readClient()
{
    if ( disabled )
    {
        return;
    }
    
    QTcpSocket* socket = ( QTcpSocket* )sender();
    Debug::log( "readClient", QString::number( socket->socketDescriptor() ) );
    
    connect( socket, SIGNAL( disconnected() ),
             this, SLOT( deleteSocket() ) );
    
   
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
            Debug::log( "server", " - " + r.trimmed() );
        }
        else
        {
            Debug::log( "server", dm + r.trimmed() );
        }

        QStringList tokens = r.split( QRegExp( "[ \r\n][ \r\n]*" ) );
        
        QTextStream os( socket );    
        os.setAutoDetectUnicode( true );
        
        QString req = tokens[ 0 ];
        QString cmd = tokens[ 1 ];

        os << "Server: kueued @ " + mHostName + " (Linux)\r\n";     
        
        if ( req == "GET" )
        {
            os << "HTTP/1.1 200 OK\r\n";
        
            if ( cmd.startsWith( "/qmon" ) )
            {
                QString xml;
                QString q = cmd.remove( "/qmon" );
                
                if ( !q.remove( "/" ).isEmpty() )
                {
                    xml = XML::qmon( Database::getSrsForQueue( q.remove( "/" ) ) );
                }
                else
                {
                    xml = XML::qmon( Database::getSrsForQueue() );
                }
                os << "Content-Type: text/xml; charset=\"utf-8\"\r\n";
                os << "\r\n";
                os << "<?xml version='1.0'?>\n\n";
                os << xml;
                
                closeSocket( socket );
            }
            else if ( cmd.startsWith( "/srnrs" ) )
            {  
                QString q = cmd.remove( "/srnrs" );

                os << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
                os << "\r\n";

                if ( q.remove( "/" ).isEmpty() )
                {  
                    os << "Please specify queue";
                }
                else if ( !q.contains( "|" ) )
                {  
                    os << "Please specify geo";
                }
                else
                {  
                    /*QStringList l = Database::getSrNumsForQueue( q.remove( "/" ).split( "|" ).at( 0 ), q.remove( "/" ).split( "|" ).at( 1 ) );

                    for ( int i = 0; i < l.size(); ++i )
                    {  
                       // os << l.at( i ) + "\n";
                    }*/
                }
                
                closeSocket( socket );
            }
            else if ( cmd.startsWith( "/latestkueue" ) )
            {
                os << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
                os << "\r\n";
                                
                os << Settings::latestVersion();
                
                socket->close();
                socket->waitForDisconnected();
                
                closeSocket( socket );
            }
            else if ( cmd.startsWith( "/updateDB" ) )
            {
                mUpdateThread->update( socket );
            }
            else if ( cmd.startsWith( "/chat" ) )
            {
                QStringList l = Database::getCurrentBomgars();
              
                os << "Content-Type: text/xml; charset=\"utf-8\"\r\n";
                os << "\r\n";
                os << "<?xml version='1.0'?>\n\n";
                os << "<chat>\n";
                
                for ( int i = 0; i < l.size(); ++i )
                {
                    os << "  <bomgar>\n";
                    os << "    <bomgarQ>" + l.at(i).split("|||").at(1) + "</bomgarQ>\n";
                    os << "    <sr>" + l.at(i).split("|||").at(0) + "</sr>\n";
                    os << "  </bomgar>\n";
                }
                
                os << "</chat>";
                
                closeSocket( socket );
            }
            else if ( cmd.startsWith( "/userqueue" ) )
            {
                QString q = cmd.remove( "/userqueue/" ).toUpper();
                
                QueueThread* t = new QueueThread( socket->socketDescriptor(), q, this );
                
                connect( t, SIGNAL( finished() ),
                         this, SLOT( deleteThread() ) );
                
                t->start();
            }
            else
            {
                os << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
                os << "\r\n";
                os << "Welcome to kueued\n\n";
                os << "Usage:\n\n";
                os << "  * http://kueue.hwlab.suse.de:8080/qmon\n    Get a list of all SRs in all pseudo queues\n\n";
                os << "  * http://kueue.hwlab.suse.de:8080/qmon/$QUEUE-NAME\n    Get a list of all SRs in $QUEUE-NAME\n\n";
                os << "    This is the qmon xml output.\n";
                os << "\n";
                os << "    Please note that not all fields are available for each SR, if they are not, they aren't shown at all.\n";
                os << "\n";
                os << "    <sr>\n";
                os << "      <id>SR number</id>\n";
                os << "      <queue>Queue</queue>\n";
                os << "      <bomgarQ>Bomgar Queue (if in chat)</bomgarQ>\n";
                os << "      <srtype>sr/cr</srtype>\n";
                os << "      <creator>Only for CRs: who created it?</creator>\n";
                os << "      <customer>\n";
                os << "        <account>Company name</account>\n";
                os << "        <firstname>Customer's first name</firstname>\n";
                os << "        <lastname>Customer's last name</lastname>\n";
                os << "        <title>Customer's title (ie. job)</title>\n";
                os << "        <email>Customer's email</email>\n";
                os << "        <phone>Customer's phone number</phone>\n";
                os << "        <onsitephone>Customer's onsite phone number</onsitephone>\n";
                os << "        <lang>Customer's preferred language</lang>\n";
                os << "      </customer>\n";
                os << "      <severity>SR severity</severity>\n";
                os << "      <status>Current status</status>\n";
                os << "      <bdesc>Brief description</bdesc>\n";
                os << "      <ddesc>Detailed description</ddesc>\n";
                os << "      <geo>Geo</geo>\n";
                os << "      <hours>Support hours</hours>\n";
                os << "      <source>How was the SR opened (web, phone..)</source>\n";
                os << "      <support_program>Customer's contract</support_program>\n";
                os << "      <support_program_long>A slightly more detailed version of the above</support_program_long>\n";
                os << "      <routing_product>Routing product</routing_product>\n";
                os << "      <support_group_routing>What group it is routed to internally</support_group_routing>\n";
                os << "      <int_type>Internal type</int_type>\n";
                os << "      <subtype>Subtype</subtype>\n";
                os << "      <servce_level>Some number indicating something</service_level>\n";
                os << "      <category>SR category (Adv/Knowledge)</category>\n";
                os << "      <respond_via>How the customer would like to be contacted</respond_via>\n";
                os << "      <age>SR age in seconds</age>\n";
                os << "      <lastupdate>Time to last update in seconds</lastupdate>\n";
                os << "      <timeinQ>Time in the current queue in seconds</timeinQ>\n";
                os << "      <sla>SLA left in seconds</sla>\n";
                os << "      <highvalue>Is the customer considered high value?</highvalue>\n";
                os << "      <critsit>Is there a critsit going on with the customer?</critsit>\n";
                os << "    </sr>   \n\n";
                os << "  * http://kueue.hwlab.suse.de:8080/bug/$SRNR\n    Get the bugreport for $SRNR (if any)\n\n";
                os << "Stay tuned for more features!";
                
                closeSocket( socket );
            }
        }
        else if ( req == "updateDB" )
        {
            mUpdateThread->update( socket );
        }
    }
}

void Server::discardClient()
{
    QTcpSocket* socket = ( QTcpSocket* )sender();
    socket->deleteLater();
}

void Server::closeSocket( QTcpSocket* socket )
{
    socket->close();
    socket->waitForDisconnected();
}

void Server::deleteSocket()
{}

void Server::deleteThread()
{
    QThread* t = qobject_cast<QThread*>( sender() );
    delete t;
}

#include "server.moc"
