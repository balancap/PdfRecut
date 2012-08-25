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

using namespace PoDoFo;

namespace PoDoFoExtended {

PdfeFontTrueType::PdfeFontTrueType( PoDoFo::PdfObject* pFont, FT_Library* ftLibrary ) :
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

    // Base font (required).
    m_baseFont = pFont->GetIndirectKey( "BaseFont" )->GetName();

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
void PdfeFontTrueType::init()
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

}
void PdfeFontTrueType::initSpaceCharacters()
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
void PdfeFontTrueType::initCharactersBBox( const PdfObject* pFont )
{
    // Font bounding box used for default height.
    PdfArray fontBBox = this->fontBBox();

    // Firt read characters widths given in font object.
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
    PdfeFontEmbedded  fontEmbedded = this->fontDescriptor().fontEmbedded();
    if( !fontEmbedded.fontFile2 && !fontEmbedded.fontFile3 ) {
        return;
    }

    // Get fontFile object.
    PdfObject* fontFile;
    if( fontEmbedded.fontFile2 ) {
        fontFile = fontEmbedded.fontFile2;
    }
    else {
        fontFile = fontEmbedded.fontFile3;
    }

    // Uncompress and copy into a buffer.
    char* buffer;
    long length;
    PdfStream* stream = fontFile->GetStream();
    stream->GetFilteredCopy( &buffer, &length );

    // Try to local font Face from the font file.
    int error;
    FT_Face face;
    error = FT_New_Memory_Face( *m_ftLibrary, reinterpret_cast<unsigned char*>( buffer ),
                                length, 0, &face );

    // Can not load: return...
    if( error ) {
        return;
    }

    long nbCharsU = 0;
    long nbCharsD = 0;

    // Try to build a map CID->GID.
    std::vector<pdf_gid>  mapCIDToGID( m_lastCID-m_firstCID+1, 0 );

    // Default unicode charmap selected: first try this way !
    if( face->charmap ) {

        QString qstr;
        QVector<uint> utf32str;
        pdf_gid glyph_idx;

        for( pdf_cid c = m_firstCID ; c <= m_lastCID ; ++c ) {
            // Get character UTF32 code.
            qstr.clear();
            qstr.append( this->toUnicode( c ) );
            utf32str = qstr.toUcs4();

            // Get glyph index: succeed if != 0
            glyph_idx = FT_Get_Char_Index( face, utf32str[0] );
            if( glyph_idx ) {
                mapCIDToGID[c-m_firstCID] = glyph_idx;
                nbCharsU++;
            }
        }
    }

    // In the case of a difference encoding: try a non-unicode charmap.
    PdfDifferenceEncoding* encoding = dynamic_cast<PdfDifferenceEncoding*>( m_encoding );
    if( encoding ) {
        // Try the first one the list !
        //error = FT_Set_Charmap( face, face->charmaps[0] );
        error = 0;
        if( !error ) {
            const PdfEncodingDifference& differences = encoding->GetDifferences();

            // Find characters defined in the encoding differences.
            PdfName name;
            pdf_utf16be code;
            pdf_gid glyph_idx;

            for( pdf_cid c = m_firstCID ; c <= m_lastCID ; ++c ) {
                if( differences.Contains( c, name, code ) ) {

                    glyph_idx = FT_Get_Name_Index( face, const_cast<char*>( name.GetName().c_str() ) );

                    //std::cout << "CID: " << c << " / " << name.GetName() << " / " << glyph_idx<< std::endl;
                    if( glyph_idx ) {
                        mapCIDToGID[c-m_firstCID] = glyph_idx;
                        nbCharsD++;
                    }
                }
            }
        }
    }

    // Get bounding box.
    for( pdf_cid c = m_firstCID ; c <= m_lastCID ; ++c ) {

        pdf_gid glyph_idx = mapCIDToGID[c-m_firstCID];
        if( glyph_idx ) {
            // Load glyph.
            error = FT_Load_Glyph( face, glyph_idx, FT_LOAD_NO_SCALE );
            if( error ) {
                continue;
            }

            // Get bounding box, computed using the outline of the glyph.
            FT_BBox glyph_bbox;
            error = FT_Outline_Get_BBox( &face->glyph->outline, &glyph_bbox );
            if( !error ) {
                // Use yMin and YMax to compute bottom and height.
                m_bboxCID[c - m_firstCID].SetBottom( glyph_bbox.yMin );
                m_bboxCID[c - m_firstCID].SetHeight( glyph_bbox.yMax - glyph_bbox.yMin );
            }
        }
    }

//    std::cout << m_baseFont.GetName()
//              << " (" << !fontEmbedded.fontFile << ") "
//              << nbCharsU << " | "
//              << nbCharsD << " / "
//              << (m_lastCID-m_firstCID+1) << " / "
//              << face->num_glyphs << " // "
//              << face->num_charmaps << "  "
//              << std::endl;

    // Free face object and font file buffer.
    FT_Done_Face( face );
    free( buffer );
}

PdfeFontTrueType::~PdfeFontTrueType()
{
    // Delete encoding object if necessary.
    if( m_encodingOwned ) {
        delete m_encoding;
    }
}

const PdfeFontDescriptor& PdfeFontTrueType::fontDescriptor() const
{
    return m_fontDescriptor;
}
PdfArray PdfeFontTrueType::fontBBox() const
{
    return m_fontDescriptor.fontBBox();
}

PdfeCIDString PdfeFontTrueType::toCIDString( const PdfString& str ) const
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
double PdfeFontTrueType::width( pdf_cid c, bool useFParams ) const
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
        width = ( width * m_fontSize + m_charSpace ) * ( m_hScale / 100. );
        if( this->isSpace( c ) == PdfeFontSpace::Code32 ) {
            width += m_wordSpace * ( m_hScale / 100. );
        }
    }
    return width;
}
PdfRect PdfeFontTrueType::bbox( pdf_cid c, bool useFParams ) const
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
QChar PdfeFontTrueType::toUnicode( pdf_cid c ) const
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
PdfeFontSpace::Enum PdfeFontTrueType::isSpace( pdf_cid c ) const
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
