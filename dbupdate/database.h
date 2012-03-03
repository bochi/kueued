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
        QString hours;
        QString geo;
        QString odate;
        QString adate;
        QString qdate;
        QString status;
        QString severity;
        QString contract;
        QString sla;
        QString creator;
        QString bdesc;
        QString customer;
        QString bomgarQ;
        QString contactvia;
        bool isCr;
        bool isChat;
        bool highValue;
        bool critSit;
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
