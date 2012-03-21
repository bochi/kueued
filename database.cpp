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
}

Database::~Database()
{   
    if ( QSqlDatabase::database( "mysqlDB" ).isOpen() )
    {
        QSqlDatabase::database( "mysqlDB" ).close();
        QSqlDatabase::removeDatabase( "mysqlDB" );
    }
    
    if ( QSqlDatabase::database( "qmonDB" ).isOpen() )
    {
        QSqlDatabase::database( "qmonDB" ).close();
        QSqlDatabase::removeDatabase( "qmonDB" );
    }
     
    if ( QSqlDatabase::database( "siebelDB" ).isOpen() )
    {
        QSqlDatabase::database( "siebelDB" ).close();
        QSqlDatabase::removeDatabase( "siebelDB" );
    }
    
    Debug::print( "database", "Destroying" );
}

void Database::insertSiebelItemIntoDB( SiebelItem item, const QString& dbname )
{
    Debug::log( "database", "Inserting SiebelItem for " + item.id + " " + item.queue );

    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    
    query.prepare( "INSERT INTO QMON_SIEBEL( ID, QUEUE, GEO, HOURS, STATUS, SEVERITY, SOURCE, RESPOND_VIA, "
                   "                         CREATED, LAST_UPDATE, INQUEUE, SLA, SUPPORT_PROGRAM, SUPPORT_PROGRAM_LONG, "
                   "                         ROUTING_PRODUCT, SUPPORT_GROUP_ROUTING, INT_TYPE, SUBTYPE, SERVICE_LEVEL, "
                   "                         BRIEF_DESC, CRITSIT, HIGH_VALUE, DETAILED_DESC, CATEGORY, CREATOR, ROW_ID ) "
                   " VALUES "
                   "( :id, :queue, :geo, :hours, :status, :severity, :source, :respond_via, :created, :last_update, :inqueue, "
                   "  :sla, :support_program, :support_program_long, :routing_product, :support_group_routing, :int_type, :subtype, "
                   "  :service_level, :brief_desc, :critsit, :high_value,  :detailed_desc, :category, :creator, :row_id )" );

    query.bindValue( ":id", item.id );
    query.bindValue( ":queue", item.queue );
    query.bindValue( ":geo", item.geo );
    query.bindValue( ":hours", item.hours );
    query.bindValue( ":status", item.status );
    query.bindValue( ":severity", item.severity );
    query.bindValue( ":source", item.source );
    query.bindValue( ":respond_via", item.respond_via );
    query.bindValue( ":created", convertTime( item.created, true) );
    query.bindValue( ":last_update", convertTime( item.last_update, true ) );
    query.bindValue( ":inqueue", QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss" ) );
    query.bindValue( ":sla", convertTime( item.sla ) );
    query.bindValue( ":support_program", item.support_program );
    query.bindValue( ":support_program_long", item.support_program_long );
    query.bindValue( ":routing_product", item.routing_product );
    query.bindValue( ":support_group_routing", item.support_group_routing );
    query.bindValue( ":int_type", item.int_type );
    query.bindValue( ":subtype", item.subtype );
    query.bindValue( ":service_level", item.service_level );
    query.bindValue( ":brief_desc", item.brief_desc );   
    query.bindValue( ":critsit", item.critsit );
    query.bindValue( ":high_value", item.high_value );    
    query.bindValue( ":detailed_desc", item.detailed_desc );
    query.bindValue( ":category", item.category );
    query.bindValue( ":creator", item.creator );
    query.bindValue( ":row_id", item.row_id );
    
    query.exec();
    
    Debug::logQuery( query, db.connectionName() );
    
    QSqlQuery cquery( db );
    
    cquery.prepare( "INSERT INTO CUSTOMER( ID, CUSTOMER, CONTACT_PHONE, CONTACT_FIRSTNAME, CONTACT_LASTNAME, "
                   "                           CONTACT_EMAIL, CONTACT_TITLE, CONTACT_LANG, ONSITE_PHONE ) "
                   "VALUES"
                   "( :id, :customer, :contact_phone, :contact_firstname, :contact_lastname, :contact_email, "
                   "  :contact_title, :contact_lang, :onsite_phone )" );
    
    cquery.bindValue( ":id", item.id );
    cquery.bindValue( ":customer", item.customer );
    cquery.bindValue( ":contact_phone", item.contact_phone );
    cquery.bindValue( ":contact_firstname", item.contact_firstname );
    cquery.bindValue( ":contact_lastname", item.contact_lastname );
    cquery.bindValue( ":contact_email", item.contact_email );
    cquery.bindValue( ":contact_title", item.contact_title );
    cquery.bindValue( ":contact_lang", item.contact_lang );
    cquery.bindValue( ":onsite_phone", item.onsite_phone );
    
    cquery.exec();                  
    
    Debug::logQuery( cquery, db.connectionName() );
}

void Database::updateSiebelItem( SiebelItem item, const QString& dbname, const QString& dbname1 )
{
    QSqlDatabase db;
    QSqlDatabase db1;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    if ( dbname1.isNull() ) 
    {
        db1 = QSqlDatabase::database( "siebelDB" );
    }
    else
    {
        db1 = QSqlDatabase::database( dbname1 );
    }
    
    QSqlQuery query( db );
    
    query.prepare( "UPDATE QMON_SIEBEL SET GEO = :geo, HOURS = :hours, STATUS = :status, SEVERITY = :severity, "
                   "                       SOURCE = :source, RESPOND_VIA = :respond_via, CREATED = :created, "
                   "                       LAST_UPDATE = :last_update, SLA = :sla, SUPPORT_PROGRAM = :support_program,"
                   "                       SUPPORT_PROGRAM_LONG = :support_program_long, ROUTING_PRODUCT = :routing_product,"
                   "                       SUPPORT_GROUP_ROUTING = :support_group_routing, INT_TYPE = :int_type,"
                   "                       SUBTYPE = :subtype, SERVICE_LEVEL = :service_level, BRIEF_DESC = :brief_desc,"
                   "                       CRITSIT = :critsit, HIGH_VALUE = :high_value, DETAILED_DESC = :detailed_desc, "
                   "                       CATEGORY = :category, CREATOR = :creator, ROW_ID = :row_id WHERE ID = :id" );
    
    query.bindValue( ":geo", item.geo );
    query.bindValue( ":hours", item.hours );
    query.bindValue( ":status", item.status );
    query.bindValue( ":severity", item.severity );
    query.bindValue( ":source", item.source );
    query.bindValue( ":respond_via", item.respond_via );
    query.bindValue( ":created", convertTime( item.created, true ) );
    query.bindValue( ":last_update", convertTime( item.last_update, true ) );
    query.bindValue( ":sla", convertTime( item.sla ) );
    query.bindValue( ":support_program", item.support_program );
    query.bindValue( ":support_program_long", item.support_program_long );
    query.bindValue( ":routing_product", item.routing_product );
    query.bindValue( ":support_group_routing", item.support_group_routing );
    query.bindValue( ":int_type", item.int_type );
    query.bindValue( ":subtype", item.subtype );
    query.bindValue( ":service_level", item.service_level );
    query.bindValue( ":brief_desc", item.brief_desc );
    query.bindValue( ":critsit", item.critsit );
    query.bindValue( ":high_value", item.high_value );
    query.bindValue( ":detailed_desc", item.detailed_desc );
    query.bindValue( ":category", item.category );
    
    if ( item.subtype == "Collaboration" )
    {
        query.bindValue( ":creator", getCreator( item.id, dbname1 ) );
    }
    query.bindValue( ":row_id", item.row_id );
    query.bindValue( ":id", item.id );

    query.exec();

    Debug::logQuery( query, db.connectionName() );
    
    QSqlQuery cquery( db );
    
    cquery.prepare( "UPDATE CUSTOMER SET CUSTOMER = :customer, CONTACT_PHONE = :contact_phone, CONTACT_FIRSTNAME = :contact_firstname, "
                   "                          CONTACT_LASTNAME = :contact_lastname, CONTACT_EMAIL = :contact_email, "
                   "                          CONTACT_TITLE = :contact_title, CONTACT_LANG = :contact_lang, ONSITE_PHONE = :onsite_phone "
                   "                          WHERE ID = :id" );
    
    cquery.bindValue( ":customer", item.customer );
    cquery.bindValue( ":contact_phone", item.contact_phone );
    cquery.bindValue( ":contact_firstname", item.contact_firstname );
    cquery.bindValue( ":contact_lastname", item.contact_lastname );
    cquery.bindValue( ":contact_email", item.contact_email );
    cquery.bindValue( ":contact_title", item.contact_title );
    cquery.bindValue( ":contact_lang", item.contact_lang );
    cquery.bindValue( ":onsite_phone", item.onsite_phone );
    cquery.bindValue( ":id", item.id );

    cquery.exec();
    
    Debug::logQuery( cquery, db.connectionName() );
}

QString Database::getDetDesc( const QString& sr, const QString& dbname )
{
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "siebelDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );

    query.prepare( "SELECT DESC_TEXT FROM SIEBEL.S_SRV_REQ WHERE SR_NUM = :sr" );
    
    query.bindValue( ":sr", sr );
    query.exec();
    
    if ( query.next() )
    {
        return query.value(0).toString();
    }
    else
    {
        return "ERROR";
    }
}

void Database::updateSiebelQueue( SiebelItem si, const QString& dbname )
{
    Debug::log( "database", "Updating Siebel queue for " + si.id + " to " + si.queue );
    
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    
    query.prepare( "UPDATE QMON_SIEBEL SET QUEUE = :queue, INQUEUE = :inqueue WHERE id = :id" );
                
    query.bindValue( ":queue", si.queue );
    query.bindValue( ":inqueue", QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss" ) );
    query.bindValue( ":id", si.id );
                
    query.exec();
    
    Debug::logQuery( query, db.connectionName() );
}

QString Database::getCreator(const QString& sr, const QString& dbname )
{
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "siebelDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    
    query.prepare( "SELECT LOGIN FROM SIEBEL.S_USER WHERE PAR_ROW_ID = ( SELECT CREATED_BY FROM SIEBEL.S_SRV_REQ WHERE SR_NUM = :sr )" );

    query.bindValue( ":sr", sr );
    
    query.exec();

    Debug::logQuery( query, db.connectionName() );
    
    if ( query.next() )
    {
        return query.value( 0 ).toString();
    }
    else
    {
        return "ERROR";
    }
}

QList< QueueItem > Database::getUserQueue( const QString& engineer, const QString& dbname )
{
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "siebelDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
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
    
    Debug::logQuery( query, db.connectionName() );
    
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
            i.creator = getCreator( i.id, dbname );
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

void Database::updatePseudoQueues( const QString& qDb, const QString& mDb )
{
    QSqlDatabase qdb;
    QSqlDatabase mdb;
    
    if ( qDb.isNull() ) 
    {
        qdb = QSqlDatabase::database( "qmonDB" );
    }
    else
    {
        qdb = QSqlDatabase::database( qDb );
    }
    
    if ( mDb.isNull() ) 
    {
        mdb = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        mdb = QSqlDatabase::database( mDb );
    }
    
    QSqlQuery query( qdb );
    QSqlQuery inQuery( mdb );
    
    query.prepare( "SELECT PseudoQueue FROM _NovQueuePseudoQueue" );
    query.exec();
    
    while ( query.next() )
    {
          inQuery.prepare( "INSERT INTO PSEUDOQ( QUEUENAME ) VALUES ( :queuename )" );
          inQuery.bindValue( ":queuename", query.value(0).toString() );
          inQuery.exec();
    }
}

QStringList Database::getPseudoQueues( const QString& dbname )
{
    QStringList list;
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    query.exec( "SELECT * FROM PSEUDOQ ORDER BY QUEUENAME ASC" );
    
    while ( query.next() )
    {
        list.append( query.value( 0 ).toString() );
    }
    
    return list;
}


void Database::deleteSiebelItemFromDB( const QString& id, const QString& dbname )
{
    Debug::log( "database", "Deleting SiebelItem " + id );
    
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    
    query.prepare( "DELETE FROM QMON_SIEBEL WHERE ID = :id" );
    query.bindValue( ":id", id );
    
    query.exec();
    
    Debug::logQuery( query, db.connectionName() );

    QSqlQuery cquery( db );
    
    cquery.prepare( "DELETE FROM CUSTOMER WHERE ID = :id" );
    cquery.bindValue( ":id", id );
    
    cquery.exec();
    
    Debug::logQuery( cquery, db.connectionName() );
}

QStringList Database::getQmonSiebelList( const QString& dbname)
{
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
   
    QStringList l;
    
    query.prepare( "SELECT ID FROM QMON_SIEBEL" );
    
    query.exec();
    
    Debug::logQuery( query, db.connectionName() );

    while( query.next() )
    {
        l.append( query.value( 0 ).toString() );
    }
    
    return l;    
}

bool Database::siebelExistsInDB( const QString& id, const QString& dbname )
{
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    
    query.prepare( "SELECT ID FROM QMON_SIEBEL WHERE ( ID = :id )" );
    query.bindValue( ":id", id );
       
    query.exec();
    
    Debug::logQuery( query, db.connectionName() );
    
    if ( query.next() )
    {
        return true;
    }
    else 
    {    
        return false;
    }
}

bool Database::siebelQueueChanged( SiebelItem si, const QString& dbname )
{
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    
    query.prepare( "SELECT QUEUE FROM QMON_SIEBEL WHERE ( ID = :id )" );
    query.bindValue( ":id", si.id );
    
    query.exec();

    Debug::logQuery( query, db.connectionName() );
    
    if ( query.next() )
    {
        if ( query.value( 0 ).toString() == si.queue )
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

bool Database::siebelSeverityChanged( SiebelItem si, const QString& dbname )
{
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    
    query.prepare( "SELECT SEVERITY FROM QMON_SIEBEL WHERE ( ID = :id )" );
    query.bindValue( ":id", si.id );
    
    query.exec();

    Debug::logQuery( query, db.connectionName() );
    
    if ( query.next() )
    {
        if ( query.value( 0 ).toString() == si.severity )
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

bool Database::isChat( const QString& id, const QString& dbname )
{
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    
    query.prepare( "SELECT ID FROM QMON_CHAT WHERE ( SR = :id )" );
    query.bindValue( ":id", id );
    
    query.exec();
    
    Debug::logQuery( query, db.connectionName() );
    
    if ( query.next() )
    {
        return true;
    }
    else 
    {    
        return false;
    }
}

QString Database::getQmonBdesc( const QString& id, const QString& dbname )
{
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    
    query.prepare( "SELECT BDESC FROM QMON_SIEBEL WHERE ( ID = :id )" );
    query.bindValue( ":id", id );
    
    query.exec();
    
    Debug::logQuery( query, db.connectionName() );
    
    if ( query.next() )
    {
        return query.value( 0 ).toString();
    }
    else
    {
        return "ERROR";
    }
}

void Database::updateBomgarItemInDB( BomgarItem bi, const QString& dbname )
{
    Debug::log( "database", "Inserting BomgarItem " + bi.id + " " + bi.sr );
        
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    
    query.prepare( "INSERT INTO QMON_CHAT( ID, SR, NAME, DATE ) VALUES ( :id, :sr, :name, :date )" );
                     
    query.bindValue( ":id", bi.id );
    query.bindValue( ":sr", bi.sr );
    query.bindValue( ":name", bi.name );
    query.bindValue( ":date", bi.date );
    
    query.exec();
    
    Debug::logQuery( query, db.connectionName() );   
}

void Database::deleteBomgarItemFromDB( const QString& id, const QString& dbname )
{
    Debug::log( "database", "Deleting BomgarItem " + id );
    
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    
    query.prepare( "DELETE FROM QMON_CHAT WHERE ID = :id" );
    query.bindValue( ":id", id );
    
    query.exec();
    
    Debug::logQuery( query, db.connectionName() );    
}


QList< SiebelItem > Database::getSrsForQueue( const QString& queue, const QString& dbname )
{
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    
    QList< SiebelItem > list;
    
    if (  queue == "NONE" )
    {    
        query.prepare( "SELECT ID, QUEUE, GEO, HOURS, STATUS, SEVERITY, SOURCE, RESPOND_VIA, CREATED, LAST_UPDATE, "
                       "INQUEUE, SLA, SUPPORT_PROGRAM, SUPPORT_PROGRAM_LONG, ROUTING_PRODUCT, SUPPORT_GROUP_ROUTING, "
                       "INT_TYPE, SUBTYPE, SERVICE_LEVEL, BRIEF_DESC, CRITSIT, HIGH_VALUE, DETAILED_DESC, CATEGORY, "
                       "CREATOR, ROW_ID from QMON_SIEBEL ORDER BY CREATED ASC" );
    }
    else
    {
        query.prepare( "SELECT ID, QUEUE, GEO, HOURS, STATUS, SEVERITY, SOURCE, RESPOND_VIA, CREATED, LAST_UPDATE, "
                       "INQUEUE, SLA, SUPPORT_PROGRAM, SUPPORT_PROGRAM_LONG, ROUTING_PRODUCT, SUPPORT_GROUP_ROUTING, "
                       "INT_TYPE, SUBTYPE, SERVICE_LEVEL, BRIEF_DESC, CRITSIT, HIGH_VALUE, DETAILED_DESC, CATEGORY, "
                       "CREATOR, ROW_ID from QMON_SIEBEL WHERE ( QUEUE = :queue ) ORDER BY CREATED ASC" );
        
        query.bindValue( ":queue", queue );
    }
    
    query.exec();
    
    Debug::logQuery( query, db.connectionName() );
    
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
        si.row_id = query.value( 25 ).toString();
        
        if ( getBomgarQueue( query.value( 0 ).toString(), dbname ) == "NOCHAT" )
        {
            si.isChat = false;
        }
        else
        {
            si.isChat = true;
            si.bomgarQ = getBomgarQueue( query.value( 0 ).toString(), dbname );
        }
        
        if ( si.subtype == "Collaboration" )
        {
            si.isCr = true;
            si.creator = query.value( 24 ).toString();
        }
        else
        {
            si.isCr = false;
        
            QSqlQuery cquery( db );
        
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

QStringList Database::getCurrentBomgars( const QString& dbname )
{
    QStringList list;
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );

    query.prepare( "SELECT SR, NAME FROM QMON_CHAT" );
    
    query.exec();
    
    Debug::logQuery( query, db.connectionName() );
        
    while( query.next() )
    {
        QString tmp = query.value( 0 ).toString() + "|||" + query.value( 1 ).toString();
        list.append( tmp );
    }

    return list;
}

QStringList Database::getQmonBomgarList( const QString& dbname )
{
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    QStringList l;
    
    query.prepare( "SELECT ID FROM QMON_CHAT" );
    
    query.exec();
    
    Debug::logQuery( query, db.connectionName() );
    
    while( query.next() ) 
    {
        l.append( query.value( 0 ).toString() );
    }
    
    return l;    
}

bool Database::bomgarExistsInDB( const QString& id, const QString& dbname )
{
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    
    query.prepare( "SELECT ID FROM QMON_CHAT WHERE ( ID = :id )" );
    query.bindValue( ":id", id );
    
    query.exec();
    
    Debug::logQuery( query, db.connectionName() );
    
    if ( query.next() )
    {
        return true;
    }
    else 
    {
        return false;
    }
}

void Database::updateBomgarQueue( BomgarItem bi, const QString& dbname )
{
    Debug::log( "database", "Updating BomgarQueue for " + bi.id + " " + bi.sr + " to " + bi.name );
    
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    
    query.prepare( "UPDATE QMON_CHAT SET NAME = :name WHERE ID = :id" );
    query.bindValue( ":name", bi.name );
    query.bindValue( ":id", bi.id );
    
    query.exec();
    
    Debug::logQuery( query, db.connectionName() );   
}

QString Database::getBomgarQueue( const QString& id, const QString& dbname )
{
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    
    query.prepare( "SELECT NAME FROM QMON_CHAT WHERE ( SR = :id )" );
    query.bindValue( ":id", id );
    
    query.exec();
    
    Debug::logQuery( query, db.connectionName() );
    
    if( query.next() ) 
    {
        return query.value( 0 ).toString();
    }
    else
    {
        return "NOCHAT";
    }
}

QString Database::getBomgarQueueById( const QString& id, const QString& dbname )
{
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "mysqlDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    
    query.prepare( "SELECT NAME FROM QMON_CHAT WHERE ( ID = :id )" );
    query.bindValue( ":id", id );
    
    query.exec();
    
    Debug::logQuery( query, db.connectionName() );
    
    if( query.next() ) 
    {
        return query.value( 0 ).toString();
    }
    else
    {
        return "NOCHAT";
    }
}

QString Database::convertTime( const QString& dt, bool correct )
{
    QDateTime d = QDateTime::fromString( dt, "M/d/yyyy hh:mm:ss AP" );
    QDateTime out;
    
    if ( !d.isValid() ) d = QDateTime::fromString( dt, "yyyy-MM-ddThh:mm:ss" );
    
    if ( correct )
    {
        QString time = d.toString( "yyyy-MM-dd" ) + " " + QString::number( d.time().hour() - 1 ) + ":" + d.toString( "mm:ss" );
        out = QDateTime::fromString( time, "yyyy-MM-dd H:mm:ss" );
    }
    else
    {
        out = d;
    }
  

    qDebug() << d.toString("yyyy-MM-dd hh:mm:ss") << out.toString("yyyy-MM-dd hh:mm:ss");
    return ( out.toString("yyyy-MM-dd hh:mm:ss") );
}

QList< SiebelItem > Database::getQmonSrs( const QString& dbname )
{
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "siebelDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    
    QList< SiebelItem > list;
    
    query.prepare(  "select "
                    "  sr.sr_num as ID, "
                    "  u.login as QUEUE, "
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
                    "  sr.SR_SUBTYPE_CD as SOURCE,  "
                    "  sr.X_RESPOND_VIA as RESPOND_VIA, "
                    "  sr.CREATED, "
                    "  sr.LAST_UPD as LAST_UPDATE,  "
                    "  case when sr.EXP_CLOSE_DT IS NULL then NVL(sr.EXP_CLOSE_DT, TO_DATE('01-01-1970', 'MM-DD-YYYY')) "
                    "  else sr.EXP_CLOSE_DT end as SLA, "
                    "  sr.X_SUPPORT_PROG as SUPPORT_PROGRAM, "
                    "  e.NAME as SUPPORT_PROGRAM_LONG,"
                    "  prd.name as ROUTING_PRODUCT, "
                    "  sr.X_SUPP_GRP_ROUTING AS SUPPORT_GROUP_ROUTING, "
                    "  sr.SR_TYPE_CD as INT_TYPE, "
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
                    "  sr.DESC_TEXT as DETAILED_DESC, "
                    "  sr.SR_CATEGORY_CD as CATEGORY, "
                    "  sr.ROW_ID "
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
                    "  and c.job_title LIKE('Pseudo Use%') "
                    "  and sr.sr_stat_id = 'Open' "
                    "  and sr.agree_id = e.row_id "
                    "  and e.svc_calendar_id = cal.row_id "
                    "  and sr.cst_ou_id = ext.row_id "
                    "  and sr.X_PROD_FEATURE_ID = prd.row_id "
                    "  and ext.row_id = flag.row_id  "
                    "union "
                    "select "
                    "  sr.sr_num as ID, "
                    "  u.login as QUEUE, "
                    "  case "
                    "    when sr.BU_ID = '0-R9NH' then 'Default Organization' "
                    "    when sr.BU_ID = '1-AHT' then 'EMEA' "
                    "    when sr.BU_ID = '1-AHZ' then 'ASIAPAC' "
                    "    when sr.BU_ID = '1-AHV' then 'ASIAPAC' "
                    "    when sr.BU_ID = '1-AHX' then 'USA' "
                    "    when sr.BU_ID = '1-AHZ' then 'LATIN AMERICA' "
                    "    when sr.BU_ID = '1-AI1' then 'CANADA' "
                    "  else 'Undefined' end as GEO, "
                    "  cal.NAME as HOURS, "
                    "  sr.SR_SUB_STAT_ID as STATUS, "
                    "  sr.SR_SEV_CD as SEVERITY, "
                    "  sr.SR_SUBTYPE_CD as SOURCE, "
                    "  sr.X_RESPOND_VIA as RESPOND_VIA, "
                    "  sr.CREATED, sr.LAST_UPD as LAST_UPDATE, "
                    "  case when sr.EXP_CLOSE_DT IS NULL then NVL(sr.EXP_CLOSE_DT, TO_DATE('01-01-1970', 'MM-DD-YYYY')) "
                    "  else sr.EXP_CLOSE_DT end as SLA, "
                    "  sr.X_SUPPORT_PROG as SUPPORT_PROGRAM, "
                    "  e.NAME as SUPPORT_PROGRAM_LONG, "
                    "  prd.name as ROUTING_PRODUCT, "
                    "  sr.X_SUPP_GRP_ROUTING AS SUPPORT_GROUP_ROUTING, "
                    "  sr.SR_TYPE_CD as INT_TYPE, "
                    "  sr.X_SR_SUB_TYPE as SUBTYPE, "
                    "  e.ENTL_PRIORITY_NUM as SERVICE_LEVEL, "
                    "  sr.SR_TITLE as BRIEF_DESC, "
                    "  flag.ATTRIB_11 as CRITSIT, "
                    "  flag.ATTRIB_56 as HIGH_VALUE, "
                    "  ext.NAME as CUSTOMER, "
                    "  '1', "
                    "  '1', "
                    "  '1',  "
                    "  '1', "
                    "  '1', "
                    "  '1', "
                    "  '1', "
                    "  '1',"
                    "  sr.SR_CATEGORY_CD as SR_CATEGORY, "
                    "  sr.ROW_ID  "
                    "from "
                    "  siebel.s_srv_req sr, "
                    "  siebel.s_user u, "
                    "  siebel.s_entlmnt e, "
                    "  siebel.s_sched_cal cal,"
                    "  siebel.s_org_ext ext, "
                    "  siebel.s_prod_int prd, "
                    "  siebel.s_org_ext_x flag "
                    "where "
                    "  sr.owner_emp_id = '0-1' "
                    "  and sr.owner_emp_id = u.row_id  "
                    "  and sr.sr_stat_id = 'Open' "
                    "  and sr.agree_id = e.row_id "
                    "  and e.svc_calendar_id = cal.row_id "
                    "  and sr.cst_ou_id = ext.row_id "
                    "  and sr.X_PROD_FEATURE_ID = prd.row_id "
                    "  and ext.row_id = flag.row_id "
                    "union "
                    "select  "
                    "  sr.sr_num, "
                    "  'null', "
                    "  case "
                    "    when sr.BU_ID = '0-R9NH' then 'Default Organization' "
                    "    when sr.BU_ID = '1-AHT' then 'EMEA' "
                    "    when sr.BU_ID = '1-AHZ' then 'ASIAPAC' "
                    "    when sr.BU_ID = '1-AHV' then 'ASIAPAC' "
                    "    when sr.BU_ID = '1-AHX' then 'USA' "
                    "    when sr.BU_ID = '1-AHZ' then 'LATIN AMERICA' "
                    "    when sr.BU_ID = '1-AI1' then 'CANADA' "
                    "  else 'Undefined' end as GEO, "
                    "  cal.NAME as HOURS, "
                    "  sr.SR_SEV_CD as SEVERITY, "
                    "  sr.SR_SUB_STAT_ID as STATUS, "
                    "  sr.SR_SUBTYPE_CD as SOURCE, "
                    "  sr.X_RESPOND_VIA as RESPOND_VIA, "
                    "  sr.CREATED, sr.LAST_UPD as LAST_UPDATE, "
                    "  case when sr.EXP_CLOSE_DT IS NULL then NVL(sr.EXP_CLOSE_DT, TO_DATE('01-01-1970', 'MM-DD-YYYY')) "
                    "  else sr.EXP_CLOSE_DT end as SLA, "
                    "  sr.X_SUPPORT_PROG as SUPPORT_PROGRAM, "
                    "  e.NAME as SUPPORT_PROGRAM_LONG, "
                    "  prd.name as ROUTING_PRODUCT, "
                    "  sr.X_SUPP_GRP_ROUTING AS SUPPORT_GROUP_ROUTING, "
                    "  sr.SR_TYPE_CD as INT_TYPE, "
                    "  sr.X_SR_SUB_TYPE as SUBTYPE, "
                    "  e.ENTL_PRIORITY_NUM as SERVICE_LEVEL, "
                    "  sr.SR_TITLE BRIEF_DESC, "
                    "  flag.ATTRIB_11 as CRITSIT, "
                    "  flag.ATTRIB_56 as HIGH_VALUE, "
                    "  ext.NAME as CUSTOMER, "
                    "  '0', "
                    "  '0', "
                    "  '0', "
                    "  '0', "
                    "  '0', "
                    "  '0', "
                    "  '0', "
                    "  '0',"
                    "  sr.SR_CATEGORY_CD as SR_CATEGORY, "
                    "  sr.ROW_ID "
                    "from "
                    "  siebel.s_srv_req sr, "
                    "  siebel.s_entlmnt e, "
                    "  siebel.s_sched_cal cal, "
                    "  siebel.s_org_ext ext,"
                    "  siebel.s_prod_int prd, "
                    "  siebel.s_org_ext_x flag "
                    "where "
                    "  sr.owner_emp_id is null "
                    "  and sr.sr_stat_id = 'Open' "
                    "  and sr.agree_id = e.row_id "
                    "  and e.svc_calendar_id = cal.row_id "
                    "  and sr.cst_ou_id = ext.row_id "
                    "  and sr.X_PROD_FEATURE_ID = prd.row_id "
                    "  and ext.row_id = flag.row_id" );

    query.exec();
    
    Debug::logQuery( query, db.connectionName() );
    
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
        si.sla = query.value( 10 ).toString();
        si.support_program = query.value( 11 ).toString();
        si.support_program_long = query.value( 12 ).toString();
        si.routing_product = query.value( 13 ).toString();
        si.support_group_routing = query.value( 14 ).toString();
        si.int_type = query.value( 15 ).toString();
        si.subtype = query.value( 16 ).toString();
        si.service_level = query.value( 17 ).toString();
        si.brief_desc = query.value( 18 ).toString();

        if ( query.value( 16 ).toString() == "Collaboration" )
        {
            si.isCr = true;
            si.creator = getCreator( si.id, dbname );
        }
        else
        {
            si.isCr = false;
        }
        
        if ( query.value( 19 ).toString() == "Y" )
        {
            si.critsit = true;
        }
        else
        {
            si.critsit = false;
        }
        
        if ( query.value( 20 ).toString() == "Y" )
        {
            si.high_value = true;
        }
        else
        {
            si.high_value = false;
        }
        
        si.customer = query.value( 21 ).toString();
        si.contact_phone = query.value( 22 ).toString();
        si.contact_firstname = query.value( 23 ).toString();
        si.contact_lastname = query.value( 24 ).toString();
        si.contact_email = query.value( 25 ).toString();
        si.contact_title = query.value( 26 ).toString();
        si.contact_lang = query.value( 27 ).toString();
        si.onsite_phone = query.value( 28 ).toString();
        si.detailed_desc = query.value( 29 ).toString();
        si.category = query.value( 30 ).toString();
        si.row_id = query.value( 31 ).toString();
        
        list.append( si );
    }

    return list;
}

QStringList Database::srInfo( const QString& sr, const QString& dbname )
{
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "siebelDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    
    query.prepare( "SELECT SR_TITLE, g.FST_NAME, g.LAST_NAME, ext.NAME "
                   "from siebel.s_srv_req sr, siebel.s_contact g, siebel.s_org_ext ext "
                   "where sr.sr_num = :sr and g.row_id = sr.CST_CON_ID and ext.row_id = sr.CST_OU_ID" );
    
    query.bindValue( ":sr", sr );
    
    query.exec();
    
    QStringList info;
    
    if ( query.next() )
    {
        info.append( query.value( 0 ).toString() );
        info.append( query.value( 1 ).toString() );
        info.append( query.value( 2 ).toString() );
        info.append( query.value( 3 ).toString() );
    }

    return info;
}

QList< BomgarItem > Database::getChats( const QString& dbname )
{
    QSqlDatabase db;
    
    if ( dbname.isNull() ) 
    {
        db = QSqlDatabase::database( "qmonDB" );
    }
    else
    {
        db = QSqlDatabase::database( dbname );
    }
    
    QSqlQuery query( db );
    
    QList< BomgarItem > list;
    
    query.prepare( "SELECT LSID, EXTERNAL_KEY, CONFERENCE_NAME, TIMESTAMP FROM Collector_ODBC_NTSCHAT" );
    query.exec();
    
    Debug::logQuery( query, db.connectionName() );
    
    while ( query.next() ) 
    {
        BomgarItem bi;
        
        bi.id = query.value( 0 ).toString();
        bi.sr = query.value( 1 ).toString();
        bi.name = query.value( 2 ).toString();
        bi.date = query.value( 3 ).toString();
     
        list.append( bi );
    }
        
    return list;
}

#include "database.moc"
