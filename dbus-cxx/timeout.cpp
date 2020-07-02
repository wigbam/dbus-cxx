/***************************************************************************
 *   Copyright (C) 2007,2008,2009 by Rick L. Vinyard, Jr.                  *
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
#include "timeout.h"

#include "dbus-cxx-private.h"
#include "error.h"
#include "utility.h"

#ifdef _WIN32
#include <boost/asio.hpp>
#else
#include <sys/time.h>
#include <sys/signal.h>
#endif /* _WIN32 */

namespace DBus
{
  #ifdef _WIN32
  static void timer_callback_proxy( const std::error_code& error, Timeout* timeout ) {
    SIMPLELOGGER_DEBUG( "dbus.Timeout", "timer_callback_proxy" );
    if ( !error && timeout != NULL && timeout->is_valid() ) timeout->handle();
  }

  class Timeout::priv_data {
      using work_guard_type = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
  public:
      priv_data() :
        m_is_armed( false ),
        timer( io_context ),
        work_guard( io_context.get_executor() )
      {
          std::thread([&] {
              io_context.run();
          }).detach();
      }

      ~priv_data()
      {
          work_guard.reset();
      }

      void arm( Timeout* timeout ) {
          timer.expires_after( std::chrono::milliseconds( timeout->interval() ) );
          timer.async_wait( std::bind( &timer_callback_proxy, std::placeholders::_1, timeout ) );
      }

      void cancel() {
          timer.cancel();
      }

      bool m_is_armed;

      boost::asio::io_context io_context;

      boost::asio::steady_timer timer;

      work_guard_type work_guard;
  };
  #else
  static void timer_callback_proxy( sigval_t sv ) {
    SIMPLELOGGER_DEBUG( "dbus.Timeout", "timer_callback_proxy" );
    Timeout* t;
    t = ( Timeout* ) sv.sival_ptr;

    if ( t != NULL and t->is_valid() ) t->handle();
  }

  class Timeout::priv_data {
  public:
      priv_data() :
        m_is_armed(false)
      {
      }

      void cancel() {
          timer_delete( m_timer_id );
      }

      void arm( Timeout* timeout ) {
          if ( not m_is_armed )
          {
            struct sigevent sigevent = {{0},0};

            sigevent.sigev_notify = SIGEV_THREAD;
            sigevent.sigev_value.sival_ptr = timeout;
            sigevent.sigev_notify_function = &timer_callback_proxy;

            timer_create( CLOCK_REALTIME, &sigevent, &m_timer_id);
          }

          int intv = timeout->interval();
          time_t sec;
          long int nsec;
          sec = intv / 1000;
          nsec = (intv % 1000) * 1000000;
          struct itimerspec its = { {sec, nsec}, {sec, nsec} };

          timer_settime( m_timer_id, 0, &its, NULL );
      }

      bool m_is_armed;

      timer_t m_timer_id;
  };
  #endif /* _WIN32 */

  Timeout::Timeout( DBusTimeout* cobj ):
      m_cobj( cobj )
  {
    m_priv = std::make_unique<priv_data>();
    if ( m_cobj ) dbus_timeout_set_data( cobj, this, NULL );
  }

  Timeout::pointer Timeout::create(DBusTimeout * cobj)
  {
    return pointer( new Timeout(cobj) );
  }
    
  Timeout::~Timeout()
  {
    std::unique_lock<std::mutex> lock( m_arming_mutex );
    if ( m_priv->m_is_armed ) m_priv->cancel();
  }

  bool Timeout::is_valid() const
  {
    return m_cobj != NULL;
  }

  Timeout::operator bool() const
  {
    return this->is_valid();
  }

  int Timeout::interval( ) const
  {
    if ( !this->is_valid() ) throw ErrorInvalidCObject::create();
    return dbus_timeout_get_interval( m_cobj );
  }

  bool Timeout::is_enabled( ) const
  {
    if ( !this->is_valid() ) throw ErrorInvalidCObject::create();
    return dbus_timeout_get_enabled( m_cobj );
  }

  bool Timeout::handle( )
  {
    if ( !this->is_valid() ) throw ErrorInvalidCObject::create();
    return dbus_timeout_handle( m_cobj );
  }

  bool Timeout::operator ==(const Timeout & other) const
  {
    return m_cobj == other.m_cobj;
  }

  bool Timeout::operator !=(const Timeout & other) const
  {
    return m_cobj != other.m_cobj;
  }

  void Timeout::arm(bool should_arm)
  {
    std::unique_lock<std::mutex> lock( m_arming_mutex );
    if ( should_arm )
    {
      m_priv->arm( this );
      m_priv->m_is_armed = true;
    }
    else if ( m_priv->m_is_armed )
    {
      m_priv->m_is_armed = false;
      m_priv->cancel();
    }
  }

  bool Timeout::is_armed()
  {
    return m_priv->m_is_armed;
  }

  DBusTimeout* Timeout::cobj( )
  {
    return m_cobj;
  }

  Timeout::operator DBusTimeout*()
  {
    return m_cobj;
  }

}
