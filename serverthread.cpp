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

#include "xml.h"
#include "settings.h"
#include "debug.h"
#include "serverthread.h"
#include "database.h"
#include "network.h"

#include <iostream>
#include <QtSql>
#include <QTcpSocket>
#include <QHostAddress>
#include <QtNetwork>
#include <QTime>

ServerThread::ServerThread( int sd, QObject *parent ) : QThread(parent)
{
    mSocket = sd;
    mTime.start();
}

ServerThread::~ServerThread()
{
    if ( QSqlDatabase::database( mMysqlDB ).isOpen() )
    {
        QSqlDatabase::database( mMysqlDB ).close();
        QSqlDatabase::removeDatabase( mMysqlDB );
    }
    
    if ( QSqlDatabase::database( mQmonDB ).isOpen() )
    {
        QSqlDatabase::database( mQmonDB ).close();
        QSqlDatabase::removeDatabase( mQmonDB );
    }
     
    if ( QSqlDatabase::database( mSiebelDB ).isOpen() )
    {
        QSqlDatabase::database( mSiebelDB ).close();
        QSqlDatabase::removeDatabase( mSiebelDB );
    }
    
    Debug::print( "serverthread", "Destroying " +  QString::number( currentThreadId() ) + ", thread took " + QString::number( mTime.elapsed() / 1000 ) + " sec" );
}


