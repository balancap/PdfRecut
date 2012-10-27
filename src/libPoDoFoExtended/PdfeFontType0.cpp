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
        this->setType( PdfeFontType::Type0 );
        this->setSubtype( PdfeFontSubType::Type0 );
    }
    else {
        PODOFO_RAISE_ERROR_INFO( ePdfError_InvalidDataType, "The PdfObject is not a Type 0 font." );
    }

    // Base font (required).
    m_baseFont = pFont->GetIndirectKey( "BaseFont" )->GetName();

    // Encoding CMap.
    PdfObject* pEncodingCMap = pFont->GetIndirectKey( "Encoding" );
    if( pEncodingCMap->IsName() ) {
        m_encodingCMap.init( pEncodingCMap->GetName() );
    }
    else {
        m_encodingCMap.init( pEncodingCMap );
    }

    // Unicode CMap.
    PdfObject* pUnicodeCMap = pFont->GetIndirectKey( "ToUnicode" );
    this->initUnicodeCMap( pUnicodeCMap );

    // Initialize descendant CID font.
    const PdfArray& descendantFonts  = pFont->GetIndirectKey( "DescendantFonts" )->GetArray();
    PdfObject* pDFont = pFont->GetOwner()->GetObject( descendantFonts[0].GetReference() );
    m_fontCID->init( pDFont );

    // FreeType font face.
    this->initFTFace( m_fontCID->fontDescriptor() );

    // Characters bounding box.
    m_fontCID->initCharactersBBox( this->ftFace() );

    // Space characters.
    const std::vector<pdfe_cid>& firstCIDs = m_fontCID->firstCIDs();
    const std::vector<pdfe_cid>& lastCIDs = m_fontCID->lastCIDs();
    for( size_t i = 0 ; i < firstCIDs.size() ; ++i ) {
        this->initSpaceCharacters( firstCIDs[i], lastCIDs[i], i == 0 );
    }

    // Log font information.
    this->initLogInformation();
}
void PdfeFontType0::init()
{
    // Initialize members to default values.
    m_baseFont = PdfName();
    m_encodingCMap.init();

    if( !m_fontCID ) {
        m_fontCID =  new PdfeFontCID();
    }
    m_fontCID->init();
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
    return PdfRectRescale( m_fontCID->fontDescriptor().fontBBox(), 0.001 );
}
PdfeVector PdfeFontType0::advance( pdfe_cid c, bool useFParams ) const
{
    // Get advance vector from CID font.
    PdfeVector advance = m_fontCID->advance( c );
    if( useFParams ) {
        this->applyFontParameters( advance, this->isSpace( c ) == PdfeFontSpace::Code32 );
    }
    return advance;
}
PdfRect PdfeFontType0::bbox( pdfe_cid c, bool useFParams ) const
{
    // Get bbox and apply font parameters.
    PdfRect cbbox = m_fontCID->bbox( c );
    if( useFParams ) {
        this->applyFontParameters( cbbox, this->isSpace( c ) == PdfeFontSpace::Code32 );
    }
    return cbbox;
}

PdfeCIDString PdfeFontType0::toCIDString( const PdfString& str ) const
{
    // Use the encoding CMap to convert the string.
    return m_encodingCMap.toCIDString( str );
}
QString PdfeFontType0::toUnicode( pdfe_cid c, bool useUCMap, bool firstTryEncoding ) const
{
    QString ustr;

    // Not empty unicode CMap : directly try this way (if allowed).
    if( !pUnicodeCMap()->emptyCodeSpaceRange() && useUCMap ) {
        // Create PdfeCMap::CharCodes from CID (might be multiple codes for with the same CID).
        std::vector<PdfeCMap::CharCode> charCodes = m_encodingCMap.toCharCode( c );

        // Convert CharCode to unicode (use arbitrarly the first one in the list...).
        if( charCodes.size() ) {
            ustr = pUnicodeCMap()->toUnicode( charCodes[0] );
        }
    }
    // Might be empty...
    return ustr;
}
double PdfeFontType0::spaceHeight() const
{
    // Default implementation.
    return this->PdfeFont::spaceHeight();
}
pdfe_gid PdfeFontType0::fromCIDToGID(pdfe_cid c) const
{
    // TODO: improve implementation using mapCIDToGID.
    return c;
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
    double defaultWidth = static_cast<double>( pFont->GetDictionary().GetKeyAsLong( "DW", 1000L ) );
    if( pWidths ) {
        m_hBBoxes.init( pWidths->GetArray(), defaultWidth, m_fontDescriptor.fontBBox() );
    }
    else {
        m_hBBoxes.init( PdfArray(), defaultWidth, m_fontDescriptor.fontBBox() );
    }
}
void PdfeFontCID::initCharactersBBox( FT_Face ftFace )
{
    m_hBBoxes.initCharactersBBox( ftFace );
}

