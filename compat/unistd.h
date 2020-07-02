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
 * unistd.h compatibility shim
 */
#ifndef DBUSCXX_UNISTD_H
#define DBUSCXX_UNISTD_H

#ifndef _MSC_VER
#include_next <unistd.h>
#else

#include <stdlib.h>
#include <io.h>
#include <process.h>

#define STDOUT_FILENO   1
#define STDERR_FILENO   2

#define R_OK    4
#define W_OK    2
#define X_OK    0
#define F_OK    0

#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2

#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef HAVE_PIPE
int pipe(int fildes[2]);
#endif /* HAVE_PIPE */

#ifndef HAVE_PIPE2
int pipe2(int fildes[2], int flags);
#endif /* HAVE_PIPE2 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DBUSCXX_UNISTD_H */
