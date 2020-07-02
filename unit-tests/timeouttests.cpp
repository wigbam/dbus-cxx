/***************************************************************************
 *   Copyright (C) 2019 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
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
#include <dbus/dbus.h>
#include <dbus-cxx.h>

#include "test_macros.h"

/*
 * FIXME: below are copy-pasted type definitions from dbus-timeout.c since the implementation is hidden.
 * Perhaps could be improved by introducing an extra layer of abstraction in timeout.cpp
 */
typedef dbus_bool_t( *DBusTimeoutHandler ) ( void* data );

struct DBusTimeout
{
  int refcount;                                /**< Reference count */
  int interval;                                /**< Timeout interval in milliseconds. */

  DBusTimeoutHandler handler;                  /**< Timeout handler. */
  void* handler_data;                          /**< Timeout handler data. */
  DBusFreeFunction free_handler_data_function; /**< Free the timeout handler data. */

  void* data;                                    /**< Application data. */
  DBusFreeFunction free_data_function;         /**< Free the application data. */
  unsigned int enabled : 1;                    /**< True if timeout is active. */
  unsigned int needs_restart : 1;              /**< Flag that timeout should be restarted after re-enabling. */
};

DBusTimeout*
_dbus_timeout_new( int interval, DBusTimeoutHandler handler, void* data, DBusFreeFunction free_data_function ){
  DBusTimeout* timeout = new DBusTimeout();
  if ( timeout == NULL )
    return NULL;

  timeout->refcount = 1;
  timeout->interval = interval;

  timeout->handler = handler;
  timeout->handler_data = data;
  timeout->free_handler_data_function = free_data_function;

  timeout->enabled = TRUE;
  timeout->needs_restart = FALSE;

  return timeout;
}

bool callback_received = false;

static dbus_bool_t callback( void* data ) {
  callback_received = true;
  return false;
}

bool timeout_expires() {
  std::unique_ptr<DBusTimeout> p = std::unique_ptr<DBusTimeout>( _dbus_timeout_new( 1, &callback, NULL, NULL ) );
  DBus::Timeout::pointer timeout = DBus::Timeout::create( p.get() );
  timeout->arm( true );
  TEST_ASSERT_RET_FAIL( timeout->is_armed() )

  std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
  TEST_ASSERT_RET_FAIL( callback_received );
  return true;
}

bool timeout_disarm() {
  std::unique_ptr<DBusTimeout> p = std::unique_ptr<DBusTimeout>( _dbus_timeout_new( 50, &callback, NULL, NULL ) );
  DBus::Timeout::pointer timeout = DBus::Timeout::create( p.get() );
  timeout->arm( true );
  TEST_ASSERT_RET_FAIL( timeout->is_armed() )

  timeout->arm( false );

  std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
  TEST_ASSERT_RET_FAIL( !callback_received );
  return true;
}

bool timeout_rearm() {
  std::unique_ptr<DBusTimeout> p = std::unique_ptr<DBusTimeout>( _dbus_timeout_new( 20, &callback, NULL, NULL ) );
  DBus::Timeout::pointer timeout = DBus::Timeout::create( p.get() );
  timeout->arm( true );
  TEST_ASSERT_RET_FAIL( timeout->is_armed() )

  std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
  TEST_ASSERT_RET_FAIL( !callback_received );
  timeout->arm( true );

  std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
  TEST_ASSERT_RET_FAIL( !callback_received );

  std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
  TEST_ASSERT_RET_FAIL( callback_received );
  return true;
}

bool timeout_outofscope() {
  std::unique_ptr<DBusTimeout> p = std::unique_ptr<DBusTimeout>( _dbus_timeout_new( 10, &callback, NULL, NULL ) );
  {
    DBus::Timeout::pointer timeout = DBus::Timeout::create( p.get() );
    timeout->arm( true );
    TEST_ASSERT_RET_FAIL( timeout->is_armed() )
  }

  std::this_thread::sleep_for( std::chrono::milliseconds( 30 ) );
  TEST_ASSERT_RET_FAIL( !callback_received );
  return true;
}

#define ADD_TEST( name ) do{ if( test_name == STRINGIFY( name ) ){ \
  ret = timeout_##name();\
} \
} while( 0 )

int main( int argc, char** argv ) {
  if( argc < 2 ) {
    std::cerr << "USAGE: <test_name>" << std::endl;
    return 1;
  }

  std::string test_name = argv[1];
  bool ret = false;

  ADD_TEST( expires );
  ADD_TEST( disarm );
  ADD_TEST( rearm );
  ADD_TEST( outofscope );

  return !ret;
}
