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

#include "dbupdate.h"
#include "database.h"
#include "settings.h"
#include "debug.h"
#include "queuethread.h"

#include <QTcpSocket>
#include <QtSql>

UpdateWorker::UpdateWorker(QObject* parent): QObject(parent)
{
    mErbert = false;
    Debug::log( "updateworker", "Constructing " + QString::number( thread()->currentThreadId() ) );
    
    QString tid = QString::number( thread()->currentThreadId() );
    mMysqlDB = tid + "mysqlDB";
    mSiebelDB = tid + "siebelDB";
    mQmonDB = tid + "mQmonDB";
    
    QSqlDatabase mysqlDB = QSqlDatabase::addDatabase( "QMYSQL", mMysqlDB );
    
    mysqlDB.setHostName( Settings::mysqlHost() );
    mysqlDB.setDatabaseName( Settings::mysqlDatabase() );
    mysqlDB.setUserName( Settings::mysqlUser() );
    mysqlDB.setPassword( Settings::mysqlPassword() );
    
    if ( !mysqlDB.open() )
    {
        Debug::log( "database", "Failed to open the database " + mysqlDB.lastError().text() );
    }
    
    QSqlDatabase qmonDB = QSqlDatabase::addDatabase( "QODBC", mQmonDB );
    
    qmonDB.setDatabaseName( Settings::qmonDbDatabase() );
    qmonDB.setUserName( Settings::qmonDbUser() );
    qmonDB.setPassword( Settings::qmonDbPassword() );
    
    if ( !qmonDB.open() )
    {
        Debug::log( "database", "Failed to open the Qmon DB " + qmonDB.lastError().text() );
    }
    
    QSqlDatabase siebelDB = QSqlDatabase::addDatabase( "QOCI", mSiebelDB );

    siebelDB.setDatabaseName( Settings::siebelDatabase() );
    siebelDB.setHostName( Settings::siebelHost() );
    siebelDB.setPort( 1521 );
    siebelDB.setUserName( Settings::siebelUser() );
    siebelDB.setPassword( Settings::siebelPassword() );

    if ( !siebelDB.open() )
    {
        Debug::log( "database", "Failed to open the Siebel DB " + siebelDB.lastError().text() );
    }
}

UpdateWorker::~UpdateWorker()
{

}

void UpdateWorker::update( QTcpSocket* socket )
{
    QTextStream out( socket );

    mErbert = true;

    QTime myTimer;
    myTimer.start();
    Debug::log( "kueued-dbupdate", "Starting Bomgar update..." );
    
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
    
    Debug::log( "kueued-dbupdate", "Bomgar update finished, took " +  QString::number( myTimer.elapsed() / 1000 ) + " sec" );
    
    int btime = myTimer.elapsed();
    myTimer.restart();
    
    Debug::log( "kueued-dbupdate", "Starting Unity update..." );

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
                    
            Database::updateSiebelItem( ql.at( i ), mMysqlDB );
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
    
    Debug::log( "kueued-dbupdate", "Unity update finished, took " + QString::number( myTimer.elapsed() / 1000 ) + " sec" );
    out << "Bomgar update took " +  QString::number( btime / 1000 ) + " sec\n";
    out << "Unity update took " + QString::number( myTimer.elapsed() / 1000 ) + " sec\n";
    out << "UPDATE FINISHED\n";
    
    mErbert = false;
    
    Debug::log( "sd", QString::number( socket->socketDescriptor() ) );
    socket->close();
    socket->waitForDisconnected();
}

UpdateThread::UpdateThread( QObject *parent ) : QThread(parent)
{
    Debug::log( "updatethread", "Constructing" );
}

void UpdateThread::run()
{
    mWorker = new UpdateWorker( this );
    
    connect( this, SIGNAL( hobbeds( QTcpSocket* ) ),
             mWorker, SLOT( update( QTcpSocket* ) ) );
    
    exec();
}

void UpdateThread::update( QTcpSocket* socket )
{
    QTextStream out( socket );
    
    if ( !mWorker->erbert() )
    {
        out << "Starting DB Update...\n";
        emit hobbeds( socket );     
    }
    else 
    {
        out << "Already updating, please try again later.\n";
        socket->close();
        socket->waitForDisconnected();
    }
}


#include "dbupdate.moc"
