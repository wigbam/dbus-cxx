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
 * sys/socket.h compatibility shim
 */
#ifndef _WIN32
#include_next <sys/socket.h>
#else
#include <win32netcompat.h>
#endif /* _WIN32 */

#if !defined(SOCK_NONBLOCK) || !defined(SOCK_CLOEXEC)
#define SOCK_CLOEXEC            0x8000  /* set FD_CLOEXEC */
#define SOCK_NONBLOCK           0x4000  /* set O_NONBLOCK */
#ifdef __cplusplus
extern "C" {
#endif
int bsd_socketpair(int domain, int type, int protocol, int socket_vector[2]);
#ifdef __cplusplus
}
#endif
#define socketpair(d,t,p,sv) bsd_socketpair(d,t,p,sv)
#endif /* !defined(SOCK_NONBLOCK) || !defined(SOCK_CLOEXEC) */
