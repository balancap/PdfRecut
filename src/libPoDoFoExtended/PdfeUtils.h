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

#ifndef PDFEUTILS_H
#define PDFEUTILS_H

#include <cstdlib>
#include <functional>

// File defining misc classes that can be useful...
namespace PoDoFoExtended {

/** Delete functional object. Delete the object and set the pointer to NULL.
 * Example : std::for_each( foobar.begin(), foobar.end(), delete_ptr_fctor<int>() );
 * See also Boost delete_ptr; C++11 std::default_delete.
 */
template<class T>
struct delete_ptr_fctor : public std::unary_function<T*,void>
{
    void operator()( T*& ptr ) {
        delete ptr;
        ptr = NULL;
    }
};

/** Free memory allocated using malloc, and set the pointer to NULL.
 * Example : std::for_each( foobar.begin(), fooabr.end(), free_ptr_fctor<int>() );
 */
template<class T>
struct free_ptr_fctor : public std::unary_function<T*,void>
{
    void operator()( T*& ptr ) {
        free( ptr );
        ptr = NULL;
    }
};

}

#endif // PDFEUTILS_H
