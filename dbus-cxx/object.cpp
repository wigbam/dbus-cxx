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
#include "object.h"

#include "dbus-cxx-private.h"
#include "locks.h"
#include "utility.h"

#include <map>
#include <sstream>
#include <cstring>
#include <dbus/dbus.h>

namespace DBus
{
  class Object::priv_data {
  public:
      priv_data() {
          DBUS_CXX_SHARED_LOCK_INIT( m_interfaces_rwlock );
      }

      ~priv_data() {
          DBUS_CXX_SHARED_LOCK_DESTROY( m_interfaces_rwlock );
      }

      mutable DBUS_CXX_SHARED_LOCK_TYPE m_interfaces_rwlock;
  };

  Object::Object( const std::string& path, PrimaryFallback pf ):
      ObjectPathHandler( path, pf )
  {
    m_priv = std::make_unique<priv_data>();
  }

  Object::~ Object( )
  {
  }

  Object::pointer Object::create( const std::string& path, PrimaryFallback pf )
  {
    return pointer( new Object( path, pf ) );
  }

  bool Object::register_with_connection(Connection::pointer conn)
  {
    SIMPLELOGGER_DEBUG("dbus.Object","Object::register_with_connection");
    if ( !ObjectPathHandler::register_with_connection(conn) ) return false;

    for (Interfaces::iterator i = m_interfaces.begin(); i != m_interfaces.end(); i++)
      i->second->set_connection(conn);

    for (Children::iterator c = m_children.begin(); c != m_children.end(); c++)
      c->second->register_with_connection(conn);

    return true;
  }

  const Object::Interfaces & Object::interfaces() const
  {
    return m_interfaces;
  }

  Interface::pointer Object::iface( const std::string & name ) const
  {
    Interfaces::const_iterator iter;

    // ========== READ LOCK ==========
    DBUS_CXX_LOCK_SHARED( m_priv->m_interfaces_rwlock );

    iter = m_interfaces.find( name );

    // ========== UNLOCK ==========
    DBUS_CXX_UNLOCK_SHARED( m_priv->m_interfaces_rwlock );

    if ( iter == m_interfaces.end() ) return Interface::pointer();

    return iter->second;
  }

  bool Object::add_interface( Interface::pointer iface )
  {
    bool result = true;

    if ( !iface ) return false;
    
    SIMPLELOGGER_DEBUG("dbus.Object","Object::add_interface " << iface->name() );

    // ========== WRITE LOCK ==========
    DBUS_CXX_LOCK_EXCLUSIVE( m_priv->m_interfaces_rwlock );

    InterfaceSignalNameConnections::iterator i;

    i = m_interface_signal_name_connections.find(iface);

    if ( i == m_interface_signal_name_connections.end() )
    {
      m_interface_signal_name_connections[iface] = iface->signal_name_changed().connect( sigc::bind(sigc::mem_fun(*this, &Object::on_interface_name_changed), iface));

      m_interfaces.insert(std::make_pair(iface->name(), iface));

      iface->set_object(this);
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

    SIMPLELOGGER_DEBUG("dbus.Object","Object::add_interface " << iface->name() << " successful: " << result);

    return result;
  }

  Interface::pointer Object::create_interface(const std::string & name)
  {
    Interface::pointer iface;

    iface = Interface::create(name);

    if ( this->add_interface(iface) ) return iface;

    return Interface::pointer();
  }

  void Object::remove_interface( const std::string & name )
  {
    Interfaces::iterator iter;
    Interface::pointer iface, old_default;
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
        iface->set_object(NULL);
      }
    
      if ( m_default_interface == iface ) {
        old_default = m_default_interface;
        m_default_interface = Interface::pointer();
        need_emit_default_changed = true;
      }

    }

    // ========== UNLOCK ==========
    DBUS_CXX_UNLOCK_EXCLUSIVE( m_priv->m_interfaces_rwlock );

    if ( iface ) m_signal_interface_removed.emit( iface );

