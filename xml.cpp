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

QString XML::qmon( QList<SiebelItem> list )
{   
    QDateTime now = QDateTime::currentDateTime();
    QString xml;
    
    xml += "<qmon>\n";
    
    for ( int i = 0; i < list.size(); ++i ) 
    {
        SiebelItem si = list.at( i );
            
        QDateTime odate = QDateTime::fromString( si.created, "yyyy-MM-dd hh:mm:ss" );
        QDateTime adate = QDateTime::fromString( si.last_update, "yyyy-MM-dd hh:mm:ss" );
        QDateTime sladate = QDateTime::fromString( si.sla, "yyyy-MM-dd hh:mm:ss" );
        QDateTime qdate = QDateTime::fromString( si.inqueue, "yyyy-MM-dd hh:mm:ss" );

        qint64 age = ( odate.secsTo( now ) - ( Settings::timezoneCorrection() * 3600 ) );
        qint64 lu = ( adate.secsTo( now ) - ( Settings::timezoneCorrection() * 3600 ));
        qint64 qt = ( qdate.secsTo( now ) );
        qint64 sla = ( now.secsTo( sladate ) );

        xml += "  <sr>\n";
        xml += "    <id><![CDATA[" + si.id + "]]></id>\n";
        xml += "    <queue><![CDATA[" + si.queue + "]]></queue>\n";
    
        if ( si.isChat )
        {
            xml += "    <bomgarQ>" + si.bomgarQ + "</bomgarQ>\n";
        }

        if ( si.isCr )
        {
            xml += "    <srtype>cr</srtype>\n";
            xml += "    <creator>" + si.creator + "</creator>\n";
            
        }
        else
        {
            xml += "    <srtype>sr</srtype>\n";
            xml += "    <cus_account><![CDATA[" + si.customer + "]]></cus_account>\n";
            xml += "    <cus_num><![CDATA[" + si.cstNum + "]]></cus_num>\n";
            
            if ( ( !si.contact_firstname.isEmpty() ) && ( si.contact_firstname != "1" ) && ( si.contact_firstname != "0" ) )
            {
                xml += "    <cus_firstname><![CDATA[" + si.contact_firstname + "]]></cus_firstname>\n";
            }
            
            if ( ( !si.contact_lastname.isEmpty() ) && ( si.contact_lastname != "1" )&& ( si.contact_lastname != "0" )   )
            {
                xml += "    <cus_lastname><![CDATA[" + si.contact_lastname + "]]></cus_lastname>\n";
            }
            
            if ( ( !si.contact_title.isEmpty() ) && ( si.contact_title != "1" ) && ( si.contact_title != "0" ) )
            {
                xml += "    <cus_title><![CDATA[" + si.contact_title + "]]></cus_title>\n";
            }
            
            if ( ( !si.contact_email.isEmpty() ) && ( si.contact_email != "1" ) && ( si.contact_email != "0" ) )
            {
                xml += "    <cus_email><![CDATA[" + si.contact_email + "]]></cus_email>\n";  
            }
            
            if ( ( !si.contact_phone.isEmpty() ) && ( si.contact_phone != "1" ) && ( si.contact_phone != "0" ) )
            {
                xml += "    <cus_phone><![CDATA[" + si.contact_phone + "]]></cus_phone>\n";
            }
            
            if ( ( !si.onsite_phone.isEmpty() ) && ( si.onsite_phone != "1" ) && ( si.onsite_phone != "0" ) )
            {
                xml += "    <cus_onsitephone><![CDATA[" + si.onsite_phone + "]]></cus_onsitephone>\n";
            }
            
            if ( ( !si.contact_lang.isEmpty() ) && ( si.contact_lang != "1" ) && ( si.contact_lang != "0" ) )
            {
                xml += "    <cus_lang><![CDATA[" + si.contact_lang + "]]></cus_lang>\n";
            }
            
            if ( !si.alt_contact.isEmpty() )
            {
                xml += "    <alt_contact><![CDATA[" + si.alt_contact + "]]></alt_contact>\n";
            }
        }

        xml += "    <severity><![CDATA[" + si.severity + "]]></severity>\n";
        xml += "    <status><![CDATA[" + si.status + "]]></status>\n";
        xml += "    <bdesc><![CDATA[" + si.brief_desc + "]]></bdesc>\n";
        
        if ( ( !si.detailed_desc.isEmpty() ) && ( si.detailed_desc != "1" ) && ( si.detailed_desc != "0" ) )
        {
            xml += "    <ddesc><![CDATA[" + si.detailed_desc + "]]></ddesc>\n";        
        }
        
        xml += "    <geo><![CDATA[" + si.geo + "]]></geo>\n";
        xml += "    <hours><![CDATA[" + si.hours + "]]></hours>\n";
        xml += "    <source><![CDATA[" + si.source + "]]></source>\n";
        xml += "    <support_program><![CDATA[" + si.support_program + "]]></support_program>\n";
        xml += "    <support_program_long><![CDATA[" + si.support_program_long + "]]></support_program_long>\n";
        xml += "    <routing_product><![CDATA[" + si.routing_product + "]]></routing_product>\n";
        xml += "    <support_group_routing><![CDATA[" + si.support_group_routing + "]]></support_group_routing>\n";
        xml += "    <int_type><![CDATA[" + si.int_type + "]]></int_type>\n";
        xml += "    <subtype><![CDATA[" + si.subtype + "]]></subtype>\n";
        xml += "    <service_level><![CDATA[" + si.service_level + "]]></service_level>\n";
        xml += "    <category><![CDATA[" + si.category + "]]></category>\n";
        
        if ( !si.respond_via.isEmpty() )
        {
            xml += "    <respond_via><![CDATA[" + si.respond_via + "]]></respond_via>\n";      
        }
        
        if ( !si.bugId.isEmpty() )
        {
            xml += "    <bug>" + si.bugId + "</bug>\n";
            //xml += "    <bug_desc>" + si.bugDesc + "</bug_desc>\n";
        }
        
        xml += "    <age>" + QString::number( age ) + "</age>\n";
        xml += "    <agedate>" + si.created + "</agedate>\n";
        xml += "    <lastupdate>" + QString::number( lu ) + "</lastupdate>\n";
        xml += "    <lastupdatedate>" + si.last_update + "</lastupdatedate>\n";
        xml += "    <timeinQ>" + QString::number( qt ) + "</timeinQ>\n";

        if ( sla > 0 )      
        {
            xml += "    <sla>" + QString::number( sla ) + "</sla>\n";
            xml += "    <sladate>" + si.sla + "</sladate>\n";
        }
        
        if ( si.high_value )
        {
            xml += "    <highvalue>1</highvalue>\n";
        }
        
        if ( si.critsit )
        {
            xml += "    <critsit>1</critsit>\n";
        }
        
        xml += "  </sr>\n";
    }
    
    xml += "</qmon>\n";
  
    return xml;
}

