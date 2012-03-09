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
        xml += "    <customer>\n";
        xml += "      <account><![CDATA[" + si->customer + "]]></account>\n";
        
        if ( ( !si->contact_firstname.isEmpty() ) && ( si->contact_firstname != "1" ) && ( si->contact_firstname != "0" ) )
        {
            xml += "      <firstname><![CDATA[" + si->contact_firstname + "]]></firstname>\n";
        }
        
        if ( ( !si->contact_lastname.isEmpty() ) && ( si->contact_lastname != "1" )&& ( si->contact_lastname != "0" )   )
        {
            xml += "      <lastname><![CDATA[" + si->contact_lastname + "]]></lastname>\n";
        }
        
        if ( ( !si->contact_title.isEmpty() ) && ( si->contact_title != "1" ) && ( si->contact_title != "0" ) )
        {
            xml += "      <title><![CDATA[" + si->contact_title + "]]></title>\n";
        }
        
        if ( ( !si->contact_email.isEmpty() ) && ( si->contact_email != "1" ) && ( si->contact_email != "0" ) )
        {
            xml += "      <email><![CDATA[" + si->contact_email + "]]></email>\n";  
        }
        
        if ( ( !si->contact_phone.isEmpty() ) && ( si->contact_phone != "1" ) && ( si->contact_phone != "0" ) )
        {
            xml += "      <phone><![CDATA[" + si->contact_phone + "]]></phone>\n";
        }
        
        if ( ( !si->onsite_phone.isEmpty() ) && ( si->onsite_phone != "1" ) && ( si->onsite_phone != "0" ) )
        {
            xml += "      <onsitephone><![CDATA[" + si->onsite_phone + "]]></onsitephone>\n";
        }
        
        if ( ( !si->contact_lang.isEmpty() ) && ( si->contact_lang != "1" ) && ( si->contact_lang != "0" ) )
        {
            xml += "      <lang><![CDATA[" + si->contact_lang + "]]></lang>\n";
        }
        
        xml += "    </customer>\n";
    }

    xml += "    <severity><![CDATA[" + si->severity + "]]></severity>\n";
    xml += "    <status><![CDATA[" + si->status + "]]></status>\n";
    xml += "    <bdesc><![CDATA[" + si->brief_desc + "]]></bdesc>\n";
    
    if ( ( !si->detailed_desc.isEmpty() ) && ( si->detailed_desc != "1" ) && ( si->detailed_desc != "0" ) )
    {
        xml += "    <ddesc><![CDATA[" + si->detailed_desc + "]]></ddesc>\n";        
    }
    
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