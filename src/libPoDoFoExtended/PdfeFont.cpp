/***************************************************************************
 *   Copyright (C) 2011 by Paul Balança                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "PdfeFont.h"
#include "podofo/podofo.h"

using namespace PoDoFo;

namespace PoDoFoExtended {

PdfeFont::PdfeFont( PoDoFo::PdfObject* pFont )
{
    this->init();

    // Check if the PdfObject is a font dictionary.
    if( pFont && pFont->IsDictionary() && pFont->GetDictionary().HasKey( PdfName::KeyType ) ) {
        const PdfName& rType = pFont->GetDictionary().GetKey( PdfName::KeyType )->GetName();
        if( rType != PdfName( "Font" ) ) {
            PODOFO_RAISE_ERROR( ePdfError_InvalidDataType );
        }
    }
    else {
        PODOFO_RAISE_ERROR( ePdfError_InvalidDataType );
    }
}
void PdfeFont::init()
{
    // Font Type and Subtype.
    m_type = PdfeFontType::Unknown;
    m_subtype = PdfeFontSubType::Unknown;

    // Default values on font parameters.
    m_fontSize = 1.0;
    m_charSpace = 0.0;
    m_wordSpace = 0.0;
    m_hScale = 0.0;
}
PdfeFont::~PdfeFont()
{
}

}
