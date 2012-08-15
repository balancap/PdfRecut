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

#include <podofo/podofo.h>
#include "PdfFontMetricsCache.h"
#include "PdfFontMetricsType0.h"
#include "PdfFontMetricsType3.h"
#include "PdfFontMetrics14.h"

#include "PdfeFont.h"
#include "PdfeFontType1.h"
using namespace PoDoFoExtended;

using namespace PoDoFo;

namespace PdfRecut {

PdfFontMetricsCache::PdfFontMetricsCache( PoDoFo::PdfMemDocument* document ):
    m_document( document )
{
}
PdfFontMetricsCache::~PdfFontMetricsCache()
{
    // Free Pdf font metrics object.
    std::map< PdfReference, PdfFontMetricsPointer >::iterator it;
    for( it = m_fontMetricsCache.begin() ; it != m_fontMetricsCache.end() ; ++it ) {
        if( it->second.owned ) {
            delete it->second.ptr;
            it->second.ptr = NULL;
        }
    }
}

PoDoFo::PdfFontMetrics* PdfFontMetricsCache::getFontMetrics( const PoDoFo::PdfReference& fontRef )
{
    // Find the reference in the cache.
    std::map< PdfReference, PdfFontMetricsPointer >::iterator it;
    it = m_fontMetricsCache.find( fontRef );

    // If not find, add to cache.
    if( it == m_fontMetricsCache.end() ) {
        this->addFontMetrics2( fontRef );
        return this->addFontMetrics( fontRef );
    }
    else {
        return it->second.ptr;
    }
}
PoDoFo::PdfFontMetrics* PdfFontMetricsCache::addFontMetrics( const PoDoFo::PdfReference& fontRef )
{
    // Get font object.
    PdfObject* fontObj = m_document->GetObjects().GetObject( fontRef );
    PdfObject* pDescriptor = NULL;
    PdfObject* pEncoding = NULL;
    PdfFontMetricsPointer fontMetricsPtr;

    // Check it is a font object and get font subtype.
    if( fontObj->GetDictionary().GetKey( PdfName::KeyType )->GetName() != PdfName("Font") ) {
        PODOFO_RAISE_ERROR( ePdfError_InvalidDataType );
    }
    const PdfName& fontSubType = fontObj->GetDictionary().GetKey( PdfName::KeySubtype )->GetName();

    if( fontSubType == PdfName("Type0") )
    {
        // The PDF reference states that DescendantFonts must be an array,
        /*const PdfArray & descendant  = fontObj->GetIndirectKey( "DescendantFonts" )->GetArray();
        PdfObject* pFontObject = fontObj->GetOwner()->GetObject( descendant[0].GetReference() );

        // Don't care about the encoding property, not used by metrics object.
        pDescriptor = pFontObject->GetIndirectKey( "FontDescriptor" );
        if ( pDescriptor ) {
           fontMetricsPtr.ptr = new PdfFontMetricsObject( pFontObject, pDescriptor, NULL );
           fontMetricsPtr.owned = true;
        }*/
        fontMetricsPtr.ptr = new PdfFontMetricsType0( fontObj );
        fontMetricsPtr.owned = true;
    }
    else if( fontSubType == PdfName("Type1") )
    {
        // Descriptor might be missing in old document, for the 14 standard fonts.
        pDescriptor = fontObj->GetIndirectKey( "FontDescriptor" );
        pEncoding   = fontObj->GetIndirectKey( "Encoding" );

        if( !pDescriptor ) {
           // Check if its a PdfFontType1Base14
           PdfObject* pBaseFont = NULL;
           pBaseFont = fontObj->GetIndirectKey( "BaseFont" );
           const char* pszBaseFontName = pBaseFont->GetName().GetName().c_str();
           PdfFontMetricsBase14* pMetrics = PODOFO_Base14FontDef_FindBuiltinData( pszBaseFontName );

           if ( pMetrics ) {
               // Create associate encoding object.
               const PdfEncoding* pPdfEncoding = NULL;
               if ( pEncoding!= NULL )
                   pPdfEncoding = PdfEncodingObjectFactory::CreateEncoding( pEncoding );
               else if ( !pMetrics->IsSymbol() )
                   pPdfEncoding = PdfEncodingFactory::GlobalStandardEncodingInstance();
               else if ( strcmp(pszBaseFontName, "Symbol") == 0 )
                   pPdfEncoding = PdfEncodingFactory::GlobalSymbolEncodingInstance();
               else if ( strcmp(pszBaseFontName, "ZapfDingbats") == 0 )
                   pPdfEncoding = PdfEncodingFactory::GlobalZapfDingbatsEncodingInstance();

               fontMetricsPtr.ptr = new PdfFontMetrics14( pMetrics, pPdfEncoding );
               fontMetricsPtr.owned = true;

               if( pEncoding ) {
                   // TODO: segfault ?
                   // delete pPdfEncoding;
               }
               // fontMetricsPtr.ptr = pMetrics;
               // fontMetricsPtr.owned = false;
           }
        }
        else {
            // Create a standard metrics object.
            fontMetricsPtr.ptr = new PdfFontMetricsObject( fontObj, pDescriptor, NULL );
            fontMetricsPtr.owned = true;
        }
    }
    else if( fontSubType == PdfName("TrueType") )
    {
        pDescriptor = fontObj->GetIndirectKey( "FontDescriptor" );
        if( pDescriptor ) {
           fontMetricsPtr.ptr = new PdfFontMetricsObject( fontObj, pDescriptor, NULL );
           fontMetricsPtr.owned = true;
        }
    }
    else if( fontSubType == PdfName("Type3") )
    {
        fontMetricsPtr.ptr = new PdfFontMetricsType3( fontObj );
        fontMetricsPtr.owned = true;
    }

    // Insert metrics object in the cache.
    m_fontMetricsCache.insert( std::make_pair( fontRef, fontMetricsPtr ) );

    return fontMetricsPtr.ptr;
}

PoDoFo::PdfFontMetrics* PdfFontMetricsCache::addFontMetrics2( const PoDoFo::PdfReference& fontRef )
{
    // Get font object.
    PdfObject* fontObj = m_document->GetObjects().GetObject( fontRef );
    PdfeFont* pFont;

    // Check it is a font object and get font subtype.
    if( fontObj->GetDictionary().GetKey( PdfName::KeyType )->GetName() != PdfName("Font") ) {
        PODOFO_RAISE_ERROR( ePdfError_InvalidDataType );
    }
    const PdfName& fontSubType = fontObj->GetDictionary().GetKey( PdfName::KeySubtype )->GetName();

    if( fontSubType == PdfName("Type0") ) {
    }
    else if( fontSubType == PdfName("Type1") ) {
        pFont = new PdfeFontType1( fontObj );
    }
    else if( fontSubType == PdfName("TrueType") ) {
    }
    else if( fontSubType == PdfName("Type3") ) {
    }

    return NULL;
}

}
