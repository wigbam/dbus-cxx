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
#include "objectproxy.h"

#include "connection.h"
#include "dbus-cxx-private.h"
#include "interface.h"
#include "locks.h"

#include <map>
#include <dbus/dbus.h>

namespace DBus
{
  class ObjectProxy::priv_data {
  public:
      priv_data() {
          DBUS_CXX_SHARED_LOCK_INIT( m_interfaces_rwlock );
      }

      ~priv_data() {
          DBUS_CXX_SHARED_LOCK_DESTROY( m_interfaces_rwlock );
      }

      mutable DBUS_CXX_SHARED_LOCK_TYPE m_interfaces_rwlock;
  };

  ObjectProxy::ObjectProxy( Connection::pointer conn, const std::string& destination, const std::string& path ):
      m_connection(conn),
      m_destination(destination),
      m_path(path)
  {
    m_priv = std::make_unique<priv_data>();
  }

  ObjectProxy::pointer ObjectProxy::create( const std::string& path )
  {
    return pointer( new ObjectProxy( Connection::pointer(), "", path ) );
  }

  ObjectProxy::pointer ObjectProxy::create( const std::string& destination, const std::string& path )
  {
    return pointer( new ObjectProxy( Connection::pointer(), destination, path ) );
  }

  ObjectProxy::pointer ObjectProxy::create( Connection::pointer conn, const std::string& path )
  {
    return pointer( new ObjectProxy( conn, "", path ) );
  }

  ObjectProxy::pointer ObjectProxy::create( Connection::pointer conn, const std::string& destination, const std::string& path )
  {
    return pointer( new ObjectProxy( conn, destination, path ) );
  }

  ObjectProxy::~ ObjectProxy( )
  {
  }

  Connection::pointer ObjectProxy::connection() const
  {
    return m_connection;
  }

  void ObjectProxy::set_connection( Connection::pointer conn )
  {
    m_connection = conn;
    for ( Interfaces::iterator i = m_interfaces.begin(); i != m_interfaces.end(); i++ )
    {
      i->second->on_object_set_connection( conn );
    }
  }

  const std::string& ObjectProxy::destination() const
  {
    return m_destination;
  }

  void ObjectProxy::set_destination( const std::string& destination )
  {
    m_destination = destination;
  }

  const Path& ObjectProxy::path() const
  {
    return m_path;
  }

  void ObjectProxy::set_path( const std::string& path )
  {
    m_path = path;
    for ( Interfaces::iterator i = m_interfaces.begin(); i != m_interfaces.end(); i++ )
    {
      i->second->on_object_set_path( path );
    }
  }

  const ObjectProxy::Interfaces & ObjectProxy::interfaces() const
  {
    return m_interfaces;
  }

  InterfaceProxy::pointer ObjectProxy::iface( const std::string & name ) const
  {
    Interfaces::const_iterator iter;

    // ========== READ LOCK ==========
    DBUS_CXX_LOCK_SHARED( m_priv->m_interfaces_rwlock );

    iter = m_interfaces.find( name );

    // ========== UNLOCK ==========
    DBUS_CXX_UNLOCK_SHARED( m_priv->m_interfaces_rwlock );

    if ( iter == m_interfaces.end() ) return InterfaceProxy::pointer();

    return iter->second;
  }

  InterfaceProxy::pointer ObjectProxy::operator[]( const std::string& name ) const
  {
    return this->iface(name);
  }

  bool ObjectProxy::add_interface( InterfaceProxy::pointer iface )
  {
    bool result = true;

    if ( !iface ) return false;

    if ( iface->m_object ) iface->m_object->remove_interface( iface );
    
    // ========== WRITE LOCK ==========
    DBUS_CXX_LOCK_EXCLUSIVE( m_priv->m_interfaces_rwlock );

    InterfaceSignalNameConnections::iterator i;

    i = m_interface_signal_name_connections.find(iface);

    if ( i == m_interface_signal_name_connections.end() )
    {
      m_interface_signal_name_connections[iface] = iface->signal_name_changed().connect( sigc::bind(sigc::mem_fun(*this, &ObjectProxy::on_interface_name_changed), iface));

      m_interfaces.insert(std::make_pair(iface->name(), iface));

      iface->m_object = this;
    }
    else
    {
      result = false;
    }

    // ========== UNLOCK ==========
    DBUS_CXX_UNLOCK_EXCLUSIVE( m_priv->m_interfaces_rwlock );

    m_signal_interface_added.emit( iface );

    // TODO allow control over this
    if ( !m_default_interface ) this->set_default_interface( iface->name() );

    return result;
  }

  InterfaceProxy::pointer ObjectProxy::create_interface(const std::string & name)
  {
    InterfaceProxy::pointer iface;

    iface = InterfaceProxy::create(name);

    if ( this->add_interface(iface) ) return iface;

    return InterfaceProxy::pointer();
  }