void ServerThread::run()
{
    QString cmd;
    QString dm;

    Debug::print( "serverthread", "New Server Thread " + QString::number( currentThreadId() ) );
    
    char hostname[ 1024 ];
    gethostname( hostname, sizeof( hostname ) );
    mHostname = hostname;
    
    QString tid = QString::number( QThread::currentThreadId() );
    
    mMysqlDB = "mysqlDB-" + tid;
    mSiebelDB = "siebelDB-" + tid;
    mQmonDB = "qmonDB-" + tid;
    
    QTcpSocket* socket= new QTcpSocket;
    
    if ( mSocket != 0 )
    {
        socket->setSocketDescriptor( mSocket );
    }
        
    if ( socket->waitForReadyRead() )
    {
        Debug::print( "serverthread", "Socket " + QString::number( mSocket ) + " connected" );
    }
    else
    {
        return;
    }
    
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
            Debug::log( "serverthread", " - " + r.trimmed() );
        }
        else
        {
            Debug::log( "serverthread", dm + r.trimmed() );
        }

        QStringList tokens = r.split( QRegExp( "[ \r\n][ \r\n]*" ) );
        
        QString req = tokens[ 0 ];
        QString cmd = tokens[ 1 ];
        
        if ( req == "GET" )
        {
            QByteArray block;
            QTextStream out( socket );
            
            out << "HTTP/1.1 200 OK\r\n";
            out << "Server: kueued @ " + mHostname + " (Linux)\r\n";     
            out << "Connection: close\r\n";
            
            if ( cmd.startsWith( "/qmon_date" ) )
            {
                openMysqlDB();
                
                QString xml = XML::qmonDate( Database::getSrsForQueue( "NONE", mMysqlDB ) );

                out << "Content-Type: text/xml; charset=\"utf-8\"\r\n";
                out << "\r\n";
                out << "<?xml version='1.0'?>\n\n";
                out << xml;
                
                socket->close();
            }
            if ( cmd.startsWith( "/qmon" ) )
            {
                openMysqlDB();
                
                QString xml = XML::qmon( Database::getSrsForQueue( "NONE", mMysqlDB ) );

                out << "Content-Type: text/xml; charset=\"utf-8\"\r\n";
                out << "\r\n";
                out << "<?xml version='1.0'?>\n\n";
                out << xml;
                
                socket->close();
            }
            else if ( cmd.startsWith( "/srnrs" ) )
            {  
                openMysqlDB();
                
                QString q = cmd.remove( "/srnrs" );

                out << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
                out << "\r\n";

                if ( q.remove( "/" ).isEmpty() )
                {  
                    out << "Please specify queue";
                }
                else if ( !q.contains( "|" ) )
                {  
                    out << "Please specify geo";
                }
                else
                {  
                    //QStringList l = Database::getSrNumsForQueue( q.remove( "/" ).split( "|" ).at( 0 ), q.remove( "/" ).split( "|" ).at( 1 ), mMysqlDB );

                    //for ( int i = 0; i < l.size(); ++i )
                    //{  
                       // out << l.at( i ) + "\n";
                    //}
                }
                
            }
            else if ( cmd.startsWith( "/latestkueue" ) )
            {
                out << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
                out << "\r\n";
                                
                out << Settings::latestVersion();
            }
            else if ( cmd.startsWith( "/detailed" ) )
            {
                QString q = cmd.remove( "/detailed" );

                if ( q.remove( "/" ).isEmpty() )
                {  
                    out << "Please specify sr number";
                }
                else
                {  
                    openSiebelDB();
                
                    out << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
                    out << "\r\n";
                                
                    out << Database::getDetDesc( q, mSiebelDB );
                }
                
                socket->close();
            }
            else if ( cmd.startsWith( "/updateDB" ) )
            {
                openMysqlDB();
                openSiebelDB();
                openQmonDB();
                
                QTime timer;
                timer.start();
              
                Debug::log( "serverthread", "Starting DB update..." );
                Debug::print( "serverthread", "Starting Bomgar update..." );
                    
                QList< BomgarItem > list = Database::getChats( mQmonDB );
                QStringList l;
                    
                for ( int i = 0; i < list.size(); ++i ) 
                {
                    l.append( list.at( i ).id );

                    if ( !Database::bomgarExistsInDB( list.at( i ).id, mMysqlDB ) )
                    {
                        Database::updateBomgarItemInDB( list.at( i ), mMysqlDB );
                    }
                    else if ( ( Database::getBomgarQueueById( list.at( i ).id, mMysqlDB ) != list.at( i ).name ) )
                    {
                        Database::updateBomgarQueue( list.at( i ), mMysqlDB );
                    }
                }
                
                QStringList existList = Database::getQmonBomgarList( mMysqlDB );

                for ( int i = 0; i < existList.size(); ++i ) 
                {
                    if ( !l.contains( existList.at( i ) ) )
                    {
                        Database::deleteBomgarItemFromDB( existList.at( i ), mMysqlDB );
                    }
                }
                
                int btime = timer.elapsed() / 1000;
                timer.restart();

                Debug::print( "serverthread", "Bomgar update finished, took " + QString::number( btime ) + " sec" );
                Debug::print( "serverthread", "Starting Unity update..." );

                out << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
                out << "\r\n";
                out << "Bomgar update took " +  QString::number( btime / 1000 ) + " sec\n";
                
                QList<SiebelItem> ql = Database::getQmonSrs( mSiebelDB );
                QStringList newList;
                    
                for ( int i = 0; i < ql.size(); ++i ) 
                {
                    newList.append( ql.at( i ).id );
                    
                    if ( !Database::siebelExistsInDB( ql.at( i ).id, mMysqlDB ) )
                    {                    
                        Database::insertSiebelItemIntoDB( ql.at( i ), mMysqlDB );
                    }
                    else
                    {
                        if ( Database::siebelQueueChanged( ql.at( i ), mMysqlDB ) )
                        {
                            Database::updateSiebelQueue( ql.at( i ), mMysqlDB );
                        }
                                
                        Database::updateSiebelItem( ql.at( i ), mMysqlDB, mSiebelDB );
                    }
                }
                
                QStringList qexistList = Database::getQmonSiebelList( mMysqlDB );
                
                for ( int i = 0; i < qexistList.size(); ++i ) 
                {
                    if ( !newList.contains( qexistList.at( i ) ) )
                    {
                        Database::deleteSiebelItemFromDB( qexistList.at( i ), mMysqlDB );
                    }
                }
               
                out << "Unity update took " + QString::number( timer.elapsed() / 1000 ) + " sec\n\n";
                out << "UPDATE FINISHED\n";
                
                socket->close();
                
                Debug::print( "serverthread", "Unity update finished, took " + QString::number( timer.elapsed() / 1000 ) + " sec" );
                Debug::log( "serverthread", "DB Update finished" );
            }
            else if ( cmd.startsWith( "/chat" ) )
            {
                openMysqlDB();
                
                QStringList l = Database::getCurrentBomgars();
              
                out << "Content-Type: text/xml; charset=\"utf-8\"\r\n";
                out << "\r\n";
                out << "<?xml version='1.0'?>\n\n";
                out << "<chat>\n";
                
                for ( int i = 0; i < l.size(); ++i )
                {
                    out << "  <bomgar>\n";
                    out << "    <bomgarQ>" + l.at(i).split("|||").at(1) + "</bomgarQ>\n";
                    out << "    <sr>" + l.at(i).split("|||").at(0) + "</sr>\n";
                    out << "  </bomgar>\n";
                }
                
                out << "</chat>";
                
            }
            else if ( cmd.startsWith( "/userqueue" ) )
            {    
                openSiebelDB();
                
                QTime uqTimer;
                uqTimer.start();
                
                QString q = cmd.remove( "/userqueue" );

                if ( q.remove( "/" ).isEmpty() )
                {  
                    out << "Please specify engineer";
                }
                else
                {
                    QString eng = q.remove( "/" ).toUpper();
                    
                    out << "Content-Type: text/xml; charset=\"utf-8\"\r\n";
                    out << "\r\n";
                    out << "<?xml version='1.0'?>\n\n";

                    out << XML::queue( Database::getUserQueue( eng, mSiebelDB ) );

                    Debug::print( "server", "Userqueue for " + eng + " took " + QString::number( uqTimer.elapsed() / 1000 ) + " sec");
                }
            }
            else if ( cmd.startsWith( "/stats" ) )
            {    
                QString q = cmd.remove( "/stats" );
                    
                if ( q.remove( "/" ).isEmpty() )
                {
                    out << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
                    out << "\r\n";
                    out << "Please specify engineer";
                }
                else
                {
                    QString wf = getWF( q );
                    
                    if ( wf == "00000" )
                    {
                        out << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
                        out << "\r\n";
                        out << "Invalid engineer";
                    }
                    else
                    {
                        mNetwork = &mNetwork->net();
                        
                        openSiebelDB();
                        
                        out << "Content-Type: text/xml; charset=\"utf-8\"\r\n";
                        out << "\r\n";
                        out << "<?xml version='1.0'?>\n\n";
                        
                        Statistics statz;
                        
                        QString numbers;
                        QNetworkReply* r = mNetwork->get( QUrl( "http://proetus.provo.novell.com/qmon/closed4.asp?tse=" + q ) );
                        QEventLoop loop;
    
                        QObject::connect( r, SIGNAL( finished() ), 
                                        &loop, SLOT( quit() ) );
                    
                        loop.exec();
                            
                        numbers = r->readAll();
                        
                        QString csat;
                        QNetworkReply* csr = mNetwork->get( QUrl( "http://proetus.provo.novell.com/qmon/custsat.asp?wf=" + wf ) );
                        
                        QObject::connect( csr, SIGNAL( finished() ), 
                                        &loop, SLOT( quit() ) );
                    
                        loop.exec();
                            
                        csat = csr->readAll();
                        
                        QStringList csatList = csat.split( "<br>" );
                        
                        QString tts;
                        QNetworkReply* ttr = mNetwork->get( QUrl( "http://proetus.provo.novell.com/qmon/timetosolutiontse.asp?tse=" +  q ) );
                        
                        QObject::connect( ttr, SIGNAL( finished() ), 
                                        &loop, SLOT( quit() ) );
                    
                        loop.exec();
                            
                        tts = ttr->readAll();
                        QStringList ttsList = tts.split( "<br>" );
                        
                        QString o = numbers.split("<br>").at(0);
                        o.remove( QRegExp( "<(?:div|span|tr|td|br|body|html|tt|a|strong|p)[^>]*>", Qt::CaseInsensitive ) );
                        
                        statz.closedSr = o.split("|").at(0).trimmed();
                        statz.closedCr = o.split("|").at(1).trimmed();
                        
                        QList<ClosedItem> closedList;
                        
                        for ( int i = 0; i < ttsList.size() - 1; ++i )
                        {
                            ClosedItem ci;
                            
                            QString tmp = ttsList.at(i);
                            tmp.remove( QRegExp( "<(?:div|span|tr|td|br|body|html|tt|a|strong|p)[^>]*>", Qt::CaseInsensitive ) );
                            
                            ci.sr = tmp.split( "|" ).at( 1 );
                            ci.tts = tmp.split( "|" ).at( 2 ).toInt();
                            
                            QStringList info = Database::srInfo( ci.sr, mSiebelDB );
                            
                            ci.customer = info.at( 3 ) + " (" + info.at(1) + " " + info.at(2) + ")";
                            ci.bdesc = info.at( 0 );
                            
                            closedList.append( ci );
                        }
                        
                        QList<CsatItem> csatItemList;
                        
                        for ( int i = 0; i < csatList.size() - 1; ++i )
                        {
                            CsatItem csi;
                           
                            QString tmp = csatList.at(i);
                            tmp.remove( QRegExp( "<(?:div|span|tr|td|br|body|html|tt|a|strong|p)[^>]*>", Qt::CaseInsensitive ) );
                            
                            csi.sr = tmp.split( "|" ).at( 1 ).trimmed();
                            
                            if ( tmp.split( "|" ).at( 2 ).isEmpty() )
                            {
                                csi.srsat = 88;
                            }
                            else
                            {
                                csi.srsat = tmp.split( "|" ).at( 2 ).trimmed().toInt();
                            }
                            
                            if ( tmp.split( "|" ).at( 3 ).isEmpty() )
                            {
                                csi.engsat = 88;
                            }
                            else
                            {
                                csi.engsat = tmp.split( "|" ).at( 3 ).trimmed().toInt();
                            }
                            
                            csi.rts = tmp.split( "|" ).at( 4 ).trimmed().toInt();
                                                            
                            QStringList info = Database::srInfo( csi.sr, mSiebelDB );
                            
                            csi.customer = info.at( 3 ) + " (" + info.at(1) + " " + info.at(2) + ")";
                            csi.bdesc = info.at(0);
                            
                            csatItemList.append( csi );
                        }
                        
                        statz.closedList = closedList;
                        statz.csatList = csatItemList;
                        
                        out << XML::stats( statz );
                        mNetwork->destroy();
                    }
                }
                
                socket->close();
            }
            else
            {
                out << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
                out << "\r\n";
                out << "Welcome to kueued\n\n";
                out << "Usage:\n\n";
                out << "  * http://kueue.hwlab.suse.de:8080/qmon\n    Get a list of all SRs in all pseudo queues\n\n";
                out << "  * http://kueue.hwlab.suse.de:8080/qmon/$QUEUE-NAME\n    Get a list of all SRs in $QUEUE-NAME\n\n";
                out << "    This is the qmon xml output.\n";
                out << "\n";
                out << "    Please note that not all fields are available for each SR, if they are not, they aren't shown at all.\n";
                out << "\n";
                out << "    <sr>\n";
                out << "      <id>SR number</id>\n";
                out << "      <queue>Queue</queue>\n";
                out << "      <bomgarQ>Bomgar Queue (if in chat)</bomgarQ>\n";
                out << "      <srtype>sr/cr</srtype>\n";
                out << "      <creator>Only for CRs: who created it?</creator>\n";
                out << "      <customer>\n";
                out << "        <account>Company name</account>\n";
                out << "        <firstname>Customer's first name</firstname>\n";
                out << "        <lastname>Customer's last name</lastname>\n";
                out << "        <title>Customer's title (ie. job)</title>\n";
                out << "        <email>Customer's email</email>\n";
                out << "        <phone>Customer's phone number</phone>\n";
                out << "        <onsitephone>Customer's onsite phone number</onsitephone>\n";
                out << "        <lang>Customer's preferred language</lang>\n";
                out << "      </customer>\n";
                out << "      <severity>SR severity</severity>\n";
                out << "      <status>Current status</status>\n";
                out << "      <bdesc>Brief description</bdesc>\n";
                out << "      <ddesc>Detailed description</ddesc>\n";
                out << "      <geo>Geo</geo>\n";
                out << "      <hours>Support hours</hours>\n";
                out << "      <source>How was the SR opened (web, phone..)</source>\n";
                out << "      <support_program>Customer's contract</support_program>\n";
                out << "      <support_program_long>A slightly more detailed version of the above</support_program_long>\n";
                out << "      <routing_product>Routing product</routing_product>\n";
                out << "      <support_group_routing>What group it is routed to internally</support_group_routing>\n";
                out << "      <int_type>Internal type</int_type>\n";
                out << "      <subtype>Subtype</subtype>\n";
                out << "      <servce_level>Some number indicating something</service_level>\n";
                out << "      <category>SR category (Adv/Knowledge)</category>\n";
                out << "      <respond_via>How the customer would like to be contacted</respond_via>\n";
                out << "      <age>SR age in seconds</age>\n";
                out << "      <lastupdate>Time to last update in seconds</lastupdate>\n";
                out << "      <timeinQ>Time in the current queue in seconds</timeinQ>\n";
                out << "      <sla>SLA left in seconds</sla>\n";
                out << "      <highvalue>Is the customer considered high value?</highvalue>\n";
                out << "      <critsit>Is there a critsit going on with the customer?</critsit>\n";
                out << "    </sr>   \n\n";
                out << "  * http://kueue.hwlab.suse.de:8080/bug/$SRNR\n    Get the bugreport for $SRNR (if any)\n\n";
                out << "Stay tuned for more features!\n";
                
                socket->close();
            }
            
            if ( socket->waitForBytesWritten() )
            {
                Debug::print( "serverthread", "Wrote to socket " + QString::number( mSocket ) );
                socket->disconnectFromHost();
                socket->waitForDisconnected();
                delete socket;
            }
        }
    }    
}

