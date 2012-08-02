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
            out.setCodec("UTF-8");
            
            out << "HTTP/1.1 200 OK\r\n";
            out << "Server: kueued @ " + mHostname + " (Linux)\r\n";     
            
            if ( cmd.startsWith( "/qmon_date" ) )
            {
                openMysqlDB();
                
                QString x = XML::qmonDate( Database::getSrsForQueue( "NONE", mMysqlDB ) );

                out << xml();
                
                out << x;
                
                out.flush();
                socket->close();
            }
            if ( cmd.startsWith( "/ptf" ) )
            {
                QString q = cmd.remove( "/ptf" );
                
                out << text();
                
                if ( q.remove( "/" ).isEmpty() )
                {  
                    out << "No filename given";
                }
                else
                {
                    Network* net = new Network();
                    
                    QEventLoop loop;
                    QNetworkReply* r;
                    QString res;
                    QString id;
                    
                    r = net->get( QUrl( Settings::l3Server() + "/api/1/rpm/?filename=" + q + "&username=" + Settings::l3User() + "&api_key=" + Settings::l3ApiKey() ) );
                    r->ignoreSslErrors();
                       
                    QObject::connect( r, SIGNAL( finished() ), 
                                      &loop, SLOT( quit() ) );
    
                    loop.exec();
                        
                    res = r->readAll();
                    
                    if ( res.contains( "build" ) )
                    {
                        QStringList values = res.split( "," );
                        
                        for ( int i = 0; i < values.size(); ++i )
                        {
                            if ( values.at( i ).contains( "build" ) )
                            {
                                id = values.at( i ).split( "build\": " ).at( 1 );
                                id.remove( "/api/1/build/" );
                                id.remove( "/" );
                                id.remove( "\"" );
                            }
                        }
                                            
                        r = net->get( QUrl( Settings::l3Server() + "/api/1/publish/?build=" + id + "&username=" + Settings::l3User() + "&api_key=" + Settings::l3ApiKey() ) );
                        r->ignoreSslErrors();
                        
                        QObject::connect( r, SIGNAL( finished() ), 
                                        &loop, SLOT( quit() ) );

                        loop.exec();
  
                        QString url = r->readAll();
  
                        if ( url.contains( "url\":" ) )
                        {
                            url = url.split( "url\": " ).at( 1 );
                            url.remove( "}" );
                            url.remove( "]" );
                            url.remove( "\"" );
                        
                            if ( url.contains( "you.novell.com" ) )
                            {
                                url.replace( "https://you.novell.com/update", "http://kueue.hwlab.suse.de/ptfold" );
                            }
                            else
                            {
                                url.replace( "https://ptf.suse.com", "http://kueue.hwlab.suse.de/ptf" );
                            }
                            
                            out << url + "/" + q;
                        }
                        else
                        {
                            out << "Unable to find " + q;
                        }
                    }
                    else
                    {
                        out << "Unable to find " + q;
                    }
                        
                    delete net;
                }
                
                out.flush(); 
                socket->close();
            }
            if ( cmd.startsWith( "/qmon" ) )
            {
                openMysqlDB();
                
                QString x = XML::qmon( Database::getSrsForQueue( "NONE", mMysqlDB ) );

                out << xml();
                
                out << x;
                out.flush();
                
                socket->close();
            }
            else if ( cmd.startsWith( "/srnrs" ) )
            {  
                openMysqlDB();
                                
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
                    //QStringList l = Database::getSrNumsForQueue( q.remove( "/" ).split( "|" ).at( 0 ), q.remove( "/" ).split( "|" ).at( 1 ), mMysqlDB );

                    //for ( int i = 0; i < l.size(); ++i )
                    //{  
                       // out << l.at( i ) + "\n";
                    //}
                }
                
                out.flush();
                
            }
            else if ( cmd.startsWith( "/rpmversions" ) )
            {  
                QString q = cmd.remove( "/rpmversions" );

                out << text();

                if ( q.remove( "/" ).isEmpty() )
                {  
                    out << "Syntax:\n\n";
                    out << "/rpmversions/PRODUCT|PACKAGE\n\n";
                    out << "Valid products:\n\n";
                    out << "  * SLES11-SP(1|2)-(i386|x86_64)\n";
                }
                else if ( !q.contains( "|" ) )
                {  
                    out << "Please specify packagename";
                }
                else
                {  
                    QProcess p;
                    QStringList args;
                    
                    args.append( q.remove( "/" ).split( "|" ).at( 0 ) );
                    args.append( q.remove( "/" ).split( "|" ).at( 1 ) );
                    
                    p.start( "/usr/bin/pversions", args );
                    
                    if (  !p.waitForFinished ( -1 ) )
                    {
                        return;
                    }
                
                    QString o = p.readAllStandardOutput(); 
                    QStringList l = o.split( "][" );
                    
                    for ( int i = 0; i < l.size(); ++i ) 
                    {
                        QString x = l.at( i );
                        x.remove( "[" ).remove( "]" );
                        out << x + "\n";
                    }
                }
                
                out.flush();
            }
            else if ( cmd.startsWith( "/validversion" ) )
            {  
                QString q = cmd.remove( "/validversion" );
                QStringList split = q.remove( "/" ).split( "|" );
 
                out << text();

                if ( ( q.remove( "/" ).isEmpty() ) || ( split.size() < 3 ) )
                {  
                    out << "Syntax:\n\n";
                    out << "/validversion/PRODUCT|PACKAGE|VERSION\n\n";
                    out << "Valid products:\n\n";
                    out << "  * SLES11-SP(1|2)-(i386|x86_64)\n\n";
                    out << "Result:\n\n";
                    out << "  * 0 (invalid version)\n";
                    out << "  * 1 (valid version)\n";
                }
                else
                {  
                    QProcess p;
                    QStringList args;
                 
                    args.append( split.at( 0 ) );
                    args.append( split.at( 1 ) );
                    
                    p.start( "/usr/bin/pversions", args );
                    
                    if (  !p.waitForFinished ( -1 ) )
                    {
                        return;
                    }
                
                    QString r = "0";
                    QString o = p.readAllStandardOutput(); 
                    QStringList l;
                    
                    if ( o.contains( "][" ) )
                    {
                        l = o.split( "][" );
                        
                        for ( int i = 0; i < l.size(); ++i ) 
                        {
                            
                            QString x = l.at( i );
                            x.remove( "[" ).remove( "]" );
                            qDebug() << x << split.at( 2 );
                            if ( x.trimmed() == split.at( 2 ).trimmed() )
                            {
                                r = "1";
                            }
                        }
                    }
                    else
                    {
                        QString x = o.remove( "[" ).remove( "]" ).trimmed();

                        if ( x == split.at( 2 ) )
                        {
                            r = "1";
                        }
                    }
                    

                    
                    out << r;
                }
                
                out.flush();
            }
            else if ( cmd.startsWith( "/srinfo" ) )
            {  
                openSiebelDB();
                openMysqlDB();
                
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
                    out << XML::sr( Database::getSrInfo( q.remove( "/" ), mSiebelDB, mMysqlDB ) );
                }
                
                out.flush();
            }
            else if ( cmd.startsWith( "/srstatus" ) )
            {  
                openSiebelDB();
                
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
                    openSiebelDB();
                
                    out << text();
                                
                    out << Database::getDetDesc( q, mSiebelDB );
                    out.flush();
                }
                
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
                
                openMysqlDB();
                openSiebelDB();
                openQmonDB();
                
                int btime;
                QTime timer;
                timer.start();
              
                Debug::log( "serverthread", "Starting DB update..." );
              
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
                out.flush();
                
                socket->close();
                
                Debug::print( "serverthread", "Unity update finished, took " + QString::number( timer.elapsed() / 1000 ) + " sec" );
                Debug::log( "serverthread", "DB Update finished" );
            }
            else if ( cmd.startsWith( "/chat" ) )
            {
                openMysqlDB();
                
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
                
            }
            else if ( cmd.startsWith( "/pseudoQ" ) )
            {
                openMysqlDB();
                
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
                            
                    ass = net->get( QUrl( "http://proetus.provo.novell.com/qmon/assign.asp?sr=" + q.remove( "/" ).split( "|" ).at( 0 ) + 
                                               "&owner=" + q.remove( "/" ).split( "|" ).at( 1 ) ) );
                    
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
                openSiebelDB();
                openMysqlDB();
                
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
                    
                    out << xml();

                    out << XML::queue( Database::getUserQueue( eng, mSiebelDB, mMysqlDB ) );
                    out.flush();
                    Debug::print( "server", "Userqueue for " + eng + " took " + QString::number( uqTimer.elapsed() / 1000 ) + " sec");
                }
                
                out.flush();
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
                        openSiebelDB();
                        
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
                Debug::print( "serverthread", "Wrote to socket " + QString::number( mSocket ) );
                
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

bool ServerThread::openMysqlDB()
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
            return false;
        }
        else
        {
            Debug::print( "database", "Opened DB " + mysqlDB.connectionName() );
            return true;
        }
    }
    else
    {
        Debug::print( "database", "DB already open in this thread " + mMysqlDB );
        return true;
    }
}

bool ServerThread::openQmonDB()
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
            return false;
        }
        else
        {
            Debug::print( "database", "Opened DB " + qmonDB.connectionName() );
            return true;
        }
    }
    else
    {
        Debug::print( "database", "DB already open in this thread " + mQmonDB );
        return true;
    }
}

bool ServerThread::openSiebelDB()
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
            return false;
        }
        else
        {
            Debug::print( "database", "Opened DB " + siebelDB.connectionName() );
            return true;
        }
    }
    else
    {
        Debug::print( "database", "DB already open in this thread " + mSiebelDB );
        return true;
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
