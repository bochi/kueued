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

#include "kueueddb.h"
#include "settings.h"
#include "network.h"
#include "database.h"
#include "network.h"
#include "debug.h"

#include <QApplication>
#include <QWebElementCollection>

KueuedDB::KueuedDB()
{
    Debug::print( "kueued-db", "Constructing" );
    
    mDB = new Database;
    
    mSiebelReply = Network::get( QUrl( Settings::dBServer() + "/stefan-siebel.asp" ) );
    mBomgarReply = Network::get( QUrl( Settings::dBServer() + "/chat.asp" ) );
		        
    connect( mSiebelReply, SIGNAL( finished() ),
             this, SLOT( siebelJobDone() ) );

    connect( mBomgarReply, SIGNAL( finished() ), 
   	     this, SLOT( bomgarJobDone() ) );

    mTimer = new QTimer(this);
    
    connect( mTimer, SIGNAL( timeout() ),
             this, SLOT( update() ) );
    
    mTimer->start( Settings::refreshSeconds() * 1000 );
}

     
KueuedDB::~KueuedDB()
{
    Debug::print( "kueued-db", "Destroying" );
    
    mQueueList.clear();
    
    if ( mTimer != 0 )
    {
        mTimer->stop();
        delete mTimer;
        mTimer = 0;
    }
}

void KueuedDB::update()
{
    if ( !mSiebelReply->isRunning() ) 
    {
        mSiebelReply = Network::get( QUrl( Settings::dBServer() + "/stefan-siebel.asp" ) );
    }
    else
    {
        Debug::print( "kueued-db", "Siebel update still running - skipping" );
    }

    if ( !mBomgarReply->isRunning() ) 
    {
        mBomgarReply = Network::get( QUrl( Settings::dBServer() + "/chat.asp" ) );
    }
    else
    {
        Debug::print( "kueued-db", "Bomgar update still running - skipping" );
    }

    connect( mSiebelReply, SIGNAL( finished() ),
             this, SLOT( siebelJobDone() ) );

    connect( mBomgarReply, SIGNAL( finished() ), 
             this, SLOT( bomgarJobDone() ) );
}

void KueuedDB::siebelJobDone()
{
    QString replydata = mSiebelReply->readAll();
   
    replydata.remove( QRegExp( "<(?:div|span|tr|td|body|html|tt|a|strong|p)[^>]*>", Qt::CaseInsensitive ) );
    
    QStringList list = replydata.split( "<br>" );
    QStringList newList;
    QStringList existList = Database::getQmonSiebelList();
        
    if ( !mSiebelReply->error() && !list.isEmpty() )
    {
        for ( int i = 0; i < list.size(); ++i ) 
        {
            if ( list.at( i ).split("|").count() == 19 )
            {
                SiebelItem* si = new SiebelItem;
                    
                si->id = list.at( i ).split( "|" ).at( 1 ).trimmed();
                si->queue = list.at( i ).split( "|" ).at( 2 ).trimmed();
                si->severity = list.at( i ).split( "|" ).at( 3 ).trimmed();
                si->hours = list.at( i ).split( "|" ).at( 4 ).trimmed();
                si->source = list.at( i ).split( "|" ).at( 5 ).trimmed();
                si->contactvia = list.at( i ).split( "|" ).at( 6 ).trimmed();
                si->odate = list.at( i ).split( "|" ).at( 7 ).trimmed();
                si->adate = list.at( i ).split( "|" ).at( 8 ).trimmed();
                si->status = list.at( i ).split( "|" ).at( 9 ).trimmed();
                si->contract = list.at( i ).split( "|" ).at( 10 ).trimmed();
                si->queue1 = list.at( i ).split( "|" ).at( 11 ).trimmed(); 
                si->phone = list.at( i ).split( "|" ).at( 12 ).trimmed();
                si->onsitephone = list.at( i ).split( "|" ).at( 13 ).trimmed();
                si->geo = list.at( i ).split( "|" ).at( 14 ).trimmed(); 
                si->wtf = list.at( i ).split( "|" ).at( 15 ).trimmed();
                si->routing = list.at( i ).split( "|" ).at( 16 ).trimmed();
                si->sla = list.at( i ).split( "|" ).at( 17 ).trimmed();
                si->bdesc = list.at( i ).split( "|" ).at( 18 ).trimmed();
                
                newList.append( si->id );
        
                if ( !Database::siebelExistsInDB( si->id ) )
                {                    
                    Database::insertSiebelItemIntoDB( si );
                }
                else
                {
                    if ( Database::siebelQueueChanged( si ) )
                    {
                        Database::updateSiebelQueue( si );
                    }
                    
                    if ( Database::siebelSeverityChanged( si ) )
                    {
                        Database::updateSiebelSeverity( si );
                    }
                }
                
                delete si;
            }
        }
        
        for ( int i = 0; i < existList.size(); ++i ) 
        {
            if ( !newList.contains( existList.at( i ) ) )
            {
                Database::deleteSiebelItemFromDB( existList.at( i ) );
            }
        }
    }
    else
    {
        Debug::print( "kueued-db", "Siebel Error: " + mSiebelReply->errorString() );
    }
}

void KueuedDB::bomgarJobDone()
{
    QString replydata = mBomgarReply->readAll();
    replydata.remove( QRegExp( "<(?:div|span|tr|td|body|html|tt|a|strong|p)[^>]*>", Qt::CaseInsensitive ) );

    QStringList list = replydata.split( "<br>" );
    list.removeDuplicates();
    
    QStringList existList( Database::getQmonBomgarList() );
    
    if ( !mBomgarReply->error() && !list.isEmpty() )
    {
        for ( int i = 0; i < list.size(); ++i ) 
        {
            if ( ( ( !list.at( i ).isEmpty() ) && 
                 ( list.at( i ).trimmed() != "no data" ) && 
                 ( list.at( i ).trimmed().contains( "|" ) ) ) )
            {
                BomgarItem* bi = new BomgarItem;
        
                bi->id = list.at( i ).trimmed().split( "|" ).at( 1 ).trimmed();
                bi->sr = list.at( i ).trimmed().split( "|" ).at( 2 ).trimmed();
                bi->repteam = list.at( i ).trimmed().split( "|" ).at( 3 ).trimmed();
                bi->name = list.at( i ).trimmed().split( "|" ).at( 4 ).trimmed();
                bi->date = list.at( i ).trimmed().split( "|" ).at( 5 ).trimmed();
                bi->someNumber = list.at( i ).trimmed().split( "|" ).at( 6 ).trimmed();
                
                list.append( bi->id );
        
                if ( !Database::bomgarExistsInDB( bi->id ) )
                {
                    Database::updateBomgarItemInDB( bi );
                }
                else if ( ( Database::getBomgarQueueById( bi->id ) != bi->name ) )
                {
                    Database::updateBomgarQueue( bi );
                }
                
                delete bi;
            }
        }
 
        for ( int i = 0; i < existList.size(); ++i ) 
        {
            if ( !list.contains( existList.at( i ) ) )
            {
                Database::deleteBomgarItemFromDB( existList.at( i ) );
            }
        }
    }
    else
    {
        Debug::print( "kueued-db", "Bomgar Error: " + mBomgarReply->errorString() );
    }
}

#include "kueueddb.moc"

