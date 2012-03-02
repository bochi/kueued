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

#include "kueueddb.h"

#include <QtSql>
#include <QMap>
#include <QList>

class SiebelItem;
class BomgarItem;
class WhoIsInBomgarItem;

class Database : public QObject
{
    Q_OBJECT

    public: 
        Database();
        ~Database();
    
        static void insertSiebelItemIntoDB( SiebelItem*, QSqlDatabase );
        static void updateSiebelQueue( SiebelItem*, QSqlDatabase );
        static void updateSiebelSeverity( SiebelItem*, QSqlDatabase );
        static void updateSiebelDisplay( const QString&, QSqlDatabase );
        static void deleteSiebelItemFromDB( const QString&, QSqlDatabase );
        static QStringList getQmonSiebelList( QSqlDatabase);
	static QStringList getOracleSrList();
        static bool siebelExistsInDB( const QString&, QSqlDatabase );
        static bool siebelQueueChanged( SiebelItem*, QSqlDatabase );
        static bool siebelSeverityChanged( SiebelItem*, QSqlDatabase );
        static bool isChat( const QString&, QSqlDatabase );
        static QString getQmonBdesc( const QString&, QSqlDatabase );
        
        static void updateBomgarItemInDB( BomgarItem*, QSqlDatabase );
        static void deleteBomgarItemFromDB( const QString&, QSqlDatabase );
        static QList< SiebelItem* > getSrsForQueue( QSqlDatabase, const QString& = "NONE" );
        static QStringList getQmonBomgarList(QSqlDatabase);
        static bool bomgarExistsInDB( const QString&, QSqlDatabase );
        static QString getBomgarQueue( const QString&, QSqlDatabase );
        static QString getBomgarQueueById( const QString&, QSqlDatabase );
        static void updateBomgarQueue( BomgarItem*, QSqlDatabase );
        
        static QStringList getCurrentBomgars( QSqlDatabase );
        
        static QString getBugForSr( const QString&, QSqlDatabase );
        static QString critSitFlagForSr( const QString&, QSqlDatabase );
        static QString highValueFlagForSr( const QString&, QSqlDatabase );
        static QString highValueCritSitFlagForSr( const QString&, QSqlDatabase );
        
        static QString convertTime( const QString& );
        
        static QList< SiebelItem* > getQmonSrs( QSqlDatabase, QSqlDatabase );
        static QList< BomgarItem* > getChats( QSqlDatabase );

    private:
        QString mDBfile;        
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
        QString crSr;
        QString bdesc;
        QString customer;
        QString bomgarQ;
        QString contactvia;
        bool isCr;
        bool isChat;
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
