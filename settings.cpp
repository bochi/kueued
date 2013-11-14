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
#include "simplecrypt/simplecrypt.h"

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

QString Settings::reportHost()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "reportHost" ).toString();
}

QString Settings::reportUser()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "reportUser" ).toString();
}

QString Settings::reportPassword()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "reportPassword" ).toString();
}

QString Settings::reportDatabase()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "reportDatabase" ).toString();
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

QString Settings::bugzillaUser()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "bugzillaUser" ).toString();
}

QString Settings::bugzillaPassword()
{
    // I know this is not really secure, but how would one hide the 
    // encryption key in OSS software? :P
    // At least it's not in the config file in cleartext...
    
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    SimpleCrypt crypto( Q_UINT64_C( 0424632454124622 ) );
    QString pw;
    
    if ( !settings.value( "bugzillaPassword" ).toString().isEmpty() )
    {
        pw = crypto.decryptToString( settings.value( "bugzillaPassword" ).toString() );
    }

    return pw;
}

QString Settings::l3Server()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "l3Server" ).toString();
}

QString Settings::l3User()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "l3User" ).toString();
}

QString Settings::l3ApiKey()
{
    QSettings settings( "/etc/kueued.conf", QSettings::NativeFormat);
    return settings.value( "l3ApiKey" ).toString();
}