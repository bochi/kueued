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

#ifndef DEBUG_H
#define DEBUG_H

#include "settings.h"
#include <iostream>
#include <QFile>
#include <QtSql>

namespace Debug
{
    static void print( const QString& c, const QString& msg )
    {   
        if ( Settings::debugLog() )
        {
            char hostname[ 1024 ];
            gethostname( hostname, sizeof( hostname ) );
            QString host = hostname;

            QString t = "[" + QDateTime::currentDateTime().toString( "MM/dd hh:mm:ss" ) + "] [" + host.toUpper() + "] [" + c.toUpper() + "] ";
            std::cout << t.toStdString() << msg.toStdString() << std::endl;
        }
    }
    
    static void log( const QString& c, QString msg )
    {   
        char hostname[ 1024 ];
        gethostname( hostname, sizeof( hostname ) );
        QString host = hostname;
        
        QFile file( "/var/log/kueued/kueued.log" );
        
        if (!file.open(QIODevice::Append | QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << "File /var/log/kueued/kueued.log not open";
            return;
        }
        
        QTextStream out(&file);

        QString t = "[" + QDateTime::currentDateTime().toString( "MM/dd hh:mm:ss" ) + "] [" + host.toUpper() + "] [" + c.toUpper() + "] ";
        
        if ( msg.contains( "%7C" ) )
        {
            msg.replace( "%7C", "|" );
        }

        out << t << msg << "\n";
        file.close();
    }
    
    static void logQuery( QSqlQuery query, const QString& dbname )
    {   
        if ( Settings::logQueries() )
        {
            char hostname[ 1024 ];
            gethostname( hostname, sizeof( hostname ) );
            QString host = hostname;
            
            QFile file( "/var/log/kueued/kueued-queries.log" );
            
            if (!file.open(QIODevice::Append | QIODevice::WriteOnly | QIODevice::Text))
            {
                qDebug() << "File /var/log/kueued/kueued-queries.log not open";
                return;
            }
            
            QString err;

            if ( query.lastError().text() == " " )
            {
                err = "None";
            }
            else
            {
                err = query.lastError().text();
            }
            
            QTextStream out(&file);

            QString t = "[" + QDateTime::currentDateTime().toString( "MM/dd hh:mm:ss" ) + "] [" + host.toUpper() + "] [" + dbname + "] Error: " + err + "\nExecuted query: " + query.executedQuery();
            out << t << "\n\n";
            file.close();
        }
    }
}

#endif
