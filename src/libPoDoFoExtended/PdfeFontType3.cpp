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

#include "PdfeFontType3.h"
#include "podofo/podofo.h"

using namespace PoDoFo;

namespace PoDoFoExtended {

//**********************************************************//
//                       PdfeFontType3                      //
//**********************************************************//
PdfeFontType3::PdfeFontType3( PoDoFo::PdfObject* pFont, FT_Library ftLibrary ) :
    PdfeFont( pFont, ftLibrary )
{
    this->init();

    // Subtype of the font.
    const PdfName& subtype = pFont->GetIndirectKey( PdfName::KeySubtype )->GetName();
    if( subtype == PdfName( "Type3" ) ) {
        this->setType( PdfeFontType::Type3 );
        this->setSubtype( PdfeFontSubType::Type3 );
    }
    else {
        PODOFO_RAISE_ERROR_INFO( ePdfError_InvalidDataType, "The PdfObject is not a Type 3 font." );
    }

    // Need the following entries in the font dictionary.
    PdfObject* pBBox = pFont->GetIndirectKey( "FontBBox" );
    PdfObject* pMatrix = pFont->GetIndirectKey( "FontMatrix" );
    PdfObject* pFChar = pFont->GetIndirectKey( "FirstChar" );
    PdfObject* pLChar = pFont->GetIndirectKey( "LastChar" );
    PdfObject* pWidths = pFont->GetIndirectKey( "Widths" );
    PdfObject* pEncoding = pFont->GetIndirectKey( "Encoding" );

    // If does not exist: raise exception.
    if( !( pBBox && pMatrix && pFChar && pLChar && pWidths && pEncoding ) ) {
        PODOFO_RAISE_ERROR_INFO( ePdfError_InvalidDataType, "Entries missing in the Type 3 font dictionary." );
    }

    // Initialize font BBox from array.
    m_fontBBox = PdfRect( pBBox->GetArray() );

    // Font matrix.
    const PdfArray& matrixA = pMatrix->GetArray();
    m_fontMatrix(0,0) = matrixA[0].GetReal();
    m_fontMatrix(0,1) = matrixA[1].GetReal();
    m_fontMatrix(1,0) = matrixA[2].GetReal();
    m_fontMatrix(1,1) = matrixA[3].GetReal();
    m_fontMatrix(2,0) = matrixA[4].GetReal();
    m_fontMatrix(2,1) = matrixA[5].GetReal();

    // Read char widths.
    m_firstCID = static_cast<pdfe_cid>( pFChar->GetNumber() );
    m_lastCID = static_cast<pdfe_cid>( pLChar->GetNumber() );

    const PdfArray&  widthsA = pWidths->GetArray();
    m_advanceCID.resize( widthsA.size(), PdfeVector() );
    for( size_t i = 0 ; i < widthsA.size() ; ++i ) {
        m_advanceCID[i](0) = widthsA[i].GetReal();
    }
    // Check the size for coherence.
    if( m_advanceCID.size() != static_cast<size_t>( m_lastCID - m_firstCID + 1 ) ) {
        m_advanceCID.resize( m_lastCID - m_firstCID + 1, PdfeVector( 1000., 0. ) );
    }

    // Font descriptor (required in Tagged documents).
    PdfObject* pDescriptor = pFont->GetIndirectKey( "FontDescriptor" );
    if( pDescriptor ) {
        m_fontDescriptor.init( pDescriptor );
    }

    // Font encoding.
    this->initEncoding( pEncoding );
    // Unicode CMap.
    PdfObject* pUnicodeCMap = pFont->GetIndirectKey( "ToUnicode" );
    this->initUnicodeCMap( pUnicodeCMap );

    // Space characters vector.
    this->initSpaceCharacters( m_firstCID, m_lastCID, true );

    // Glyph vectors.
    this->initGlyphs( pFont );

    // Log font information.
    this->initLogInformation();
}
void PdfeFontType3::init()
{
    // Initialize members to default values.
    m_fontDescriptor.init();

    m_fontBBox = PdfRect( 0.0, 0.0, 0.0, 0.0 );
    m_fontMatrix.init();

    // Last CID < First CID to avoid problems.
    m_firstCID = 1;
    m_lastCID = 0;
    m_advanceCID.clear();

    m_mapCIDToGID.clear();
    m_glyphs.clear();
}
void PdfeFontType3::initGlyphs( const PdfObject* pFont )
{
    // CharProcs and Resources objects.
    PdfObject* pCharProcs = pFont->GetIndirectKey( "CharProcs" );
    PdfObject* pResources = pFont->GetIndirectKey( "Resources" );

    // No CharProcs: exception raised.
    if( !pCharProcs || !pCharProcs->IsDictionary() ) {
        PODOFO_RAISE_ERROR_INFO( ePdfError_InvalidDataType, "Entries missing in the Type 3 font dictionary." );
    }

    // Resize space vectors.
    m_mapCIDToGID.resize( m_lastCID-m_firstCID+1, 0 );
    m_glyphs.resize( m_lastCID-m_firstCID+1, PdfeGlyphType3() );

    // Look at each character.
    PdfName cname;
    PdfObject* pGlyph;
    for( pdfe_cid c = m_firstCID ; c <= m_lastCID ; ++c ) {
        // Get character name and search it in CharProcs.
        cname = this->fromCIDToName( c );
        pGlyph = pCharProcs->GetIndirectKey( cname );

        // If found, set GID to c-m_firstCID+1 and create glyph.
        if( pGlyph ) {
            m_mapCIDToGID[c-m_firstCID] = c-m_firstCID+1;
            m_glyphs[c-m_firstCID] = PdfeGlyphType3( cname, pGlyph, pResources );
        }
    }
}

PdfeFontType3::~PdfeFontType3()
{
}

const PdfeFontDescriptor& PdfeFontType3::fontDescriptor() const
{
    return m_fontDescriptor;
}
PdfRect PdfeFontType3::fontBBox() const
{
    // Apply font transformation to font bbox.
    PdfeORect fontBBox( m_fontBBox );
    fontBBox = m_fontMatrix.map( fontBBox );

    // Return the bounding box of the oriented rectangle.
    return fontBBox.toPdfRect( true );
}
PdfeVector PdfeFontType3::advance( pdfe_cid c, bool useFParams ) const
{
    PdfeVector advance;
    if( c >= m_firstCID && c <= m_lastCID ) {
        advance = m_advanceCID[ c - m_firstCID ];
        advance = m_fontMatrix.map( advance );

        // Only keep the horizontal component.
        advance(1) = 0.0;
    }
    else {
        // Return 0 according to PDF Reference.
        return advance;
    }
    // Apply font parameters.
    if( useFParams ) {
        this->applyFontParameters( advance, this->isSpace( c ) == PdfeFontSpace::Code32 );
    }
    return advance;
}
PdfRect PdfeFontType3::bbox( pdfe_cid c, bool useFParams ) const
{
    PdfRect cbbox;
    if( c >= m_firstCID && c <= m_lastCID && m_mapCIDToGID[c-m_firstCID] ) {
        // Get glyph bounding box.
        cbbox = m_glyphs[c - m_firstCID].bbox();
        // Empty glyph bbox: call default implementation.
        if( cbbox.GetHeight() == 0 ) {
            return PdfeFont::bbox( c, useFParams );
        }

        // Apply font transformation to char bbox.
        PdfeORect oBBox( cbbox );
        oBBox = m_fontMatrix.map( oBBox );
        cbbox = oBBox.toPdfRect( true );
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

double PdfeFontType3::spaceHeight() const
{
    static double spaceHeight(-1);

    // Compute mean width.
    if( spaceHeight <= 0 ) {
        size_t nbCID = 0;
        double spaceHeight = 0;

        for( pdfe_cid c = m_firstCID ; c <= m_lastCID ; ++c ) {
            if( m_advanceCID[c - m_firstCID](0) > 0) {
                spaceHeight += m_advanceCID[c - m_firstCID](0);
                ++nbCID;
            }
        }
        if( spaceHeight > 0 ) {
            spaceHeight = spaceHeight / nbCID;
        }
        else {
            spaceHeight = m_fontBBox.GetHeight();
        }

        // Multiply by a magic! constant factor.
        spaceHeight = spaceHeight * 0.8;
    }
    return spaceHeight;
}
pdfe_gid PdfeFontType3::fromCIDToGID(pdfe_cid c) const
{
    return ( c + 1 );
}

//**********************************************************//
//                      PdfeGlyphType3                      //
//**********************************************************//
PdfeGlyphType3::PdfeGlyphType3() :
    PdfeCanvasAnalysis(), PdfCanvas(),
    m_name(), m_pStream( NULL ), m_pResources( NULL ),
    m_isBBoxComputed( false ), m_bboxD1( 0,0,0,0 ), m_cbox( 0,0,0,0 )

{
}

PdfeGlyphType3::PdfeGlyphType3( const PdfName& glyphName,
                                PdfObject* glyphStream,
                                PdfObject* fontResources ) :
    PdfeCanvasAnalysis(), PdfCanvas(),
    m_name( glyphName ), m_pStream( glyphStream ), m_pResources( fontResources ),
    m_isBBoxComputed( false ), m_bboxD1( 0,0,0,0 ), m_cbox( 0,0,0,0 )
{
    // Not stream in the object...
    if( !m_pStream->HasStream() ) {
        PODOFO_RAISE_ERROR_INFO( ePdfError_InvalidDataType, "The glyph description object does not contain a stream." );
    }

    // Compute glyph bounding box.
    this->computeBBox();
}
PdfRect PdfeGlyphType3::bbox() const
{
    // Return bounding box given by d1 command. Can be empty if d0 is used in contents stream.
    // TODO: use computed cbox.
    return m_bboxD1;
}

void PdfeGlyphType3::computeBBox()
{
    // Reset bounding boxes to zero.
    m_bboxD1 = PdfRect( 0, 0, 0, 0 );
    m_cbox = PdfRect( 0, 0, 0, 0 );

    // Analyse contents stream of the glyph.
    this->analyseContents( this, PdfeGraphicsState(), PdfeResources() );
}

//**********************************************//
//         PdfeCanvasAnalysis interface         //
//**********************************************//
void PdfeGlyphType3::fPathPainting( const PdfeStreamState& streamState,
                                    const PdfePath& currentPath )
{
    // TODO: implement the computation of cbox.
//    std::cout << this << " / " << streamState.gOperator.name << " : ";
//    std::copy( streamState.gOperands.begin(),
//               streamState.gOperands.end(),
//               std::ostream_iterator<std::string>( std::cout, " " ) );
//    std::cout << std::endl;
}
void PdfeGlyphType3::fType3Fonts( const PdfeStreamState& streamState )
{
    // Update d1 bounding box if possible.
    if( streamState.gOperator.code == ePdfGOperator_d1 ) {
        double left, right, bottom, top;
        size_t nbvars = streamState.gOperands.size();

        // Read bbox coordinates.
        this->readValue( streamState.gOperands[nbvars-4], left );
        this->readValue( streamState.gOperands[nbvars-3], bottom );
        this->readValue( streamState.gOperands[nbvars-2], right );
        this->readValue( streamState.gOperands[nbvars-1], top );

        m_bboxD1 = PdfRect( left, bottom, right-left, top-bottom );
    }
}

void PdfeGlyphType3::fUnknown( const PdfeStreamState& streamState )
{
}

//**********************************************//
//              PdfCanvas interface             //
//**********************************************//
PdfObject* PdfeGlyphType3::GetContents() const
{
    return m_pStream;
}
PdfObject* PdfeGlyphType3::GetContentsForAppending() const
{
    // No way someone's gonna modify the content!
    return NULL;
}
PdfObject* PdfeGlyphType3::GetResources() const
{
    return m_pResources;
}
const PdfRect PdfeGlyphType3::GetPageSize() const
{
    // Don't care...
    return PdfRect( 0, 0, 0, 0 );
}

}
