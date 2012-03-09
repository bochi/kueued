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
    QDateTime odate = QDateTime::fromString( si->created, "yyyy-MM-dd hh:mm:ss" );
    QDateTime adate = QDateTime::fromString( si->last_update, "yyyy-MM-dd hh:mm:ss" );
    QDateTime sladate = QDateTime::fromString( si->sla, "yyyy-MM-dd hh:mm:ss" );
    QDateTime qdate = QDateTime::fromString( si->inqueue, "yyyy-MM-dd hh:mm:ss" );
    
    QString xml;
    
    qint64 age = ( odate.secsTo( now ) - ( Settings::timezoneCorrection() * 3600 ) );
    qint64 lu = ( adate.secsTo( now ) - ( Settings::timezoneCorrection() * 3600 ));
    qint64 qt = ( qdate.secsTo( now ) );
    qint64 sla = ( now.secsTo( sladate ) );

    xml += "  <sr>\n";
    xml += "    <id><![CDATA[" + si->id + "]]></id>\n";
    xml += "    <queue><![CDATA[" + si->queue + "]]></queue>\n";
   
    if ( si->isChat )
    {
        xml += "    <bomgarQ>" + si->bomgarQ + "</bomgarQ>\n";
    }

    if ( si->isCr )
    {
        xml += "    <srtype>cr</srtype>\n";
        xml += "    <creator>" + si->creator + "</creator>\n";
        
    }
    else
    {
        xml += "    <srtype>sr</srtype>\n";
        xml += "    <customer><![CDATA[" + si->customer + "]]></customer>\n";
    }

    xml += "    <severity><![CDATA[" + si->severity + "]]></severity>\n";
    xml += "    <status><![CDATA[" + si->status + "]]></status>\n";
    xml += "    <bdesc><![CDATA[" + si->brief_desc + "]]></bdesc>\n";
    xml += "    <ddesc><![CDATA[" + si->detailed_desc + "]]></ddesc>\n";
    xml += "    <geo><![CDATA[" + si->geo + "]]></geo>\n";
    xml += "    <hours><![CDATA[" + si->hours + "]]></hours>\n";
    xml += "    <source><![CDATA[" + si->source + "]]></source>\n";
    xml += "    <support_program><![CDATA[" + si->support_program + "]]></support_program>\n";
    xml += "    <support_program_long><![CDATA[" + si->support_program_long + "]]></support_program_long>\n";
    xml += "    <routing_product><![CDATA[" + si->routing_product + "]]></routing_product>\n";
    xml += "    <support_group_routing><![CDATA[" + si->support_group_routing + "]]></support_group_routing>\n";
    xml += "    <int_type><![CDATA[" + si->int_type + "]]></int_type>\n";
    xml += "    <subtype><![CDATA[" + si->subtype + "]]></subtype>\n";
    xml += "    <service_level><![CDATA[" + si->service_level + "]]></service_level>\n";
    xml += "    <contact_phone><![CDATA[" + si->contact_phone + "]]></contact_phone>\n";
    xml += "    <onsite_phone><![CDATA[" + si->onsite_phone + "]]></onsite_phone>\n";
    xml += "    <category><![CDATA[" + si->category + "]]></category>\n";
    xml += "    <respond_via><![CDATA[" + si->respond_via + "]]></respond_via>\n";
    xml += "    <age>" + QString::number( age ) + "</age>\n";
    xml += "    <lastupdate>" + QString::number( lu ) + "</lastupdate>\n";
    xml += "    <timeinQ>" + QString::number( qt ) + "</timeinQ>\n";

    if ( sla > 0 )      
    {
         xml += "    <sla>" + QString::number( sla ) + "</sla>\n";
    }
    
    if ( si->high_value )
    {
        xml += "    <highvalue>yes</highvalue>\n";
    }
    
    if ( si->critsit )
    {
        xml += "    <critsit>yes</critsit>\n";
    }
    
    xml += "  </sr>\n";
  
    return xml;
}