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

#include "PdfeFont.h"
#include "podofo/podofo.h"

#include <QsLog/QsLog.h>

#include FT_BBOX_H

using namespace PoDoFo;

namespace PoDoFoExtended {

//**********************************************************//
//                          PdfeFont                        //
//**********************************************************//
PdfeFont::PdfeFont( PdfObject* pFont, FT_Library ftLibrary ) :
    m_ftLibrary( NULL ), m_ftFace( NULL ),
    m_pEncoding( NULL ), m_encodingOwned( false )
{
    this->init();

    // Check if the PdfObject is a font dictionary.
    if( pFont && pFont->IsDictionary() && pFont->GetDictionary().HasKey( PdfName::KeyType ) ) {
        const PdfName& rType = pFont->GetDictionary().GetKey( PdfName::KeyType )->GetName();
        if( rType != PdfName( "Font" ) ) {
            PODOFO_RAISE_ERROR( ePdfError_InvalidDataType );
        }
    }
    else {
        PODOFO_RAISE_ERROR( ePdfError_InvalidDataType );
    }
    m_ftLibrary = ftLibrary;
}
void PdfeFont::init()
{
    // Destroy objects if necessary.
    if( m_pEncoding && m_encodingOwned ) {
        delete m_pEncoding;
    }
    if( m_ftFace ) {
        FT_Done_Face( m_ftFace );
    }

    // Font Type and Subtype.
    m_type = PdfeFontType::Unknown;
    m_subtype = PdfeFontSubType::Unknown;

    // Default values on font parameters.
    m_fontSize = 1.0;
    m_charSpace = 0.0;
    m_wordSpace = 0.0;
    m_hScale = 0.0;

    // Common elements shared by fonts.
    m_ftLibrary = NULL;
    m_ftFace = NULL;
    m_ftFaceData.clear();
    m_pEncoding = NULL;
    m_encodingOwned = false;
    m_unicodeCMap.init();
    m_spaceCharacters.clear();
}
PdfeFont::~PdfeFont()
{
    // Destroy FT_Face if necessary.
    if( m_ftFace ) {
        FT_Done_Face( m_ftFace );
    }
}

double PdfeFont::width( const PdfeCIDString& str ) const
{
    double width = 0;
    for( size_t i = 0 ; i < str.length() ; ++i ) {
        pdfe_cid c = str[i];
        width += this->width( c, false );

        // Space character 32: add word spacing.
        if( this->isSpace( c ) == PdfeFontSpace::Code32 ) {
            width += m_wordSpace / m_fontSize;
        }
    }
    // Adjust using font parameters.
    width = width * m_fontSize * ( m_hScale / 100. );
    width += m_charSpace * str.length() * ( m_hScale / 100. );

    return width;
}
double PdfeFont::width( const PoDoFo::PdfString& str ) const
{
    return this->width( this->toCIDString( str ) );
}

QString PdfeFont::toUnicode( const PdfeCIDString& str , bool useUCMap ) const
{
    // Default implementation: get the unicode string of every CID in the string..
    QString ustr;
    ustr.reserve( str.length() );
    for( size_t i = 0 ; i < str.length() ; ++i ) {
        ustr += this->toUnicode( str[i], useUCMap );
    }
    return ustr;
}
QString PdfeFont::toUnicode( const PoDoFo::PdfString& str , bool useUCMap ) const
{
    QString ustr;

    // Not empty unicode CMap : directly try this way (if allowed).
    if( !m_unicodeCMap.emptyCodeSpaceRange() && useUCMap ) {
         ustr = m_unicodeCMap.toUnicode( str );
    }
    // No result: try using Pdf encoding (not for Type 0 fonts). No need to retry CMap.
    if( !ustr.length() && this->type() != PdfeFontType::Type0 ) {
        ustr = this->toUnicode( this->toCIDString( str ), false );
    }
    return ustr;
}

// Default implementation for simple fonts (Type 1, TrueType and Type 3).
QString PdfeFont::toUnicode( pdfe_cid c, bool useUCMap ) const
{
    QString ustr;

    // Not empty unicode CMap : directly try this way (if allowed).
    if( !m_unicodeCMap.emptyCodeSpaceRange() && useUCMap ) {
        //pdf_uint16 code = PDFE_UTF16BE_HBO( c );
        //PdfeCMap::CharCode charCode( code );

        // Create PdfeCMap::CharCode from CID (convert it back to pdf_uint8).
        pdf_uint8 uint8_c( c ) ;
        PdfeCMap::CharCode charCode( uint8_c );
        ustr = m_unicodeCMap.toUnicode( charCode );
    }
    // No result: Pdf encoding (not for Type 0 fonts).
    if( m_pEncoding && !ustr.length() && this->type() != PdfeFontType::Type0 ) {
        // Get UTF16 code from PdfEncoding object.
        pdf_utf16be ucode = m_pEncoding->GetCharCode( c );
        ucode = PDFE_UTF16BE_HBO( ucode );
        return QString::fromUtf16( &ucode, 1 );
    }
    // Might be empty...
    return ustr;
}
// Default implementation for simple fonts (Type 1, TrueType and Type 3).
PdfeCIDString PdfeFont::toCIDString( const PdfString& str ) const
{
    // PDF String data.
    const char* pstr = str.GetString();
    size_t length = str.GetLength();

    // Perform a simple copy of string bytes.
    PdfeCIDString cidstr;
    cidstr.resize( length, 0 );
    for( size_t i = 0 ; i < length ; ++i ) {
        cidstr[i] = static_cast<unsigned char>( pstr[i] );
    }
    return cidstr;
}
// Default simple implementation using font bounding box.
PoDoFo::PdfRect PdfeFont::bbox( pdfe_cid c, bool useFParams ) const
{
    // Font BBox for default height.
    PdfRect fontBBox = this->fontBBox();

    // CID width.
    double width = this->width( c, false );

    // Default bottom and height.
    double bottom = fontBBox.GetBottom();
    double height = fontBBox.GetHeight();

    // Apply font parameters.
    PdfRect cbbox( 0.0, bottom, width, height );
    if( useFParams ) {
        this->applyFontParameters( cbbox, this->isSpace( c ) == PdfeFontSpace::Code32 );
    }
    return cbbox;
}
// Default implementation.
PdfeFontSpace::Enum PdfeFont::isSpace( pdfe_cid c ) const
{
    // Does the character belongs to the space characters vector ?
    for( size_t i = 0 ; i < m_spaceCharacters.size() ; ++i ) {
        if( c == m_spaceCharacters[i].first ) {
            return m_spaceCharacters[i].second;
        }
    }
    return PdfeFontSpace::None;
}

void PdfeFont::initEncoding( PoDoFo::PdfObject* pEncodingObj )
{
    // Create encoding if necessary.
    if( pEncodingObj ) {
        m_pEncoding = const_cast<PdfEncoding*>( PdfEncodingObjectFactory::CreateEncoding( pEncodingObj ) );

        // According to PoDoFo implementation.
        m_encodingOwned = !pEncodingObj->IsName() || ( pEncodingObj->IsName() && (pEncodingObj->GetName() == PdfName("Identity-H")) );
    }
}
void PdfeFont::initUnicodeCMap( PdfObject* pUCMapObj )
{
    // Read unicode CMap (must have a stream!).
    if( pUCMapObj && pUCMapObj->HasStream() ) {
        // Initialize CMap object.
        m_unicodeCMap.init( pUCMapObj );

        // Save CMap (Debug...)
//        std::string path( "./cmaps/" );
//        path += this->fontDescriptor().fontName().GetName();
//        path += ".txt";

//        PdfOutputDevice outFile( path.c_str() );
//        PdfMemStream* pStream = dynamic_cast<PdfMemStream*>( pUCMapObj->GetStream() );
//        pStream->Uncompress();
//        pStream->Write( &outFile );
    }
}
void PdfeFont::initFTFace( const PdfeFontDescriptor& fontDescriptor )
{
    // Embedded font program.
    if( fontDescriptor.fontEmbedded().fontFile() ) {
        // Copy font program in a buffer.
        char* pBuffer;
        long length;
        fontDescriptor.fontEmbedded().fontProgram( &pBuffer, &length );
        if( !pBuffer ) {
            // No font program found...
            m_ftFace = NULL;
            m_ftFaceData.clear();
            return;
        }

        // Copy data into the member buffer.
        m_ftFaceData.clear();
        m_ftFaceData.append( pBuffer, length );
        free( pBuffer );

        // Load FreeType face from data buffer.
        int error;
        error = FT_New_Memory_Face( m_ftLibrary,
                                    reinterpret_cast<unsigned char*>( m_ftFaceData.data() ),
                                    m_ftFaceData.size(), 0,
                                    &m_ftFace );
        if( error ) {
            // Can not load: return...
            m_ftFace = NULL;
            m_ftFaceData.clear();
            return;
        }
    }
    // Font program not embedded in the PDF: try to load on the host system.
    else {
        // TODO.
        m_ftFace = NULL;
        m_ftFaceData.clear();
    }
}
void PdfeFont::initSpaceCharacters( pdfe_cid firstCID, pdfe_cid lastCID, bool clearContents )
{
    // Clear content if necessary.
    if( clearContents ) {
        m_spaceCharacters.clear();
    }

    // Get the vector of predefined space characters.
    const std::vector<QChar>& spaceChars = PdfeFont::spaceCharacters();

    // Check CID for spaces.
    for( pdfe_cid c = firstCID ; c <= lastCID ; ++c ) {
        QString ustr = this->toUnicode( c );

        // Specific case of the code 32 space character.
        if( ustr.length() == 1 && ustr[0] == spaceChars[0] &&
            this->type() != PdfeFontType::Type0 ) {
            m_spaceCharacters.push_back(
                        std::pair<pdfe_cid,PdfeFontSpace::Enum>( c, PdfeFontSpace::Code32 ) );
        }
        else if( ustr.length() ) {
            bool isSpaceChar = true;
            // Check every QChar in the string is a space.
            for( int j = 0 ; j < ustr.length() ; ++j ) {
                if( std::find( spaceChars.begin(), spaceChars.end(), ustr[j] ) == spaceChars.end() ) {
                    isSpaceChar = false;
                    break;
                }
            }
            if( isSpaceChar ) {
                m_spaceCharacters.push_back(
                            std::pair<pdfe_cid,PdfeFontSpace::Enum>( c, PdfeFontSpace::Other ) );
            }
        }
    }
}
void PdfeFont::initLogInformation()
{
    QLOG_INFO() << QString( "PdfeFont:%1 ;" ).arg( this->fontDescriptor().fontName().GetName().c_str(), 30 ).toAscii().constData()
                << QString( "Type (%1,%2) ;" ).arg( this->type() ).arg( this->subtype() ).toAscii().constData()
                << QString( "Encoding (%1) ;" ).arg( bool( m_pEncoding ) ).toAscii().constData()
                << QString( "Unicode CMap (%1) ;" ).arg( bool( !m_unicodeCMap.emptyCodeSpaceRange() ) ).toAscii().constData()
                << QString( "Embedded font program (%1)." ).arg( bool( this->fontDescriptor().fontEmbedded().fontFile() ) ).toAscii().constData();
}

PdfName PdfeFont::fromCIDToName( pdfe_cid c ) const
{
    PdfName cname;
    pdf_utf16be ucode;

    // Is the font encoding a difference encoding?
    PdfDifferenceEncoding* pDiffEncoding = dynamic_cast<PdfDifferenceEncoding*>( m_pEncoding );
    if( pDiffEncoding ) {
        const PdfEncodingDifference& differences = pDiffEncoding->GetDifferences();
        if( differences.Contains( c, cname, ucode ) ) {
            // Name found!
            return cname;
        }
    }
    // Else: try using the unicode code of c.
    QString ustr = this->toUnicode( c, true );
    if( ustr.length() == 1 ) {
        ucode = ustr[0].unicode();
        ucode = PDFE_UTF16BE_HBO( ucode );
        cname = PdfDifferenceEncoding::UnicodeIDToName( ucode );

        // Check the name does no correspond to default PoDoFo construction.
        QString defName = QString("uni%1").arg( ucode, 4, 16, QLatin1Char('0') );
        if( defName.toStdString() != cname.GetName() ) {
            return cname;
        }
    }
    return PdfName();
}
pdfe_gid PdfeFont::fromCIDToGID( pdfe_cid c ) const
{
    // No FreeType face loaded: return 0 GID.
    if( !m_ftFace ) {
        return 0;
    }

    // Set a CharMap: keep the default one if selected.
    if( !m_ftFace->charmap ) {
        FT_Set_Charmap( m_ftFace, m_ftFace->charmaps[0] );
    }

    // Get the glyph index of the character.
    PdfName cname;
    pdf_utf16be ucode( 0 );
    pdfe_gid gid( 0 );

    // First try using the character name, obtain from its CID.
    cname = this->fromCIDToName( c );
    if( cname.GetLength() ) {
        gid = FT_Get_Name_Index( m_ftFace, const_cast<char*>( cname.GetName().c_str() ) );
        if( gid ) {
            return gid;
        }
    }

    // Unicode of the character.
    QString ustr = this->toUnicode( c );
    if( ustr.length() == 1 ) {
        ucode = ustr[0].unicode();
    }
    // No glyph index yet: try using the character code and the different charmaps.
    for( int n = 0; n < m_ftFace->num_charmaps && !gid ; n++ ) {
        FT_Set_Charmap( m_ftFace, m_ftFace->charmaps[n] );
        // Unicode first...
        if( ucode ) {
            gid = FT_Get_Char_Index( m_ftFace, PDFE_UTF16BE_HBO( ucode ) );
        }
        // Also try character code.
        if( !gid ) {
            gid = FT_Get_Char_Index( m_ftFace, c );
        }
    }
    return gid;
}

void PdfeFont::applyFontParameters( double& width, bool space32 ) const
{
    // Apply font parameters.
    width = ( width * m_fontSize + m_charSpace ) * ( m_hScale / 100. );
    if( space32 ) {
        width += m_wordSpace * ( m_hScale / 100. );
    }
}
void PdfeFont::applyFontParameters( PdfRect& bbox, bool space32 ) const
{
    double width = bbox.GetWidth();
    width = ( width * m_fontSize + m_charSpace ) * ( m_hScale / 100. );
    if( space32 ) {
        width += m_wordSpace * ( m_hScale / 100. );
    }
    bbox.SetWidth( width );
    bbox.SetBottom( bbox.GetBottom() * m_fontSize );
    bbox.SetHeight( bbox.GetHeight() * m_fontSize );
}

PdfRect PdfeFont::ftGlyphBBox( FT_Face ftFace, pdfe_gid glyph_idx, const PdfRect& fontBBox )
{
    // Glyph bounding box.
    PdfRect glyphBBox( 0, 0, 0, 0 );

    // Try to load the glyph.
    int error = FT_Load_Glyph( ftFace, glyph_idx, FT_LOAD_NO_SCALE );
    if( error ) {
        return glyphBBox;
    }
    // Get bounding box, computed using the outline of the glyph.
//    FT_BBox glyph_bbox;
//    error = FT_Outline_Get_BBox( &face->glyph->outline, &glyph_bbox );
//    double bottom = glyph_bbox.yMin;
//    double top = glyph_bbox.yMax;

    // Scaling factor due to face units per EM.
    double scaling = 1000. / double( ftFace->units_per_EM );

    // Get bounding box using glyph metrics, in 1000 scale font units.
    FT_Glyph_Metrics metrics = ftFace->glyph->metrics;
    double left = double( metrics.horiBearingX ) * scaling;
    double right = double( metrics.horiBearingX + metrics.width ) * scaling;
    double bottom = double( metrics.horiBearingY - metrics.height ) * scaling;
    double top = double( metrics.horiBearingY ) * scaling;
    //double advance = double( metrics.horiAdvance ) * scaling;

    // Perform some corrections using font bounding box.
    if( fontBBox.GetWidth() > 0  && fontBBox.GetHeight() > 0 ) {
        left = std::max( fontBBox.GetLeft(), left );
        right = std::min( fontBBox.GetLeft()+fontBBox.GetWidth(), right );
        bottom = std::max( fontBBox.GetBottom(), bottom );
        top = std::min( fontBBox.GetBottom()+fontBBox.GetHeight(), top );
    }

    // Set glyph bounding box.
    glyphBBox.SetLeft( left );
    glyphBBox.SetWidth( right-left );
    glyphBBox.SetBottom( bottom );
    glyphBBox.SetHeight( top-bottom );

    return glyphBBox;
}

PdfeFont::GlyphImage PdfeFont::ftGlyphRender( FT_Face ftFace, pdfe_gid glyph_idx,
                                              unsigned int charHeight, long resolution )
{
    // Static color table (always the same!).
    static QVector<QRgb> colorTable;
    if( !colorTable.size() ) {
        for( int i = 0 ; i < 256 ; ++i ) {
            colorTable << qRgba(0, 0, 0, i);
        }
    }

    // Set character size.
    int error = FT_Set_Char_Size( ftFace,
                                  0, charHeight * 64,
                                  0, resolution );

    // Error : return empty image.
    if( error ) {
        return GlyphImage();
    }

    // Load glyph and render it.
    error = FT_Load_Glyph( ftFace, glyph_idx, FT_LOAD_DEFAULT);
    if( error ) {
        return GlyphImage();
    }
    error = FT_Render_Glyph( ftFace->glyph, FT_RENDER_MODE_NORMAL);
    if( error ) {
        return GlyphImage();
    }

    // Create QImage from the glyph bitmap.
    GlyphImage glyph;
    glyph.image = QImage( ftFace->glyph->bitmap.buffer,
                       ftFace->glyph->bitmap.width,
                       ftFace->glyph->bitmap.rows,
                       ftFace->glyph->bitmap.pitch,
                       QImage::Format_Indexed8 );
    glyph.image.setColorTable(colorTable);

    // TODO: set transformation matrix.
    // pixel_size = point_size * resolution / 72
    // pixel_coord = grid_coord * pixel_size / EM_size

    return glyph;
}

const std::vector<QChar>& PdfeFont::spaceCharacters()
{
    // Static variable containing elements.
    static std::vector<QChar> spaceChars;

    // Insert characters in the vector if empty (should happen once).
    if( !spaceChars.size() ) {
        spaceChars.push_back( QChar( 0x0020 ) );    // Space
        spaceChars.push_back( QChar( 0x0009 ) );    // Tabular
        spaceChars.push_back( QChar( 0x000d ) );    // Carriage return
        spaceChars.push_back( QChar( 0x00a0 ) );    // No break space
    }
    return spaceChars;
}

}
