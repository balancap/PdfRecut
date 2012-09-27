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
//                          PdfeFont0                       //
//**********************************************************//
PdfeFontType0::PdfeFontType0( PoDoFo::PdfObject* pFont, FT_Library ftLibrary ) :
    PdfeFont( pFont, ftLibrary ), m_fontCID( NULL )
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
    m_fontCID->init( pDFont );

    // TODO: unicode CMap.

}
void PdfeFontType0::init()
{
    // Initialize members to default values.
    m_baseFont = PdfName();

    if( !m_fontCID ) {
        m_fontCID =  new PdfeFontCID();
    }
    m_fontCID->init();
}

void PdfeFontType0::initSpaceCharacters()
{
    m_spaceCharacters.clear();

    // TODO: use unicode CMap.
}

PdfeFontType0::~PdfeFontType0()
{
    delete m_fontCID;
}

const PdfeFontDescriptor& PdfeFontType0::fontDescriptor() const
{
    return m_fontCID->fontDescriptor();
}
PdfRect PdfeFontType0::fontBBox() const
{
    // Font bbox rescaled.
    PdfRect fontBBox = m_fontCID->fontDescriptor().fontBBox();
    fontBBox.SetLeft( fontBBox.GetLeft() / 1000. );
    fontBBox.SetBottom( fontBBox.GetBottom() / 1000. );
    fontBBox.SetWidth( fontBBox.GetWidth() / 1000. );
    fontBBox.SetHeight( fontBBox.GetHeight() / 1000. );

    return fontBBox;
}

PdfeCIDString PdfeFontType0::toCIDString( const PdfString& str ) const
{
    // Use the encoding CMap to convert the string.
    return m_encodingCMap.toCIDString( str );
}
double PdfeFontType0::width( pdf_cid c, bool useFParams ) const
{
    // Use CID font to obtain the width.
    double width = m_fontCID->width( c );

    // Apply font parameters.
    if( useFParams ) {
        width = ( width * m_fontSize + m_charSpace ) * ( m_hScale / 100. );
        if( this->isSpace( c ) == PdfeFontSpace::Code32 ) {
            width += m_wordSpace * ( m_hScale / 100. );
        }
    }
    return width;
}
PoDoFo::pdf_utf16be PdfeFontType0::toUnicode( pdf_cid c ) const
{
    // TODO: unicode map.
    return 0;
}
PdfeFontSpace::Enum PdfeFontType0::isSpace( pdf_cid c ) const
{
    return PdfeFontSpace::None;
}

//**********************************************************//
//                        PdfeFontCID                       //
//**********************************************************//
PdfeFontCID::PdfeFontCID()
{
    this->init();
}
void PdfeFontCID::init()
{
    m_type = PdfeFontType::Unknown;
    m_subtype =PdfeFontSubType::Unknown;

    m_baseFont = PdfName();
    m_cidSystemInfo.init();

    m_fontDescriptor.init();
}
void PdfeFontCID::init( PoDoFo::PdfObject* pFont )
{
    this->init();

    // Subtype of the font.
    const PdfName& subtype = pFont->GetIndirectKey( PdfName::KeySubtype )->GetName();
    if( subtype == PdfName( "CIDFontType0" ) ) {
        m_type = PdfeFontType::CIDFont;
        m_subtype = PdfeFontSubType::CIDFontType0;
    }
    else if( subtype == PdfName( "CIDFontType2" ) ) {
        m_type = PdfeFontType::CIDFont;
        m_subtype = PdfeFontSubType::CIDFontType2;
    }
    else {
        PODOFO_RAISE_ERROR_INFO( ePdfError_InvalidDataType, "The PdfObject is not a CID font." );
    }

    // Base font (required).
    m_baseFont = pFont->GetIndirectKey( "BaseFont" )->GetName();

    // Need the following entries in the dictionary.
    PdfObject* pCIDSytemInfo = pFont->GetIndirectKey( "CIDSystemInfo" );
    PdfObject* pDescriptor = pFont->GetIndirectKey( "FontDescriptor" );
    PdfObject* pWidths = pFont->GetIndirectKey( "W" );

    // Initialize CID system info and font descriptor.
    m_cidSystemInfo.init( pCIDSytemInfo );
    m_fontDescriptor.init( pDescriptor );

    // Read width of glyphs CID.
    m_hWidths.setDefaultWidth( static_cast<double>( pFont->GetDictionary().GetKeyAsLong( "DW", 1000L ) ) );
    if( pWidths ) {
        m_hWidths.init( pWidths->GetArray() );
    }
}

//**********************************************************//
//                 PdfeFontCID::HWidthsArray                //
//**********************************************************//
PdfeFontCID::HWidthsArray::HWidthsArray()
{
    this->init();
}
void PdfeFontCID::HWidthsArray::init()
{
    m_firstCID.clear();
    m_lastCID.clear();
    m_widthsCID.clear();
    m_defaultWidth = 1000.;
}
void PdfeFontCID::HWidthsArray::init( const PdfArray& widths )
{
    this->init();

    size_t i = 0;
    while( i < widths.size() ) {
        // Increase size of vectors.
        m_firstCID.push_back( 0 );
        m_lastCID.push_back( 0 );
        m_widthsCID.push_back( std::vector<double>() );

        // First CID value.
        m_firstCID.back() = static_cast<pdf_cid>( widths[i].GetNumber() );
        ++i;

        // Read array of widths.
        if( widths[i].IsArray() ) {
            PdfArray widthsCID = widths[i].GetArray();
            m_widthsCID.back().resize( widthsCID.size() );
            for( size_t j = 0 ; j < widthsCID.size() ; ++j ) {
                m_widthsCID.back()[j] = static_cast<double>( widthsCID[j].GetNumber() );
            }
            m_lastCID.back() = m_firstCID.back() + widthsCID.size() - 1;
            ++i;
        }
        // Read width for a range of CIDs.
        else {
            m_lastCID.back() = static_cast<pdf_cid>( widths[i].GetNumber() );
            ++i;
            m_widthsCID.back().push_back( static_cast<double>( widths[i].GetNumber() )  );
            ++i;
        }
    }
}


}
