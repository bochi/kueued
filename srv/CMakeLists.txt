#
#             kueued - create xml data for kueue's qmon 
#             (C) 2012 Stefan Bogner <sbogner@suse.com>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the
#
#   Free Software Foundation, Inc.
#   59 Temple Place - Suite 330
#   Boston, MA  02111-1307, USA
#
#   Have a lot of fun :-)

CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} .. ) 

SET( kueued-srv_CPP main.cpp
                    kueuedsrv.cpp
                    database.cpp
                    settings.cpp
                    server.cpp
                    xml.cpp)

QT4_AUTOMOC( ${kueued-srv_CPP} )

ADD_EXECUTABLE( kueued-srv ${kueued-srv_CPP} )
INSTALL( TARGETS kueued-srv DESTINATION bin )
INSTALL( PROGRAMS scripts/kueued-srv DESTINATION /etc/init.d )

TARGET_LINK_LIBRARIES( kueued-srv ${QT_LIBRARIES} 
                                  ${QT_QTCORE_LIBRARY} 
                                  ${QT_QTSQL_LIBRARIES} 
                                  ${QT_QTNETWORK_LIBRARY} 
                                  ${QT_QTXML_LIBRARY}
                                  ${QT_SINGLEAPP_LIBRARY} 
                                  ${QT_SERVICE_LIBRARY} )

