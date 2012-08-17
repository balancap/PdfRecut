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

#include "PdfeFontType0.h"
#include "podofo/podofo.h"

using namespace PoDoFo;

namespace PoDoFoExtended {

//**********************************************************//
//                        PdfeFontCID                       //
//**********************************************************//
PdfeFontCID::PdfeFontCID()
{
    this->init();
}
void PdfeFontCID::init()
{

}
void PdfeFontCID::init( PoDoFo::PdfObject* pFont )
{

}

//**********************************************************//
//                          PdfeFont0                       //
//**********************************************************//
PdfeFontType0::PdfeFontType0( PoDoFo::PdfObject *pFont ) :
    PdfeFont( pFont )
{
    this->init();

    // Subtype of the font.
    const PdfName& subtype = pFont->GetIndirectKey( PdfName::KeySubtype )->GetName();
    if( subtype == PdfName( "Type0" ) ) {
        m_type = PdfeFontType::Type0;
        m_subtype = PdfeFontSubType::Type0;
    }
    else {
        PODOFO_RAISE_ERROR_INFO( ePdfError_InvalidDataType, "The PdfObject is not a Type 0 font." );
    }

    // Base font (required).
    m_baseFont = pFont->GetIndirectKey( "BaseFont" )->GetName();

    // Read CMap encoding.
    PdfObject* pEncodingCMap = pFont->GetIndirectKey( "Encoding" );
    if( pEncodingCMap->IsName() ) {
        m_encodingCMap.init( pEncodingCMap->GetName() );
    }
    else {
        m_encodingCMap.init( pEncodingCMap );
    }

    // Get descendant CID font.
    const PdfArray& descendantFonts  = pFont->GetIndirectKey( "DescendantFonts" )->GetArray();
    PdfObject* pDFont = pFont->GetOwner()->GetObject( descendantFonts[0].GetReference() );
    m_fontCID.init( pDFont );

    // TODO: unicode CMap.

}
void PdfeFontType0::init()
{
    // Initialize members to default values.
    m_baseFont = PdfName();
}
PdfeFontType0::~PdfeFontType0()
{
}

const PdfeFontDescriptor& PdfeFontType0::fontDescriptor() const
{
    //return m_fontDescriptor;
}
PdfeCIDString PdfeFontType0::toCIDString( const std::string& str ) const
{
    // Perform a simple copy.
    /*PdfeCIDString cidStr;
    cidStr.reserve( str.length() );
    for( size_t i = 0 ; i < str.length() ; ++i ) {
        cidStr.push_back( static_cast<pdf_cid>( str[i] ) );
    }
    return cidStr;*/
}
double PdfeFontType0::width( pdf_cid c ) const
{
//    if( c >= m_firstCID && c <= m_lastCID ) {
//        return m_widthsCID[ static_cast<size_t>( c - m_firstCID ) ] / 1000.;
//    }
//    else {
//        return m_fontDescriptor.missingWidth() / 1000.;
//    }
}
QChar PdfeFontType0::toUnicode( pdf_cid c ) const
{
    // TODO: unicode map.

//    if( m_encoding ) {
//        // Get utf16 code from PdfEncoding object.
//        return QChar( m_encoding->GetCharCode( c - m_firstCID ) );
//    }
//    else {
//        // Assume some kind of identity map...
//        return QChar( c );
//    }
}
PdfeFontSpace::Enum PdfeFontType0::isSpace( pdf_cid c ) const
{
//    return PdfeFontSpace::None;
}

}
