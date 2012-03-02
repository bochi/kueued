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

    QSqlDatabase mysqlDB = QSqlDatabase::addDatabase("QMYSQL", "mysqlDB" );
    
    mysqlDB.setHostName( Settings::mysqlHost() );
    mysqlDB.setDatabaseName( Settings::mysqlDatabase() );
    mysqlDB.setUserName( Settings::mysqlUser() );
    mysqlDB.setPassword( Settings::mysqlPassword() );
    
    if ( !mysqlDB.open() )
    {
        Debug::print( "database", "Failed to open the database " + mysqlDB.lastError().text() );
    }

    QSqlDatabase oracleDB = QSqlDatabase::addDatabase( "QOCI", "oracleDB" );
    
    oracleDB.setDatabaseName( Settings::oracleDatabase() );
    oracleDB.setHostName( Settings::oracleHost() );
    oracleDB.setPort( Settings::oraclePort() );
    oracleDB.setUserName( Settings::oracleUser() );
    oracleDB.setPassword( Settings::oraclePassword() );

    if ( !oracleDB.open() )
    {
        Debug::print( "database", "Failed to open the Oracle DB " + oracleDB.lastError().text() );
    }
    
    QSqlDatabase siebelDB = QSqlDatabase::addDatabase( "QOCI", "siebelDB" );
    
    siebelDB.setDatabaseName( Settings::siebelDatabase() );
    siebelDB.setHostName( Settings::siebelHost() );
    siebelDB.setPort( 1521 );
    siebelDB.setUserName( Settings::siebelUser() );
    siebelDB.setPassword( Settings::siebelPassword() );

    if ( !siebelDB.open() )
    {
        Debug::print( "database", "Failed to open the Siebel DB " + siebelDB.lastError().text() );
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
    QSqlDatabase::removeDatabase( "oracleDB" );
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

QStringList Database::getSrsForUser( const QString& user )
{
    QStringList list;
    
    QSqlQuery query( QSqlDatabase::database( "siebelDB" ) );
    query.prepare( "SELECT SR_NUM FROM SIEBEL.S_SRV_REQ WHERE OWNER_EMP_ID = ( SELECT PAR_ROW_ID FROM SIEBEL.S_USER WHERE LOGIN = :login ) AND SR_STAT_ID = 'Open'" );
    //query.prepare( "SELECT * FROM ALL_TABLES" );
    query.bindValue( ":login", user.toUpper() );
    query.exec();
    
    qDebug() << query.lastError().text();

    while ( query.next() )
    {
        list.append( query.value( 0 ).toString() );
    }
    
    return list;
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

QList< PseudoQueueItem* > Database::getPseudoQueues()
{
    QSqlQuery query( QSqlDatabase::database( "qmonDB" ) );
    QList< PseudoQueueItem* > list;
    
    query.prepare( "SELECT DisplayName, PseudoQueue from _NovQueuePseudoQueue" );
    query.exec();

    while ( query.next() )
    {
        PseudoQueueItem* i = new PseudoQueueItem;
        i->display = query.value( 0 ).toString();
        i->name = query.value( 1 ).toString();
        list.append( i );
    }
    
    return list;
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
        //si->critSit = highValueCritSitFlagForSr( query.value( 0 ).toString() );
     
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
        QString tmp = query.value(0).toString() + "|||" + query.value(1).toString();
        list.append(tmp);
    }

    return list;
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

QString Database::convertTime( const QString& dt )
{
    QDateTime d = QDateTime::fromString( dt, "M/d/yyyy h:mm:ss AP" );
    return ( d.toString("yyyy-MM-dd hh:mm:ss") );
}

#include "database.moc"
