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

#include "kueueddbupdate.h"
#include "debug.h"

KueuedDbUpdate::KueuedDbUpdate()
{
    mBomgarDone = false;
    mSiebelDone = false;
    
    mDB = new Database();
    
    updateBomgar();
    updateUnity();
}

KueuedDbUpdate::~KueuedDbUpdate()
{
    delete mDB;
}

void KueuedDbUpdate::updateBomgar()
{
    Debug::print( "kueued-dbupdate", "Starting Bomgar update..." );
    
    QList< BomgarItem* > list = Database::getChats();
    QStringList l;
    
    for ( int i = 0; i < list.size(); ++i ) 
    {
        l.append( list.at( i )->id );

        if ( !Database::bomgarExistsInDB( list.at( i )->id ) )
        {
            Database::updateBomgarItemInDB( list.at( i ) );
        }
        else if ( ( Database::getBomgarQueueById( list.at( i )->id ) != list.at( i )->name ) )
        {
            Database::updateBomgarQueue( list.at( i ) );
        }
        
        delete list.at( i );
    }
    
    QStringList existList = Database::getQmonBomgarList();

    for ( int i = 0; i < existList.size(); ++i ) 
    {
        if ( !l.contains( existList.at( i ) ) )
        {
            Database::deleteBomgarItemFromDB( existList.at( i ) );
        }
    }
    
    Debug::print( "kueued-dbupdate", "Bomgar update finished" );
    
    mBomgarDone = true;
    finished();
}

void KueuedDbUpdate::updateUnity()
{
    Debug::print( "kueued-dbupdate", "Starting Unity update..." );
    
    QList<SiebelItem*> l = Database::getQmonSrs();
    QStringList newList;
        
    for ( int i = 0; i < l.size(); ++i ) 
    {
        newList.append( l.at( i )->id );
        
        if ( !Database::siebelExistsInDB( l.at( i )->id ) )
        {                    
            Database::insertSiebelItemIntoDB( l.at( i ) );
        }
        else
        {
            if ( Database::siebelQueueChanged( l.at( i ) ) )
            {
                Database::updateSiebelQueue( l.at( i ) );
            }
                    
            Database::updateSiebelItem( l.at( i ) );
        }
        
        delete l.at( i );
    }
    
    QStringList existList = Database::getQmonSiebelList();
    
    for ( int i = 0; i < existList.size(); ++i ) 
    {
        if ( !newList.contains( existList.at( i ) ) )
        {
            Database::deleteSiebelItemFromDB( existList.at( i ) );
        }
    }
    
    Debug::print( "kueued-dbupdate", "Unity update finished" );
    
    mSiebelDone = true;
    finished();
}

void KueuedDbUpdate::finished()
{
    if ( ( mBomgarDone ) && ( mSiebelDone ) )
    {
        Debug::print( "kueued-dbupdate", "Exiting" );
        exit( 0 );
    }
}

#include "kueueddbupdate.moc"
