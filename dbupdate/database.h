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

class Database : public QObject
{
    Q_OBJECT

    public: 
        Database();
        ~Database();
    
        static void insertSiebelItemIntoDB( SiebelItem* );
        static void updateSiebelQueue( SiebelItem* );
        static void updateSiebelItem( SiebelItem* );
        static void deleteSiebelItemFromDB( const QString& );
        static QStringList getQmonSiebelList();
	static QStringList getOracleSrList();
        static bool siebelExistsInDB( const QString& );
        static bool siebelQueueChanged( SiebelItem* );
        static bool siebelSeverityChanged( SiebelItem* );
        static bool isChat( const QString& );
        static QString getQmonBdesc( const QString& );
        static QString getDetDesc( const QString& );
        static QString getCreator( const QString& );
        
        static void updateBomgarItemInDB( BomgarItem* );
        static void deleteBomgarItemFromDB( const QString& );
        static QList< SiebelItem* > getSrsForQueue( const QString& = "NONE" );
        static QStringList getQmonBomgarList();
        static bool bomgarExistsInDB( const QString& );
        static QString getBomgarQueue( const QString& );
        static QString getBomgarQueueById( const QString& );
        static void updateBomgarQueue( BomgarItem* );
        
        static QStringList getCurrentBomgars();
        
        static QString convertTime( const QString& );
        
        static QList< SiebelItem* > getQmonSrs();
        static QList< BomgarItem* > getChats();  
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
        bool isCr;
        bool isChat;
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

#endif
