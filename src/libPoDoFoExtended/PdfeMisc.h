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

#ifndef PDFEMISC_H
#define PDFEMISC_H

#include <sstream>
#include <algorithm>

namespace PoDoFoExtended {

/** Redefine an ostringstream class which behaves with float/double as
 * stated in Pdf reference (no "1e5" notation).
 */
class PdfeOStringStream : public std::ostringstream
{
public:
    explicit PdfeOStringStream ( openmode which = ios_base::out )
        : std::ostringstream( which )
    {
        this->initStream();
    }
    explicit PdfeOStringStream ( const std::string& str, openmode which = ios_base::out )
        : std::ostringstream( str, which )
    {
        this->initStream();
    }

    PdfeOStringStream( const PdfeOStringStream& stream )
        : std::basic_ios<char>(), std::ostringstream( stream.str() )
    {
        this->initStream();
    }
    PdfeOStringStream& operator= ( const PdfeOStringStream& stream )
    {
        this->str( stream.str() );
        this->initStream();
        return *this;
    }

    // Redefine operators to avoid ambiguity.
    PdfeOStringStream& operator<< (bool val)
    {
        std::ostringstream::operator<< ( val );
        return *this;
    }
    PdfeOStringStream& operator<< (short val)
    {
        std::ostringstream::operator<< ( val );
        return *this;
    }
    PdfeOStringStream& operator<< (unsigned short val)
    {
        std::ostringstream::operator<< ( val );
        return *this;
    }
    PdfeOStringStream& operator<< (int val)
    {
        std::ostringstream::operator<< ( val );
        return *this;
    }
    PdfeOStringStream& operator<< (unsigned int val)
    {
        std::ostringstream::operator<< ( val );
        return *this;
    }
    PdfeOStringStream& operator<< (long val)
    {
        std::ostringstream::operator<< ( val );
        return *this;
    }
    PdfeOStringStream& operator<< (unsigned long val)
    {
        std::ostringstream::operator<< ( val );
        return *this;
    }
    PdfeOStringStream& operator<< (std::streambuf* sb)
    {
        std::ostringstream::operator<< ( sb );
        return *this;
    }
    PdfeOStringStream& operator<< (std::ostream& ( *pf )(std::ostream&))
    {
        std::ostringstream::operator<< ( pf );
        return *this;
    }
    PdfeOStringStream& operator<< (std::ios& ( *pf )(std::ios&))
    {
        std::ostringstream::operator<< ( pf );
        return *this;
    }
    PdfeOStringStream& operator<< (std::ios_base& ( *pf )(std::ios_base&))
    {
        std::ostringstream::operator<< ( pf );
        return *this;
    }

    /** Operator<< on float/double, using fixed precision when  notation 'e' appears.
     */
    PdfeOStringStream& operator<< ( float val )
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
    PdfeOStringStream& operator<< ( double val )
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
    PdfeOStringStream& operator<< ( long double val )
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

inline PdfeOStringStream& operator<< (PdfeOStringStream& out, char c )
{
    std::operator<< (out, c);
    return out;
}
inline PdfeOStringStream& operator<< (PdfeOStringStream& out, signed char c )
{
    std::operator<< (out, c);
    return out;
}
inline PdfeOStringStream& operator<< (PdfeOStringStream& out, unsigned char c )
{
    std::operator<< (out, c);
    return out;
}

inline PdfeOStringStream& operator<< (PdfeOStringStream& out, const char* s )
{
    std::operator<< (out, s);
    return out;
}
inline PdfeOStringStream& operator<< (PdfeOStringStream& out, const signed char* s )
{
    std::operator<< (out, s);
    return out;
}
inline PdfeOStringStream& operator<< (PdfeOStringStream& out, const unsigned char* s )
{
    std::operator<< (out, s);
    return out;
}
inline PdfeOStringStream& operator<< (PdfeOStringStream& out, const std::string s )
{
    std::operator<< (out, s);
    return out;
}

}

#endif // PDFEMISC_H
