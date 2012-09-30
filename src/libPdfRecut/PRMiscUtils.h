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

#ifndef PRMISCUTILS_H
#define PRMISCUTILS_H

#include <cstdlib>
#include <functional>

// File defining misc classes that can be useful...
namespace PdfRecut {

/** Delete functional object. Delete the object and set the pointer to NULL.
 * Example : std::for_each( foobar.begin(), fooabr.end(), DeleteFunctor<int>() );
 * See also Boost delete_ptr; C++11 std::default_delete.
 */
template<class T>
struct DeleteFunctor : public std::unary_function<T*,void>
{
    void operator()( T*& ptr ) {
        delete ptr;
        ptr = NULL;
    }
};

}

#endif // PRMISCUTILS_H