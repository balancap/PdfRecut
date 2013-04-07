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

#ifndef PDFEDATA_H
#define PDFEDATA_H

#include <vector>
#include <string>
#include <cstring>
#include <ostream>

#include <iosfwd>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/stream.hpp>

namespace PoDoFoExtended {

//**********************************************************//
//                          PdfeData                        //
//**********************************************************//
/** Basic class used for storage of binary data.
 * Based on std::vector<char>. A few functionalities
 * are added to the container interface for simplicity of use.
 */
class PdfeData : private std::vector<char>
{
public:
    // Use std::vector typedef definitions.
    using std::vector<char>::value_type;
    using std::vector<char>::allocator_type;
    using std::vector<char>::reference;
    using std::vector<char>::const_reference;
    using std::vector<char>::pointer;
    using std::vector<char>::iterator;
    using std::vector<char>::const_iterator;
    using std::vector<char>::reverse_iterator;
    using std::vector<char>::const_reverse_iterator;
    using std::vector<char>::difference_type;
    using std::vector<char>::size_type;

    // Use std::vector interface.
    using std::vector<char>::begin;
    using std::vector<char>::end;
    using std::vector<char>::rbegin;
    using std::vector<char>::rend;

    using std::vector<char>::size;
    using std::vector<char>::max_size;
    using std::vector<char>::resize;
    using std::vector<char>::capacity;
    using std::vector<char>::empty;
    using std::vector<char>::reserve;

    using std::vector<char>::operator[];
    using std::vector<char>::at;
    using std::vector<char>::front;
    using std::vector<char>::back;

    using std::vector<char>::assign;
    using std::vector<char>::push_back;
    using std::vector<char>::pop_back;
    using std::vector<char>::insert;
    using std::vector<char>::erase;
    using std::vector<char>::clear;
    using std::vector<char>::swap;

public:
    // Constructors definitions.
    PdfeData() :
        std::vector<char>() { }
    PdfeData( size_type n, char val = 0 ) :
        std::vector<char>( n, val ) { }
    template <class InputIterator>
    PdfeData( InputIterator first, InputIterator last ) :
        std::vector<char>( first, last ) { }
    PdfeData( const PdfeData& rhs ) :
        std::vector<char>( rhs ) { }

    PdfeData( const char* s );
    PdfeData( const char* s, size_t n );
    PdfeData( const std::string& str );

    // Operator=.
    PdfeData& operator=( const PdfeData& data ) {
        this->std::vector<char>::operator=( data );
        return *this;
    }
    PdfeData& operator=( const char* s ) {
        this->assign( s, s+std::strlen(s) );
        return *this;
    }
    PdfeData& operator=( const std::string& str ) {
        this->assign( str.begin(), str.end() );
        return *this;
    }

    // Operator<<, used to append data.
    PdfeData& operator<<( const PdfeData& data ) {
        this->insert( this->end(), data.begin(), data.end() );
        return *this;
    }
    PdfeData& operator<<( const std::string& str ) {
        this->insert( this->end(), str.begin(), str.end() );
        return *this;
    }
    PdfeData& operator<<( const char* s ) {
        this->insert( this->end(), s, s+std::strlen(s) );
        return *this;
    }

public:
    // A few supplementary member functions.
    const char* data() const {
        return &this->operator[]( 0 );
    }
    char* data() {
        return &this->operator[]( 0 );
    }
    // Convert into string.
    std::string to_string() const {
        return std::string( this->data(), this->size() );
    }
    void to_string( std::string& str ) const {
        str.assign( this->data(), this->size() );
    }
};

inline std::ostream& operator<<( std::ostream& os, const PdfeData& data ) {
    os.write( data.data(), data.size() );
    return os;
}
inline std::string to_string( const PdfeData& data ) {
    return data.to_string();
}

//**********************************************************//
//                       PdfeDataDevice                     //
//**********************************************************//
/** Implement the idiom Device described in boost::iostreams.
 * It describes a SeekableDevice based on PdfeData object.
 * Mainly inspired by the example given in the documentation.
 */
class PdfeDataDevice
{
public:
    // Boost::iostreams device.
    typedef PdfeData::value_type    char_type;
    typedef boost::iostreams::seekable_device_tag     category;

    /** Default constructor.
     * \param data Reference to the data buffer used by the device.
     */
    PdfeDataDevice( PdfeData& data ) :
        m_data( data ), m_pos( 0 ) { }

public:
    // Seekable device interface.
    /** Read in the data at the current position.
     * \param s Buffer where to store the data read.
     * \param n Number of bytes to read.
     * \return Number of bytes read (-1 if EOF).
     */
    std::streamsize read( char* s, std::streamsize n );
    /** Write data at the current position.
     * \param s Data to write down.
     * \param n Number of bytes to write.
     * \return Number of bytes written.
     */
    std::streamsize write( const char* s, std::streamsize n );
    /** Seek to position off.
     * \param off Position.
     * \param way Interpretation of off:
     *    - std::ios_base::beg indicates an offset from the
     *     sequence beginning;
     *    - std::ios_base::cur indicates an offset from the
     *      current character position;
     *    - std::ios_base::end indicates an offset from the
     *      sequence end.
     */
    boost::iostreams::stream_offset seek( boost::iostreams::stream_offset off,
                                          std::ios_base::seekdir way );

private:
    typedef PdfeData::size_type   size_type;
    /// Associated data object.
    PdfeData&  m_data;
    /// Position in the io data device.
    size_type  m_pos;
};

//**********************************************************//
//                       PdfeBoostStream                    //
//**********************************************************//
/** Sub-class of boost stream, specialized to handle
 * PDF contents (i.e. fixed locale and fixed notations).
 */
template< typename Device>
class PdfeBoostStream : public boost::iostreams::stream<Device>
{
public:
    /// Default constructor.
    PdfeBoostStream() :
        boost::iostreams::stream<Device>() {
        this->init();
    }
    /// Construct from a device.
    PdfeBoostStream( const Device& device ) :
        boost::iostreams::stream<Device>( device ) {
        this->init();
    }
private:
    /// Initialize stream parameters.
    void init();
};

template< typename Device>
void PdfeBoostStream<Device>::init()
{
    // Float precision.
    this->setf( std::ios::fixed, std:: ios::floatfield );
    this->precision( 8 );
    // Set locale.
    static const std::locale pdflocale( "C" );
    this->imbue( pdflocale );

//    try {
//        s.imbue( cachedLocale );
//    } catch (const std::runtime_error & e) {
//        std::ostringstream s;
//        s << "Failed to set safe locale on stream being used for PDF I/O.";
//        s << "Locale set was: \"" << PdfIOLocale << "\".";
//        s << "Error reported by STL std::locale: \"" << e.what() << "\"";
//        // The info string is copied by PdfError so we're ok to just:
//        PODOFO_RAISE_ERROR_INFO(
//                    ePdfError_InvalidDeviceOperation,
//                    s.str().c_str()
//                    );
//    }
}

/// Stream specialization with PdfeData.
typedef PdfeBoostStream<PdfeDataDevice>  PdfeDataStream;

}

#endif // PDFEDATA_H
