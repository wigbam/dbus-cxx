/***************************************************************************
 *   Copyright (C) 2007,2008,2009 by Rick L. Vinyard, Jr.                  *
 *   rvinyard@cs.nmsu.edu                                                  *
 *   Copyright (C) 2014- by Robert Middleton                               *
 *   rm5248@rm5248.com                                                     *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 *                                                                         *
 *   The dbus-cxx library is free software; you can redistribute it and/or *
 *   modify it under the terms of the GNU General Public License           *
 *   version 3 as published by the Free Software Foundation.               *
 *                                                                         *
 *   The dbus-cxx library is distributed in the hope that it will be       *
 *   useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU   *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this software. If not see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/
#ifndef DBUSCXX_MACROS_H
#define DBUSCXX_MACROS_H

#if defined( DBUS_CXX_EXPORT )
 /* value forced by compiler command line, don't redefine */
#elif defined( _WIN32 )
#  if defined( DBUS_CXX_STATIC_BUILD )
#    define DBUS_CXX_EXPORT
#  elif defined( dbus_cxx_EXPORTS )
#    define DBUS_CXX_EXPORT __declspec( dllexport )
#  else
#    define DBUS_CXX_EXPORT __declspec( dllimport )
#  endif
#elif defined( __GNUC__ ) && __GNUC__ >= 4
#  define DBUS_CXX_EXPORT __attribute__ ( (__visibility__ ("default")) )
#else
#define DBUS_CXX_EXPORT
#endif

#endif /* DBUSCXX_MACROS_H */
