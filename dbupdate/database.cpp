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
    QSqlDatabase mysqlDB = QSqlDatabase::addDatabase( "QMYSQL", "mysqlDB" );
    
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

    QSqlQuery query( mysqlDB );
       
    if ( !query.exec( "CREATE TABLE IF NOT EXISTS qmon_siebel( ID VARCHAR(20) PRIMARY KEY UNIQUE, QUEUE TEXT, "
                      "GEO TEXT, HOURS TEXT, STATUS TEXT, SEVERITY TEXT, SOURCE TEXT, RESPOND_VIA TEXT, "
                      "CREATED TEXT, LAST_UPDATE TEXT, INQUEUE TEXT, SLA TEXT, SUPPORT_PROGRAM TEXT, "
                      "SUPPORT_PROGRAM_LONG TEXT, ROUTING_PRODUCT TEXT, SUPPORT_GROUP_ROUTING TEXT, "
                      "INT_TYPE TEXT, SUBTYPE TEXT, SERVICE_LEVEL TEXT, BRIEF_DESC TEXT, CRITSIT TINYINT, "
                      "HIGH_VALUE TINYINT, CUSTOMER TEXT, CONTACT_PHONE TEXT, ONSITE_PHONE TEXT, "
                      "DETAILED_DESC TEXT, CATEGORY TEXT, CREATOR TEXT, ROW_ID TEXT ) ENGINE='NDB'" ) );
    {
        Debug::print( "database", "Error qmon_siebel: " + query.lastError().text() + query.executedQuery() );
    }
    
    if ( !query.exec( "CREATE TABLE IF NOT EXISTS qmon_chat( ID VARCHAR(40) PRIMARY KEY UNIQUE, SR VARCHAR(15), "
                      "NAME TEXT, DATE TEXT ) ENGINE='NDB'" ) ) 
    {
        Debug::print( "database", "Error qmon_chat: " + query.lastError().text() );
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
    
    query.prepare( "INSERT INTO qmon_siebel( ID, QUEUE, GEO, HOURS, STATUS, SEVERITY, SOURCE, RESPOND_VIA, "
                   "                         CREATED, LAST_UPDATE, INQUEUE, SLA, SUPPORT_PROGRAM, SUPPORT_PROGRAM_LONG, "
                   "                         ROUTING_PRODUCT, SUPPORT_GROUP_ROUTING, INT_TYPE, SUBTYPE, SERVICE_LEVEL, "
                   "                         BRIEF_DESC, CRITSIT, HIGH_VALUE, CUSTOMER, CONTACT_PHONE, ONSITE_PHONE, "
                   "                         DETAILED_DESC, CATEGORY, CREATOR, ROW_ID ) "
                   " VALUES "
                   "( :id, :queue, :geo, :hours, :status, :severity, :source, :respond_via, :created, :last_update, :inqueue, "
                   "  :sla, :support_program, :support_program_long, :routing_product, :support_group_routing, :int_type, :subtype, "
                   "  :service_level, :brief_desc, :critsit, :high_value, :customer, :contact_phone, :onsite_phone, :detailed_desc, "
                   "  :category, :creator, :row_id )" );

    query.bindValue( ":id", item->id );
    query.bindValue( ":queue", item->queue );
    query.bindValue( ":geo", item->geo );
    query.bindValue( ":hours", item->hours );
    query.bindValue( ":status", item->status );
    query.bindValue( ":severity", item->severity );
    query.bindValue( ":source", item->source );
    query.bindValue( ":respond_via", item->respond_via );
    query.bindValue( ":created", convertTime( item->created ) );
    query.bindValue( ":last_update", convertTime( item->last_update ) );
    query.bindValue( ":inqueue", QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss" ) );
    query.bindValue( ":sla", convertTime( item->sla ) );
    query.bindValue( ":support_program", item->support_program );
    query.bindValue( ":support_program_long", item->support_program_long );
    query.bindValue( ":routing_product", item->routing_product );
    query.bindValue( ":support_group_routing", item->support_group_routing );
    query.bindValue( ":int_type", item->int_type );
    query.bindValue( ":subtype", item->subtype );
    query.bindValue( ":service_level", item->service_level );
    query.bindValue( ":brief_desc", item->brief_desc );   
    query.bindValue( ":critsit", item->critsit );
    query.bindValue( ":high_value", item->high_value );    
    query.bindValue( ":customer", item->customer );
    query.bindValue( ":contact_phone", item->contact_phone );
    query.bindValue( ":onsite_phone", item->onsite_phone );
    query.bindValue( ":detailed_desc", item->detailed_desc );
    query.bindValue( ":category", item->category );
    query.bindValue( ":creator", item->creator );
    query.bindValue( ":row_id", item->row_id );
    
    query.exec();
}

void Database::updateSiebelItem( SiebelItem* item )
{
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    
    query.prepare( "UPDATE qmon_siebel SET GEO = :geo, HOURS = :hours, STATUS = :status, SEVERITY = :severity, "
                   "                       SOURCE = :source, RESPOND_VIA = :respond_via, CREATED = :created, "
                   "                       LAST_UPDATE = :last_update, SLA = :sla, SUPPORT_PROGRAM = :support_program,"
                   "                       SUPPORT_PROGRAM_LONG = :support_program_long, ROUTING_PRODUCT = :routing_product,"
                   "                       SUPPORT_GROUP_ROUTING = :support_group_routing, INT_TYPE = :int_type,"
                   "                       SUBTYPE = :subtype, SERVICE_LEVEL = :service_level, BRIEF_DESC = :brief_desc,"
                   "                       CRITSIT = :critsit, HIGH_VALUE = :high_value, CUSTOMER = :customer, "
                   "                       CONTACT_PHONE = :contact_phone, ONSITE_PHONE = :onsite_phone, "
                   "                       DETAILED_DESC = :detailed_desc, CATEGORY = :category, CREATOR = :creator,"
                   "                       ROW_ID = :row_id WHERE ID = :id" );
    
    query.bindValue( ":geo", item->geo );
    query.bindValue( ":hours", item->hours );
    query.bindValue( ":status", item->status );
    query.bindValue( ":severity", item->severity );
    query.bindValue( ":source", item->source );
    query.bindValue( ":respond_via", item->respond_via );
    query.bindValue( ":created", convertTime( item->created ) );
    query.bindValue( ":last_update", convertTime( item->last_update ) );
    query.bindValue( ":sla", convertTime( item->sla ) );
    query.bindValue( ":support_program", item->support_program );
    query.bindValue( ":support_program_long", item->support_program_long );
    query.bindValue( ":routing_product", item->routing_product );
    query.bindValue( ":support_group_routing", item->support_group_routing );
    query.bindValue( ":int_type", item->int_type );
    query.bindValue( ":subtype", item->subtype );
    query.bindValue( ":service_level", item->service_level );
    query.bindValue( ":brief_desc", item->brief_desc );
    query.bindValue( ":critsit", item->critsit );
    query.bindValue( ":high_value", item->high_value );
    query.bindValue( ":customer", item->customer );
    query.bindValue( ":contact_phone", item->contact_phone );
    query.bindValue( ":onsite_phone", item->onsite_phone );
    query.bindValue( ":detailed_desc", item->detailed_desc );
    query.bindValue( ":category", item->category );
    query.bindValue( ":creator", getCreator( item->id ) );
    query.bindValue( ":row_id", item->row_id );
    query.bindValue( ":id", item->id );

    query.exec();
     qDebug() << query.lastError().text();
}

void Database::updateSiebelQueue( SiebelItem* si )
{
    Debug::print( "database", "Updating Siebel queue for " + si->id + " to " + si->queue );
    
    QSqlQuery query( QSqlDatabase::database( "mysqlDB" ) );
    
    query.prepare( "UPDATE qmon_siebel SET QUEUE = :queue, INQUEUE = :inqueue WHERE id = :id" );
                
    query.bindValue( ":queue", si->queue );
    query.bindValue( ":inqueue", QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss" ) );
    query.bindValue( ":id", si->id );
                
    query.exec();
}

QString Database::getCreator(const QString& sr)
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
        query.prepare( "SELECT ID, QUEUE, GEO, HOURS, STATUS, SEVERITY, SOURCE, RESPOND_VIA, CREATED, LAST_UPDATE, "
                       "INQUEUE, SLA, SUPPORT_PROGRAM, SUPPORT_PROGRAM_LONG, ROUTING_PRODUCT, SUPPORT_GROUP_ROUTING, "
                       "INT_TYPE, SUBTYPE, SERVICE_LEVEL, BRIEF_DESC, CRITSIT, HIGH_VALUE, CUSTOMER, CONTACT_PHONE, "
                       "ONSITE_PHONE, DETAILED_DESC, CATEGORY, CREATOR, ROW_ID from qmon_siebel" );
    }
    else
    {
        query.prepare( "SELECT ID, QUEUE, GEO, HOURS, STATUS, SEVERITY, SOURCE, RESPOND_VIA, CREATED, LAST_UPDATE, "
                       "INQUEUE, SLA, SUPPORT_PROGRAM, SUPPORT_PROGRAM_LONG, ROUTING_PRODUCT, SUPPORT_GROUP_ROUTING, "
                       "INT_TYPE, SUBTYPE, SERVICE_LEVEL, BRIEF_DESC, CRITSIT, HIGH_VALUE, CUSTOMER, CONTACT_PHONE, "
                       "ONSITE_PHONE, DETAILED_DESC, CATEGORY, CREATOR, ROW_ID from qmon_siebel WHERE ( QUEUE = :queue )" );
        
        query.bindValue( ":queue", queue );
    }
    
    query.exec();
    
    while ( query.next() ) 
    {
        SiebelItem* si = new SiebelItem;
        
        si->id = query.value( 0 ).toString();
        si->queue = query.value( 1 ).toString();
        si->geo = query.value( 2 ).toString();
        si->hours = query.value( 3 ).toString();
        si->status = query.value( 4 ).toString();
        si->severity = query.value( 5 ).toString();
        si->source = query.value( 6 ).toString();
        si->respond_via = query.value( 7 ).toString();
        si->created = query.value( 8 ).toString();
        si->last_update = query.value( 9 ).toString();
        si->inqueue = query.value( 10 ).toString();
        si->sla = query.value( 11 ).toString();
        si->support_program = query.value( 12 ).toString();
        si->support_program_long = query.value( 13 ).toString();
        si->routing_product = query.value( 14 ).toString();
        si->support_group_routing = query.value( 15 ).toString();
        si->int_type = query.value( 16 ).toString();
        si->subtype = query.value( 17 ).toString();
        si->service_level = query.value( 18 ).toString();
        si->brief_desc = query.value( 19 ).toString();
        si->critsit = query.value( 20 ).toBool();
        si->high_value = query.value( 21 ).toBool();
        si->customer = query.value( 22 ).toString();
        si->contact_phone = query.value( 23 ).toString();
        si->onsite_phone = query.value( 24 ).toString();
        si->detailed_desc = query.value( 25 ).toString();
        si->category = query.value( 26 ).toString();
        si->creator = query.value( 27 ).toString();
        si->row_id = query.value( 28 ).toString();
        
        if ( getBomgarQueue( query.value( 0 ).toString() ) == "NOCHAT" )
        {
            si->isChat = false;
        }
        else
        {
            si->isChat = true;
            si->bomgarQ = getBomgarQueue( query.value( 0 ).toString() );
        }
        
        if ( si->creator != "" )
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
    QDateTime d = QDateTime::fromString( dt, "yyyy-MM-ddThh:mm:ss" );
    return ( d.toString("yyyy-MM-dd hh:mm:ss") );
}

QList< SiebelItem* > Database::getQmonSrs()
{
    QSqlQuery query( QSqlDatabase::database( "siebelDB" ) );
    QList< SiebelItem* > list;
    
    query.prepare(  "select sr.sr_num as ID, u.login as QUEUE, "
                    "case when sr.BU_ID = '0-R9NH' then 'Default Organization' "
                    "when sr.BU_ID = '1-AHT' then 'EMEA' "
                    "when sr.BU_ID = '1-AHV' then 'ASIAPAC' "
                    "when sr.BU_ID = '1-AHX' then 'USA' "
                    "when sr.BU_ID = '1-AHZ' then 'LATIN AMERICA' "
                    "when sr.BU_ID = '1-AI1' then 'CANADA' "
                    "else 'Undefined' end as GEO,  "
                    "cal.NAME as HOURS, sr.SR_SUB_STAT_ID as STATUS, sr.SR_SEV_CD as SEVERITY, sr.SR_SUBTYPE_CD as SOURCE,  "
                    "sr.X_RESPOND_VIA as RESPOND_VIA, sr.CREATED, sr.LAST_UPD as LAST_UPDATE,  "
                    "case when sr.EXP_CLOSE_DT IS NULL then NVL(sr.EXP_CLOSE_DT, TO_DATE('01-01-1970', 'MM-DD-YYYY')) else sr.EXP_CLOSE_DT end as SLA, "
                    "sr.X_SUPPORT_PROG as SUPPORT_PROGRAM, e.NAME as SUPPORT_PROGRAM_LONG,prd.name as ROUTING_PRODUCT, "
                    "sr.X_SUPP_GRP_ROUTING AS SUPPORT_GROUP_ROUTING, sr.SR_TYPE_CD as INT_TYPE, sr.X_SR_SUB_TYPE as SUBTYPE, "
                    "e.ENTL_PRIORITY_NUM as SERVICE_LEVEL, CAST(sr.SR_TITLE as varchar(99)) as BRIEF_DESC, flag.ATTRIB_11 as CRITSIT, "
                    "flag.ATTRIB_56 as HIGH_VALUE, cast(ext.NAME as varchar(80)) as CUSTOMER, g.WORK_PH_NUM as CONTACT_PHONE, "
                    "sr.X_ONSITE_PH_NUM as ONSITE_PHONE, sr.DESC_TEXT as DETAILED_DESC, sr.SR_CATEGORY_CD as CATEGORY, sr.ROW_ID "
                    "from siebel.s_srv_req sr, siebel.s_user u, siebel.s_contact c, siebel.s_contact g, siebel.s_entlmnt e, siebel.s_sched_cal cal, siebel.s_org_ext ext, siebel.s_prod_int prd, siebel.s_org_ext_x flag "
                    "where sr.owner_emp_id = u.row_id and u.row_id = c.row_id and g.row_id = sr.CST_CON_ID  "
                    "and c.job_title LIKE('Pseudo Use%') and sr.sr_stat_id = 'Open' and sr.agree_id = e.row_id "
                    "and e.svc_calendar_id = cal.row_id and sr.cst_ou_id = ext.row_id and sr.X_PROD_FEATURE_ID = prd.row_id "
                    "and ext.row_id = flag.row_id  "
                    "union "
                    "select sr.sr_num as ID, u.login as QUEUE, "
                    "case when sr.BU_ID = '0-R9NH' then 'Default Organization' "
                    "when sr.BU_ID = '1-AHT' then 'EMEA' "
                    "when sr.BU_ID = '1-AHZ' then 'ASIAPAC' "
                    "when sr.BU_ID = '1-AHV' then 'ASIAPAC' "
                    "when sr.BU_ID = '1-AHX' then 'USA' "
                    "when sr.BU_ID = '1-AHZ' then 'LATIN AMERICA' "
                    "when sr.BU_ID = '1-AI1' then 'CANADA' "
                    "else 'Undefined' end as GEO, cal.NAME as HOURS, sr.SR_SUB_STAT_ID as STATUS, sr.SR_SEV_CD as SEVERITY, "
                    "sr.SR_SUBTYPE_CD as SOURCE, sr.X_RESPOND_VIA as RESPOND_VIA, sr.CREATED, sr.LAST_UPD as LAST_UPDATE, "
                    "case when sr.EXP_CLOSE_DT IS NULL then NVL(sr.EXP_CLOSE_DT, TO_DATE('01-01-1970', 'MM-DD-YYYY')) else sr.EXP_CLOSE_DT end as SLA, "
                    "sr.X_SUPPORT_PROG as SUPPORT_PROGRAM, e.NAME as SUPPORT_PROGRAM_LONG, prd.name as ROUTING_PRODUCT, "
                    "sr.X_SUPP_GRP_ROUTING AS SUPPORT_GROUP_ROUTING, sr.SR_TYPE_CD as INT_TYPE, sr.X_SR_SUB_TYPE as SUBTYPE, "
                    "e.ENTL_PRIORITY_NUM as SERVICE_LEVEL, CAST(sr.SR_TITLE as varchar(99)) as BRIEF_DESC, flag.ATTRIB_11 as CRITSIT, "
                    "flag.ATTRIB_56 as HIGH_VALUE, cast(ext.NAME as varchar(80)) as CUSTOMER, '1', '1', '1',  "
                    "sr.SR_CATEGORY_CD as SR_CATEGORY, sr.ROW_ID  "
                    "from siebel.s_srv_req sr, siebel.s_user u, siebel.s_entlmnt e, siebel.s_sched_cal cal, siebel.s_org_ext ext, siebel.s_prod_int prd, siebel.s_org_ext_x flag "
                    "where sr.owner_emp_id = '0-1' and sr.owner_emp_id = u.row_id  and sr.sr_stat_id = 'Open' "
                    "and sr.agree_id = e.row_id and e.svc_calendar_id = cal.row_id and sr.cst_ou_id = ext.row_id "
                    "and sr.X_PROD_FEATURE_ID = prd.row_id and ext.row_id = flag.row_id "
                    "union "
                    "select  sr.sr_num, 'null', "
                    "case when sr.BU_ID = '0-R9NH' then 'Default Organization' "
                    "when sr.BU_ID = '1-AHT' then 'EMEA' "
                    "when sr.BU_ID = '1-AHZ' then 'ASIAPAC' "
                    "when sr.BU_ID = '1-AHV' then 'ASIAPAC' "
                    "when sr.BU_ID = '1-AHX' then 'USA' "
                    "when sr.BU_ID = '1-AHZ' then 'LATIN AMERICA' "
                    "when sr.BU_ID = '1-AI1' then 'CANADA' "
                    "else 'Undefined' end as GEO, cal.NAME as HOURS, sr.SR_SEV_CD as SEVERITY, sr.SR_SUB_STAT_ID as STATUS, "
                    "sr.SR_SUBTYPE_CD as SOURCE, sr.X_RESPOND_VIA as RESPOND_VIA, sr.CREATED, sr.LAST_UPD as LAST_UPDATE, "
                    "case when sr.EXP_CLOSE_DT IS NULL then NVL(sr.EXP_CLOSE_DT, TO_DATE('01-01-1970', 'MM-DD-YYYY')) else sr.EXP_CLOSE_DT end as SLA, "
                    "sr.X_SUPPORT_PROG as SUPPORT_PROGRAM, e.NAME as SUPPORT_PROGRAM_LONG, prd.name as ROUTING_PRODUCT, "
                    "sr.X_SUPP_GRP_ROUTING AS SUPPORT_GROUP_ROUTING, sr.SR_TYPE_CD as INT_TYPE, sr.X_SR_SUB_TYPE as SUBTYPE, "
                    "e.ENTL_PRIORITY_NUM as SERVICE_LEVEL, CAST(sr.SR_TITLE as varchar(99)) as BRIEF_DESC, flag.ATTRIB_11 as CRITSIT, "
                    "flag.ATTRIB_56 as HIGH_VALUE, cast(ext.NAME as varchar(80)) as CUSTOMER, '0', '0', '0', "
                    "sr.SR_CATEGORY_CD as SR_CATEGORY, sr.ROW_ID "
                    "from siebel.s_srv_req sr, siebel.s_entlmnt e, siebel.s_sched_cal cal, siebel.s_org_ext ext, siebel.s_prod_int prd, siebel.s_org_ext_x flag "
                    "where sr.owner_emp_id is null and sr.sr_stat_id = 'Open' and sr.agree_id = e.row_id "
                    "and e.svc_calendar_id = cal.row_id and sr.cst_ou_id = ext.row_id and sr.X_PROD_FEATURE_ID = prd.row_id "
                    "and ext.row_id = flag.row_id" );
    
    query.exec();
    qDebug() << query.lastError();
    while ( query.next() ) 
    {
        SiebelItem* si = new SiebelItem;
                
        si->id = query.value( 0 ).toString();
        si->queue = query.value( 1 ).toString();
        si->geo = query.value( 2 ).toString();
        si->hours = query.value( 3 ).toString();
        si->status = query.value( 4 ).toString();
        si->severity = query.value( 5 ).toString();
        si->source = query.value( 6 ).toString();
        si->respond_via = query.value( 7 ).toString();
        si->created = query.value( 8 ).toString();
        si->last_update = query.value( 9 ).toString();
        si->sla = query.value( 10 ).toString();
        si->support_program = query.value( 11 ).toString();
        si->support_program_long = query.value( 12 ).toString();
        si->routing_product = query.value( 13 ).toString();
        si->support_group_routing = query.value( 14 ).toString();
        si->int_type = query.value( 15 ).toString();
        si->subtype = query.value( 16 ).toString();
        si->service_level = query.value( 17 ).toString();
        si->brief_desc = query.value( 18 ).toString();
        
        if ( si->subtype == "Collaboration" )
        {
            si->isCr = true;
            si->creator = getCreator( si->id );
        }
        else
        {
            si->isCr = false;
        }
        
        if ( query.value( 19 ).toString() == "Y" )
        {
            si->critsit = true;
        }
        else
        {
            si->critsit = false;
        }
        
        if ( query.value( 20 ).toString() == "Y" )
        {
            si->high_value = true;
        }
        else
        {
            si->high_value = false;
        }
        
        si->customer = query.value( 21 ).toString();
        si->contact_phone = query.value( 22 ).toString();
        si->onsite_phone = query.value( 23 ).toString();
        si->detailed_desc = query.value( 24 ).toString();
        si->category = query.value( 25 ).toString();
        si->row_id = query.value( 26 ).toString();
        
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