void ServerThread::openMysqlDB()
{
    if ( !QSqlDatabase::database( mMysqlDB ).isOpen() )
    {
        QSqlDatabase mysqlDB = QSqlDatabase::addDatabase( "QMYSQL", mMysqlDB );
       
        mysqlDB.setHostName( Settings::mysqlHost() );
        mysqlDB.setDatabaseName( Settings::mysqlDatabase() );
        mysqlDB.setUserName( Settings::mysqlUser() );
        mysqlDB.setPassword( Settings::mysqlPassword() );
        
        if ( !mysqlDB.open() )
        {
            Debug::print( "database", "Failed to open the database " + mysqlDB.lastError().text() );
        }
        else
        {
            Debug::print( "database", "Opened DB " + mysqlDB.connectionName() );
        }
    }
    else
    {
        Debug::print( "database", "DB already open in this thread " + mMysqlDB );
    }
}

void ServerThread::openQmonDB()
{
    if ( !QSqlDatabase::database( mQmonDB ).isOpen() )
    {
        QSqlDatabase qmonDB = QSqlDatabase::addDatabase( "QODBC", mQmonDB );
        
        qmonDB.setDatabaseName( Settings::qmonDbDatabase() );
        qmonDB.setUserName( Settings::qmonDbUser() );
        qmonDB.setPassword( Settings::qmonDbPassword() );
        
        if ( !qmonDB.open() )
        {
            Debug::print( "database", "Failed to open the Qmon DB " + qmonDB.lastError().text() );
        }
        else
        {
            Debug::print( "database", "Opened DB " + qmonDB.connectionName() );
        }
    }
    else
    {
        Debug::print( "database", "DB already open in this thread " + mQmonDB );
    }
}

