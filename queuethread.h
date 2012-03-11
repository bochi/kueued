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

#ifndef QUEUETHREAD_H
#define QUEUETHREAD_H

#include <QThread>
#include <QTcpSocket>

class QueueThread : public QThread
{
    Q_OBJECT

    public:
        QueueThread( int sd, const QString& eng, QObject *parent );
        
        void run();

    signals:
        void done();

    private:
        int mSocket;
        QString mDbName;
        QString mEngineer;
};


#endif
