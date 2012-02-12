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
#include <QMap>
#include <QList>

class SiebelItem;

class Database : public QObject
{
    Q_OBJECT

    public: 
        Database();
        ~Database();
    
        static bool isChat( const QString& );
        static QList< SiebelItem* > getSrsForQueue( const QString& = "NONE" );
        static QString getBomgarQueue( const QString& );
        static QStringList getCurrentBomgars();
        static QString getBugForSr( const QString& );
        static QString critSitFlagForSr( const QString& );
        static QString highValueFlagForSr( const QString& );
        static QString highValueCritSitFlagForSr( const QString& );
	static QStringList getOracleSrList();        
        static QString convertTime( const QString& );
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

#endif
