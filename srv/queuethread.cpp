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
#include "queuethread.h"
#include "database.h"

#include <QtSql>
#include <QTcpSocket>
#include <QTime>

QueueThread::QueueThread( int sd, const QString& eng, QObject *parent )
     : QThread(parent), 
       mSocket( sd ), 
       mEngineer( eng )
{
}

void QueueThread::run()
{
    QTime myTimer;
    myTimer.start();

    QTcpSocket socket;
    socket.setSocketDescriptor( mSocket );
    
    QByteArray block;
    QTextStream out( &block, QIODevice::WriteOnly );
    
    mDbName = QString::number( currentThreadId() );
    
    if ( !QSqlDatabase::database( mDbName ).isOpen() )
    {
        QSqlDatabase siebelDB = QSqlDatabase::addDatabase( "QOCI", mDbName );

        siebelDB.setDatabaseName( Settings::siebelDatabase() );
        siebelDB.setHostName( Settings::siebelHost() );
        siebelDB.setPort( 1521 );
        siebelDB.setUserName( Settings::siebelUser() );
        siebelDB.setPassword( Settings::siebelPassword() );

        if ( !siebelDB.open() )
        {
            Debug::log( "queuethread", "Failed to open the Siebel DB " + siebelDB.lastError().text() );
        }
    }
    
    QSqlQuery query( QSqlDatabase::database( mDbName ) );
    
    out << (quint16)0;
    out << "Content-Type: text/xml; charset=\"utf-8\"\r\n";
    out << "\r\n";
    out << "<?xml version='1.0'?>\n\n";

    out << XML::queue( Database::getUserQueue( mEngineer, QSqlDatabase::database( mDbName ) ) );

    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));

    socket.write( block );
    socket.disconnectFromHost();
    socket.waitForDisconnected();
    
    Debug::log( "queuethread", "SR list for " + mEngineer + " took " + QString::number( myTimer.elapsed() / 1000 ) + " sec");
}

#include "queuethread.moc"