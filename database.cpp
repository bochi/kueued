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

#include <QFile>
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
    
    QSqlDatabase mysqlDB = QSqlDatabase::addDatabase("QMYSQL", "mysqlDB" );
    mysqlDB.setHostName("localhost");
    mysqlDB.setDatabaseName("kueued");
    mysqlDB.setUserName("kueued");
    mysqlDB.setPassword("kueued");
    //QSqlDatabase mysqlDB = QSqlDatabase::addDatabase( "QSQLITE", "mysqlDB" );
    //mysqlDB.setDatabaseName( mDBfile );
    
    if ( !mysqlDB.open() )
    {
        Debug::print( "database", "Failed to open the database " + mysqlDB.lastError().text() );
    }

    QSqlDatabase oracleDB = QSqlDatabase::addDatabase( "QOCI", "oracleDB" );
    oracleDB.setDatabaseName( "report" );
    oracleDB.setHostName( Settings::oracleHost() );
    oracleDB.setPort( Settings::oraclePort() );
    oracleDB.setUserName( Settings::oracleUser() );
    oracleDB.setPassword( Settings::oraclePassword() );

    if ( !oracleDB.open() )
    {
        Debug::print( "database", "Failed to open the Oracle DB " + oracleDB.lastError().text() );
    }
    
    QSqlQuery query( mysqlDB );
       
    if ( !query.exec( "CREATE TABLE IF NOT EXISTS qmon_siebel( ID VARCHAR(20) PRIMARY KEY UNIQUE, QUEUE TEXT, SEVERITY TEXT, HOURS TEXT, "
                      "SOURCE TEXT, CONTACTVIA TEXT, ODATE TEXT, ADATE TEXT, QDATE TEXT, STATUS TEXT, "
                      "CONTRACT TEXT, QUEUE1 TEXT, PHONE TEXT, ONSITEPHONE TEXT, GEO TEXT, "
                      "WTF TEXT, ROUTING TEXT, BDESC TEXT, SLA TEXT, DISPLAY TEXT )" ) )

    {
        Debug::print( "database", "Error: " + query.lastError().text() );
    }
    
    if ( !query.exec( "CREATE TABLE IF NOT EXISTS qmon_chat( ID VARCHAR(40) PRIMARY KEY UNIQUE, SR VARCHAR(15), REPTEAM TEXT, "
                      "NAME TEXT, DATE TEXT, QDATE TEXT, SOMENR VARCHAR(15) )" ) ) 
    {
        Debug::print( "database", "Error: " + query.lastError().text() );
    }
}

Database::~Database()
{   
    Debug::print( "database", "Destroying" );
    QSqlDatabase::removeDatabase( "mysqlDB" );
}

