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
        this->setType( PdfeFontType::Type1 );
        this->setSubtype( PdfeFontSubType::Type1 );
    }
    else if( subtype == PdfName( "MMType1" ) ) {
        this->setType( PdfeFontType::Type1 );
        this->setSubtype( PdfeFontSubType::MMType1 );
    }
    else {
        PODOFO_RAISE_ERROR_INFO( ePdfError_InvalidDataType, "The PdfObject is not a Type 1 font." );
    }

    // Read base font (required for standard font!).
    PdfObject* pFontName = pFont->GetIndirectKey( "BaseFont" );
    if( pFontName ) {
        m_baseFont = pFontName->GetName();
    }
    else {
        m_baseFont = PdfName();
    }

    // Standard 14 font?
    if( pFontName && ( PdfeFont::isStandard14Font( m_baseFont.GetName() ) != PdfeFont14Standard::None ) ) {
        this->initStandard14Font( pFont );
        return;
    }

    // Need the following entries in the dictionary.
    PdfObject* pFChar = pFont->GetIndirectKey( "FirstChar" );
    PdfObject* pLChar = pFont->GetIndirectKey( "LastChar" );
    PdfObject* pWidths = pFont->GetIndirectKey( "Widths" );
    PdfObject* pDescriptor = pFont->GetIndirectKey( "FontDescriptor" );

    // If does not exist: must be a 14 standard font.
    if( !( pFChar && pLChar && pWidths && pDescriptor ) ) {
        PODOFO_RAISE_ERROR( ePdfError_InvalidDataType );
    }

    // First and last characters.
    m_firstCID = static_cast<pdfe_cid>( pFChar->GetNumber() );
    m_lastCID = static_cast<pdfe_cid>( pLChar->GetNumber() );

    // Font descriptor.
    m_fontDescriptor.init( pDescriptor );
    m_baseFont = m_fontDescriptor.fontName();

    // Font encoding.
    PdfObject* pEncoding = pFont->GetIndirectKey( "Encoding" );
    this->initEncoding( pEncoding );
    // Unicode CMap.
    PdfObject* pUnicodeCMap = pFont->GetIndirectKey( "ToUnicode" );
    this->initUnicodeCMap( pUnicodeCMap );
    // Space characters.
    this->initSpaceCharacters( m_firstCID, m_lastCID, true );

    // FreeType font face.
    this->initFTFace( m_fontDescriptor );
    // Characters bounding box.
    this->initCharactersBBox( pFont );

    // Log font information.
    this->initLogInformation();
}
void PdfeFontType1::init()
{
    // Initialize members to default values.
    m_baseFont = PdfName();
    m_fontDescriptor.init();

    // Last CID < First CID to avoid problems.
    m_firstCID = 1;
    m_lastCID = 0;

    m_advanceCID.clear();
    m_bboxCID.clear();
}
void PdfeFontType1::initCharactersBBox( const PdfObject* pFont )
{
    // Font bounding box used for default height.
    PdfRect fontBBox = m_fontDescriptor.fontBBox();

    // Read characters afavance given in font object and set default bbox.
    PdfObject* pWidths = pFont->GetIndirectKey( "Widths" );
    const PdfArray&  widthsA = pWidths->GetArray();
    m_advanceCID.resize( widthsA.size(), PdfeVector() );
    m_bboxCID.resize( widthsA.size(), PdfRect( 0, 0, 0, 0 ) );

    for( size_t i = 0 ; i < widthsA.size() ; ++i ) {
        m_advanceCID[i](0) = widthsA[i].GetReal();
        m_bboxCID[i].SetWidth( widthsA[i].GetReal() );
        m_bboxCID[i].SetHeight( fontBBox.GetHeight() + fontBBox.GetBottom() );
    }
    // Check the size for coherence.
    if( m_bboxCID.size() != static_cast<size_t>( m_lastCID - m_firstCID + 1 ) ) {
        m_advanceCID.resize( m_lastCID - m_firstCID + 1, PdfeVector( 1000., 0. ) );
        m_bboxCID.resize( m_lastCID - m_firstCID + 1, PdfRect( 0, 0, 1000., fontBBox.GetHeight() + fontBBox.GetBottom() ) );
    }

     // Get glyph bounding box.
    for( pdfe_cid c = m_firstCID ; c <= m_lastCID ; ++c ) {
        // Not a space character.
        if( this->isSpace( c ) == PdfeFontSpace::None ) {
            // Glyph ID.
            pdfe_gid gid = this->fromCIDToGID( c );
            if( gid ) {
                PdfRect glyphBBox = this->ftGlyphBBox( ftFace(), gid );
                if( glyphBBox.GetWidth() > 0 && glyphBBox.GetHeight() > 0 ) {
                    glyphBBox = m_bboxCID[c - m_firstCID];
                }
            }
        }
        else {
            // Default height for space characters.
            m_bboxCID[c - m_firstCID].SetBottom( 0.0 );
            m_bboxCID[c - m_firstCID].SetHeight( this->spaceHeight() );
        }
    }
}
void PdfeFontType1::initStandard14Font( const PoDoFo::PdfObject* pFont )
{
    // Read base font (required for standard font!).
    m_baseFont = pFont->GetIndirectKey( "BaseFont" )->GetName();

    // Get PoDoFo Metrics object and set metrics paramters.
    PdfFontMetricsBase14* pMetrics = PODOFO_Base14FontDef_FindBuiltinData( m_baseFont.GetName().c_str() );
    pMetrics->SetFontSize( 1.0 );
    pMetrics->SetFontScale( 100.0 );
    pMetrics->SetFontCharSpace( 0.0 );

    // Can retrieve: widths, symbol, ascent, descent, xHeight, capHeight, BBox.
    m_fontDescriptor.setFontName( m_baseFont );
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
    PdfObject* pEncodingObj = pFont->GetIndirectKey( "Encoding" );
    if( pEncodingObj ) {
        this->initEncoding( pEncodingObj );
    }
    else if( !pMetrics->IsSymbol() ) {
        this->initEncoding( const_cast<PdfEncoding*>( PdfEncodingFactory::GlobalStandardEncodingInstance() ),
                            false );
    }
    else if( strcmp( m_baseFont.GetName().c_str(), "Symbol" ) == 0 ) {
        this->initEncoding( const_cast<PdfEncoding*>( PdfEncodingFactory::GlobalSymbolEncodingInstance() ),
                            false );
    }
    else if( strcmp( m_baseFont.GetName().c_str(), "ZapfDingbats" ) == 0 ) {
        this->initEncoding( const_cast<PdfEncoding*>( PdfEncodingFactory::GlobalZapfDingbatsEncodingInstance() ),
                            false );
    }
    // Unicode CMap.
    PdfObject* pUnicodeCMap = pFont->GetIndirectKey( "ToUnicode" );
    this->initUnicodeCMap( pUnicodeCMap );

    // FreeType font face.
    this->initFTFace( m_fontDescriptor );

    // Construct widths array using the font encoding.
    m_firstCID = pEncoding()->GetFirstChar();
    m_lastCID = pEncoding()->GetLastChar();

    pdf_utf16be ucode;
    double widthCID;
    for( pdfe_cid c = m_firstCID ; c <= m_lastCID ; ++c ) {
        ucode = pEncoding()->GetCharCode( c );

        // Dumb bug in PoDoFo: why bytes are inverted in GetCharCode but not UnicodeCharWidth ???
        ucode = PDFE_UTF16BE_HBO( ucode );

        widthCID = pMetrics->UnicodeCharWidth( ucode ) * 1000.0;
        m_advanceCID.push_back( PdfeVector( widthCID, 0.0 ) );
        m_bboxCID.push_back( PdfRect( 0, 0, widthCID, fontBBox[3].GetReal() ) );

        // Get bounding box from FTFace.
        if( this->isSpace( c ) == PdfeFontSpace::None ) {
            // Glyph ID.
            pdfe_gid gid = this->fromCIDToGID( c );
            if( gid ) {
                PdfRect glyphBBox = this->ftGlyphBBox( ftFace(), gid );
                if( glyphBBox.GetWidth() > 0 && glyphBBox.GetHeight() > 0 ) {
                    m_bboxCID.back() = glyphBBox;
                }
            }
        }
        else {
            // Default height for space characters.
            m_bboxCID[c - m_firstCID].SetBottom( 0.0 );
            m_bboxCID[c - m_firstCID].SetHeight( this->spaceHeight() );
        }
    }

    // Space characters.
    this->initSpaceCharacters( m_firstCID, m_lastCID, true );

    // Log font information.
    this->initLogInformation();
}

