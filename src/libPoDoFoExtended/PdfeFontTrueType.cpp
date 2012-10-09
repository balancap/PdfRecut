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

#include "PdfeFontTrueType.h"
#include "podofo/podofo.h"

#include FT_BBOX_H
#include FT_GLYPH_H
#include FT_OUTLINE_H

using namespace PoDoFo;

namespace PoDoFoExtended {

PdfeFontTrueType::PdfeFontTrueType( PoDoFo::PdfObject* pFont, FT_Library ftLibrary ) :
    PdfeFont( pFont, ftLibrary )
{
    this->init();

    // Subtype of the font.
    const PdfName& subtype = pFont->GetIndirectKey( PdfName::KeySubtype )->GetName();
    if( subtype == PdfName( "TrueType" ) ) {
        m_type = PdfeFontType::TrueType;
        m_subtype = PdfeFontSubType::TrueType;
    }
    else {
        PODOFO_RAISE_ERROR_INFO( ePdfError_InvalidDataType, "The PdfObject is not a TrueType font." );
    }

    // Need the following entries in the dictionary.
    PdfObject* pFChar = pFont->GetIndirectKey( "FirstChar" );
    PdfObject* pLChar = pFont->GetIndirectKey( "LastChar" );
    PdfObject* pWidths = pFont->GetIndirectKey( "Widths" );
    PdfObject* pDescriptor = pFont->GetIndirectKey( "FontDescriptor" );

    // If does not exist: raise exception.
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
    // Space characters vector.
    this->initSpaceCharacters( m_firstCID, m_lastCID, true );

    // FreeType font face.
    this->initFTFace( m_fontDescriptor );
    // Characters bounding box.
    this->initCharactersBBox( pFont );
}
void PdfeFontTrueType::init()
{
    // Initialize members to default values.
    m_baseFont = PdfName();
    m_fontDescriptor.init();

    // Last CID < First CID to avoid problems.
    m_firstCID = 1;
    m_lastCID = 0;
    m_widthsCID.clear();
}
void PdfeFontTrueType::initCharactersBBox( const PdfObject* pFont )
{
    // Font bounding box used for default height.
    PdfRect fontBBox = m_fontDescriptor.fontBBox();

    // First read characters widths given in font object.
    PdfObject* pWidths = pFont->GetIndirectKey( "Widths" );
    const PdfArray&  widthsA = pWidths->GetArray();

    m_bboxCID.resize( widthsA.size(), PdfRect( 0, 0, 0, 0 ) );
    for( size_t i = 0 ; i < widthsA.size() ; ++i ) {
        m_bboxCID[i].SetWidth( widthsA[i].GetReal() );
        m_bboxCID[i].SetHeight( fontBBox.GetHeight() + fontBBox.GetBottom() );
    }
    // Check the size for coherence.
    if( m_bboxCID.size() != static_cast<size_t>( m_lastCID - m_firstCID + 1 ) ) {
        m_bboxCID.resize( m_lastCID - m_firstCID + 1, PdfRect( 0, 0, 1000., fontBBox.GetHeight() + fontBBox.GetBottom() ) );
    }

    // Construct the map CID to GID.
    std::vector<pdfe_gid> mapCIDToGID = this->mapCIDToGID( m_ftFace,
                                                           m_firstCID, m_lastCID,
                                                           dynamic_cast<PdfDifferenceEncoding*>( m_pEncoding ) );


    // Get glyph bounding box.
    for( pdfe_cid c = m_firstCID ; c <= m_lastCID ; ++c ) {
        // Not a space character.
        if( this->isSpace( c ) == PdfeFontSpace::None ) {
            // Glyph ID.
            pdfe_gid glyph_idx = mapCIDToGID[c - m_firstCID];
            if( glyph_idx ) {
                PdfRect glyphBBox = this->ftGlyphBBox( m_ftFace, glyph_idx, fontBBox );
                if( glyphBBox.GetWidth() > 0 && glyphBBox.GetHeight() > 0 ) {
                    //m_bboxCID[c - m_firstCID].SetLeft( 0.0 );
                    //m_bboxCID[c - m_firstCID].SetWidth( glyphBBox.GetWidth() );
                    m_bboxCID[c - m_firstCID].SetBottom( glyphBBox.GetBottom() );
                    m_bboxCID[c - m_firstCID].SetHeight( glyphBBox.GetHeight() );
                }
            }
        }
        else {
            m_bboxCID[c - m_firstCID].SetBottom( 0.0 );
            m_bboxCID[c - m_firstCID].SetHeight( this->spaceHeight() );
        }
    }
}

PdfeFontTrueType::~PdfeFontTrueType()
{
    // Delete encoding object if necessary.
    if( m_encodingOwned ) {
        delete m_pEncoding;
    }
}

const PdfeFontDescriptor& PdfeFontTrueType::fontDescriptor() const
{
    return m_fontDescriptor;
}
PdfRect PdfeFontTrueType::fontBBox() const
{
    // Font bbox rescaled.
    PdfRect fontBBox = m_fontDescriptor.fontBBox();
    fontBBox.SetLeft( fontBBox.GetLeft() / 1000. );
    fontBBox.SetBottom( fontBBox.GetBottom() / 1000. );
    fontBBox.SetWidth( fontBBox.GetWidth() / 1000. );
    fontBBox.SetHeight( fontBBox.GetHeight() / 1000. );

    return fontBBox;
}
double PdfeFontTrueType::width( pdfe_cid c, bool useFParams ) const
{
    double width;
    if( c >= m_firstCID && c <= m_lastCID ) {
        //width = m_widthsCID[ static_cast<size_t>( c - m_firstCID ) ] / 1000.;
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
PdfRect PdfeFontTrueType::bbox( pdfe_cid c, bool useFParams ) const
{
    //return PdfeFont::bbox( c, useFParams );

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
    if( useFParams ) {
        this->applyFontParameters( cbbox, this->isSpace( c ) == PdfeFontSpace::Code32 );
    }
    return cbbox;
}

}
