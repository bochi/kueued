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
#include "kueuedthreads.h"
#include "dbupdatejob.h"
#include "bomgarupdatejob.h"

#include <QApplication>
#include <QWebElementCollection>
#include <QEvent>

KueuedDB::KueuedDB()
{
    Debug::print( "kueued-db", "Constructing" );
    
    Database* db = new Database();
    
    mThreads = &mThreads->getInstance();
    mTimer = new QTimer(this);
    
    DBUpdateJob j;
    j.start();
    
    connect( mTimer, SIGNAL( timeout() ),
             &j, SLOT( update() ) );    
    
    mTimer->start( Settings::refreshSeconds() * 100 );
    //update();
    qDebug() << "TID" << this->thread() << j.thread();
}

     
KueuedDB::~KueuedDB()
{
    Debug::print( "kueued-db", "Destroying" );
   
    if ( mTimer != 0 )
    {
        mTimer->stop();
        delete mTimer;
        mTimer = 0;
    }
}

void KueuedDB::update()
{

    //mThreads->enqueue( j );

    BomgarUpdateJob* bj = new BomgarUpdateJob();
    mThreads->enqueue( bj );
}

#include "kueueddb.moc"

