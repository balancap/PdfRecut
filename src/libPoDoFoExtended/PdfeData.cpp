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

namespace PoDoFoExtended {

PdfeData::PdfeData( const char* s, size_t n )
{
    std::copy( s, s+n, this->begin() );
}
PdfeData::PdfeData( const std::string& str )
{
    std::copy( str.begin(), str.end(), this->begin() );
}




}
