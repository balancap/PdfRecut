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
#include <ostream>

namespace PoDoFoExtended {

/** Basic class used for storage of binary data.
 * Based on std::vector<char>.
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
    using std::vector<char>::operator=;

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

    PdfeData( const char* s, size_t n );
    PdfeData( const std::string& str );

public:
    // A few supplementary member functions.
    const char* data() const {
        return &this->operator[]( 0 );
    }
    char* data() {
        return &this->operator[]( 0 );
    }
};

inline std::ostream& operator<<( std::ostream& os, const PdfeData& data ) {
    os.write( data.data(), data.size() );
    return os;
}

}

#endif // PDFEDATA_H
