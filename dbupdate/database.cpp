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
#include "settings.h"

#include <QDateTime>

Database::Database()
{   
    QSqlDatabase mysqlDB = QSqlDatabase::addDatabase("QMYSQL", "mysqlDB" );
    
    mysqlDB.setHostName( Settings::mysqlHost() );
    mysqlDB.setDatabaseName( Settings::mysqlDatabase() );
    mysqlDB.setUserName( Settings::mysqlUser() );
    mysqlDB.setPassword( Settings::mysqlPassword() );
    
    if ( !mysqlDB.open() )
    {
        Debug::print( "database", "Failed to open the database " + mysqlDB.lastError().text() );
    }
    
    QSqlDatabase qmonDB = QSqlDatabase::addDatabase( "QODBC", "qmonDB" );
    
    qmonDB.setDatabaseName( Settings::qmonDbDatabase() );
    qmonDB.setUserName( Settings::qmonDbUser() );
    qmonDB.setPassword( Settings::qmonDbPassword() );
    
    if ( !qmonDB.open() )
    {
        Debug::print( "database", "Failed to open the Qmon DB " + qmonDB.lastError().text() );
    }
    
    QSqlQuery query( mysqlDB );
       
    if ( !query.exec( "CREATE TABLE IF NOT EXISTS qmon_siebel( ID VARCHAR(20) PRIMARY KEY UNIQUE, QUEUE TEXT, "
                      "HOURS TEXT, GEO TEXT, ODATE TEXT, ADATE TEXT, QDATE TEXT, STATUS TEXT, SEVERITY TEXT, "
                      "CONTRACT TEXT, SLA TEXT, CRSR TEXT, BDESC TEXT, CUSTOMER TEXT, CONTACTVIA TEXT )" ) )
    {
        Debug::print( "database", "Error: " + query.lastError().text() );
    }
    
    if ( !query.exec( "CREATE TABLE IF NOT EXISTS qmon_chat( ID VARCHAR(40) PRIMARY KEY UNIQUE, SR VARCHAR(15), "
                      "NAME TEXT, DATE TEXT )" ) ) 
    {
        Debug::print( "database", "Error: " + query.lastError().text() );
    }
}

Database::~Database()
{   
    QSqlDatabase::removeDatabase( "mysqlDB" );
    QSqlDatabase::removeDatabase( "qmonDB" );
}

