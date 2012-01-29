/***************************************************************************
 *   Copyright (C) 2011 by Paul Balança                                    *
 *   paul.balanca@gmail.com                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef PDFMISC_H
#define PDFMISC_H

#include <sstream>
#include <algorithm>

namespace PdfeBooker {

/** Redefine an ostringstream class which behaves with float/double as
 * stated in Pdf reference (no "1e5" notation).
 */
class PdfOStringStream : public std::ostringstream
{
public:

    explicit PdfOStringStream ( openmode which = ios_base::out )
        : std::ostringstream( which )
    {
        this->initStream();
    }
    explicit PdfOStringStream ( const std::string& str, openmode which = ios_base::out )
        : std::ostringstream( str, which )
    {
        this->initStream();
    }

    PdfOStringStream( const PdfOStringStream& stream )
        : std::basic_ios<char>(), std::ostringstream( stream.str() )
    {
        this->initStream();
    }
    PdfOStringStream& operator= ( const PdfOStringStream& stream )
    {
        this->str( stream.str() );
        this->initStream();
        return *this;
    }

    // Redefine operators to avoid ambiguity.
    PdfOStringStream& operator<< (bool val)
    {
        std::ostringstream::operator<< ( val );
        return *this;
    }
    PdfOStringStream& operator<< (short val)
    {
        std::ostringstream::operator<< ( val );
        return *this;
    }
    PdfOStringStream& operator<< (unsigned short val)
    {
        std::ostringstream::operator<< ( val );
        return *this;
    }
    PdfOStringStream& operator<< (int val)
    {
        std::ostringstream::operator<< ( val );
        return *this;
    }
    PdfOStringStream& operator<< (unsigned int val)
    {
        std::ostringstream::operator<< ( val );
        return *this;
    }
    PdfOStringStream& operator<< (long val)
    {
        std::ostringstream::operator<< ( val );
        return *this;
    }
    PdfOStringStream& operator<< (unsigned long val)
    {
        std::ostringstream::operator<< ( val );
        return *this;
    }
    PdfOStringStream& operator<< (std::streambuf* sb)
    {
        std::ostringstream::operator<< ( sb );
        return *this;
    }
    PdfOStringStream& operator<< (std::ostream& ( *pf )(std::ostream&))
    {
        std::ostringstream::operator<< ( pf );
        return *this;
    }
    PdfOStringStream& operator<< (std::ios& ( *pf )(std::ios&))
    {
        std::ostringstream::operator<< ( pf );
        return *this;
    }
    PdfOStringStream& operator<< (std::ios_base& ( *pf )(std::ios_base&))
    {
        std::ostringstream::operator<< ( pf );
        return *this;
    }

    /** Operator<< on float/double, using fixed precision when  notation 'e' appears.
     */
    PdfOStringStream& operator<< ( float val )
    {
        m_scientificBuf.str("");
        m_scientificBuf << val;
        m_buffer = m_scientificBuf.str();

        if( m_buffer.find_first_of( 'e' ) == std::string::npos ) {
            std::operator<< ( *this, m_buffer );
        }
        else {
            m_fixedBuf.str("");
            m_fixedBuf << val;
            std::operator<< ( *this, m_fixedBuf.str() );
        }
        return *this;
    }
    PdfOStringStream& operator<< ( double val )
    {
        m_scientificBuf.str("");
        m_scientificBuf << val;
        m_buffer = m_scientificBuf.str();

        if( m_buffer.find_first_of( 'e' ) == std::string::npos ) {
            std::operator<< ( *this, m_buffer );
        }
        else {
            m_fixedBuf.str("");
            m_fixedBuf << val;
            std::operator<< ( *this, m_fixedBuf.str() );
        }
        return *this;
    }
    PdfOStringStream& operator<< ( long double val )
    {
        m_scientificBuf.str("");
        m_scientificBuf << val;
        m_buffer = m_scientificBuf.str();

        if( m_buffer.find_first_of( 'e' ) == std::string::npos ) {
            std::operator<< ( *this, m_buffer );
        }
        else {
            m_fixedBuf.str("");
            m_fixedBuf << val;
            std::operator<< ( *this, m_fixedBuf.str() );
        }
        return *this;
    }

private:
    void initStream()
    {
        m_fixedBuf.precision(8);
        m_fixedBuf.setf( std::ios::fixed, std::ios::floatfield );
    }

private:
    std::ostringstream m_scientificBuf, m_fixedBuf;
    std::string m_buffer;
};

inline PdfOStringStream& operator<< (PdfOStringStream& out, char c )
{
    std::operator<< (out, c);
    return out;
}
inline PdfOStringStream& operator<< (PdfOStringStream& out, signed char c )
{
    std::operator<< (out, c);
    return out;
}
inline PdfOStringStream& operator<< (PdfOStringStream& out, unsigned char c )
{
    std::operator<< (out, c);
    return out;
}

inline PdfOStringStream& operator<< (PdfOStringStream& out, const char* s )
{
    std::operator<< (out, s);
    return out;
}
inline PdfOStringStream& operator<< (PdfOStringStream& out, const signed char* s )
{
    std::operator<< (out, s);
    return out;
}
inline PdfOStringStream& operator<< (PdfOStringStream& out, const unsigned char* s )
{
    std::operator<< (out, s);
    return out;
}
inline PdfOStringStream& operator<< (PdfOStringStream& out, const std::string s )
{
    std::operator<< (out, s);
    return out;
}

}

#endif // PDFMISC_H
