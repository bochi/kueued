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

#include "database.h"
#include "debug.h"

#include <QFile>
#include <QDebug>
#include <QDesktopServices>
#include <QDateTime>

Database::Database()
{   
    Debug::print( "database", "Constructing" );
    
    QDir dir = QDir( "/etc/kueued" );

    if ( !dir.exists() )
    {
        dir.mkpath( dir.path() );
    }
    
    mDBfile = dir.path() + "/db.sqlite";
    
    mDb = QSqlDatabase::addDatabase( "QSQLITE" );
    mDb.setDatabaseName( mDBfile );
    
    if ( !mDb.open() )
    {
        Debug::print( "database", "Failed to open the database." );
    }
                         
    QSqlQuery query( mDb );
    
    if ( !query.exec("PRAGMA temp_store = MEMORY") )
    {
        Debug::print( "database", "Error: " + query.lastError().text() );
    }
    
    if ( !query.exec("PRAGMA synchronous = OFF") )
    {
        Debug::print( "database", "Error: " + query.lastError().text() );
    }
    
    if ( !query.exec("PRAGMA journal_mode = MEMORY") )
    {
        Debug::print( "database", "Error: " + query.lastError().text() );
    }
    
    if ( !query.exec("PRAGMA locking_mode = EXCLUSIVE") )
    {
        Debug::print( "database", "Error: " + query.lastError().text() );
    }
    
    if ( !query.exec( "CREATE TABLE IF NOT EXISTS qmon_siebel( ID INTEGER PRIMARY KEY UNIQUE, QUEUE TEXT, SEVERITY TEXT, HOURS TEXT, "
                      "SOURCE TEXT, CONTACTVIA TEXT, ODATE TEXT, ADATE TEXT, QDATE TEXT, STATUS TEXT, "
                      "CONTRACT TEXT, QUEUE1 TEXT, PHONE TEXT, ONSITEPHONE TEXT, GEO TEXT, "
                      "WTF TEXT, ROUTING TEXT, BDESC TEXT, SLA TEXT, DISPLAY TEXT )" ) )

    {
        Debug::print( "database", "Error: " + query.lastError().text() );
    }
    
    if ( !query.exec( "CREATE TABLE IF NOT EXISTS qmon_chat( ID TEXT PRIMARY KEY UNIQUE, SR INTEGER, REPTEAM TEXT, "
                      "NAME TEXT, DATE TEXT, QDATE TEXT, SOMENR INTEGER )" ) )
        {
        Debug::print( "database", "Error: " + query.lastError().text() );
    }
}

Database::~Database()
{   
    Debug::print( "database", "Destroying" );

    mDb.close();    
    QSqlDatabase::removeDatabase( mDb.connectionName() );
}