void ServerThread::openSiebelDB()
{
    if ( !QSqlDatabase::database( mSiebelDB ).isOpen() )
    {
        QSqlDatabase siebelDB = QSqlDatabase::addDatabase( "QOCI", mSiebelDB );

        siebelDB.setDatabaseName( Settings::siebelDatabase() );
        siebelDB.setHostName( Settings::siebelHost() );
        siebelDB.setPort( 1521 );
        siebelDB.setUserName( Settings::siebelUser() );
        siebelDB.setPassword( Settings::siebelPassword() );

        if ( !siebelDB.open() )
        {
            Debug::print( "database", "Failed to open the Siebel DB " + siebelDB.lastError().text() );
        }
        else
        {
            Debug::print( "database", "Opened DB " + siebelDB.connectionName() );
        }
    }
    else
    {
        Debug::print( "database", "DB already open in this thread " + mSiebelDB );
    }
}

QString ServerThread::getWF( const QString& engineer )
{
    QEventLoop loop;
    QString wfid;
    
    QNetworkReply *reply = mNetwork->get( QUrl( Settings::dBServer() + "/workforce.asp?tse=" + engineer ) );
    
    loop.connect( reply, SIGNAL( readyRead() ),
                  &loop, SLOT( quit() ) );
        
    loop.exec();
       
    wfid = reply->readAll();
    wfid.remove( QRegExp( "<(?:div|span|tr|td|br|body|html|tt|a|strong|p)[^>]*>", Qt::CaseInsensitive ) );
   
    return wfid.trimmed();
}

#include "serverthread.moc"
