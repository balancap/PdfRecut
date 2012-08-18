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

PdfeFontType3::PdfeFontType3( PoDoFo::PdfObject *pFont ) :
    PdfeFont( pFont )
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
        m_widthsCID.push_back( widthsA[i].GetReal() );
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
PdfeCIDString PdfeFontType3::toCIDString( const std::string& str ) const
{
    // Perform a simple copy.
    PdfeCIDString cidStr;
    cidStr.reserve( str.length() );
    for( size_t i = 0 ; i < str.length() ; ++i ) {
        cidStr.push_back( static_cast<pdf_cid>( str[i] ) );
    }
    return cidStr;
}
double PdfeFontType3::width( pdf_cid c ) const
{
    if( c >= m_firstCID && c <= m_lastCID ) {
        // Assume the letter is a square...
        double width = m_widthsCID[ static_cast<size_t>( c - m_firstCID ) ];
        PdfeVector cVect( width, width );
        cVect = cVect * m_fontMatrix;
        return cVect(0);
    }
    else {
        // Return 0 according to PDF Reference.
        return 0.;
    }
}
QChar PdfeFontType3::toUnicode( pdf_cid c ) const
{
    // TODO: unicode map.

    if( m_encoding ) {
        // Get Utf16 code from PdfEncoding object.
        return QChar( m_encoding->GetCharCode( c - m_firstCID ) );
    }
    else {
        // Assume some identity map...
        return QChar( c );
    }
}
PdfeFontSpace::Enum PdfeFontType3::isSpace( pdf_cid c ) const
{
    return PdfeFontSpace::None;
}

}
