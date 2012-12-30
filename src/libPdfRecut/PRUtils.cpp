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

#include "PRUtils.h"

namespace PdfRecut {

bool QStringIsLettersNumbers( const QString& ustr )
{
    if( ustr.isEmpty() ) {
        return false;
    }
    for( int i = 0 ; i < ustr.length() ; ++i ) {
        if( !ustr.at( i ).isLetterOrNumber() ) {
            return false;
        }
    }
    return true;
}

}
