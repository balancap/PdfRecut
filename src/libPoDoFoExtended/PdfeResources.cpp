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

#include "PdfeResources.h"
#include <podofo/podofo.h>

using namespace PoDoFo;

namespace PoDoFoExtended {

const char* PdfeResources::CTypes[] = { "ExtGState",
                                       "ColorSpace",
                                       "Pattern",
                                       "Shading",
                                       "XObject",
                                       "Font",
                                       "ProcSet",
                                       "Properties" };

PdfeResources::PdfeResources()
{
    // Nothing to do!
}
PdfeResources::PdfeResources(const PdfeResources& rhs ) :
    m_resources( rhs.m_resources )
{
}
PdfeResources& PdfeResources::operator=( const PdfeResources& rhs )
{
    m_resources = rhs.m_resources;
    return *this;
}

void PdfeResources::pushBack( PoDoFo::PdfObject* resourcesObj )
{
    // Null object!
    if( !resourcesObj ) {
        return;
    }

    // Check it is a dictionary.
    if( resourcesObj->IsDictionary() ) {
        m_resources.push_back( resourcesObj );
    }
    else {
        // TODO: raise exception...
    }
}
const std::vector<PoDoFo::PdfObject*>& PdfeResources::resources() const
{
    return m_resources;
}

void PdfeResources::addKey( PdfeResourcesType::Enum resource, const PoDoFo::PdfName& key, const PoDoFo::PdfObject* object )
{
    // Empty resources object...
    if( m_resources.empty() ) {
        return;
    }

    // Get resources dict. Create it, if necessary.
    if( !m_resources.back()->GetDictionary().HasKey( CTypes[resource] ) ) {
        m_resources.back()->GetDictionary().AddKey( CTypes[resource], PdfDictionary() );
    }
    PdfObject* resDict = m_resources.back()->GetIndirectKey( CTypes[resource] );

    // Add key.
    resDict->GetDictionary().AddKey( key, object );
}

PoDoFo::PdfObject* PdfeResources::getKey( PdfeResourcesType::Enum resource, const PoDoFo::PdfName& key ) const
{
    PdfObject* keyObj = NULL;

    // Look in every resources object.
    for( int i = m_resources.size()-1 ; i >= 0 ; --i )
    {
        // Get resources dict.
        PdfObject* resDict = m_resources[i]->GetIndirectKey( CTypes[resource] );

        if( resDict ) {
            // Find the key (can be a PdfReference).
            keyObj = resDict->GetDictionary().GetKey( key );
            if( keyObj ) {
                return keyObj;
            }
        }
    }
    return keyObj;
}
PoDoFo::PdfObject* PdfeResources::getIndirectKey( PdfeResourcesType::Enum resource, const PoDoFo::PdfName& key ) const
{
    PdfObject* keyObj = NULL;

    // Look in every resources object.
    for( int i = m_resources.size()-1 ; i >= 0 ; --i )
    {
        // Get resources dict.
        PdfObject* resDict = m_resources[i]->GetIndirectKey( CTypes[resource] );

        if( resDict ) {
            // Find the key.
            keyObj = resDict->GetIndirectKey( key );
            if( keyObj ) {
                return keyObj;
            }
        }
    }
    return keyObj;
}

}
