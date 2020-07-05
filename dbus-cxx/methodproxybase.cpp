/***************************************************************************
 *   Copyright (C) 2009 by Rick L. Vinyard, Jr.                            *
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

#include "methodproxybase.h"

#include "interfaceproxy.h"

namespace DBus
{

  MethodProxyBase::MethodProxyBase(const std::string& name):
      m_interface(NULL),
      m_name(name)
  {
  }

  MethodProxyBase::MethodProxyBase(const MethodProxyBase& other):
      m_interface(other.m_interface),
      m_name(other.m_name)
  {
  }

  MethodProxyBase::pointer MethodProxyBase::create(const std::string & name)
  {
    return pointer( new MethodProxyBase(name) );
  }

  MethodProxyBase::~MethodProxyBase()
  {
  }

  InterfaceProxy* MethodProxyBase::iface() const
  {
    return m_interface;
  }

  const std::string & MethodProxyBase::name() const
  {
    return m_name;
  }

  void MethodProxyBase::set_name(const std::string & name)
  {
    m_name_mutex.lock();
    std::string old_name = m_name;
    m_name = name;
    m_name_mutex.unlock();
    m_signal_name_changed.emit(old_name, m_name);
  }

  CallMessage::pointer DBus::MethodProxyBase::create_call_message() const
  {
    if ( !m_interface ) return CallMessage::pointer();
    CallMessage::pointer cm = m_interface->create_call_message( m_name );
    cm->set_no_reply(false);
    return cm;
  }

  ReturnMessage::const_pointer DBus::MethodProxyBase::call(CallMessage::const_pointer call_message, int timeout_milliseconds) const
  {
    if ( !m_interface ) return ReturnMessage::const_pointer();
    return m_interface->call(call_message, timeout_milliseconds);
  }

  PendingCall::pointer DBus::MethodProxyBase::call_async(CallMessage::const_pointer call_message, int timeout_milliseconds) const
  {
    if ( !m_interface ) return PendingCall::pointer();
    return m_interface->call_async(call_message, timeout_milliseconds);
  }

  sigc::signal< void, const std::string &, const std::string & > MethodProxyBase::signal_name_changed()
  {
    return m_signal_name_changed;
  }

}