QString XML::qmonDate( QList<SiebelItem> list )
{   
    QString xml;
    QDateTime now = QDateTime::currentDateTime();
    
    xml += "<qmon>\n";
    
    for ( int i = 0; i < list.size(); ++i ) 
    {
        SiebelItem si = list.at( i );
        QDateTime sladate = QDateTime::fromString( si.sla, "yyyy-MM-dd hh:mm:ss" );

        xml += "  <sr>\n";
        xml += "    <id><![CDATA[" + si.id + "]]></id>\n";
        xml += "    <queue><![CDATA[" + si.queue + "]]></queue>\n";
    
        if ( si.isChat )
        {
            xml += "    <bomgarQ>" + si.bomgarQ + "</bomgarQ>\n";
        }

        if ( si.isCr )
        {
            xml += "    <srtype>cr</srtype>\n";
            xml += "    <creator>" + si.creator + "</creator>\n";
            
        }
        else
        {
            xml += "    <srtype>sr</srtype>\n";
            xml += "    <cus_account><![CDATA[" + si.customer + "]]></cus_account>\n";
            xml += "    <cus_num><![CDATA[" + si.cstNum + "]]></cus_num>\n";
            
            if ( ( !si.contact_firstname.isEmpty() ) && ( si.contact_firstname != "1" ) && ( si.contact_firstname != "0" ) )
            {
                xml += "    <cus_firstname><![CDATA[" + si.contact_firstname + "]]></cus_firstname>\n";
            }
            
            if ( ( !si.contact_lastname.isEmpty() ) && ( si.contact_lastname != "1" )&& ( si.contact_lastname != "0" )   )
            {
                xml += "    <cus_lastname><![CDATA[" + si.contact_lastname + "]]></cus_lastname>\n";
            }
            
            if ( ( !si.contact_title.isEmpty() ) && ( si.contact_title != "1" ) && ( si.contact_title != "0" ) )
            {
                xml += "    <cus_title><![CDATA[" + si.contact_title + "]]></cus_title>\n";
            }
            
            if ( ( !si.contact_email.isEmpty() ) && ( si.contact_email != "1" ) && ( si.contact_email != "0" ) )
            {
                xml += "    <cus_email><![CDATA[" + si.contact_email + "]]></cus_email>\n";  
            }
            
            if ( ( !si.contact_phone.isEmpty() ) && ( si.contact_phone != "1" ) && ( si.contact_phone != "0" ) )
            {
                xml += "    <cus_phone><![CDATA[" + si.contact_phone + "]]></cus_phone>\n";
            }
            
            if ( ( !si.onsite_phone.isEmpty() ) && ( si.onsite_phone != "1" ) && ( si.onsite_phone != "0" ) )
            {
                xml += "    <cus_onsitephone><![CDATA[" + si.onsite_phone + "]]></cus_onsitephone>\n";
            }
            
            if ( ( !si.contact_lang.isEmpty() ) && ( si.contact_lang != "1" ) && ( si.contact_lang != "0" ) )
            {
                xml += "    <cus_lang><![CDATA[" + si.contact_lang + "]]></cus_lang>\n";
            }
        }

        xml += "    <severity><![CDATA[" + si.severity + "]]></severity>\n";
        xml += "    <status><![CDATA[" + si.status + "]]></status>\n";
        xml += "    <bdesc><![CDATA[" + si.brief_desc + "]]></bdesc>\n";
        
        if ( ( !si.detailed_desc.isEmpty() ) && ( si.detailed_desc != "1" ) && ( si.detailed_desc != "0" ) )
        {
            xml += "    <ddesc><![CDATA[" + si.detailed_desc + "]]></ddesc>\n";        
        }
        
        xml += "    <geo><![CDATA[" + si.geo + "]]></geo>\n";
        xml += "    <hours><![CDATA[" + si.hours + "]]></hours>\n";
        xml += "    <source><![CDATA[" + si.source + "]]></source>\n";
        xml += "    <support_program><![CDATA[" + si.support_program + "]]></support_program>\n";
        xml += "    <support_program_long><![CDATA[" + si.support_program_long + "]]></support_program_long>\n";
        xml += "    <routing_product><![CDATA[" + si.routing_product + "]]></routing_product>\n";
        xml += "    <support_group_routing><![CDATA[" + si.support_group_routing + "]]></support_group_routing>\n";
        xml += "    <int_type><![CDATA[" + si.int_type + "]]></int_type>\n";
        xml += "    <subtype><![CDATA[" + si.subtype + "]]></subtype>\n";
        xml += "    <service_level><![CDATA[" + si.service_level + "]]></service_level>\n";
        xml += "    <category><![CDATA[" + si.category + "]]></category>\n";
        
        if ( !si.respond_via.isEmpty() )
        {
            xml += "    <respond_via><![CDATA[" + si.respond_via + "]]></respond_via>\n";      
        }
        
        xml += "    <created>" + si.created + "</created>\n";
        xml += "    <lastupdate>" + si.last_update + "</lastupdate>\n";
        xml += "    <queuedate>" + si.inqueue + "</queuedate>\n";

        if ( now.secsTo( sladate ) > 0 )
        {
            xml += "    <sla>" + si.sla + "</sla>\n";
        }
        
        if ( si.high_value )
        {
            xml += "    <highvalue>1</highvalue>\n";
        }
        
        if ( si.critsit )
        {
            xml += "    <critsit>1</critsit>\n";
        }
        
        xml += "  </sr>\n";
    }
    
    xml += "</qmon>\n";
  
    return xml;
}

