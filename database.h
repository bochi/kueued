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

#include "kueued.h"

#include <QtSql>
#include "../qt-examples/tools/customcompleter/textedit.h"

class SiebelItem;
class BomgarItem;
class WhoIsInBomgarItem;

class Database : public QObject
{
    Q_OBJECT

    public: 
        Database();
        ~Database();
    
        static void insertSiebelItemIntoDB( SiebelItem* );
        static void updateSiebelQueue( SiebelItem* );
        static void updateSiebelSeverity( SiebelItem* );
        static void updateSiebelDisplay( const QString& );
        static void deleteSiebelItemFromDB( const QString& );
        static QStringList getQmonSiebelList();
	static QStringList getOracleSrList();
        static bool siebelExistsInDB( const QString& );
        static bool siebelQueueChanged( SiebelItem* );
        static bool siebelSeverityChanged( SiebelItem* );
        static bool isChat( const QString& );
        static QString getQmonBdesc( const QString& );
        
        static void updateBomgarItemInDB( BomgarItem* );
        static void deleteBomgarItemFromDB( const QString& );
        static QList< SiebelItem* > getSrsForQueue( const QString& = "NONE" );
        static QStringList getQmonBomgarList();
        static bool bomgarExistsInDB( const QString& );
        static QString getBomgarQueue( const QString& );
        static QString getBomgarQueueById( const QString& );
        static void updateBomgarQueue( BomgarItem* );
        
        static QString getBugForSr( const QString& );
        static QString critSitFlagForSr( const QString& );
        static QString highValueFlagForSr( const QString& );
        static QString highValueCritSitFlagForSr( const QString& );
        
        static QString convertTime( const QString& );

    private:
        QString mDBfile;        
};


class SiebelItem 
{
    public:
        QString id;
        QString queue;
        QString severity;
        QString hours;
        QString source;
        QString contactvia;
        QString odate;
        QString adate;
        QString qdate;
        QString status;
        QString contract;
        QString queue1; 
        QString phone;
        QString onsitephone;
        QString geo; 
        QString wtf;
        QString routing;
        QString bdesc;
        QString sla;
        QString display;
        QString bomgarQ;
        bool isChat;
        QString critSit;
        QString highValue;
};

class BomgarItem 
{
    public:
        QString id;
        QString sr;
        QString repteam;
        QString name;
        QString date;
        QString qdate;
        QString someNumber;
};

class WhoIsInBomgarItem
{
    public:
        QString name;
        QString sr;
        QString timeInQueue;
        QString timeInSystem;
};

#endif
