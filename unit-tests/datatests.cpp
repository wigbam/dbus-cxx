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
#include <dbus-cxx.h>

#include "test_macros.h"

DBus::Dispatcher::pointer dispatch;
DBus::Connection::pointer conn;

DBus::ObjectProxy::pointer proxy;
DBus::MethodProxy<int,int,int>::pointer int_method_proxy;
DBus::MethodProxy<void>::pointer void_method_proxy;

DBus::Object::pointer object;
DBus::Method<int,int,int>::pointer int_method;
DBus::Method<void>::pointer void_method;

int add(int a, int b){
    return a + b;
}

void void_method_symbol(){}

void client_setup(){
    proxy = conn->create_object_proxy( "dbuscxx.test", "/test" );

    int_method_proxy = proxy->create_method<int,int,int>( "foo.what", "add" );
    void_method_proxy = proxy->create_method<void>( "foo.what", "void" );
}

void server_setup(){
    int ret = conn->request_name( "dbuscxx.test", DBUS_NAME_FLAG_REPLACE_EXISTING );
    if( ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER ) exit(1);

    object = conn->create_object("/test");
    int_method = object->create_method<int,int,int>("foo.what", "add", sigc::ptr_fun( add ) );
    void_method = object->create_method<void>("foo.what", "void", sigc::ptr_fun( void_method_symbol ) );
}

bool data_send_integers(){
    int val = (*int_method_proxy)( 2, 3 );
    
    return TEST_EQUALS( val, 5 );
}

bool data_void_method(){
    (*void_method_proxy)();

    return true;
}

#define ADD_TEST(name) do{ if( test_name == STRINGIFY(name) ){ \
  ret = data_##name();\
} \
} while( 0 )

int main(int argc, char** argv){
  if( argc < 3 ) {
    std::cerr << "USAGE: <server|client> <test_name>" << std::endl;
    return 1;
  }

  std::string test_name = argv[2];
  bool ret = false;
  bool is_client = std::string( argv[1] ) == "client";

  DBus::init();
  dispatch = DBus::Dispatcher::create();
  conn = dispatch->create_connection(DBus::BUS_SESSION);

  if( is_client ){
    client_setup();
    ADD_TEST(send_integers);
    ADD_TEST(void_method);
  }else{
    server_setup();
    ret = true;
    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
  }


  return !ret;
}