  void ObjectProxy::remove_interface( const std::string & name )
  {
    Interfaces::iterator iter;
    InterfaceProxy::pointer iface, old_default;
    InterfaceSignalNameConnections::iterator i;
    
    bool need_emit_default_changed = false;

    // ========== WRITE LOCK ==========
    DBUS_CXX_LOCK_EXCLUSIVE( m_priv->m_interfaces_rwlock );
    
    iter = m_interfaces.find( name );
    if ( iter != m_interfaces.end() )
    {
      iface = iter->second;
      m_interfaces.erase(iter);
    }

    if ( iface )
    {
      i = m_interface_signal_name_connections.find(iface);
      if ( i != m_interface_signal_name_connections.end() )
      {
        i->second.disconnect();
        m_interface_signal_name_connections.erase(i);
      }
    
      if ( m_default_interface == iface ) {
        old_default = m_default_interface;
        m_default_interface = InterfaceProxy::pointer();
        need_emit_default_changed = true;
      }

    }

    // ========== UNLOCK ==========
    DBUS_CXX_UNLOCK_EXCLUSIVE( m_priv->m_interfaces_rwlock );

    if ( iface ) m_signal_interface_removed.emit( iface );

    if ( need_emit_default_changed ) m_signal_default_interface_changed.emit( old_default, m_default_interface );
  }

  void ObjectProxy::remove_interface( InterfaceProxy::pointer iface )
  {
    Interfaces::iterator current, upper;
    InterfaceProxy::pointer old_default;
    InterfaceSignalNameConnections::iterator i;
    
    bool need_emit_default_changed = false;
    bool interface_removed = false;

    if ( !iface ) return;

    // ========== WRITE LOCK ==========
    DBUS_CXX_LOCK_EXCLUSIVE( m_priv->m_interfaces_rwlock );

    current = m_interfaces.lower_bound( iface->name() );
    upper = m_interfaces.upper_bound( iface->name() );

    for ( ; current != upper; current++ )
    {
      if ( current->second == iface )
      {
        i = m_interface_signal_name_connections.find(iface);
        if ( i != m_interface_signal_name_connections.end() )
        {
          i->second.disconnect();
          m_interface_signal_name_connections.erase(i);
        }
    
        if ( m_default_interface == iface )
        {
          old_default = m_default_interface;
          m_default_interface = InterfaceProxy::pointer();
          need_emit_default_changed = true;
        }

        iface->m_object = NULL;
        m_interfaces.erase(current);

        interface_removed = true;
        
        break;
      }
    }

    // ========== UNLOCK ==========
    DBUS_CXX_UNLOCK_EXCLUSIVE( m_priv->m_interfaces_rwlock );

    if ( interface_removed ) m_signal_interface_removed.emit( iface );

    if ( need_emit_default_changed ) m_signal_default_interface_changed.emit( old_default, m_default_interface );
  }

  bool ObjectProxy::has_interface( const std::string & name ) const
  {
    Interfaces::const_iterator i;
    
    // ========== READ LOCK ==========
    DBUS_CXX_LOCK_SHARED( m_priv->m_interfaces_rwlock );

    i = m_interfaces.find( name );

    // ========== UNLOCK ==========
    DBUS_CXX_UNLOCK_SHARED( m_priv->m_interfaces_rwlock );

    return ( i != m_interfaces.end() );
  }

  bool ObjectProxy::has_interface( InterfaceProxy::pointer iface ) const
  {
    if ( !iface ) return false;
    
    Interfaces::const_iterator current, upper;
    bool result = false;

    // ========== READ LOCK ==========
    DBUS_CXX_LOCK_SHARED( m_priv->m_interfaces_rwlock );

    current = m_interfaces.lower_bound(iface->name());

    if ( current != m_interfaces.end() )
    {
      upper = m_interfaces.upper_bound(iface->name());
      for ( ; current != upper; current++)
      {
        if ( current->second == iface )
        {
          result = true;
          break;
        }
      }
    }

    // ========== UNLOCK ==========
    DBUS_CXX_UNLOCK_SHARED( m_priv->m_interfaces_rwlock );

    return result;
  }

  InterfaceProxy::pointer ObjectProxy::default_interface() const
  {
    return m_default_interface;
  }

  bool ObjectProxy::set_default_interface( const std::string& new_default_name )
  {
    Interfaces::iterator iter;
    InterfaceProxy::pointer old_default;
    bool result = false;

    // ========== READ LOCK ==========
    DBUS_CXX_LOCK_SHARED( m_priv->m_interfaces_rwlock );

    iter = m_interfaces.find( new_default_name );

    if ( iter != m_interfaces.end() )
    {
      result = true;
      old_default = m_default_interface;
      m_default_interface = iter->second;
    }
    
    // ========== UNLOCK ==========
    DBUS_CXX_UNLOCK_SHARED( m_priv->m_interfaces_rwlock );

    if ( result ) m_signal_default_interface_changed.emit( old_default, m_default_interface );

    return result;
  }

