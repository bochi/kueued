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

#include "xml.h"
#include "settings.h"
#include "debug.h"

QString XML::sr( SiebelItem* si )
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime odate = QDateTime::fromString( si->odate, "yyyy-MM-dd hh:mm:ss" );
    QDateTime adate = QDateTime::fromString( si->adate, "yyyy-MM-dd hh:mm:ss" );
    QDateTime sladate = QDateTime::fromString( si->sla, "yyyy-MM-dd hh:mm:ss" );
    QDateTime qdate = QDateTime::fromString( si->qdate, "yyyy-MM-dd hh:mm:ss" );
    QString xml;
    
    qint64 lu = ( adate.secsTo( now ) - ( Settings::timezoneCorrection() * 3600 ));
    qint64 age = ( odate.secsTo( now ) - ( Settings::timezoneCorrection() * 3600 ) );
    qint64 qt = ( qdate.secsTo( now ) );
    qint64 sla = ( now.secsTo( sladate ) );
    
    xml += "  <sr>\n";
    xml += "    <id>" + si->id + "</id>\n";
    xml += "    <queue>" + si->queue + "</queue>\n";
    xml += "    <severity>" + si->severity + "</severity>\n";
    xml += "    <geo>" + si->geo + "</geo>\n";
    
    if (  si->source == "Internal" )
    {
        xml += "    <type>cr</type>\n";
    }
    else
    {
        xml += "    <type>sr</type>\n";
    }
    
    if ( si->isChat )
    {
        xml += "    <bomgarQ>" + si->bomgarQ + "</bomgarQ>\n";
    }    
    
       
    xml += "    <age>" + QString::number( age ) + "</age>\n";
   
    if ( sla > 0 )      
    {
         xml += "    <sla>" + QString::number( sla ) + "</sla>\n";
    }

    if ( lu < age )
    {
        xml += "    <lastupdate>" + QString::number( lu ) + "</lastupdate>\n";
    }
    
    xml += "    <timeinqueue>" +  QString::number( qt ) + "</timeinqueue>\n"; 
    xml += "    <description><![CDATA[" + si->bdesc + "]]></description>\n";
    xml += "    <status>" + si->status + "</status>\n";
    xml += "    <contract>" + si->contract + "</contract>\n";
    xml += "    <contact>" + si->contactvia + "</contact>\n";
    xml += "  </sr>\n";
  
    return xml;
}