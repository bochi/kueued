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

#ifndef DATABASE_H_
#define DATABASE_H_

#include <QtSql>

class SiebelItem;
class BomgarItem;
class QueueItem;
class PseudoQueueItem;
class LTSScustomer;

class Database : public QObject
{
    Q_OBJECT

    public: 
        Database();
        ~Database();
    
        static void insertSiebelItemIntoDB( SiebelItem, const QString& = QString::Null() );
        static void updateSiebelQueue( SiebelItem, const QString& = QString::Null() );
        static void updateSiebelItem( SiebelItem, const QString& = QString::Null(), const QString& = QString::Null() );
        static void deleteSiebelItemFromDB( const QString&, const QString& = QString::Null() );
        static QStringList getQmonSiebelList( const QString& = QString::Null());
        static QStringList getOracleSrList( const QString& = QString::Null() );
        static bool siebelExistsInDB( const QString&, const QString& = QString::Null() );
        static bool siebelQueueChanged( SiebelItem, const QString& = QString::Null() );
        static bool siebelSeverityChanged( SiebelItem, const QString& = QString::Null() );
        static bool isChat( const QString&, const QString& = QString::Null() );
        static QString getQmonBdesc( const QString&, const QString& = QString::Null() );
        static QString getDetDesc( const QString&, const QString& = QString::Null() );
        static QString getCreator( const QString&, const QString& = QString::Null() );
        static QString getSrForCr( const QString&, const QString& = QString::Null(), const QString& = QString::Null(), const QString& = QString::Null() );
        static QString getSrForCrMysql( const QString&, const QString& = QString::Null() );
        static QString getSrForCrReport( const QString&, const QString& = QString::Null(), const QString& = QString::Null(), const QString& = QString::Null() );
        
        static void updateBomgarItemInDB( BomgarItem, const QString& = QString::Null() );
        static void deleteBomgarItemFromDB( const QString&, const QString& = QString::Null() );
        static QList< SiebelItem > getSrsForQueue( const QString& = "NONE", const QString& = QString::Null() );
        static QStringList getQmonBomgarList( const QString& = QString::Null() );
        static bool bomgarExistsInDB( const QString&, const QString& = QString::Null() );
        static QString getBomgarQueue( const QString&, const QString& = QString::Null() );
        static QString getBomgarQueueById( const QString&, const QString& = QString::Null() );
        static void updateBomgarQueue( BomgarItem, const QString& = QString::Null() );
        static void updatePseudoQueues( const QString& = QString::Null(), const QString& = QString::Null() );
        static void updateLTSScustomers( const QString& = QString::Null(), const QString& = QString::Null() );
        static QStringList getPseudoQueues( const QString& =  QString::Null() );
        static QStringList getCurrentBomgars( const QString& = QString::Null() );
        static QStringList getSrnumsForQueue( const QString&, const QString&, const QString& = QString::Null() );
        static QString convertTime( const QString& );
        static QString formatPhone( QString, const QString& );
        
        static QList< LTSScustomer > getLTSScustomersExt( const QString& );
        static QList< LTSScustomer > getLTSScustomers( const QString& );
        static bool checkLTSSbyId( const QString&, const QString& );
        
        static QList< QueueItem > getUserQueue( const QString&, const QString& = QString::Null(), const QString& = QString::Null(), const QString& = QString::Null(), bool = false );
        static QueueItem getSrInfo( const QString&, const QString& = QString::Null(), const QString& = QString::Null(), const QString& = QString::Null() );
        static QString getSrStatus( const QString&, const QString& = QString::Null() );
        static QList< SiebelItem > getQmonSrs( const QString& = QString::Null(), const QString& = QString::Null(), const QString& = QString::Null() );
        static QList< BomgarItem > getChats( const QString& = QString::Null() );  
        
        static QStringList srInfo( const QString&, const QString& );
        static QString getBugDesc( QString, const QString& );

        static bool isBugID( const QString& );
        
        static QString escapeString( QString );
        
        static bool openMysqlDB( const QString& );
        static bool openSiebelDB( const QString& );
        static bool openQmonDB( const QString& );
        static bool openReportDB( const QString& );
};

class SiebelItem 
{ 
    public:
        QString id;
        QString queue;
        QString geo;
        QString hours;
        QString status;
        QString severity;
        QString source;
        QString respond_via;
        QString created;
        QString last_update;
        QString inqueue;
        QString sla;
        QString support_program;
        QString support_program_long;
        QString routing_product;
        QString support_group_routing;
        QString int_type;
        QString subtype;
        QString service_level;
        QString brief_desc;
        QString customer;
        QString contact_phone;
        QString contact_firstname;
        QString contact_lastname;
        QString contact_email;
        QString contact_title;
        QString contact_lang;
        QString onsite_phone;
        QString detailed_desc;
        QString category;
        QString row_idetailed_desc;
        QString row_id;
        QString creator;
        QString bomgarQ;
        QString alt_contact;
        QString bugId;
        QString cstNum;
        QString crsr;
        QString crsrcust;
	QString crsrdesc;
	QString subowner;
        bool isCr;
        bool isChat;
        bool critsit;
        bool high_value;
};

class QueueItem
{
    public:
        QString id;
        QString status;
        QString geo;
        QString hours;
        QString severity;
        QString created;
        QString last_update;
        QString support_program;
        QString subtype;
        int service_level;
        QString brief_desc;
        QString detailed_desc;
        QString customer;
        QString contact_phone;
        QString contact_firstname;
        QString contact_lastname;
        QString contact_email;
        QString contact_title;
        QString contact_lang;
        QString onsite_phone;
        QString creator;
        QString format_string;
        QString alt_contact;
        QString bugId;
        QString bugDesc;
        QString cstNum;
        QString owner;
        QString subOwner;
        QString crsr;
        bool isCr;
        bool critsit;
        bool high_value;
};

class BomgarItem 
{
    public:
        QString id;
        QString sr;
        QString name;
        QString date;
};

class PseudoQueueItem
{
    public:
        QString displayname;
        QString queuename;
};

class LTSScustomer
{
    public:
        QString entitlement_name;
        QString agreement_nr;
        QString entitlement_id;
        QString entitlement_end_date;
        QString agreement_status;
        QString account_name;
        QString entitlement_start_date;
        QString support_program;
        QString geo;
        QString oracle_customer_nr;
};

#endif
