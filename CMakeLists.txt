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

PROJECT( kueued )
SET( KUEUED_VERSION git )

FIND_PACKAGE( Qt4 REQUIRED )

INCLUDE( ${QT_USE_FILE} )

INCLUDE_DIRECTORIES(  ${QT_INCLUDES} 
                      ${CMAKE_CURRENT_BINARY_DIR} 
                      ${CMAKE_CURRENT_SOURCE_DIR} ) 

ADD_DEFINITIONS( ${QT_DEFINITIONS} )

SET( QT_USE_QTSQL TRUE )
SET( QT_USE_QTNETWORK TRUE )
SET( QT_USE_XML TRUE )

ADD_SUBDIRECTORY( qtsingleapplication )

SET( QtSingleApp_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/qtsingleapplication )
SET( QT_SINGLEAPP_LIBRARY qtsingleapplication )

ADD_SUBDIRECTORY( qtservice )

SET( QtService_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/qtservice )
SET( QT_SERVICE_LIBRARY qtservice )

INCLUDE_DIRECTORIES(${QtService_INCLUDE_DIR} ${QtSingleApp_INCLUDE_DIR} )

SET( kueued_CPP main.cpp
                kueued.cpp
		database.cpp
                server.cpp
		network.cpp
		settings.cpp
		xml.cpp)

QT4_AUTOMOC( ${kueued_CPP} )

ADD_EXECUTABLE( kueued ${kueued_CPP} )
INSTALL( TARGETS kueued DESTINATION bin )
INSTALL( PROGRAMS scripts/kueued DESTINATION /etc/init.d )
#INSTALL( FILES kueued.conf DESTINATION /etc/kueued )
INSTALL( FILES kueued.log DESTINATION /var/log )
INSTALL( SCRIPT scripts/kueued_post.cmake )

TARGET_LINK_LIBRARIES( kueued ${QT_LIBRARIES} 
                              ${QT_QTCORE_LIBRARY} 
                              ${QT_QTSQL_LIBRARIES} 
                              ${QT_QTNETWORK_LIBRARY} 
			      ${QT_QTXML_LIBRARY}
			      ${QT_SINGLEAPP_LIBRARY} 
                              ${QT_SERVICE_LIBRARY} )


