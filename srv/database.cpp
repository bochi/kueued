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
        Debug::log( "database", "Failed to open the database " + mysqlDB.lastError().text() );
    }
    
    QSqlDatabase siebelDB = QSqlDatabase::addDatabase( "QOCI", "siebelDB" );

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

Database::~Database()
{   
    QSqlDatabase::removeDatabase( "mysqlDB" );
}

QStringList Database::getQmonSiebelList()
{
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    QStringList l;
    
    query.prepare( "SELECT ID FROM QMON_SIEBEL" );
    
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
    
    query.prepare( "SELECT ID FROM QMON_SIEBEL WHERE ( ID = :id )" );
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

bool Database::isChat( const QString& id )
{
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    
    query.prepare( "SELECT ID FROM QMON_CHAT WHERE ( SR = :id )" );
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

QList< SiebelItem > Database::getSrsForQueue( const QString& queue )
{
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    QList< SiebelItem > list;
    
    if (  queue == "NONE" )
    {    
        query.prepare( "SELECT ID, QUEUE, GEO, HOURS, STATUS, SEVERITY, SOURCE, RESPOND_VIA, CREATED, LAST_UPDATE, "
                       "INQUEUE, SLA, SUPPORT_PROGRAM, SUPPORT_PROGRAM_LONG, ROUTING_PRODUCT, SUPPORT_GROUP_ROUTING, "
                       "INT_TYPE, SUBTYPE, SERVICE_LEVEL, BRIEF_DESC, CRITSIT, HIGH_VALUE, DETAILED_DESC, CATEGORY, "
                       "CREATOR, ROW_ID from QMON_SIEBEL" );
    }
    else
    {
        query.prepare( "SELECT ID, QUEUE, GEO, HOURS, STATUS, SEVERITY, SOURCE, RESPOND_VIA, CREATED, LAST_UPDATE, "
                       "INQUEUE, SLA, SUPPORT_PROGRAM, SUPPORT_PROGRAM_LONG, ROUTING_PRODUCT, SUPPORT_GROUP_ROUTING, "
                       "INT_TYPE, SUBTYPE, SERVICE_LEVEL, BRIEF_DESC, CRITSIT, HIGH_VALUE, DETAILED_DESC, CATEGORY, "
                       "CREATOR, ROW_ID from QMON_SIEBEL WHERE ( QUEUE = :queue )" );
        
        query.bindValue( ":queue", queue );
    }
    
    query.exec();
    
    while ( query.next() ) 
    {
        SiebelItem si;
        
        si.id = query.value( 0 ).toString();
        si.queue = query.value( 1 ).toString();
        si.geo = query.value( 2 ).toString();
        si.hours = query.value( 3 ).toString();
        si.status = query.value( 4 ).toString();
        si.severity = query.value( 5 ).toString();
        si.source = query.value( 6 ).toString();
        si.respond_via = query.value( 7 ).toString();
        si.created = query.value( 8 ).toString();
        si.last_update = query.value( 9 ).toString();
        si.inqueue = query.value( 10 ).toString();
        si.sla = query.value( 11 ).toString();
        si.support_program = query.value( 12 ).toString();
        si.support_program_long = query.value( 13 ).toString();
        si.routing_product = query.value( 14 ).toString();
        si.support_group_routing = query.value( 15 ).toString();
        si.int_type = query.value( 16 ).toString();
        si.subtype = query.value( 17 ).toString();
        si.service_level = query.value( 18 ).toString();
        si.brief_desc = query.value( 19 ).toString();
        si.critsit = query.value( 20 ).toBool();
        si.high_value = query.value( 21 ).toBool();
        si.detailed_desc = query.value( 22 ).toString();
        si.category = query.value( 23 ).toString();
        si.creator = query.value( 24 ).toString();
        si.row_id = query.value( 25 ).toString();
        
        if ( getBomgarQueue( query.value( 0 ).toString() ) == "NOCHAT" )
        {
            si.isChat = false;
        }
        else
        {
            si.isChat = true;
            si.bomgarQ = getBomgarQueue( query.value( 0 ).toString() );
        }
        
        if ( si.creator != "" )
        {
            si.isCr = true;
        }
        else
        {
            si.isCr = false;
        
            QSqlQuery cquery( QSqlDatabase::database( "mysqlDB" ) );
        
            cquery.prepare( "SELECT CUSTOMER, CONTACT_PHONE, CONTACT_FIRSTNAME, CONTACT_LASTNAME, CONTACT_EMAIL, "
                            "       CONTACT_TITLE, CONTACT_LANG, ONSITE_PHONE FROM CUSTOMER WHERE ID = :id" );
        
            cquery.bindValue( ":id", si.id );
        
            cquery.exec();
        
            if ( cquery.next() ) 
            {
                si.customer = cquery.value( 0 ).toString();
                si.contact_phone = cquery.value( 1 ).toString();
                si.contact_firstname = cquery.value( 2 ).toString();
                si.contact_lastname = cquery.value( 3 ).toString();
                si.contact_email = cquery.value( 4 ).toString();
                si.contact_title = cquery.value( 5 ).toString();
                si.contact_lang = cquery.value( 6 ).toString();
                si.onsite_phone = cquery.value( 7 ).toString(); 
            }
        }
       
        list.append( si );        
    }
        
    return list;
}

QStringList Database::getCurrentBomgars()
{
    QStringList list;
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );

    query.prepare( "SELECT SR, NAME FROM QMON_CHAT" );
    
    query.exec();
    
    while( query.next() )
    {
        QString tmp = query.value( 0 ).toString() + "|||" + query.value( 1 ).toString();
        list.append( tmp );
    }

    return list;
}

QString Database::getBomgarQueue( const QString& id )
{
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    
    query.prepare( "SELECT NAME FROM QMON_CHAT WHERE ( SR = :id )" );
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
    
    if ( !d.isValid() )
    {
        d = QDateTime::fromString( dt, "yyyy-MM-ddThh:mm:ss" );
    }
    
    return ( d.toString("yyyy-MM-dd hh:mm:ss") );
}

QStringList Database::getSrNumsForQueue( const QString& queue, const QString& geo )
{
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    QStringList list;

    query.prepare( "SELECT ID FROM QMON_SIEBEL WHERE ( QUEUE = :queue ) AND ( GEO = :geo )" );
    query.bindValue( ":queue", queue );
    query.bindValue( ":geo", geo );

    query.exec();

    while( query.next() )
    {  
        list.append( query.value( 0 ).toString() );
    }

    return list;
}

QString Database::getPhoneNumber( const QString& engineer )
{
    QString e = engineer;
    QSqlQuery query( QSqlDatabase::database( "siebelDB" ) );
    query.prepare( "select CTI_ACD_USERID from siebel.s_user where login = :engineer" );
    query.bindValue( ":engineer", e.toUpper() );
    query.exec();
    
    if ( query.next() )
    {
        return query.value(0).toString();
    }
    
    return QString::Null();
}

QList< QueueItem > Database::getUserQueue( const QString& engineer, QSqlDatabase db )
{
    if ( !db.isValid() ) db = QSqlDatabase::database( "siebelDB" );
    
    QSqlQuery query( db );
    QList< QueueItem > list;

    query.prepare( "select "
                    "  sr.sr_num as ID, "
                    "  case "
                    "    when sr.BU_ID = '0-R9NH' then 'Default Organization' "
                    "    when sr.BU_ID = '1-AHT' then 'EMEA' "
                    "    when sr.BU_ID = '1-AHV' then 'ASIAPAC' "
                    "    when sr.BU_ID = '1-AHX' then 'USA' "
                    "    when sr.BU_ID = '1-AHZ' then 'LATIN AMERICA' "
                    "    when sr.BU_ID = '1-AI1' then 'CANADA' "
                    "  else 'Undefined' end as GEO,  "
                    "  cal.NAME as HOURS, "
                    "  sr.SR_SUB_STAT_ID as STATUS, "
                    "  sr.SR_SEV_CD as SEVERITY, "
                    "  sr.CREATED, "
                    "  sr.CREATED_BY, "
                    "  sr.LAST_UPD as LAST_UPDATE,  "
                    "  e.NAME as SUPPORT_PROGRAM,"
                    "  sr.X_SR_SUB_TYPE as SUBTYPE, "
                    "  e.ENTL_PRIORITY_NUM as SERVICE_LEVEL, "
                    "  SR_TITLE as BRIEF_DESC, "
                    "  flag.ATTRIB_11 as CRITSIT, "
                    "  flag.ATTRIB_56 as HIGH_VALUE, "
                    "  ext.NAME as CUSTOMER, "
                    "  g.WORK_PH_NUM as CONTACT_PHONE, "
                    "  g.FST_NAME as CONTACT_FIRSTNAME, "
                    "  g.LAST_NAME as CONTACT_LASTNAME, "
                    "  g.EMAIL_ADDR as CONTACT_EMAIL, "
                    "  g.JOB_TITLE as CONTACT_TITLE, "
                    "  g.PREF_LANG_ID as CONTACT_LANG,"
                    "  sr.X_ONSITE_PH_NUM as ONSITE_PHONE, "
                    "  sr.DESC_TEXT as DETAILED_DESC "
                    "from "
                    "  siebel.s_srv_req sr, "
                    "  siebel.s_user u, "
                    "  siebel.s_contact c, "
                    "  siebel.s_contact g, "
                    "  siebel.s_entlmnt e, "
                    "  siebel.s_sched_cal cal, "
                    "  siebel.s_org_ext ext, "
                    "  siebel.s_prod_int prd, "
                    "  siebel.s_org_ext_x flag "
                    "where "
                    "  sr.owner_emp_id = u.row_id "
                    "  and u.row_id = c.row_id "
                    "  and g.row_id = sr.CST_CON_ID  "
                    "  and u.login = :engineer "
                    "  and sr.sr_stat_id = 'Open' "
                    "  and sr.agree_id = e.row_id "
                    "  and e.svc_calendar_id = cal.row_id "
                    "  and sr.cst_ou_id = ext.row_id "
                    "  and sr.X_PROD_FEATURE_ID = prd.row_id "
                    "  and ext.row_id = flag.row_id" );
        
    query.bindValue( ":engineer", engineer );
    query.exec();

    while ( query.next() )
    {
        QueueItem i;
        
        i.id = query.value( 0 ).toString();
        i.geo = query.value( 1 ).toString();
        i.hours = query.value( 2 ).toString();
        i.status = query.value( 3 ).toString();
        i.severity = query.value( 4 ).toString();
        i.created = convertTime( query.value( 5 ).toString() );
        i.last_update = convertTime( query.value( 7 ).toString() );
        i.support_program = query.value( 8 ).toString();
        i.subtype = query.value( 9 ).toString();
        
        if ( query.value( 9 ).toString() == "Collaboration" )
        {
            i.isCr = true;
            i.creator = getCreator( i.id );
        }
        else
        {
            i.isCr = false;
            i.customer = query.value( 14 ).toString();
            i.contact_phone = query.value( 15 ).toString();
            i.contact_firstname = query.value( 16 ).toString();
            i.contact_lastname = query.value( 17 ).toString();
            i.contact_email = query.value( 18 ).toString();
            i.contact_title = query.value( 19 ).toString();
            i.contact_lang = query.value( 20 ).toString();
            i.onsite_phone = query.value( 21 ).toString();
        }
            
        i.service_level = query.value( 10 ).toInt();
        i.brief_desc = query.value( 11 ).toString();
        
        if ( query.value( 12 ).toString() == "Y" )
        {
            i.critsit = true;
        }
        else
        {
            i.critsit = false;
        }
        
        if ( query.value( 13 ).toString() == "Y" )
        {
            i.high_value = true;
        }
        else
        {
            i.high_value = false;
        }
        
        i.detailed_desc = query.value( 22 ).toString();
        
        list.append( i );
    }
        
    return list;
}

QString Database::getCreator(const QString& sr )
{
    
    QSqlQuery query( QSqlDatabase::database( "siebelDB" ) );
    query.prepare( "SELECT LOGIN FROM SIEBEL.S_USER WHERE PAR_ROW_ID = ( SELECT CREATED_BY FROM SIEBEL.S_SRV_REQ WHERE SR_NUM = :sr )" );

    query.bindValue( ":sr", sr );
    
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
#include "database.moc"
