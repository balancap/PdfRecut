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

#include <podofo/base/PdfObject.h>
#include <podofo/base/PdfVecObjects.h>

// File defining misc classes/functions that can be useful...
namespace PoDoFoExtended {

/** Get the indirect reference of a PoDoFo object. I.e. check if the object
 * is a reference, which is resolved in that case.
 * \param pObj Pointer to the object.
 * \return Pointer to the indirect object.
 */
inline const PoDoFo::PdfObject* PdfeIndirectObject( const PoDoFo::PdfObject* pObj,
                                                    const PoDoFo::PdfVecObjects* pOwner = NULL )
{
    if( pObj->IsReference() ) {
        pOwner = ( pObj->GetOwner() ? pObj->GetOwner() : pOwner );
        if( !pOwner ) {
            PODOFO_RAISE_ERROR_INFO( PoDoFo::ePdfError_InvalidHandle,
                                     "Object is a reference but does not have an owner!" );
        }
        while( pObj->IsReference() ) {
            pObj = pOwner->GetObject( pObj->GetReference() );
        }
        return pObj;
    }
    return pObj;
}
inline PoDoFo::PdfObject* PdfeIndirectObject( PoDoFo::PdfObject* pObj,
                                              const PoDoFo::PdfVecObjects* pOwner = NULL )
{
    return const_cast<PoDoFo::PdfObject*>(
                PdfeIndirectObject( const_cast<PoDoFo::PdfObject*>( pObj ), pOwner ) );
}


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
