/***************************************************************************
 *   Copyright (C) 2009,2010 by Rick L. Vinyard, Jr.                       *
 *   rvinyard@cs.nmsu.edu                                                  *
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
/*
 * fcntl.h compatibility shim
 */
#ifndef _WIN32
#include_next <fcntl.h>
#else

#ifdef _MSC_VER
#if _MSC_VER >= 1900
#include <../ucrt/fcntl.h>
#else
#include <../include/fcntl.h>
#endif
#else
#include_next <fcntl.h>
#endif /* _MSC_VER */

#endif /* _WIN32 */

#ifndef O_NONBLOCK
#define O_NONBLOCK      0x100000
#endif

#ifndef O_CLOEXEC
#define O_CLOEXEC       0x200000
#endif

#ifndef FD_CLOEXEC
#define FD_CLOEXEC      1
#endif