PdfeVector PdfeFontCID::advance( pdfe_cid c ) const
{
    // TODO: vertical fonts.
    PdfeVector advance = m_hBBoxes.advance( c ) * 0.001;
    return advance;
}
PoDoFo::PdfRect PdfeFontCID::bbox( pdfe_cid c ) const
{
    // Get bbox and rescale in page coordinates.
    PdfRect cbbox = m_hBBoxes.bbox( c );
    return PdfRectRescale( cbbox, 0.001 );
}

//**********************************************************//
//                   PdfeFontCID::HBBoxArray                //
//**********************************************************//
PdfeFontCID::HBBoxArray::HBBoxArray()
{
    this->init();
}
void PdfeFontCID::HBBoxArray::init()
{
    m_firstCID.clear();
    m_lastCID.clear();

    m_advanceCID.clear();
    m_bboxCID.clear();

    m_defaultAdvance = PdfeVector( 1000., 0. );
    m_defaultBBox = PdfRect( 0., 0., 1000., 500. );
}
void PdfeFontCID::HBBoxArray::init( const PdfArray& widths,
                                    double defaultWidth,
                                    const PdfRect& fontBBox )
{
    this->init();

    // Default height used (from fontBBox) and default bounding box.
    double defaultHeight = fontBBox.GetHeight()+fontBBox.GetBottom();
    m_defaultAdvance = PdfeVector( defaultWidth, 0. );
    m_defaultBBox = PdfRect( 0., 0., defaultWidth, defaultHeight );

    size_t i = 0;
    while( i < widths.size() ) {
        // Increase size of vectors.
        m_firstCID.push_back( 0 );
        m_lastCID.push_back( 0 );
        m_bboxCID.push_back( std::vector<PdfRect>() );
        m_advanceCID.push_back( std::vector<PdfeVector>() );

        // First CID value.
        m_firstCID.back() = static_cast<pdfe_cid>( widths[i].GetNumber() );
        ++i;

        // Read array of widths.
        if( widths[i].IsArray() ) {
            const PdfArray& widthsCID = widths[i].GetArray();

            // Resize advance bbox vector with default height set.
            m_advanceCID.back().resize( widthsCID.size(),
                                        PdfeVector() );
            m_bboxCID.back().resize( widthsCID.size(),
                                     PdfRect( 0, 0, 0, defaultHeight ) );

            // Set advance and default bbox for each glyph.
            for( size_t j = 0 ; j < widthsCID.size() ; ++j ) {
                m_advanceCID.back()[j](0) = widthsCID[j].GetReal();
                m_bboxCID.back()[j].SetWidth( widthsCID[j].GetReal() );
            }
            m_lastCID.back() = m_firstCID.back() + widthsCID.size() - 1;
            ++i;
        }
        // Read width for a range of CIDs.
        else {
            m_lastCID.back() = static_cast<pdfe_cid>( widths[i].GetNumber() );
            ++i;

            // Create advance vector and bounding box for each glyph.
            m_advanceCID.back().resize( m_lastCID.back()-m_firstCID.back()+1,
                                        PdfeVector( widths[i].GetReal(), 0. ) );
            m_bboxCID.back().resize( m_lastCID.back()-m_firstCID.back()+1,
                                     PdfRect( 0., 0., widths[i].GetReal(), defaultHeight ) );
            ++i;
        }
    }
}
void PdfeFontCID::HBBoxArray::initCharactersBBox( FT_Face ftFace )
{
    // Get glyph bbox for characters in each group.
    for( size_t i = 0 ; i < m_bboxCID.size() ; ++i ) {
        for( pdfe_cid c = m_firstCID[i] ; c <= m_lastCID[i] ; ++c ) {
            // Glyph ID. TODO: map CID to GID...
            pdfe_gid glyph_idx = c;

            PdfRect glyphBBox = PdfeFont::ftGlyphBBox( ftFace, glyph_idx );
            if( glyphBBox.GetWidth() > 0 && glyphBBox.GetHeight() > 0 ) {    
                m_bboxCID[i][ c - m_firstCID[i] ] = glyphBBox;
            }
        }
    }
}

}
