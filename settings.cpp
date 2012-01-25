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

#include "settings.h"

QString Settings::dBServer()
{
    QSettings settings( "/etc/kueued/kueued.conf" );
    return settings.value( "DBServer" ).toString();
}

void Settings::setdBServer( const QString& s )
{
    QSettings settings( "/etc/kueued/kueued.conf" );;
    settings.setValue( "DBServer", s );
    settings.sync();
}

int Settings::refreshSeconds()
{
    QSettings settings( "/etc/kueued/kueued.conf" );;
    return settings.value( "refreshSeconds", 60 ).toInt();
}

void Settings::setRefreshSeconds( const int& i )
{
    QSettings settings( "/etc/kueued/kueued.conf" );;
    settings.setValue( "refreshSeconds", i );
    settings.sync();
}

QString Settings::outputFile()
{
    QSettings settings( "/etc/kueued/kueued.conf" );;
    return settings.value( "outputFile" ).toString();
}

void Settings::setOutputFile( const QString& s )
{
    QSettings settings( "/etc/kueued/kueued.conf" );;
    settings.setValue( "outputFile", s );
    settings.sync();
}
