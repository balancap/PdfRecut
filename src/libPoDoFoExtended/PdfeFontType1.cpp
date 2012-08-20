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

using namespace PoDoFo;

namespace PoDoFoExtended {

PdfeFontType1::PdfeFontType1( PoDoFo::PdfObject* pFont, FT_Library* ftLibrary ) :
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

    // Character size.
    this->initCharactersSize( pFont );
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

    m_encoding = NULL;
    m_encodingOwned = false;
    m_spaceCharacters.clear();
}
void PdfeFontType1::initStandard14Font( const PoDoFo::PdfObject* pFont )
{
    // Get PoDoFo Metrics object.
    PdfFontMetricsBase14* pMetrics = PODOFO_Base14FontDef_FindBuiltinData( m_baseFont.GetName().c_str() );

    // Can retrieve: widths, symbol, ascent, descent, xHeight, capHeight, BBox.
    m_fontDescriptor.setAscent( pMetrics->GetAscent() );
    m_fontDescriptor.setDescent( pMetrics->GetDescent() );
    m_fontDescriptor.setCapHeight( pMetrics->GetCapHeight() );

    PdfArray bbox;
    pMetrics->GetBoundingBox( bbox );
    m_fontDescriptor.setFontBBox( bbox );

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
void PdfeFontType1::initCharactersSize( const PdfObject* pFont )
{
    // Is the font embedded ?
    PdfeFontEmbedded  fontEmbedded = this->fontDescriptor().fontEmbedded();
    if( !fontEmbedded.fontFile && !fontEmbedded.fontFile3 ) {
        std::cout << "FT_Face " << m_baseFont.GetName() << " : No font file.";
        return;
    }

    // Get fontFile object.
    PdfObject* fontFile;
    if( fontEmbedded.fontFile ) {
        fontFile = fontEmbedded.fontFile;
    }
    else {
        fontFile = fontEmbedded.fontFile3;
    }

    // Get the buffer.
    char* buffer;
    long length;
    PdfStream* stream = fontFile->GetStream();
    stream->GetFilteredCopy( &buffer, &length );

    // Load Font Face from the font file.
    FT_Face face;
    int error;

    error = FT_New_Memory_Face( *m_ftLibrary,
                                reinterpret_cast<unsigned char*>( buffer ),
                                length, 0, &face );


    std::cout << "FT_Face " << m_baseFont.GetName() << " : ";
    if( error ) {
        std::cout << "Error.";
        PODOFO_RAISE_ERROR( ePdfError_FreeType );
    }
    else {
        std::cout << "Ok : " << face->num_glyphs << " / " << (m_lastCID-m_firstCID+1);
    }

//    for(size_t) {
//        FT_Get_Char_Index( face, charcode );
//    }

    std::cout << std::endl;


//    // Initialize freetype library.
//    if( FT_Init_FreeType( &m_ftLibrary ) ) {
//        PODOFO_RAISE_ERROR( ePdfError_FreeType );
//    }

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
        width = m_widthsCID[ static_cast<size_t>( c - m_firstCID ) ] / 1000.;
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
QChar PdfeFontType1::toUnicode( pdf_cid c ) const
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