    if ( need_emit_default_changed ) m_signal_default_interface_changed.emit( old_default, m_default_interface );
  }

  bool Object::has_interface( const std::string & name )
  {
    Interfaces::const_iterator i;
    
    // ========== READ LOCK ==========
    DBUS_CXX_LOCK_SHARED( m_priv->m_interfaces_rwlock );

    i = m_interfaces.find( name );

    // ========== UNLOCK ==========
    DBUS_CXX_UNLOCK_SHARED( m_priv->m_interfaces_rwlock );

    return ( i != m_interfaces.end() );
  }

  Interface::pointer Object::default_interface() const
  {
    return m_default_interface;
  }

  bool Object::set_default_interface( const std::string& new_default_name )
  {
    Interfaces::iterator iter;
    Interface::pointer old_default;
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

  void Object::remove_default_interface()
  {
    if ( !m_default_interface ) return;

    Interface::pointer old_default = m_default_interface;
    m_default_interface = Interface::pointer();
    m_signal_default_interface_changed.emit( old_default, m_default_interface );
  }

  const Object::Children& Object::children() const
  {
    return m_children;
  }

  Object::pointer Object::child(const std::string& name) const
  {
    Children::const_iterator i = m_children.find(name);
    if ( i == m_children.end() ) return Object::pointer();
    return i->second;
  }

  bool Object::add_child(const std::string& name, Object::pointer child, bool force)
  {
    if ( !child ) return false;
    if ( !force && this->has_child(name) ) return false;
    m_children[name] = child;
    if ( m_connection ) child->register_with_connection(m_connection);
    return true;
  }

  bool Object::remove_child(const std::string& name)
  {
    Children::iterator i = m_children.find(name);
    if ( i == m_children.end() ) return false;
    m_children.erase(i);
    return true;
  }

  bool Object::has_child(const std::string& name) const
  {
    return m_children.find(name) != m_children.end();
  }

  std::string Object::introspect(int space_depth) const
  {
    std::ostringstream sout;
    std::string spaces;
    Interfaces::const_iterator i;
    Children::const_iterator c;
    for (int i=0; i < space_depth; i++ ) spaces += " ";
    sout << spaces << "<node name=\"" << this->path() << "\">\n"
         << spaces << "  <interface name=\"" << DBUS_CXX_INTROSPECTABLE_INTERFACE << "\">\n"
         << spaces << "    <method name=\"Introspect\">\n"
         << spaces << "      <arg name=\"data\" type=\"s\" direction=\"out\"/>\n"
         << spaces << "    </method>\n"
         << spaces << "  </interface>\n";
    for ( i = m_interfaces.begin(); i != m_interfaces.end(); i++ )
      sout << i->second->introspect(space_depth+2);
    for ( c = m_children.begin(); c != m_children.end(); c++ )
      sout << spaces << "  <node name=\"" << c->first << "\"/>\n";
    sout << spaces << "</node>\n";
    return sout.str();
  }

  sigc::signal< void, Interface::pointer > Object::signal_interface_added()
  {
    return m_signal_interface_added;
  }

  sigc::signal< void, Interface::pointer > Object::signal_interface_removed()
  {
    return m_signal_interface_removed;
  }

  sigc::signal< void, Interface::pointer, Interface::pointer > Object::signal_default_interface_changed()
  {
    return m_signal_default_interface_changed;
  }

  HandlerResult Object::handle_message( Connection::pointer connection , Message::const_pointer message )
  {
    Interfaces::iterator current, upper;
    Interface::pointer iface;
    HandlerResult result = NOT_HANDLED;

    SIMPLELOGGER_DEBUG("dbus.Object","Object::handle_message: before call message test");

    CallMessage::const_pointer callmessage;
    try{
      callmessage = CallMessage::create( message );
    }catch(DBusCxxPointer<DBus::ErrorInvalidMessageType> err){
      return NOT_HANDLED;
    }

    if ( !callmessage ) return NOT_HANDLED;

    SIMPLELOGGER_DEBUG("dbus.Object","Object::handle_message: message is good (it's a call message) for interface '" << callmessage->interface_name() << "'");

    if ( callmessage->interface_name() == NULL ){
        //If for some reason the message that we are getting has a NULL interface, we will segfault.
        return NOT_HANDLED;
    }

    // Handle the introspection interface
    if ( strcmp(callmessage->interface_name(), DBUS_CXX_INTROSPECTABLE_INTERFACE) == 0 )
    {
      SIMPLELOGGER_DEBUG("dbus.Object","Object::handle_message: introspection interface called");
      ReturnMessage::pointer return_message = callmessage->create_reply();
      std::string introspection = DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE;
      introspection += this->introspect();
      *return_message << introspection;
      connection << return_message;
      return HANDLED;
    }

    // ========== READ LOCK ==========
    DBUS_CXX_LOCK_SHARED( m_priv->m_interfaces_rwlock );

    current = m_interfaces.lower_bound( callmessage->interface_name() );

    // Do we have an interface or do we need to use the default???
    if ( current == m_interfaces.end() )
    {
      SIMPLELOGGER_DEBUG("dbus.Object","Object::handle_message: trying to handle with the default interface");
      // Do we have have a default to use, if so use it to try and handle the message
      if ( m_default_interface )
        result = m_default_interface->handle_call_message(connection, callmessage);
    }
    else
    {
      // Set up the upper limit of interfaces
      upper = m_interfaces.upper_bound( callmessage->interface_name() );


      SIMPLELOGGER_DEBUG("dbus.Object","Object::handle_message: before handling lower bound interface is " << current->second->name() );
      SIMPLELOGGER_DEBUG("dbus.Object","Object::handle_message: before handling upper bound interface is " << (upper == m_interfaces.end() ? "" : upper->second->name()));

      // Iterate through each interface with a matching name
      for ( ; current != upper; current++ )
      {
        SIMPLELOGGER_DEBUG("dbus.Object","Object::handle_message: trying to handle with interface " << current->second->name() );
        // If an interface handled the message unlock and return
        if ( current->second->handle_call_message(connection, callmessage) == HANDLED )
        {
          result = HANDLED;
          break;
        }
      }
    }

    if (result == NOT_HANDLED && m_default_interface)
      result = m_default_interface->handle_call_message(connection, callmessage);

    // ========== UNLOCK ==========
    DBUS_CXX_UNLOCK_SHARED( m_priv->m_interfaces_rwlock );

    SIMPLELOGGER_DEBUG("dbus.Object","Object::handle_message: message was " << ((result==HANDLED)?"handled":"not handled"));

    return result;
  }

  void Object::on_interface_name_changed(const std::string & oldname, const std::string & newname, Interface::pointer iface)
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
          iface->signal_name_changed().connect(sigc::bind(sigc::mem_fun(*this,&Object::on_interface_name_changed),iface));
    }
    
    // ========== UNLOCK ==========
    DBUS_CXX_UNLOCK_EXCLUSIVE( m_priv->m_interfaces_rwlock );
  }


}