void Database::insertSiebelItemIntoDB( SiebelItem* item )
{
    Debug::print( "database", "Inserting SiebelItem for " + item->id + " " + item->queue );

    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    
    query.prepare( "INSERT INTO qmon_siebel( ID, QUEUE, HOURS, GEO, ODATE, ADATE, QDATE, STATUS, SEVERITY, "
                     "CONTRACT, SLA, CRSR, BDESC, CUSTOMER, CONTACTVIA )"
                     " VALUES "
                     "( :id, :queue, :hours, :geo, :odate, :adate, :qdate, :status, :severity, "
                     ":contract, :sla, :crsr, :bdesc, :customer, :contactvia )" );

    query.bindValue( ":id", item->id );
    query.bindValue( ":queue", item->queue );
    query.bindValue( ":hours", item->hours );
    query.bindValue( ":geo", item->geo );
    query.bindValue( ":odate", convertTime( item->odate ) );
    query.bindValue( ":adate", convertTime( item->adate ) );
    query.bindValue( ":qdate", QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss" ) );
    query.bindValue( ":status", item->status );
    query.bindValue( ":severity", item->severity );
    query.bindValue( ":contract", item->contract );
    query.bindValue( ":sla", convertTime( item->sla ) );
    query.bindValue( ":crsr", item->crSr );
    query.bindValue( ":bdesc", item->bdesc );
    query.bindValue( ":customer", item->customer );
    query.bindValue( ":contactvia", item->contactvia );
    
    query.exec();
}

void Database::updateSiebelQueue( SiebelItem* si )
{
    Debug::print( "database", "Updating Siebel queue for " + si->id + " to " + si->queue );
    
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    
    query.prepare( "UPDATE qmon_siebel SET QUEUE = :queue, QDATE = :qdate WHERE id = :id" );
                
    query.bindValue( ":queue", si->queue );
    query.bindValue( ":qdate", QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss" ) );
    query.bindValue( ":id", si->id );
                
    query.exec();
}

void Database::updateSiebelSeverity( SiebelItem* si )
{
    Debug::print( "database", "Updating SR Severity for " + si->id + " to " + si->severity );
    
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    
    query.prepare( "UPDATE qmon_siebel SET SEVERITY = :severity WHERE id = :id" );
                
    query.bindValue( ":severity", si->severity );
    query.bindValue( ":id", si->id );
                
    query.exec();
}

void Database::deleteSiebelItemFromDB( const QString& id )
{
    Debug::print( "database", "Deleting SiebelItem " + id );
    
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    
    query.prepare( "DELETE FROM qmon_siebel WHERE ID = :id" );
    query.bindValue( ":id", id );
    
    query.exec();
}

QStringList Database::getQmonSiebelList()
{
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
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
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    
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

bool Database::siebelQueueChanged( SiebelItem* si )
{
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    
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

bool Database::siebelSeverityChanged( SiebelItem* si )
{
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    
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
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    
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
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    
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

void Database::updateBomgarItemInDB( BomgarItem* bi )
{
    Debug::print( "database", "Inserting BomgarItem " + bi->id + " " + bi->sr );
        
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    
    query.prepare( "INSERT INTO qmon_chat( ID, SR, NAME, DATE ) VALUES ( :id, :sr, :name, :date )" );
                     
    query.bindValue( ":id", bi->id );
    query.bindValue( ":sr", bi->sr );
    query.bindValue( ":name", bi->name );
    query.bindValue( ":date", bi->date );
    
    query.exec();
}

void Database::deleteBomgarItemFromDB( const QString& id )
{
    Debug::print( "database", "Deleting BomgarItem " + id );
    
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    
    query.prepare( "DELETE FROM qmon_chat WHERE ID = :id" );
    query.bindValue( ":id", id );
    
    query.exec();
}

QList< SiebelItem* > Database::getSrsForQueue( const QString& queue )
{
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    QList< SiebelItem* > list;
    
    if (  queue == "NONE" )
    {
        query.prepare( "SELECT ID, QUEUE, HOURS, GEO, ODATE, ADATE, QDATE, STATUS, SEVERITY, "
                       "CONTRACT, SLA, CRSR, BDESC, CUSTOMER, CONTACTVIA FROM qmon_siebel" );
    }
    else
    {
        query.prepare( "SELECT ID, QUEUE, HOURS, GEO, ODATE, ADATE, QDATE, STATUS, SEVERITY, "
                       "CONTRACT, SLA, CRSR, BDESC, CUSTOMER, CONTACTVIA FROM qmon_siebel WHERE ( QUEUE = :queue )" );
        
        query.bindValue( ":queue", queue );
    }
    
    query.exec();
    
    while ( query.next() ) 
    {
        SiebelItem* si = new SiebelItem;
        
        si->id = query.value( 0 ).toString();
        si->queue = query.value( 1 ).toString();
        si->hours = query.value( 2 ).toString();
        si->geo = query.value( 3 ).toString();
        si->odate = query.value( 4 ).toString();
        si->adate = query.value( 5 ).toString();
        si->qdate = query.value( 6 ).toString();
        si->status = query.value( 7 ).toString();
        si->severity = query.value( 8 ).toString();
        si->contract = query.value( 9 ).toString();
        si->sla = query.value( 10 ).toString();
        si->crSr = query.value( 11 ).toString();
        si->bdesc = query.value( 12 ).toString();
        si->customer = query.value( 13 ).toString();
        si->contactvia = query.value( 14 ).toString();
        
        if ( getBomgarQueue( query.value( 0 ).toString() ) == "NOCHAT" )
        {
            si->isChat = false;
        }
        else
        {
            si->isChat = true;
            si->bomgarQ = getBomgarQueue( query.value( 0 ).toString() );
        }
     
        if ( query.value( 11 ).toString().isEmpty() )
        {
            si->isCr = false;
        }
        else
        {
            si->isCr = true;
        }
        
        list.append( si );
    }
        
    return list;
}

QStringList Database::getCurrentBomgars()
{
    QStringList list;
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );

    query.prepare( "SELECT SR, NAME FROM qmon_chat" );
    
    query.exec();
    
    while( query.next() )
    {
        QString tmp = query.value( 0 ).toString() + "|||" + query.value( 1 ).toString();
        list.append( tmp );
    }

    return list;
}

QStringList Database::getQmonBomgarList()
{
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    QStringList l;
    
    query.prepare( "SELECT ID FROM qmon_chat" );
    
    query.exec();
    
    while( query.next() ) 
    {
        l.append( query.value( 0 ).toString() );
    }
    
    return l;    
}

bool Database::bomgarExistsInDB( const QString& id )
{
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    
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
    
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    
    query.prepare( "UPDATE qmon_chat SET NAME = :name WHERE ID = :id" );
    query.bindValue( ":name", bi->name );
    query.bindValue( ":id", bi->id );
    
    query.exec();
}

QString Database::getBomgarQueue( const QString& id )
{
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    
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
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    
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

QList< SiebelItem* > Database::getQmonSrs()
{
    QSqlQuery query( QSqlDatabase::database( "qmonDB" ) );
    QList< SiebelItem* > list;
    
    query.prepare( "SELECT SR_NUM, PSEUDOQUEUE_ID, SUPPORT_HOURS, RESPOND_VIA, CREATED, LAST_UPD, SR_SUBSTATUS, "
                          "SR_SEVERITY, SR_SUPPORT_PROGRAM, SR_SLA_REMAINING, SR_SUBTYPE_SUPT_PROD_COLLAB, CID, SR_BRIEF_DESC, ACCOUNT "
                          "FROM Collector_ODBC_SIEBEL" );
    query.exec();
    
    while ( query.next() ) 
    {
        SiebelItem* si = new SiebelItem;
        
        si->id = query.value( 0 ).toString();
        si->queue = query.value( 1 ).toString();
        si->hours = query.value( 2 ).toString().split("|").at(0).trimmed();
        si->geo = query.value( 2 ).toString().split("|").at(1).trimmed();
        si->contactvia = query.value( 3 ).toString();
        si->odate = query.value( 4 ).toString();
        si->adate = query.value( 5 ).toString();
        si->status = query.value( 6 ).toString();
        si->severity = query.value( 7 ).toString();
        si->contract = query.value( 8 ).toString();
        si->sla = query.value( 9 ).toString();
        
        if ( query.value( 10 ).toString() == "Collaboration" )
        {
            si->isCr = true;
        }
        else
        {
            si->isCr = false;
        }
        
        si->crSr = query.value( 11 ).toString();
        si->bdesc = query.value( 12 ).toString();
        si->customer = query.value( 13 ).toString();
        si->bomgarQ = getBomgarQueue( query.value( 0 ).toString() );
     
        list.append( si );
    }
        
    return list;
}

QList< BomgarItem* > Database::getChats()
{
    QSqlQuery query( QSqlDatabase::database( "qmonDB" ) );
    QList< BomgarItem* > list;
    
    query.prepare( "SELECT LSID, EXTERNAL_KEY, CONFERENCE_NAME, TIMESTAMP FROM Collector_ODBC_NTSCHAT" );
    query.exec();
    
    while ( query.next() ) 
    {
        BomgarItem* bi = new BomgarItem;
        
        bi->id = query.value( 0 ).toString();
        bi->sr = query.value( 1 ).toString();
        bi->name = query.value( 2 ).toString();
        bi->date = query.value( 3 ).toString();
     
        list.append( bi );
    }
        
    return list;
}

#include "database.moc"