PdfeFontType1::~PdfeFontType1()
{
}

const PdfeFontDescriptor& PdfeFontType1::fontDescriptor() const
{
    return m_fontDescriptor;
}
PdfRect PdfeFontType1::fontBBox() const
{
    // Font bbox rescaled.
    return PdfRectRescale( m_fontDescriptor.fontBBox(), 0.001 );
}
double PdfeFontType1::width( pdfe_cid c, bool useFParams ) const
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
        this->applyFontParameters( width, this->isSpace( c ) == PdfeFontSpace::Code32 );
    }
    return width;
}
PdfeVector PdfeFontType1::advance( pdfe_cid c, bool useFParams ) const
{
    PdfeVector advance;
    if( c >= m_firstCID && c <= m_lastCID ) {
        advance = m_advanceCID[ static_cast<size_t>( c - m_firstCID ) ] * 0.001;
    }
    else {
        advance(0) = m_fontDescriptor.missingWidth() * 0.001;
    }
    // Apply font parameters.
    if( useFParams ) {
        this->applyFontParameters( advance, this->isSpace( c ) == PdfeFontSpace::Code32 );
    }
    return advance;
}
PdfRect PdfeFontType1::bbox( pdfe_cid c, bool useFParams ) const
{
    // Get glyph bbox and rescale it.
    PdfRect cbbox;
    if( c >= m_firstCID && c <= m_lastCID ) {
        cbbox = m_bboxCID[ c - m_firstCID ];
        cbbox = PdfRectRescale( cbbox, 0.001 );
    }
    else {
        // Call default implementation.
        return PdfeFont::bbox( c, useFParams );
    }
    // Apply font parameters.
    if( useFParams ) {
        this->applyFontParameters( cbbox, this->isSpace( c ) == PdfeFontSpace::Code32 );
    }
    return cbbox;
}
pdfe_gid PdfeFontType1::fromCIDToGID( pdfe_cid c ) const
{
    // No FreeType face loaded: return 0 GID.
    if( !ftFace() ) {
        return 0;
    }
    // Get the glyph index of the character from its name.
    PdfName cname = this->fromCIDToName( c );
    return this->ftGIDFromName( cname );
}

}
