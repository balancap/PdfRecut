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

#include "PdfeFontType1.h"
#include "podofo/podofo.h"

#include FT_BBOX_H
#include FT_GLYPH_H

using namespace PoDoFo;

namespace PoDoFoExtended {

PdfeFontType1::PdfeFontType1( PoDoFo::PdfObject* pFont, FT_Library ftLibrary ) :
    PdfeFont( pFont, ftLibrary )
{
    this->init();

    // Subtype of the font.
    const PdfName& subtype = pFont->GetIndirectKey( PdfName::KeySubtype )->GetName();
    if( subtype == PdfName( "Type1" ) ) {
        m_type = PdfeFontType::Type1;
        m_subtype = PdfeFontSubType::Type1;
    }
    else if( subtype == PdfName( "MMType1" ) ) {
        m_type = PdfeFontType::Type1;
        m_subtype = PdfeFontSubType::MMType1;
    }
    else {
        PODOFO_RAISE_ERROR_INFO( ePdfError_InvalidDataType, "The PdfObject is not a Type 1 font." );
    }

    // Base font (required).
    m_baseFont = pFont->GetIndirectKey( "BaseFont" )->GetName();

    // Need the following entries in the dictionary.
    PdfObject* pFChar = pFont->GetIndirectKey( "FirstChar" );
    PdfObject* pLChar = pFont->GetIndirectKey( "LastChar" );
    PdfObject* pWidths = pFont->GetIndirectKey( "Widths" );
    PdfObject* pDescriptor = pFont->GetIndirectKey( "FontDescriptor" );

    // If does not exist: must be a 14 standard font.
    if( !( pFChar && pLChar && pWidths && pDescriptor ) ) {
        this->initStandard14Font( pFont );
        return;
    }

    // First and last characters.
    m_firstCID = static_cast<pdf_cid>( pFChar->GetNumber() );
    m_lastCID = static_cast<pdf_cid>( pLChar->GetNumber() );

    // Font descriptor.
    m_fontDescriptor.init( pDescriptor );

    // Font encoding.
    PdfObject* pEncoding = pFont->GetIndirectKey( "Encoding" );
    if( pEncoding ) {
        m_encoding = const_cast<PdfEncoding*>( PdfEncodingObjectFactory::CreateEncoding( pEncoding ) );

        // According to PoDoFo implementation.
        m_encodingOwned = !pEncoding->IsName() || ( pEncoding->IsName() && (pEncoding->GetName() == PdfName("Identity-H")) );
    }

    // TODO: unicode CMap.

    // Space characters vector.
    this->initSpaceCharacters();

    // Characters bounding box.
    this->initCharactersBBox( pFont );
}
void PdfeFontType1::init()
{
    // Initialize members to default values.
    m_baseFont = PdfName();
    m_fontDescriptor.init();

    // Last CID < First CID to avoid problems.
    m_firstCID = 1;
    m_lastCID = 0;
    m_widthsCID.clear();
    m_bboxCID.clear();

    m_encoding = NULL;
    m_encodingOwned = false;
    m_spaceCharacters.clear();
}
void PdfeFontType1::initStandard14Font( const PoDoFo::PdfObject* pFont )
{
    // Get PoDoFo Metrics object and set metrics paramters.
    PdfFontMetricsBase14* pMetrics = PODOFO_Base14FontDef_FindBuiltinData( m_baseFont.GetName().c_str() );
    pMetrics->SetFontSize( 1.0 );
    pMetrics->SetFontScale( 100.0 );
    pMetrics->SetFontCharSpace( 0.0 );

    // Can retrieve: widths, symbol, ascent, descent, xHeight, capHeight, BBox.
    m_fontDescriptor.setAscent( pMetrics->GetAscent() );
    m_fontDescriptor.setDescent( pMetrics->GetDescent() );
    m_fontDescriptor.setCapHeight( pMetrics->GetCapHeight() );

    PdfArray fontBBox;
    pMetrics->GetBoundingBox( fontBBox );
    m_fontDescriptor.setFontBBox( fontBBox );

    // Descriptor flags (at least partially...).
    if( pMetrics->IsSymbol() ) {
        m_fontDescriptor.setFlags( PdfeFontDescriptor::FlagSymbolic );
    }
    else {
        m_fontDescriptor.setFlags( PdfeFontDescriptor::FlagNonsymbolic );
    }

    // To be fixed and improved...
    m_fontDescriptor.setXHeight( 500. );

    // Create associate encoding object.
    PdfObject* pEncoding = pFont->GetIndirectKey( "Encoding" );
    if( pEncoding ) {
        m_encoding = const_cast<PdfEncoding*>( PdfEncodingObjectFactory::CreateEncoding( pEncoding ) );

        // According to PoDoFo implementation.
        m_encodingOwned = !pEncoding->IsName() || ( pEncoding->IsName() && (pEncoding->GetName() == PdfName("Identity-H")) );
    }
    else if( !pMetrics->IsSymbol() ) {
        m_encoding = const_cast<PdfEncoding*>( PdfEncodingFactory::GlobalStandardEncodingInstance() );
        m_encodingOwned = false;
    }
    else if( strcmp( m_baseFont.GetName().c_str(), "Symbol" ) == 0 ) {
        m_encoding = const_cast<PdfEncoding*>( PdfEncodingFactory::GlobalSymbolEncodingInstance() );
        m_encodingOwned = false;
    }
    else if( strcmp( m_baseFont.GetName().c_str(), "ZapfDingbats" ) == 0 ) {
        m_encoding = const_cast<PdfEncoding*>( PdfEncodingFactory::GlobalZapfDingbatsEncodingInstance() );
        m_encodingOwned = false;
    }

    // Construct widths array using the font encoding.
    m_firstCID = m_encoding->GetFirstChar();
    m_lastCID = m_encoding->GetLastChar();

    pdf_utf16be ucode;
    double widthCID;
    for( int i = m_firstCID ; i <= m_lastCID ; ++i ) {
        ucode = m_encoding->GetCharCode( i );

        // Dumb bug in PoDoFo: why bytes are inverted in GetCharCode but not UnicodeCharWidth ???
        ucode = PDF_UTF16_BE_LE( ucode );

        widthCID = pMetrics->UnicodeCharWidth( ucode ) * 1000.0;
        m_widthsCID.push_back( widthCID );
        m_bboxCID.push_back( PdfRect( 0, 0, widthCID, fontBBox[3].GetReal() ) );
    }

    // TODO: unicode CMap.

    // Space characters vector.
    this->initSpaceCharacters();
}
void PdfeFontType1::initSpaceCharacters()
{
    m_spaceCharacters.clear();

    // Find CIDs which correspond to the space character.
    QChar qcharSpace( ' ' );
    for( pdf_cid c = m_firstCID ; c <= m_lastCID ; ++c ) {
        if( this->toUnicode( c ) == qcharSpace ) {
            m_spaceCharacters.push_back( c );
        }
    }
}
void PdfeFontType1::initCharactersBBox( const PdfObject* pFont )
{
    // Font bounding box used for default height.
    PdfArray fontBBox = this->fontBBox();

    // First read characters widths given in font object.
    PdfObject* pWidths = pFont->GetIndirectKey( "Widths" );
    const PdfArray&  widthsA = pWidths->GetArray();

    m_bboxCID.resize( widthsA.size(), PdfRect( 0, 0, 0, 0) );
    for( size_t i = 0 ; i < widthsA.size() ; ++i ) {
        m_bboxCID[i].SetWidth( widthsA[i].GetReal() );
        m_bboxCID[i].SetHeight( fontBBox[3].GetReal() );
    }
    // Check the size for coherence.
    if( m_bboxCID.size() != static_cast<size_t>( m_lastCID - m_firstCID + 1 ) ) {
        m_bboxCID.resize( m_lastCID - m_firstCID + 1, PdfRect( 0, 0, 1000., fontBBox[3].GetReal() ) );
    }

    // For embedded fonts: try to get bottom and height using the font program and FreeType library.
    char* buffer;
    long length;
    PdfeFontEmbedded fontEmbedded = this->fontDescriptor().fontEmbedded();
    fontEmbedded.fontProgram( &buffer, &length );
    if( !buffer ) {
        // No font program found...
        return;
    }

    // Load FreeType face from buffer.
    int error;
    FT_Face face;
    error = FT_New_Memory_Face( m_ftLibrary, reinterpret_cast<unsigned char*>( buffer ),
                                length, 0, &face );
    if( error ) {
        // Can not load: return...
        return;
    }

    // Construct the map CID to GID.
    std::vector<pdf_gid> mapCIDToGID = this->mapCIDToGID( face, m_firstCID, m_lastCID,
                                                          dynamic_cast<PdfDifferenceEncoding*>( m_encoding ) );

    // Get glyph bounding box.
    pdf_gid glyphIdx;
    PdfRect glyphBBox;

    for( pdf_cid c = m_firstCID ; c <= m_lastCID ; ++c ) {
        glyphIdx = mapCIDToGID[c-m_firstCID];
        if( glyphIdx ) {
            // Load glyph.
            error = this->glyphBBox( face, glyphIdx, fontBBox, &glyphBBox );
            if( !error ) {
                //m_bboxCID[c - m_firstCID].SetLeft( 0.0 );
                //m_bboxCID[c - m_firstCID].SetWidth( glyphBBox.GetWidth() );
                m_bboxCID[c - m_firstCID].SetBottom( glyphBBox.GetBottom() );
                m_bboxCID[c - m_firstCID].SetHeight( glyphBBox.GetHeight() );
            }
        }
    }

    // Free face object and font file buffer.
    FT_Done_Face( face );
    free( buffer );
}

PdfeFontType1::~PdfeFontType1()
{
    // Delete encoding object if necessary.
    if( m_encodingOwned ) {
        delete m_encoding;
    }
}

const PdfeFontDescriptor& PdfeFontType1::fontDescriptor() const
{
    return m_fontDescriptor;
}
PdfArray PdfeFontType1::fontBBox() const
{
    return m_fontDescriptor.fontBBox();
}

PdfeCIDString PdfeFontType1::toCIDString( const PdfString& str ) const
{
    // PDF String data.
    const char* pstr = str.GetString();
    size_t length = str.GetLength();

    // Perform a simple copy.
    PdfeCIDString cidstr;
    cidstr.resize( length, 0 );
    for( size_t i = 0 ; i < length ; ++i ) {
        cidstr[i] = static_cast<unsigned char>( pstr[i] );
    }
    return cidstr;
}
double PdfeFontType1::width( pdf_cid c, bool useFParams ) const
{
    double width;
    if( c >= m_firstCID && c <= m_lastCID ) {
        width = m_bboxCID[ static_cast<size_t>( c - m_firstCID ) ].GetWidth() / 1000.;
    }
    else {
        width = m_fontDescriptor.missingWidth() / 1000.;
    }

    // Apply font parameters.
    if( useFParams ) {
        width = ( width * m_fontSize + m_charSpace ) * ( m_hScale / 100. );
        if( this->isSpace( c ) == PdfeFontSpace::Code32 ) {
            width += m_wordSpace * ( m_hScale / 100. );
        }
    }
    return width;
}
PdfRect PdfeFontType1::bbox( pdf_cid c, bool useFParams ) const
{
    PdfRect cbbox;
    if( c >= m_firstCID && c <= m_lastCID ) {
        cbbox = m_bboxCID[ static_cast<size_t>( c - m_firstCID ) ];
        cbbox.SetLeft( 0. );
        cbbox.SetWidth( cbbox.GetWidth() / 1000. );
        cbbox.SetBottom( cbbox.GetBottom() / 1000. );
        cbbox.SetHeight( cbbox.GetHeight() / 1000. );
    }
    else {
        // Call default implementation.
        return PdfeFont::bbox( c, useFParams );
    }

    // Apply font parameters.
    double width = cbbox.GetWidth();
    if( useFParams ) {
        width = ( width * m_fontSize + m_charSpace ) * ( m_hScale / 100. );
        if( this->isSpace( c ) == PdfeFontSpace::Code32 ) {
            width += m_wordSpace * ( m_hScale / 100. );
        }
        cbbox.SetWidth( width );
        cbbox.SetBottom( cbbox.GetBottom() * m_fontSize );
        cbbox.SetHeight( cbbox.GetHeight() * m_fontSize );
    }
    return cbbox;
}
PoDoFo::pdf_utf16be PdfeFontType1::toUnicode( pdf_cid c ) const
{
    // TODO: unicode map.

    if( m_encoding ) {
        // Get UTF16 code from PdfEncoding object.
        pdf_utf16be ucode = m_encoding->GetCharCode( c );
        ucode = PDF_UTF16_BE_LE( ucode );
        return ucode;
    }
    else {
        // Default empty character.
        return 0;
    }
}
PdfeFontSpace::Enum PdfeFontType1::isSpace( pdf_cid c ) const
{
    // Does the character belongs to the space vector ?
    for( size_t i = 0 ; i < m_spaceCharacters.size() ; ++i ) {
        if( c == m_spaceCharacters[i] ) {
            if( c == 32 ) {
                return PdfeFontSpace::Code32;
            }
            else {
                return PdfeFontSpace::Other;
            }
        }
    }
    return PdfeFontSpace::None;
}

}