void Database::insertSiebelItemIntoDB( SiebelItem* item )
{
    Debug::print( "database", "Inserting SiebelItem for " + item->id + " " + item->queue );

    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    query.prepare( "INSERT INTO qmon_siebel( ID, QUEUE, SEVERITY, HOURS, SOURCE, CONTACTVIA, ODATE, ADATE, QDATE, "
                     "STATUS, CONTRACT, QUEUE1, PHONE, ONSITEPHONE, GEO, WTF, ROUTING, BDESC, SLA )"
                     " VALUES "
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
    Debug::print( "database", "Updating Siebel queue for " + si->id + " to " + si->queue );
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    query.prepare( "UPDATE qmon_siebel SET QUEUE = :queue WHERE id = :id" );
                
    query.bindValue( ":queue", si->queue );
    query.bindValue( ":id", si->id );
                
    query.exec();
}

void Database::updateSiebelSeverity( SiebelItem* si )
{
    Debug::print( "database", "Updating Siebel Severity for " + si->id + " to " + si->severity );
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    query.prepare( "UPDATE qmon_siebel SET SEVERITY = :severity WHERE id = :id" );
                
    query.bindValue( ":severity", si->severity );
    query.bindValue( ":id", si->id );
                
    query.exec();
}

QString Database::getBugForSr( const QString& sr )
{
    QSqlQuery query( QSqlDatabase::database( "oracleDB" ) );
    query.prepare( "SELECT DEFECT_ID from NTSDM.OLAP_SR_DX2 WHERE SR_NUM = :srnum" );
    query.bindValue( ":srnum", sr );
    query.exec();

    if ( query.next() )
    {
        return query.value( 0 ).toString();
    }
    else
    {
        return "NOBUG";
    }
}

QString Database::critSitFlagForSr( const QString& sr )
{
    QSqlQuery query( QSqlDatabase::database( "oracleDB" ) );
    query.prepare("SELECT CRITSIT_FLAG from NTSDM.OLAP_SR_DX2 WHERE SR_NUM = :srnum");
    query.bindValue( ":srnum", sr );
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

QString Database::highValueFlagForSr( const QString& sr )
{
    QSqlQuery query( QSqlDatabase::database( "oracleDB" ) );
    query.prepare("SELECT HIGHVALUE_FLAG from NTSDM.OLAP_SR_DX2 WHERE SR_NUM = :srnum");
    query.bindValue( ":srnum", sr );
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

QString Database::highValueCritSitFlagForSr( const QString& sr )
{
    QSqlQuery query( "SELECT HIGHVALUE_FLAG, CRITSIT_FLAG from NTSDM.OLAP_SR_DX2 WHERE SR_NUM = :srnum", QSqlDatabase::database( "oracleDB" ) );
    query.bindValue( ":srnum", sr );
    query.exec();

    if ( query.next() )
    {
        return query.value( 0 ).toString() + query.value( 1 ).toString();
    }
    else
    {
        return "ERROR";
    }
}


void Database::updateSiebelDisplay( const QString& display )
{
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    query.prepare("UPDATE qmon_siebel SET DISPLAY = :display WHERE id = :id");
                
    query.bindValue( ":display", display.split( "-" ).at( 1 ) );
    query.bindValue( ":id", display.split( "-" ).at( 0 ) );
                
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

bool Database::siebelQueueChanged( SiebelItem* si  )
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

bool Database::siebelSeverityChanged( SiebelItem* si  )
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


/*


                B O M G A R 


*/

void Database::updateBomgarItemInDB( BomgarItem* bi )
{
    Debug::print( "database", "Inserting BomgarItem " + bi->id + " " + bi->sr );
        
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    query.prepare( "INSERT INTO qmon_chat( ID, SR, REPTEAM, NAME, DATE, QDATE, SOMENR ) VALUES ( :id, :sr, :repteam, :name, :date, :qdate, :somenr )" );
                     
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
        
    query.prepare( "SELECT ID, QUEUE, SEVERITY, HOURS, SOURCE, CONTACTVIA, ODATE, ADATE, QDATE, STATUS, CONTRACT, GEO, BDESC, SLA, DISPLAY "
                   "FROM qmon_siebel" );
    }
    else
    {
    query.prepare( "SELECT ID, QUEUE, SEVERITY, HOURS, SOURCE, CONTACTVIA, ODATE, ADATE, QDATE, STATUS, CONTRACT, GEO, BDESC, SLA, DISPLAY "
                   "FROM qmon_siebel WHERE ( QUEUE = :queue )" );
    query.bindValue( ":queue", queue );
    }
    
    query.exec();
    
    while ( query.next() ) 
    {
        SiebelItem* si = new SiebelItem;
        
        si->id = query.value( 0 ).toString();
        si->queue = query.value( 1 ).toString();
        si->severity = query.value( 2 ).toString();
        si->hours = query.value( 3 ).toString();
        si->source = query.value( 4 ).toString();
        si->contactvia = query.value( 5 ).toString();
        si->odate = query.value( 6 ).toString();
        si->adate = query.value( 7 ).toString();
        si->qdate = query.value( 8 ).toString();
        si->status = query.value( 9 ).toString();
        si->contract = query.value( 10 ).toString();
        si->geo = query.value( 11 ).toString();
        si->bdesc = query.value( 12 ).toString();
        si->sla = query.value( 13 ).toString();
        si->display = query.value( 14 ).toString();
        si->isChat = isChat( query.value( 0 ).toString() );
        si->bomgarQ = getBomgarQueue( query.value( 0 ).toString() );
        si->critSit = highValueCritSitFlagForSr( query.value( 0 ).toString() );
     
        list.append( si );
    }
        
    return list;
}

QStringList Database::getQmonBomgarList()
{
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    QStringList l;
    query.prepare( "SELECT ID FROM qmon_chat" );
    query.exec();
    
    while( query.next() ) l.append( query.value( 0 ).toString() );
    
    return l;    
}


QStringList Database::getOracleSrList()
{
    QSqlQuery query( QSqlDatabase::database( "oracleDB" ) );
    QStringList l;
    query.prepare( "SELECT SR_NUM FROM NTSDM.NTS_OPEN_SR WHERE OWNER='SBOGNER'" );
    query.exec();

    while( query.next() ) 
    {
	    l.append( query.value( 0 ).toString() );
	    qDebug() << "append";
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

#include "database.moc"
