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

#ifndef KUEUED_H
#define KUEUED_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QtNetwork>

class SiebelItem;
class BomgarItem;
class WhoIsInBomgarItem;

class Kueued : public QObject
{
    Q_OBJECT

    public: 
        Kueued();
        ~Kueued();
        
    private:
        QNetworkAccessManager* mNAM;
        QStringList mNotifiedList;
        QStringList mQueueList;
        QNetworkReply* mBomgarReply;
        QNetworkReply* mSiebelReply;
        QTimer* mTimer;
                
    public slots:
        void update();
    
    private slots:
        void siebelJobDone();
        void bomgarJobDone();
        void whoIsInBomgarJobDone( QNetworkReply* );
        
    signals:
        void initialUpdate( int, int );
        void initialUpdateProgress( int );
        void initialUpdateDone();
        void qmonDataChanged();
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
};

class BomgarItem 
{
    public:
        QString id;
        QString sr;
        QString repteam;
        QString name;
        QString date;
        QString someNumber;
};

class WhoIsInBomgarItem
{
    public:
        QString name;
        QString sr;
        QString timeInQueue;
        QString timeInSystem;
};


#endif
