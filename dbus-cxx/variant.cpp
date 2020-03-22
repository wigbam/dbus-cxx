/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
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
#include <dbus-cxx/variant.h>
#include <dbus-cxx/messageiterator.h>

using DBus::Variant;

Variant::Variant() :
    m_currentType( DataType::INVALID )
{}

Variant::Variant( uint8_t byte ) :
    m_currentType( DataType::BYTE ),
    m_signature( DBus::signature( byte ) ),
    m_data( byte )
{}

Variant::Variant( bool b ) :
    m_currentType( DataType::BOOLEAN ),
    m_signature( DBus::signature( b ) ),
    m_data( b )
{}

Variant::Variant( int16_t i ) :
    m_currentType( DataType::INT16 ),
    m_signature( DBus::signature( i ) ),
    m_data( i )
{}

Variant::Variant( uint16_t i ) :
    m_currentType( DataType::UINT16 ),
    m_signature( DBus::signature( i ) ),
    m_data( i )
{}

Variant::Variant( int32_t i ) :
    m_currentType( DataType::INT32 ),
    m_signature( DBus::signature( i ) ),
    m_data( i )
{}

Variant::Variant( uint32_t i ) :
    m_currentType( DataType::UINT32 ),
    m_signature( DBus::signature( i ) ),
    m_data( i )
{}

Variant::Variant( int64_t i ) :
    m_currentType( DataType::INT64 ),
    m_signature( DBus::signature( i ) ),
    m_data( i )
{}

Variant::Variant( uint64_t i ) :
    m_currentType( DataType::UINT64 ),
    m_signature( DBus::signature( i ) ),
    m_data( i )
{}

Variant::Variant( double i ) :
    m_currentType( DataType::DOUBLE ),
    m_signature( DBus::signature( i ) ),
    m_data( i )
{}

Variant::Variant( std::string str ) :
    m_currentType( DataType::STRING ),
    m_signature( DBus::signature( str ) ),
    m_data( str )
{}

Variant::Variant( DBus::Signature sig ) :
    m_currentType( DataType::SIGNATURE ),
    m_signature( DBus::signature( sig ) ),
    m_data( sig )
{}

Variant::Variant( DBus::Path path ) :
    m_currentType( DataType::OBJECT_PATH ),
    m_signature( DBus::signature( path ) ),
    m_data( path )
{}

Variant::Variant( std::shared_ptr<FileDescriptor> fd ) :
    m_currentType( DataType::UNIX_FD ),
    m_signature( DBus::signature( fd ) ),
    m_data( fd )
{}

Variant::Variant( const Variant& other ) :
    m_signature( other.m_signature ),
    m_currentType( other.m_currentType ),
    m_data( other.m_data ) {}

std::string Variant::signature() const {
    return m_signature;
}

DBus::DataType Variant::currentType() const {
    return m_currentType;
}

std::any Variant::value() const {
    return m_data;
}

DBus::Variant Variant::createFromMessage( MessageIterator iter ){
    std::string signature = iter.signature();
    if( signature == DBUS_TYPE_BYTE_AS_STRING ){
        return Variant( (uint8_t)iter );
    }

    return Variant();
}
