/*
                kueue - keep track of your SR queue
             (C) 2011 Stefan Bogner <sbogner@suse.com>

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

#include "qmon.h"
#include "data/database.h"
#include "settings.h"
#include "network.h"
#include "kueue.h"
#include "ui/html.h"

#include <QMessageBox>
#include <QMenu>
#include <QWebFrame>
#include <QWidgetAction>
#include <QWebInspector>
#include <QShortcut>
#include <QGridLayout>
#include <QToolButton>
#include <QWebElementCollection>

Qmon::Qmon()
{
    qDebug() << "[MONITOR] Constructing";
    
    mTimer = new QTimer( this );
    
    connect( mTimer, SIGNAL( timeout() ),
             this, SLOT( update() ) );

    if ( Settings::monitorEnabled() )
    {
        mBomgarReply = Kueue::download( QUrl( Settings::dBServer() + "/chat.asp" ) );
        mSiebelReply = Kueue::download( QUrl( Settings::dBServer() + "/stefan-siebel.asp" ) );
        
        connect( mSiebelReply, SIGNAL( finished() ),
                 this, SLOT( siebelJobDone() ) );
        connect( mBomgarReply, SIGNAL( finished() ), 
                 this, SLOT( bomgarJobDone() ) );
        
        mTimer->start( Settings::monitorMinutes() * 60000 );
    }
    
    QStringList queueList = Settings::queuesToMonitor();
    
    for ( int i = 0; i < queueList.size(); ++i ) 
    {
        mQueueList.append( queueList.at( i ).split( "|" ).at( 1 ) );
    }
}

Qmon::~Qmon()
{
    qDebug() << "[MONITOR] Destroying";
    
    mQueueList.clear();
    
    if ( mTimer != 0 )
    {
        mTimer->stop();
        delete mTimer;
        mTimer = 0;
    }
}

void Qmon::update()
{
    if ( !mBomgarReply->isRunning() ) 
    {
        mBomgarReply = Kueue::download( QUrl( Settings::dBServer() + "/chat.asp" ) );
        
        connect( mBomgarReply, SIGNAL( finished() ), 
                 this, SLOT( bomgarJobDone() ) );
    }
    else
    {
        qDebug() << "[MONITOR] Bomgar update still running - skipping";
    }

    if ( !mSiebelReply->isRunning() ) 
    {
        mSiebelReply = Kueue::download( QUrl( Settings::dBServer() + "/stefan-siebel.asp" ) );
        
        connect( mSiebelReply, SIGNAL( finished() ), 
                 this, SLOT( siebelJobDone() ) );
    }
    else
    {
        qDebug() << "[MONITOR] Siebel update still running - skipping";
    }
}

void Qmon::siebelJobDone()
{
    QString replydata = mSiebelReply->readAll();
    bool initial = false;
    
    replydata.remove( QRegExp( "<(?:div|span|tr|td|body|html|tt|a|strong|p)[^>]*>", Qt::CaseInsensitive ) );
    
    QStringList list = replydata.split( "<br>" );
    QStringList newList;
    QStringList existList = Database::getQmonSiebelList();
    
    if ( existList.isEmpty() )
    {
        initial = true;
    }
        
    if ( !mSiebelReply->error() )
    {
        if ( initial ) emit initialUpdate( list.size(), 1u );
        
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
                    
                    if ( !initial && mQueueList.contains( si->queue ) )
                    {
                        if ( si->severity == "Low" )
                        {               
                            Kueue::notify( "kueue-monitor-low", "New SR in " + QString( si->queue ), 
                                               "<b>SR#" + si->id + "</b><br>" + si->bdesc, si->id );
                        }
                        else if ( si->severity == "Medium" )
                        {
                            Kueue::notify( "kueue-monitor-medium", "New SR in " + QString( si->queue ), 
                                               "<b>SR#" + si->id + "</b><br>" + si->bdesc, si->id );
                        }
                        else if ( si->severity == "Urgent" )
                        {
                            Kueue::notify( "kueue-monitor-urgent", "New SR in " + QString( si->queue ), 
                                               "<b>SR#" + si->id + "</b><br>" + si->bdesc, si->id );
                        }
                        else if ( si->severity == "High" )
                        {
                            Kueue::notify( "kueue-monitor-high", "New SR in " + QString( si->queue ), 
                                               "<b>SR#" + si->id + "</b><br>" + si->bdesc, si->id );
                        }

                        if ( Settings::animateQmon() ) 
                        {
			    Kueue::attention( true );
			}
                    }
                }
                else
                {
                    if ( Database::siebelQueueChanged( si ) )
                    {
                        qDebug() << "[MONITOR] Siebel Queue Changed for SR" << list.at( i ).split( "|" ).at( 1 ).trimmed();
                        Database::updateSiebelQueue( si );
                    }
                    
                    if ( Database::siebelSeverityChanged( si ) )
                    {
                        qDebug() << "[MONITOR] Siebel Severity Changed for SR" << list.at( i ).split( "|" ).at( 1 ).trimmed();
                        Database::updateSiebelSeverity( si );
                    }
                }
                
                delete si;
            }
            
            emit initialUpdateProgress( i );
        }
        
        emit initialUpdateDone();
    
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
        qDebug() << "[MONITOR] Siebel Error:" << mSiebelReply->errorString();
    }
    
    emit qmonDataChanged();
}

void Qmon::bomgarJobDone()
{
    QString replydata = mBomgarReply->readAll();
    QStringList list;
    QStringList existList( Database::getQmonBomgarList() );
    bool changed = false;
    
    if ( !mBomgarReply->error() )
    {
        replydata.remove( QRegExp( "<(?:div|span|tr|td|body|html|tt|a|strong|p)[^>]*>", Qt::CaseInsensitive ) );
    
        QStringList list = replydata.split( "<br>" );
        list.removeDuplicates();
    
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
                    changed = true;
                }
                else if ( ( Database::getBomgarQueueById( bi->id ) != bi->name ) )
                {
                    Database::updateBomgarQueue( bi );
                    mNotifiedList.removeAll( bi->sr );
                    changed = true;
                }
                
                if ( ( Settings::monitorPersonalBomgar() ) && 
                     ( bi->name == Settings::bomgarName() ) && 
                     ( !mNotifiedList.contains( bi->sr ) ) )
                {
                    mNotifiedList.append( bi->sr );
                    Kueue::notify( "kueue-personal-bomgar", "Customer in Bomgar", "<b>SR#" + bi->sr + "</b>", "<br>" );
                    if ( Settings::animateQmon() ) Kueue::attention( true );
                }
                
                delete bi;
            }
        }
 
        for ( int i = 0; i < existList.size(); ++i ) 
        {
            if ( !list.contains( existList.at( i ) ) )
            {
                Database::deleteBomgarItemFromDB( existList.at( i ) );
                changed = true;
            }
        }
    }
    else
    {
        Kueue::notify( "kueue-general", "Error", "<b>Updating Bomgar Data failed.</b><br>No VPN connection or networking issues?", "" );
        qDebug() << "[MONITOR] Bomgar Error:" << mBomgarReply->errorString();
    }
    
    if ( changed )
    {
        emit qmonDataChanged();
    }
}


void Qmon::whoIsInBomgarJobDone( QNetworkReply* reply )
{
    QString replydata = reply->readAll();
    QString who;
    QString whoItem;
    QStringList whoList;
    
    if ( !reply->error() )
    {
        who = replydata.split( "<b>Time In System (min)</b></tr>" ).at(1);
        whoList = who.split( "<tr><tr><td><td>" );
        
        for ( int i = 0; i < whoList.size(); ++i ) 
        {
           WhoIsInBomgarItem* w = new WhoIsInBomgarItem;
           whoItem = whoList.at( i );
           whoItem.remove( "<tr><td><td>" );
           
           w->name = whoItem.split( "<td><td>" ).at( 0 );
           w->sr = whoItem.split( "<td><td>" ).at( 1 );
           w->timeInQueue = whoItem.split( "<td><td>" ).at( 2 );
           w->timeInSystem = whoItem.split( "<td><td>" ).at( 3 );
           
           qDebug() << "[MONITOR] New WhoIsInBomgarItem" << w->name << w->sr << w->timeInQueue << w->timeInSystem;
           
           delete w;
        }        
    }
}


#include "qmon.moc"

