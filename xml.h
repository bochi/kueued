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

#ifndef _XML_H_
#define _XML_H_

#include <QString>
#include "database.h"

class SiebelItem;
//class BomgarItem;
class Statistics;

class XML
{
    public:
        static QString qmon( QList<SiebelItem> );
        static QString qmonDate( QList<SiebelItem> );
        static QString queue( QList<QueueItem> );
        static QString ltssCust( QList<LTSScustomer> );
        static QString sr( QueueItem );
        static QString stats( Statistics );
    
};

class CsatItem
{
    public:
        QString sr;
        int engsat;
        int srsat;
        QString customer;
        QString bdesc;
        int rts;
};

class ClosedItem
{
    public:
        QString sr;
        int tts;
        QString customer;
        QString bdesc;
};
    
class Statistics
{
    public:
        QString closedSr;
        QString closedCr;
        QString openSr;
        QList<CsatItem> csatList;
        QList<ClosedItem> closedList;
};


#endif
