/***************************************************************************
 *   Copyright (C) 2007,2008,2009,2010 by Rick L. Vinyard, Jr.             *
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
#ifndef DBUSCXX_LOCKS_H
#define DBUSCXX_LOCKS_H

#ifdef _WIN32
#  ifndef HAVE_SHARED_MUTEX
#    if __cplusplus >= 201703L
#      define HAVE_SHARED_MUTEX
#    elif __cplusplus >= 201402L
#      define HAVE_SHARED_TIMED_MUTEX
#    endif
#  endif
#  if defined( HAVE_SHARED_MUTEX ) || defined( HAVE_SHARED_TIMED_MUTEX )
#    include <shared_mutex>
#    define DBUS_CXX_LOCK_SHARED( LOCK ) LOCK.lock_shared()
#    define DBUS_CXX_UNLOCK_SHARED( LOCK ) LOCK.unlock_shared()
#    ifdef HAVE_SHARED_MUTEX
#      define DBUS_CXX_SHARED_LOCK_TYPE std::shared_mutex
#    else
#      define DBUS_CXX_SHARED_LOCK_TYPE std::shared_timed_mutex
#    endif
#  else
#    include <mutex>
#    define DBUS_CXX_SHARED_LOCK_TYPE std::mutex
#    define DBUS_CXX_LOCK_SHARED( LOCK ) LOCK.lock()
#    define DBUS_CXX_UNLOCK_SHARED( LOCK ) LOCK.unlock()
#  endif
#  define DBUS_CXX_SHARED_LOCK_INIT( LOCK ) ((void)0)
#  define DBUS_CXX_SHARED_LOCK_DESTROY( LOCK ) ((void)0)
#  define DBUS_CXX_LOCK_EXCLUSIVE( LOCK ) LOCK.lock()
#  define DBUS_CXX_UNLOCK_EXCLUSIVE( LOCK ) LOCK.unlock()
#else
#  define DBUS_CXX_SHARED_LOCK_TYPE pthread_rwlock_t
#  define DBUS_CXX_SHARED_LOCK_INIT( LOCK ) pthread_rwlock_init( &LOCK, NULL )
#  define DBUS_CXX_SHARED_LOCK_DESTROY( LOCK ) pthread_rwlock_destroy( &LOCK )
#  define DBUS_CXX_LOCK_SHARED( LOCK ) pthread_rwlock_rdlock( &LOCK )
#  define DBUS_CXX_UNLOCK_SHARED( LOCK ) pthread_rwlock_unlock( &LOCK )
#  define DBUS_CXX_LOCK_EXCLUSIVE( LOCK ) pthread_rwlock_wrlock( &LOCK )
#  define DBUS_CXX_UNLOCK_EXCLUSIVE( LOCK ) pthread_rwlock_unlock( &LOCK )
#endif /* _WIN32 */

#endif /* DBUSCXX_LOCKS_H */
