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

ServerThread::ServerThread( int sd, QObject *parent ) : QRunnable()
{
    mSocket = sd;
    //Database::getLTSScustomers();
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
    
    if ( QSqlDatabase::database( mReportDB ).isOpen() )
    {
        QSqlDatabase::database( mReportDB ).close();
        QSqlDatabase::removeDatabase( mReportDB );
    }
}

void ServerThread::run()
{
    mThreadID = QString::number( QThread::currentThreadId() );

    mMysqlDB = "mysqlDB-" + mThreadID;
    mSiebelDB = "siebelDB-" + mThreadID;
    mQmonDB = "qmonDB-" + mThreadID;
    mReportDB = "reportDB-" + mThreadID;

    QString cmd;
    QString dm;

    char hostname[ 1024 ];
    gethostname( hostname, sizeof( hostname ) );
    mHostname = hostname;
    
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
            out.setCodec("UTF-8");
            
            out << "HTTP/1.1 200 OK\r\n";
            out << "Server: kueued @ " + mHostname + " (Linux)\r\n";     
            
            if ( cmd.startsWith( "/qmon_date" ) )
            {
                Database::openMysqlDB( mMysqlDB );
                
                QString x = XML::qmonDate( Database::getSrsForQueue( "NONE", mMysqlDB ) );

                out << xml();
                
                out << x;
                
                out.flush();
                socket->close();
            }
            if ( cmd.startsWith( "/ltsscustomers" ) )
            {
                Database::openReportDB( mReportDB );
                
                QString x = XML::ltssCust( Database::getLTSScustomersExt( mReportDB ) );
                
                out << xml();
                
                out << x;
                out.flush();
                
                socket->close();
            }
            if ( cmd.startsWith( "/qmon" ) )
            {
                Database::openMysqlDB( mMysqlDB );
                
                QString x = XML::qmon( Database::getSrsForQueue( "NONE", mMysqlDB ) );

                out << xml();
                
                out << x;
                out.flush();
                
                socket->close();
            }
            else if ( cmd.startsWith( "/srnrs" ) )
            {  
                Database::openMysqlDB( mMysqlDB );
                                
                QString q = cmd.remove( "/srnrs" );

                out << text();

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
                    QStringList l = Database::getSrnumsForQueue( q.remove( "/" ).split( "|" ).at( 0 ), q.remove( "/" ).split( "|" ).at( 1 ), mMysqlDB );

                    for ( int i = 0; i < l.size(); ++i )
                    {  
                        out << l.at( i ) + "\n";
                    }
                }
                
                out.flush();
                
            }
            else if ( cmd.startsWith( "/srforcr" ) )
            {  
                Database::openMysqlDB( mMysqlDB );
                
                QRegExp srnr( "^[0-9]{11}$" );
                QString q = cmd.remove( "/srforcr" );
                
                if ( ( q.remove( "/" ).isEmpty() ) || ( !srnr.exactMatch( q.remove( "/" ) ) ) )
                {  
                    out << text();
                    out << "No SR number";
                }
                else
                {  
                    out << text();
                    out << Database::getSrForCr( q, mMysqlDB, mReportDB );
                }
                
                out.flush();
            }
            else if ( cmd.startsWith( "/srinfo" ) )
            {  
                Database::openSiebelDB( mSiebelDB );
                Database::openMysqlDB( mMysqlDB );
                
                QRegExp srnr( "^[0-9]{11}$" );
                QString q = cmd.remove( "/srinfo" );

                if ( ( q.remove( "/" ).isEmpty() ) || ( !srnr.exactMatch( q.remove( "/" ) ) ) )
                {  
                    out << text();
                    out << "No SR number";
                }
                else
                {  
                    out << xml();
                    out << XML::sr( Database::getSrInfo( q.remove( "/" ), mSiebelDB, mMysqlDB, mReportDB ) );
                }
                
                out.flush();
            }
            else if ( cmd.startsWith( "/srstatus" ) )
            {  
                Database::openSiebelDB( mSiebelDB );
                
                QRegExp srnr( "^[0-9]{11}$" );
                QString q = cmd.remove( "/srstatus" );

                if ( ( q.remove( "/" ).isEmpty() ) || ( !srnr.exactMatch( q.remove( "/" ) ) ) )
                {  
                    out << text();
                    out << "No SR number";
                }
                else
                {  
                    out << text();
                    out << Database::getSrStatus( q.remove( "/" ), mSiebelDB );
                }
                
                out.flush();
            }
            else if ( cmd.startsWith( "/latestkueue" ) )
            {
                out << text();
                                
                out << Settings::latestVersion();
                
                out.flush();
            }
            else if ( cmd.startsWith( "/detailed" ) )
            {
                QString q = cmd.remove( "/detailed" );

                if ( q.remove( "/" ).isEmpty() )
                {  
                    out << "Please specify sr number";
                    out.flush();
                }
                else
                {  
                    Database::openSiebelDB( mSiebelDB );
                
                    out << text();
                                
                    out << Database::getDetDesc( q, mSiebelDB );
                    out.flush();
                }
                
                socket->close();
            }
            else if ( cmd.startsWith( "/ltssupdate" ) )
            {
                Database::openMysqlDB( mMysqlDB );
                Database::openReportDB( mReportDB );
                
                int btime;
                QTime timer;
                timer.start();
                
                Debug::print( "serverthread", "Starting LTSS update..." );
                
                Database::updateLTSScustomers( mReportDB, mMysqlDB );
                btime = timer.elapsed() / 1000;
                
                Debug::print( "serverthread", "LTSS update finished, took " + QString::number( btime ) + " sec" );
                
                out << text();
                
                out << "LTSS update took " +  QString::number( btime ) + " sec\n";   
                out << "LTSS Customer List updated.\n";

                out.flush();
                socket->close();
            }
            else if ( cmd.startsWith( "/updateDB" ) )
            {
                bool full = false;
                QString q = cmd.remove( "/updateDB" );

                if ( q.remove( "/" ).isEmpty() )
                {  
                    full = false;
                }
                else if ( q.remove( "/" ) == "full" )
                {
                    full = true;
                }
                else
                {
                    full = false;
                }
                
                Database::openMysqlDB( mMysqlDB );
                Database::openSiebelDB( mSiebelDB );
                Database::openQmonDB( mQmonDB );
                
                int btime;
                QTime timer;
                timer.start();
              
                Debug::print( "serverthread", "Starting DB update..." );
              
                if ( full )
                {
                    Debug::print( "serverthread", "Starting PseudoQ update..." );
                    
                    Database::updatePseudoQueues( mQmonDB, mMysqlDB );
                    
                    btime = timer.elapsed() / 1000;
                    timer.restart();
                    
                    Debug::print( "serverthread", "PseudoQ update finished, took " + QString::number( btime ) + " sec" );
                    
                    out << text();
                    
                    out << "PseudoQ update took " +  QString::number( btime ) + " sec\n";
                }
                    
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
                
                btime = timer.elapsed() / 1000;
                timer.restart();

                Debug::print( "serverthread", "Bomgar update finished, took " + QString::number( btime ) + " sec" );
                Debug::print( "serverthread", "Starting Unity update..." );

                out << "Bomgar update took " +  QString::number( btime ) + " sec\n";
                
                QList<SiebelItem> ql = Database::getQmonSrs( mSiebelDB, mMysqlDB );
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
                out.flush();
                
                socket->close();
                
                Debug::print( "serverthread", "Unity update finished, took " + QString::number( timer.elapsed() / 1000 ) + " sec" );
                Debug::print( "serverthread", "DB Update finished" );
            }
            else if ( cmd.startsWith( "/chat" ) )
            {
                Database::openMysqlDB( mMysqlDB );
                
                QStringList l = Database::getCurrentBomgars();
              
                out << xml();
                
                out << "<chat>\n";
                
                for ( int i = 0; i < l.size(); ++i )
                {
                    out << "  <bomgar>\n";
                    out << "    <bomgarQ>" + l.at(i).split("|||").at(1) + "</bomgarQ>\n";
                    out << "    <sr>" + l.at(i).split("|||").at(0) + "</sr>\n";
                    out << "  </bomgar>\n";
                }
                
                out << "</chat>";
                out.flush();
                socket->close();
                
            }
            else if ( cmd.startsWith( "/pseudoQ" ) )
            {
                Database::openMysqlDB( mMysqlDB );
                
                QStringList pl = Database::getPseudoQueues( mMysqlDB );
                
                out << text();
                
                for ( int i = 0; i < pl.size(); ++i )
                {
                    out << pl.at(i) + "\n";
                }
                
                out.flush();
                socket->close();
            }
            else if ( cmd.startsWith( "/unityURL" ) )
            {
                out << text();
                
                out << Settings::unityURL() + "\n";
                
                out.flush();
                socket->close();
            }
            else if ( cmd.startsWith( "/assign" ) )
            {
                QString q = cmd.remove( "/assign" );
                
                if ( q.contains( "%7C" ) )
                {
                    q.replace( "%7C", "|" );
                }
                
                if ( q.remove( "/" ).isEmpty() )
                {  
                    out << "Please specify sr number and engineer delimited by |";
                }
                else if ( !q.contains( "|" ) )
                {  
                    out << "Please specify engineer";
                }
                
                else
                {
                    Network* net = new Network();
                
                    QEventLoop loop;
                    QString o;
                    QNetworkReply* ass;
                            
                    ass = net->get( QUrl( "http://proetus.provo.novell.com/qmon/assign2.asp?sr=" + q.remove( "/" ).split( "|" ).at( 0 ) + 
                                               "&owner=" + q.remove( "/" ).split( "|" ).at( 1 ) + "&force=1" ) );
                    
                    QObject::connect( ass, SIGNAL( finished() ), 
                                      &loop, SLOT( quit() ) );
  
                    loop.exec();
                    
                    o = ass->readAll();

                    out << text();
                    
                    out << o;
                    
                    socket->close();
                    
                    delete net;
                }
                
                out.flush();
            }
            else if ( cmd.startsWith( "/userqueue" ) )
            {    
                Database::openSiebelDB( mSiebelDB );
                Database::openMysqlDB( mMysqlDB );
                
                QTime uqTimer;
                uqTimer.start();
                
                QString q = cmd.remove( "/userqueue" );

                if ( q.startsWith( "/full/" ) )
		        {
		            QString eng = q.remove( "/full/" ).remove( "/" ).toUpper();
                   
                    out << xml();

                    out << XML::queue( Database::getUserQueue( eng, mSiebelDB, mMysqlDB, mReportDB, true ) );
                    out.flush();
                }
                else if ( q.remove( "/" ).isEmpty() )
                {
		            out << "Please specify engineer.";
                }
                else
                {
                    QString eng = q.remove( "/" ).toUpper();
                    
                    out << xml();

                    out << XML::queue( Database::getUserQueue( eng, mSiebelDB, mMysqlDB, mReportDB ) );
                    out.flush();
                }
                
                out.flush();
                socket->close();
            }
            else if ( cmd.startsWith( "/stats" ) )
            {    
                QString q = cmd.remove( "/stats" );
                    
                if ( q.remove( "/" ).isEmpty() )
                {
                    out << text();
                    out << "Please specify engineer";
                }
                else
                {
                    Network* net = new Network();
                    
                    QString wf = getWF( q, net );
                    
                    if ( wf == "00000" )
                    {
                        out << "Invalid engineer";
                    }
                    else
                    {
                        Database::openSiebelDB( mSiebelDB );
                        
                        Statistics statz;
                        
                        QString numbers;
                        QNetworkReply* r = net->get( QUrl( "http://proetus.provo.novell.com/qmon/closed2.asp?tse=" + q ) );
                        QEventLoop loop;
    
                        QObject::connect( r, SIGNAL( finished() ), 
                                        &loop, SLOT( quit() ) );
                    
                        loop.exec();
                        
                        if ( !r->error() )
                        {
                            numbers = r->readAll();
                        }
                        else
                        {
                            numbers = "ERROR";
                        }
                        
                        QString csat;
                        QNetworkReply* csr = net->get( QUrl( "http://proetus.provo.novell.com/qmon/custsat.asp?wf=" + wf ) );
                        
                        QObject::connect( csr, SIGNAL( finished() ), 
                                        &loop, SLOT( quit() ) );
                    
                        loop.exec();
                         
                        QStringList csatList;
                        
                        if ( !csr->error() )
                        {
                            csat = csr->readAll();
                            
                            if ( csat.contains( "<br>" ) )
                            {
                                csatList = csat.split( "<br>" );
                            }
                        
                        }
                        else
                        {
                            csat = "ERROR";
                        }
                        
                        QString tts;
                        QNetworkReply* ttr = net->get( QUrl( "http://proetus.provo.novell.com/qmon/timetosolutiontse.asp?tse=" +  q ) );
                        
                        QObject::connect( ttr, SIGNAL( finished() ), 
                                        &loop, SLOT( quit() ) );
                    
                        loop.exec();
                        
                        QStringList ttsList;
                        
                        if ( !ttr->error() )
                        {
                            tts = ttr->readAll();
                            
                            if ( tts.contains( "<br>" ) )
                            {
                                ttsList = tts.split( "<br>" );
                            }
                        }
                        else
                        {
                            tts = "ERROR";
                        }
                        
                        if ( tts != "ERROR" && csat != "ERROR" && numbers != "ERROR" )
                        {
                            if ( numbers.contains( "<br>" ) )
                            {
                                QString o = numbers.split("<br>").at(0);
                                o.remove( QRegExp( "<(?:div|span|tr|td|br|body|html|tt|a|strong|p)[^>]*>", Qt::CaseInsensitive ) );
                            
                                statz.closedSr = o.split("|").at(0).trimmed();
                                statz.closedCr = o.split("|").at(1).trimmed();
                            }
                            
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
                            
                            out << xml();
                            
                            out << XML::stats( statz );
                        }
                        else
                        {
                            out << text();
                            out << "ERROR";
                        }
                    }
                    
                    delete net;      
                }
                
                out.flush();
                socket->close();
            }
            else
            {
                out << text();
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
                
                out.flush();
                socket->close();
            }
            
            if ( socket->waitForBytesWritten() )
            {
                socket->disconnectFromHost();
                
                if ( socket->state() != QAbstractSocket::UnconnectedState )
                {
                    socket->waitForDisconnected();
                }
                
                delete socket;
            }
        }
    }    
}



QString ServerThread::text()
{
    return( "Content-Type: text/plain; charset=\"utf-8\"\r\n"
            "\r\n" );
}

QString ServerThread::xml()
{
    return( "Content-Type: text/xml; charset=\"utf-8\"\r\n"
            "\r\n"
            "<?xml version='1.0'?>\n\n" );
}

QString ServerThread::getWF( const QString& engineer, Network* net )
{
    QEventLoop loop;
    QString wfid;
    
    QNetworkReply *reply = net->get( QUrl( Settings::dBServer() + "/workforce.asp?tse=" + engineer ) );
    
    loop.connect( reply, SIGNAL( readyRead() ),
                  &loop, SLOT( quit() ) );
        
    loop.exec();
       
    wfid = reply->readAll();
    wfid.remove( QRegExp( "<(?:div|span|tr|td|br|body|html|tt|a|strong|p)[^>]*>", Qt::CaseInsensitive ) );
   
    return wfid.trimmed();
}

#include "serverthread.moc"
