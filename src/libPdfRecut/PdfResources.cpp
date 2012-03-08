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

#include "PdfResources.h"
#include <podofo/podofo.h>

namespace PdfRecut {

PdfResources::PdfResources()
{
    // Initialize resources types.
    m_resourcesTypes.push_back( "ExtGState" );
    m_resourcesTypes.push_back( "ColorSpace" );
    m_resourcesTypes.push_back( "Pattern" );
    m_resourcesTypes.push_back( "Shading" );
    m_resourcesTypes.push_back( "XObject" );
    m_resourcesTypes.push_back( "Font" );
    m_resourcesTypes.push_back( "ProcSet" );
    m_resourcesTypes.push_back( "Properties" );
}
PdfResources::PdfResources( const PdfResources& resources )
{
    m_resources = resources.m_resources;
    m_resourcesTypes = resources.m_resourcesTypes;
}
PdfResources& PdfResources::operator=( const PdfResources& resources )
{
    m_resources = resources.m_resources;
    m_resourcesTypes = resources.m_resourcesTypes;
    return *this;
}

void PdfResources::pushBack( PoDoFo::PdfObject* resourcesDict )
{
    // Check it is a dictionary.
    if( resourcesDict->IsDictionary() ) {
        m_resources.push_back( resourcesDict );
    }
    else {
        // TODO: raise exception...
    }
}

std::vector<PoDoFo::PdfObject*> PdfResources::getResources()
{
    return m_resources;
}

}