void Database::insertSiebelItemIntoDB( SiebelItem* item )
{
    Debug::print( "database", "Inserting SiebelItem for " + item->id + " " + item->queue );

    QSqlQuery query( "INSERT INTO qmon_siebel( ID, QUEUE, SEVERITY, HOURS, SOURCE, CONTACTVIA, ODATE, ADATE, QDATE, "
                     "STATUS, CONTRACT, QUEUE1, PHONE, ONSITEPHONE, GEO, WTF, ROUTING, BDESC, SLA )"
                     "VALUES"
                     "( :id, :queue, :severity, :hours, :source, :contactvia, :odate, :adate, :qdate, :status, :contract, "
                     ":queue1, :phone, :onsitephone, :geo, :wtf, :routing, :bdesc, :sla )" );

    query.bindValue( ":id", item->id );
    query.bindValue( ":queue", item->queue );
    query.bindValue( ":severity", item->severity );
    query.bindValue( ":hours", item->hours );
    query.bindValue( ":source", item->source );
    query.bindValue( ":contactvia", item->contactvia );
    query.bindValue( ":odate", convertTime( item->odate ) );
    query.bindValue( ":adate", convertTime( item->adate ) );
    query.bindValue( ":qdate", QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss" ) );
    query.bindValue( ":status", item->status );
    query.bindValue( ":contract", item->contract );
    query.bindValue( ":queue1", item->queue1 );
    query.bindValue( ":phone", item->phone );
    query.bindValue( ":onsitephone", item->onsitephone );
    query.bindValue( ":geo", item->geo );
    query.bindValue( ":wtf", item->wtf );
    query.bindValue( ":routing", item->routing );
    query.bindValue( ":bdesc", item->bdesc );
    query.bindValue( ":sla", convertTime( item->sla ) );

    query.exec();
}

void Database::updateSiebelQueue( SiebelItem* si )
{
    qDebug() << "[" + QDateTime::currentDateTime().toString( "MM-dd hh:mm:ss" ) + "[" + QDateTime::currentDateTime().toString( "MM-dd hh:mm:ss" ) + "DATABASE] Updating Siebel Queue" << si->id << si->queue;
    QSqlQuery query( "UPDATE qmon_siebel SET QUEUE = :queue WHERE id = :id" );
                
    query.bindValue( ":queue", si->queue );
    query.bindValue( ":id", si->id );
                
    query.exec();
}

void Database::updateSiebelSeverity( SiebelItem* si )
{
    Debug::print( "database", "Updating Siebel Severity for " + si->id + " to " + si->severity );
    QSqlQuery query( "UPDATE qmon_siebel SET SEVERITY = :severity WHERE id = :id" );
                
    query.bindValue( ":severity", si->severity );
    query.bindValue( ":id", si->id );
                
    query.exec();
}

void Database::updateSiebelDisplay( const QString& display )
{
    QSqlQuery query( "UPDATE qmon_siebel SET DISPLAY = :display WHERE id = :id" );
                
    query.bindValue( ":display", display.split( "-" ).at( 1 ) );
    query.bindValue( ":id", display.split( "-" ).at( 0 ) );
                
    query.exec();   
}

void Database::deleteSiebelItemFromDB( const QString& id )
{
    Debug::print( "database", "Deleting SiebelItem " + id );
    
    QSqlQuery query;
    query.prepare( "DELETE FROM qmon_siebel WHERE ID = :id" );
    query.bindValue( ":id", id );
    query.exec();
}

QStringList Database::getQmonSiebelList()
{
    QSqlQuery query;
    QStringList l;
    query.prepare( "SELECT ID FROM qmon_siebel" );
    query.exec();
    
    while( query.next() )
    {
        l.append( query.value( 0 ).toString() );
    }
    
    return l;    
}

bool Database::siebelExistsInDB( const QString& id )
{
    QSqlQuery query;
    query.prepare( "SELECT ID FROM qmon_siebel WHERE ( ID = :id )" );
    query.bindValue( ":id", id );
    query.exec();
    
    if ( query.next() )
    {
        return true;
    }
    else 
    {    
        return false;
    }
}

bool Database::siebelQueueChanged( SiebelItem* si  )
{
    QSqlQuery query;
    query.prepare( "SELECT QUEUE FROM qmon_siebel WHERE ( ID = :id )" );
    query.bindValue( ":id", si->id );
    query.exec();

    if ( query.next() )
    {
        if ( query.value( 0 ).toString() == si->queue )
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return true;
    }
}

bool Database::siebelSeverityChanged( SiebelItem* si  )
{
    QSqlQuery query;
    query.prepare( "SELECT SEVERITY FROM qmon_siebel WHERE ( ID = :id )" );
    query.bindValue( ":id", si->id );
    query.exec();

    if ( query.next() )
    {
        if ( query.value( 0 ).toString() == si->severity )
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return true;
    }
}

bool Database::isChat( const QString& id )
{
    QSqlQuery query;
    query.prepare( "SELECT ID FROM qmon_chat WHERE ( SR = :id )" );
    query.bindValue( ":id", id );
    query.exec();
    
    if ( query.next() )
    {
        return true;
    }
    else 
    {    
        return false;
    }
}

QString Database::getQmonBdesc( const QString& id )
{
    QSqlQuery query;
    query.prepare( "SELECT BDESC FROM qmon_siebel WHERE ( ID = :id )" );
    query.bindValue( ":id", id );
    query.exec();
    
    if ( query.next() )
    {
        return query.value( 0 ).toString();
    }
    else
    {
        return "ERROR";
    }
}


/*


                B O M G A R 


*/

void Database::updateBomgarItemInDB( BomgarItem* bi )
{
    Debug::print( "database", "Inserting BomgarItem " + bi->id + " " + bi->sr );
        
    QSqlQuery query( "INSERT INTO qmon_chat( ID, SR, REPTEAM, NAME, DATE, QDATE, SOMENR )"
                     "VALUES"
                     "( :id, :sr, :repteam, :name, :date, :qdate, :somenr )" );
                     
    query.bindValue( ":id", bi->id );
    query.bindValue( ":sr", bi->sr );
    query.bindValue( ":repteam", bi->repteam );
    query.bindValue( ":name", bi->name );
    query.bindValue( ":date", bi->date );
    query.bindValue( ":qdate", QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss" ) );
    query.bindValue( ":somenr", bi->someNumber );
    query.exec();
}

void Database::deleteBomgarItemFromDB( const QString& id )
{
    Debug::print( "database", "Deleting BomgarItem " + id );
    
    QSqlQuery query;
    query.prepare( "DELETE FROM qmon_chat WHERE ID = :id" );
    query.bindValue( ":id", id );
    query.exec();
}

QList< SiebelItem* > Database::getSrsForQueue( const QString& queue )
{
    QSqlQuery query;
    QList< SiebelItem* > list;
    
    query.prepare( "SELECT ID, SEVERITY, HOURS, SOURCE, CONTACTVIA, ODATE, ADATE, QDATE, STATUS, CONTRACT, GEO, BDESC, SLA, DISPLAY "
                   "FROM qmon_siebel WHERE ( QUEUE = :queue )" );
    query.bindValue( ":queue", queue );
    query.exec();
    
    while ( query.next() ) 
    {
        SiebelItem* si = new SiebelItem;
        
        si->id = query.value( 0 ).toString();
        si->severity = query.value( 1 ).toString();
        si->hours = query.value( 2 ).toString();
        si->source = query.value( 3 ).toString();
        si->contactvia = query.value( 4 ).toString();
        si->adate = query.value( 5 ).toString();
        si->odate = query.value( 6 ).toString();
        si->qdate = query.value( 7 ).toString();
        si->status = query.value( 8 ).toString();
        si->contract = query.value( 9 ).toString();
        si->geo = query.value( 10 ).toString();
        si->bdesc = query.value( 11 ).toString();
        si->sla = query.value( 12 ).toString();
        si->display = query.value( 13 ).toString();
        si->isChat = isChat( query.value( 0 ).toString() );
        si->bomgarQ = getBomgarQueue( query.value( 0 ).toString() );
        
        list.append( si );
    }
        
    return list;
}

QStringList Database::getQmonBomgarList()
{
    QSqlQuery query;
    QStringList l;
    query.prepare( "SELECT ID FROM qmon_chat" );
    query.exec();
    
    while( query.next() ) l.append( query.value( 0 ).toString() );
    
    return l;    
}

bool Database::bomgarExistsInDB( const QString& id )
{
    QSqlQuery query;
    query.prepare( "SELECT ID FROM qmon_chat WHERE ( ID = :id )" );
    query.bindValue( ":id", id );
    query.exec();
    
    if ( query.next() )
    {
        return true;
    }
    else 
    {
        return false;
    }
}

void Database::updateBomgarQueue( BomgarItem* bi )
{
    Debug::print( "database", "Updating BomgarQueue for " + bi->id + " " + bi->sr + " to " + bi->name );
    
    QSqlQuery query;
    query.prepare( "UPDATE qmon_chat SET NAME = :name WHERE ID = :id" );
    query.bindValue( ":name", bi->name );
    query.bindValue( ":id", bi->id );
    query.exec();
}

QString Database::getBomgarQueue( const QString& id )
{
    QSqlQuery query;
    query.prepare( "SELECT NAME FROM qmon_chat WHERE ( SR = :id )" );
    query.bindValue( ":id", id );
    query.exec();
    
    if( query.next() ) 
    {
        return query.value( 0 ).toString();
    }
    else
    {
        return "NOCHAT";
    }
}

QString Database::getBomgarQueueById( const QString& id )
{
    QSqlQuery query;
    query.prepare( "SELECT NAME FROM qmon_chat WHERE ( ID = :id )" );
    query.bindValue( ":id", id );
    query.exec();
    
    if( query.next() ) 
    {
        return query.value( 0 ).toString();
    }
    else
    {
        return "NOCHAT";
    }
}

QString Database::convertTime( const QString& dt )
{
    QDateTime d = QDateTime::fromString( dt, "M/d/yyyy h:mm:ss AP" );
    return ( d.toString("yyyy-MM-dd hh:mm:ss") );
}

#include "database.moc"
