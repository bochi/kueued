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
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "DBServer" ).toString();
}

int Settings::refreshSeconds()
{
    QSettings settings( "/etc/kueued.conf",QSettings::NativeFormat);
    return settings.value( "refreshSeconds", 60 ).toInt();
}

QString Settings::mysqlHost()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "mysqlHost" ).toString();
}

QString Settings::mysqlUser()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "mysqlUser" ).toString();
}

QString Settings::mysqlDatabase()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "mysqlDatabase" ).toString();
}

QString Settings::mysqlPassword()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "mysqlPassword" ).toString();
}

QString Settings::oracleHost()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "oracleHost" ).toString();
}

QString Settings::oracleUser()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "oracleUser" ).toString();
}

QString Settings::oraclePassword()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "oraclePassword" ).toString();
}

QString Settings::oracleDatabase()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "oracleDatabase" ).toString();
}

QString Settings::latestVersion()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "latestVersion" ).toString();
}

int Settings::oraclePort()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "oraclePort" ).toInt();
}

int Settings::timezoneCorrection()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "timezoneCorrection" ).toInt();
}

QString Settings::siebelDatabase()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "siebelDatabase" ).toString();
}

QString Settings::siebelHost()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "siebelHost" ).toString();
}

QString Settings::siebelUser()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "siebelUser" ).toString();
}

QString Settings::siebelPassword()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "siebelPassword" ).toString();
}

QString Settings::qmonDbDatabase()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "qmonDbDatabase" ).toString();
}

QString Settings::qmonDbUser()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "qmonDbUser" ).toString();
}

QString Settings::qmonDbPassword()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "qmonDbPassword" ).toString();
}

QString Settings::unityURL()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "unityURL" ).toString();
}

bool Settings::logQueries()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "logQueries" ).toBool();
}

bool Settings::debugLog()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "debugLog" ).toBool();
}