  bool ObjectProxy::set_default_interface( InterfaceProxy::pointer iface )
  {
    Interfaces::iterator iter;
    InterfaceProxy::pointer old_default;

    if ( !iface ) return false;

    if ( !this->has_interface(iface) ) this->add_interface(iface);

    old_default = m_default_interface;
    m_default_interface = iface;

    m_signal_default_interface_changed.emit( old_default, m_default_interface );

    return true;
  }

  void ObjectProxy::remove_default_interface()
  {
    if ( !m_default_interface ) return;

    InterfaceProxy::pointer old_default = m_default_interface;
    m_default_interface = InterfaceProxy::pointer();
    m_signal_default_interface_changed.emit( old_default, m_default_interface );
  }

  bool ObjectProxy::add_method( const std::string& ifacename, MethodProxyBase::pointer method )
  {
    if ( !method ) return false;
    
    InterfaceProxy::pointer iface = this->iface(ifacename);

    if ( !iface ) iface = this->create_interface(ifacename);

    return iface->add_method( method );
  }

  bool ObjectProxy::add_method( MethodProxyBase::pointer method )
  {
    if ( !method ) return false;
    
    InterfaceProxy::pointer iface = m_default_interface;
    
    if ( !iface )
    {
      iface = this->iface("");
      if ( !iface ) iface = this->create_interface("");
      if ( !m_default_interface ) this->set_default_interface(iface);
    }

    return iface->add_method(method);
  }

  CallMessage::pointer ObjectProxy::create_call_message( const std::string& interface_name, const std::string& method_name ) const
  {
    CallMessage::pointer call_message;
    
    if ( m_destination.empty() )
    {
      call_message = CallMessage::create( m_path, interface_name, method_name );
    }
    else
    {
      call_message = CallMessage::create( m_destination, m_path, interface_name, method_name );
    }

    return call_message;
  }

  CallMessage::pointer ObjectProxy::create_call_message( const std::string& method_name ) const
  {
    CallMessage::pointer call_message;
    
    if ( m_destination.empty() )
    {
      call_message = CallMessage::create( m_path, method_name );
    }
    else
    {
      call_message = CallMessage::create( m_destination, m_path, "", method_name );
    }

    return call_message;
  }

  ReturnMessage::const_pointer ObjectProxy::call( CallMessage::const_pointer call_message, int timeout_milliseconds ) const
  {
    if ( !m_connection || !m_connection->is_valid() ) return ReturnMessage::const_pointer();

//     if ( not call_message->expects_reply() )
//     {
//       m_connection->send( call_message );
//       return ReturnMessage::const_pointer();
//     }

    return m_connection->send_with_reply_blocking( call_message, timeout_milliseconds );
  }

  PendingCall::pointer ObjectProxy::call_async( CallMessage::const_pointer call_message, int timeout_milliseconds ) const
  {
    if ( !m_connection || !m_connection->is_valid() ) return PendingCall::pointer();

    return m_connection->send_with_reply_async( call_message, timeout_milliseconds );
  }

  sigc::signal< void, InterfaceProxy::pointer > ObjectProxy::signal_interface_added()
  {
    return m_signal_interface_added;
  }

  sigc::signal< void, InterfaceProxy::pointer > ObjectProxy::signal_interface_removed()
  {
    return m_signal_interface_removed;
  }

  sigc::signal< void, InterfaceProxy::pointer, InterfaceProxy::pointer > ObjectProxy::signal_default_interface_changed()
  {
    return m_signal_default_interface_changed;
  }

  void ObjectProxy::on_interface_name_changed(const std::string & oldname, const std::string & newname, InterfaceProxy::pointer iface)
  {
  
    // ========== WRITE LOCK ==========
      DBUS_CXX_LOCK_EXCLUSIVE( m_priv->m_interfaces_rwlock );

    Interfaces::iterator current, upper;
    current = m_interfaces.lower_bound(oldname);

    if ( current != m_interfaces.end() )
    {
      upper = m_interfaces.upper_bound(oldname);

      for ( ; current != upper; current++ )
      {
        if ( current->second == iface )
        {
          m_interfaces.erase(current);
          break;
        }
      }
    }

    m_interfaces.insert( std::make_pair(newname, iface) );

    InterfaceSignalNameConnections::iterator i;
    i = m_interface_signal_name_connections.find(iface);
    if ( i == m_interface_signal_name_connections.end() )
    {
      m_interface_signal_name_connections[iface] =
          iface->signal_name_changed().connect(sigc::bind(sigc::mem_fun(*this,&ObjectProxy::on_interface_name_changed),iface));
    }
    
    // ========== UNLOCK ==========
    DBUS_CXX_UNLOCK_EXCLUSIVE( m_priv->m_interfaces_rwlock );
  }

}



