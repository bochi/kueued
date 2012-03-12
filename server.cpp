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
#include "serverthread.h"
#include <iostream>
#include <QThread>
#include <QThreadPool>

Server::Server( quint16 port, QObject* parent )
    : QTcpServer( parent )
{
    Debug::log( "server", "Constructing " + QString::number( thread()->currentThreadId() ) );
    
    QThreadPool::globalInstance()->setExpiryTimeout( -1 );
    
    for ( int i = 0; i < QThread::idealThreadCount(); ++i ) 
    {
        ServerThread* s = new ServerThread( 0, this );
        
        connect( s, SIGNAL( finished() ), 
                 this, SLOT( deleteThread() ) );
    }
    
    listen( QHostAddress::Any, port );   
}

void Server::incomingConnection( int socket )
{
    qDebug() << "Threads:" << QThreadPool::globalInstance()->activeThreadCount();
    
    ServerThread* q = new ServerThread( socket, this );
    
    connect( q, SIGNAL( finished() ), 
             this, SLOT( deleteThread() ) );
    
    q->start();
}

void Server::deleteThread()
{
    QThread* t = qobject_cast<QThread*>( sender() );
    Debug::log( "server", "Deleting SrvThread " + QString::number( t->currentThreadId() ) );
    delete t;
}

#include "server.moc"