QString XML::queue( QList<QueueItem> list )
{
    QString xml;

    xml += "<queue>\n";
    xml += "<total>" + QString::number( list.size() ) + "</total>\n";

    for ( int i = 0; i < list.size(); ++i ) 
    {
        xml += sr( list.at( i ) );
    }
    
    xml += "</queue>\n";
    
    return xml;
}

QString XML::sr( QueueItem qi )
{
    QString xml;

    xml += "  <sr>\n";
    xml += "    <id><![CDATA[" + qi.id + "]]></id>\n";
    
    if ( qi.isCr )
    {
        xml += "    <srtype>cr</srtype>\n";
        xml += "    <creator>" + qi.creator + "</creator>\n";
    }
    else
    {
        xml += "    <srtype>sr</srtype>\n";
       xml += "     <cus_account><![CDATA[" + qi.customer + "]]></cus_account>\n";
        xml += "    <cus_num><![CDATA[" + qi.cstNum + "]]></cus_num>\n";
        
        if ( ( !qi.contact_firstname.isEmpty() ) && ( qi.contact_firstname != "1" ) && ( qi.contact_firstname != "0" ) )
        {
            xml += "    <cus_firstname><![CDATA[" + qi.contact_firstname + "]]></cus_firstname>\n";
        }
        
        if ( ( !qi.contact_lastname.isEmpty() ) && ( qi.contact_lastname != "1" )&& ( qi.contact_lastname != "0" )   )
        {
            xml += "    <cus_lastname><![CDATA[" + qi.contact_lastname + "]]></cus_lastname>\n";
        }
        
        if ( ( !qi.contact_title.isEmpty() ) && ( qi.contact_title != "1" ) && ( qi.contact_title != "0" ) )
        {
            xml += "    <cus_title><![CDATA[" + qi.contact_title + "]]></cus_title>\n";
        }
        
        if ( ( !qi.contact_email.isEmpty() ) && ( qi.contact_email != "1" ) && ( qi.contact_email != "0" ) )
        {
            xml += "    <cus_email><![CDATA[" + qi.contact_email + "]]></cus_email>\n";  
        }
        
        if ( ( !qi.contact_phone.isEmpty() ) && ( qi.contact_phone != "1" ) && ( qi.contact_phone != "0" ) )
        {
            xml += "    <cus_phone><![CDATA[" + qi.contact_phone + "]]></cus_phone>\n";
        }
        
        if ( ( !qi.onsite_phone.isEmpty() ) && ( qi.onsite_phone != "1" ) && ( qi.onsite_phone != "0" ) )
        {
            xml += "    <cus_onsitephone><![CDATA[" + qi.onsite_phone + "]]></cus_onsitephone>\n";
        }
        
        if ( ( !qi.contact_lang.isEmpty() ) && ( qi.contact_lang != "1" ) && ( qi.contact_lang != "0" ) )
        {
            xml += "    <cus_lang><![CDATA[" + qi.contact_lang + "]]></cus_lang>\n";
        }
        
        if ( !qi.alt_contact.isEmpty() )
        {
            xml += "    <alt_contact><![CDATA[" + qi.alt_contact + "]]></alt_contact>\n";
        }
    }

    xml += "    <owner><![CDATA[" + qi.owner + "]]></owner>\n";
    
    if ( !qi.subOwner.isEmpty() )
    {
        xml += "    <subowner><![CDATA[" + qi.subOwner + "]]></subowner>\n";
    }
    
    xml += "    <severity><![CDATA[" + qi.severity + "]]></severity>\n";
    xml += "    <status><![CDATA[" + qi.status + "]]></status>\n";
    xml += "    <bdesc><![CDATA[" + qi.brief_desc + "]]></bdesc>\n";
    
    if ( ( !qi.detailed_desc.isEmpty() ) && ( qi.detailed_desc != "1" ) && ( qi.detailed_desc != "0" ) )
    {
        xml += "    <ddesc><![CDATA[" + qi.detailed_desc + "]]></ddesc>\n";        
    }
    
    if ( !qi.bugId.isEmpty() )
    {
        xml += "    <bug>" + qi.bugId + "</bug>\n";
        xml += "    <bug_desc><![CDATA[" + qi.bugDesc + "]]></bug_desc>\n";
    }
    
    xml += "    <geo><![CDATA[" + qi.geo + "]]></geo>\n";
    xml += "    <hours><![CDATA[" + qi.hours + "]]></hours>\n";
    xml += "    <contract><![CDATA[" + qi.support_program + "]]></contract>\n";
    xml += "    <service_level><![CDATA[" + QString::number( qi.service_level ) + "]]></service_level>\n";
    xml += "    <created>" + qi.created + "</created>\n";
    xml += "    <lastupdate>" + qi.last_update + "</lastupdate>\n";
    
    if ( qi.high_value )
    {
        xml += "    <highvalue>1</highvalue>\n";
    }
    
    if ( qi.critsit )
    {
        xml += "    <critsit>1</critsit>\n";
    }
    
    xml += "  </sr>\n";
    
    return xml;
}

