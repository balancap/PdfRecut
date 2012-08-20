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

PdfeFontType3::PdfeFontType3( PoDoFo::PdfObject* pFont, FT_Library* ftLibrary ) :
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

    // Base font (required).
    m_baseFont = pFont->GetIndirectKey( "BaseFont" )->GetName();

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

    // Font BBox.
    m_fontBBox = pBBox->GetArray();

    // Font matrix.
    const PdfArray& matrixA = pMatrix->GetArray();
    m_fontMatrix(0,0) = matrixA[0].GetReal();
    m_fontMatrix(1,0) = matrixA[1].GetReal();
    m_fontMatrix(2,0) = matrixA[2].GetReal();
    m_fontMatrix(0,1) = matrixA[3].GetReal();
    m_fontMatrix(1,1) = matrixA[4].GetReal();
    m_fontMatrix(2,1) = matrixA[5].GetReal();

    // Read char widths.
    m_firstCID = static_cast<pdf_cid>( pFChar->GetNumber() );
    m_lastCID = static_cast<pdf_cid>( pLChar->GetNumber() );

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

    // Font encoding.
    m_encoding = const_cast<PdfEncoding*>( PdfEncodingObjectFactory::CreateEncoding( pEncoding ) );
    // According to PoDoFo implementation.
    m_encodingOwned = !pEncoding->IsName() || ( pEncoding->IsName() && (pEncoding->GetName() == PdfName("Identity-H")) );


    // TODO: unicode CMap.

    // Space characters vector.
    this->initSpaceCharacters();
}
void PdfeFontType3::init()
{
    // Initialize members to default values.
    m_baseFont = PdfName();
    m_fontDescriptor.init();

    m_fontBBox.resize( 4, 0.0 );
    m_fontMatrix.init();

    // Last CID < First CID to avoid problems.
    m_firstCID = 1;
    m_lastCID = 0;
    m_widthsCID.clear();

    m_encoding = NULL;
    m_encodingOwned = false;

}
void PdfeFontType3::initSpaceCharacters()
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

PdfeFontType3::~PdfeFontType3()
{
    // Delete encoding object if necessary.
    if( m_encodingOwned ) {
        delete m_encoding;
    }
}

const PdfeFontDescriptor& PdfeFontType3::fontDescriptor() const
{
    return m_fontDescriptor;
}
PdfArray PdfeFontType3::fontBBox() const
{
    PdfArray bbox;

    // Approximation of bounding box ~ remove the rotation component of the font matrix.
    bbox[0] = m_fontBBox[0].GetReal() * m_fontMatrix(0,0);
    bbox[1] = m_fontBBox[1].GetReal() * m_fontMatrix(1,1);
    bbox[2] = m_fontBBox[2].GetReal() * m_fontMatrix(0,0);
    bbox[3] = m_fontBBox[3].GetReal() * m_fontMatrix(1,1);

    return bbox;
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
double PdfeFontType3::width( pdf_cid c, bool useFParams ) const
{
    double width;
    if( c >= m_firstCID && c <= m_lastCID ) {
        // Assume the letter is a square...
        width = m_widthsCID[ static_cast<size_t>( c - m_firstCID ) ];
        PdfeVector cVect( width, width );
        cVect = cVect * m_fontMatrix;
        width = cVect(0);
    }
    else {
        // Return 0 according to PDF Reference.
        return 0.;
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
QChar PdfeFontType3::toUnicode( pdf_cid c ) const
{
    // TODO: unicode map.

    if( m_encoding ) {
        // Get UTF16 code from PdfEncoding object.
        pdf_utf16be ucode = m_encoding->GetCharCode( c );
        return QChar( PDF_UTF16_BE_LE( ucode ) );
    }
    else {
        // Default empty character.
        return QChar( 0 );
    }
}
PdfeFontSpace::Enum PdfeFontType3::isSpace( pdf_cid c ) const
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
