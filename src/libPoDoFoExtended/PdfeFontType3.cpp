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
        m_type = PdfeFontType::Type3;
        m_subtype = PdfeFontSubType::Type3;
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
    m_widthsCID.resize( widthsA.size() );
    for( size_t i = 0 ; i < widthsA.size() ; ++i ) {
        m_widthsCID[i] =  widthsA[i].GetReal();
    }
    // Check the size for coherence.
    if( m_widthsCID.size() != static_cast<size_t>( m_lastCID - m_firstCID + 1 ) ) {
        m_widthsCID.resize( m_lastCID - m_firstCID + 1, 1000. );
    }

    // Font descriptor.
    PdfObject* pDescriptor = pFont->GetIndirectKey( "FontDescriptor" );
    if( pDescriptor ) {
        m_fontDescriptor.init( pDescriptor );
    }

    // Font encoding. Should be a difference encoding.
    m_pEncoding = const_cast<PdfEncoding*>( PdfEncodingObjectFactory::CreateEncoding( pEncoding ) );
    // According to PoDoFo implementation.
    m_encodingOwned = !pEncoding->IsName() || ( pEncoding->IsName() && (pEncoding->GetName() == PdfName("Identity-H")) );

    // TODO: unicode CMap.

    // Space characters vector.
    this->initSpaceCharacters();

    // Glyph vectors.
    this->initGlyphs( pFont );
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
    m_widthsCID.clear();

    m_pEncoding = NULL;
    m_encodingOwned = false;

    m_mapCIDToGID.clear();
    m_glyphs.clear();
}
void PdfeFontType3::initSpaceCharacters()
{
    m_spaceCharacters.clear();

    // Find CIDs which correspond to the space character.
    QChar qcharSpace( ' ' );
    for( pdfe_cid c = m_firstCID ; c <= m_lastCID ; ++c ) {
        if( this->toUnicode( c ) == qcharSpace ) {
            m_spaceCharacters.push_back( c );
        }
    }
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
        PdfeFont::cidToName( m_pEncoding, c, cname );
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
    // Delete encoding object if necessary.
    if( m_encodingOwned ) {
        delete m_pEncoding;
    }
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

PdfeCIDString PdfeFontType3::toCIDString( const PdfString& str ) const
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
double PdfeFontType3::width( pdfe_cid c, bool useFParams ) const
{
    double width;
    if( c >= m_firstCID && c <= m_lastCID ) {
        // Assume the letter is a square...
        width = m_widthsCID[ static_cast<size_t>( c - m_firstCID ) ];
        PdfeVector cVect( width, 0.0 );
        cVect = cVect * m_fontMatrix;
        width = cVect(0);
    }
    else {
        // Return 0 according to PDF Reference.
        return 0.;
    }

    // Apply font parameters.
    if( useFParams ) {
        this->applyFontParameters( width, this->isSpace( c ) == PdfeFontSpace::Code32 );
    }
    return width;
}
PdfRect PdfeFontType3::bbox( pdfe_cid c, bool useFParams ) const
{
    PdfRect cbbox;
    if( c >= m_firstCID && c <= m_lastCID && m_mapCIDToGID[c-m_firstCID] ) {
        // Get glyph bounding box.
        cbbox = m_glyphs[c-m_firstCID].bbox();

        // Empty glyph bbox: call default implementation.
        if( cbbox.GetHeight() == 0 ) {
            return PdfeFont::bbox( c, useFParams );
        }

        // Modify left and width accordingly to char width.
        cbbox.SetLeft( 0.0 );
        cbbox.SetWidth( m_widthsCID[c-m_firstCID] );

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
QString PdfeFontType3::toUnicode( pdfe_cid c ) const
{
    // TODO: unicode map.

    if( m_pEncoding ) {
        // Get UTF16 code from PdfEncoding object.
        pdf_utf16be ucode = m_pEncoding->GetCharCode( c );
        ucode = PDFE_UTF16BE_TO_HBO( ucode );
        return QString::fromUtf16( &ucode, 1 );
    }
    else {
        // Default empty string.
        return QString();
    }
}
PdfeFontSpace::Enum PdfeFontType3::isSpace( pdfe_cid c ) const
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
    // TODO: implement the computation of cbox.

//    std::cout << this << " / " << streamState.gOperator.name << " : ";
//    std::copy( streamState.gOperands.begin(),
//               streamState.gOperands.end(),
//               std::ostream_iterator<std::string>( std::cout, " " ) );
//    std::cout << std::endl;
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