QString XML::stats( Statistics s )
{
    QString xml;
    QList<ClosedItem> closedList = s.closedList;
    QList<CsatItem> csatList = s.csatList;
    
    int ttsAvg = 0;
    
    for ( int i = 0; i < closedList.size(); ++i ) 
    {
        ttsAvg = ( ttsAvg + closedList.at( i ).tts );
    }
    
    if ( closedList.size() > 0 )
    {
        ttsAvg = ttsAvg / closedList.size();
    }
    
    int engSatAvg = 0;
    int srSatAvg = 0;
    int rtsPercent = 0;
    
    int engTotal = 0;
    int srTotal = 0;
   
    
    for ( int i = 0; i < csatList.size(); ++i ) 
    {
        if ( csatList.at( i ).engsat != 88 )
        {
            engSatAvg = engSatAvg + csatList.at( i ).engsat;
            engTotal = engTotal + 1;
        }
        
        if ( csatList.at( i ).srsat != 88 )
        {
            srSatAvg = srSatAvg + csatList.at( i ).srsat;
            srTotal = srTotal + 1;
        }
        
        rtsPercent = rtsPercent + csatList.at( i ).rts;
    }
    
    if ( engTotal > 0 )
    {
        engSatAvg = ( engSatAvg / engTotal );
    }
    
    if ( srTotal > 0 )
    {
        srSatAvg = ( srSatAvg / srTotal );
    }
    
    if ( csatList.size() > 0 )
    {
        rtsPercent = ( rtsPercent * 100 / csatList.size() );
    }
    
    xml += "<stats>\n\n";

    xml += "  <closeddata>\n";
    xml += "    <srs>" + s.closedSr + "</srs>\n";
    xml += "    <crs>" + s.closedCr + "</crs>\n";
    xml += "    <srttsavg>" + QString::number( ttsAvg ) + "</srttsavg>\n\n";
    xml += "  </closeddata>\n";
    xml += "  <closed>\n\n";
    
    for ( int i = 0; i < closedList.size(); ++i ) 
    {
        xml += "    <closedsr>\n";
        xml += "      <sr>" + closedList.at(i).sr + "</sr>\n";
        xml += "      <tts>" + QString::number( closedList.at(i).tts ) + "</tts>\n";
        xml += "      <customer><![CDATA[" + closedList.at(i).customer + "]]></customer>\n";
        xml += "      <bdesc><![CDATA[" + closedList.at(i).bdesc + "]]></bdesc>\n";
        xml += "    </closedsr>\n\n";
    }
    
    xml += "  </closed>\n\n";
    
    xml += "  <csatdata>\n\n";
    xml += "    <engavg>" + QString::number( engSatAvg ) + "</engavg>\n";
    xml += "    <sravg>" + QString::number( srSatAvg ) + "</sravg>\n";
    xml += "    <rtsavg>" + QString::number( rtsPercent ) + "</rtsavg>\n\n";
    xml += "  </csatdata>\n\n";
    xml += "  <csat>\n\n";
    
    for ( int i = 0; i < csatList.size(); ++i ) 
    {
        xml += "    <survey>\n";
        xml += "      <sr>" + csatList.at(i).sr + "</sr>\n";
        xml += "      <rts>" + QString::number( csatList.at(i).rts ) + "</rts>\n";
        
        if ( csatList.at( i ).engsat != 88 )
        {
            xml += "      <engsat>" + QString::number( csatList.at(i).engsat ) + "</engsat>\n";
        }
        else
        {
            //xml += "      <engsat>NONE</engsat>\n";
        }
        
        if ( csatList.at( i ).srsat != 88 )
        {
            xml += "      <srsat>" + QString::number( csatList.at(i).srsat ) + "</srsat>\n";
        }
        else
        {
            //xml += "      <srsat>NONE</srsat>\n";
        }
        
        xml += "      <customer><![CDATA[" + csatList.at(i).customer + "]]></customer>\n";
        xml += "      <bdesc><![CDATA[" + csatList.at(i).bdesc + "]]></bdesc>\n";
        xml += "    </survey>\n\n";
    }
    
    xml += "  </csat>\n\n";
    xml += "</stats>\n";
    
    return xml;
}
