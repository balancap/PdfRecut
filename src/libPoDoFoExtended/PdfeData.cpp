/***************************************************************************
 * Copyright (C) Paul Balança - All Rights Reserved                        *
 *                                                                         *
 * NOTICE:  All information contained herein is, and remains               *
 * the property of Paul Balança. Dissemination of this information or      *
 * reproduction of this material is strictly forbidden unless prior        *
 * written permission is obtained from Paul Balança.                       *
 *                                                                         *
 * Written by Paul Balança <paul.balanca@gmail.com>, 2012                  *
 ***************************************************************************/

#include "PdfeData.h"

#include <algorithm>

namespace io = boost::iostreams;

namespace PoDoFoExtended {

//**********************************************************//
//                          PdfeData                        //
//**********************************************************//
PdfeData::PdfeData( const char* s )
{
    this->assign( s, s+std::strlen(s) );
}
PdfeData::PdfeData( const char* s, size_t n )
{
    this->assign( s, s+n );
}
PdfeData::PdfeData( const std::string& str )
{
    this->assign( str.begin(), str.end() );
}

//**********************************************************//
//                       PdfeDataDevice                     //
//**********************************************************//
std::streamsize PdfeDataDevice::read( char* s, std::streamsize n )
{
    std::streamsize amt = static_cast<std::streamsize>( m_data.size() - m_pos );
    std::streamsize result = std::min( n, amt );
    if( result != 0 ) {
        std::copy( m_data.begin() + m_pos,
                   m_data.begin() + m_pos + result,
                   s );
        m_pos += result;
        return result;
    } else {
        return -1; // EOF
    }
}
std::streamsize PdfeDataDevice::write( const char* s, std::streamsize n )
{
    size_type nsize = m_pos + n;
    if( nsize > m_data.size() ) {
        m_data.resize( nsize, 0 );
    }
    std::copy( s, s + n, m_data.begin() + m_pos );
    m_pos += n;
    return n;
}
io::stream_offset PdfeDataDevice::seek( io::stream_offset off, std::ios_base::seekdir way )
{
    // Determine new value of m_pos.
    io::stream_offset next;
    if( way == std::ios_base::beg ) {
        next = off;
    }
    else if( way == std::ios_base::cur ) {
        next = m_pos + off;
    }
    else if( way == std::ios_base::end ) {
        next = m_data.size() + off;
    }
    else {
        throw std::ios_base::failure( "<PdfeData> Bad seek direction." );
    }
    // Check for errors.
    if( next < 0 || next > static_cast<io::stream_offset>( m_data.size() ) ) {
        throw std::ios_base::failure( "<PdfeData> Bad seek offset." );
    }
    m_pos = next;
    return m_pos;
}


}
