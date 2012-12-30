/***************************************************************************
 * Copyright (C) Paul Balança - All Rights Reserved                        *
 *                                                                         *
 * NOTICE:  All information contained herein is, and remains               *
 * the property of Paul Balança. Dissemination of this information or      *
 * reproduction of this material is strictly forbidden unless prior        *
 * written permission is obtained from Paul Balança.                       *
 *                                                                         *
 * Written by Paul Balança <paul.balanca@gmail.com>, 2012                  *
 ***************************************************************************/

#include "PdfeFont.h"
#include "podofo/podofo.h"

#include <QsLog/QsLog.h>

#include <QFont>

#include FT_BBOX_H

using namespace PoDoFo;

namespace PoDoFoExtended {

//**********************************************************//
//                          PdfeFont                        //
//**********************************************************//
QDir PdfeFont::Standard14FontsDir;

const char* PdfeFont::Standard14FontNames[][10] =
{
    { "Times-Roman", "TimesNewRomanPSMT", "TimesNewRoman",
        "TimesNewRomanPS", NULL },
    { "Times-Bold", "TimesNewRomanPS-BoldMT", "TimesNewRoman,Bold",
        "TimesNewRomanPS-Bold", "TimesNewRoman-Bold", NULL },
    { "Times-Italic", "TimesNewRomanPS-ItalicMT", "TimesNewRoman,Italic",
        "TimesNewRomanPS-Italic", "TimesNewRoman-Italic", NULL },
    { "Times-BoldItalic", "TimesNewRomanPS-BoldItalicMT",
        "TimesNewRoman,BoldItalic", "TimesNewRomanPS-BoldItalic",
        "TimesNewRoman-BoldItalic", NULL },
    { "Helvetica", "ArialMT", "Arial", NULL },
    { "Helvetica-Bold", "Arial-BoldMT", "Arial,Bold", "Arial-Bold",
        "Helvetica,Bold", NULL },
    { "Helvetica-Oblique", "Arial-ItalicMT", "Arial,Italic", "Arial-Italic",
        "Helvetica,Italic", "Helvetica-Italic", NULL },
    { "Helvetica-BoldOblique", "Arial-BoldItalicMT",
        "Arial,BoldItalic", "Arial-BoldItalic",
        "Helvetica,BoldItalic", "Helvetica-BoldItalic", NULL },
    { "Courier", "CourierNew", "CourierNewPSMT", NULL },
    { "Courier-Bold", "CourierNew,Bold", "Courier,Bold",
        "CourierNewPS-BoldMT", "CourierNew-Bold", NULL },
    { "Courier-Oblique", "CourierNew,Italic", "Courier,Italic",
        "CourierNewPS-ItalicMT", "CourierNew-Italic", NULL },
    { "Courier-BoldOblique", "CourierNew,BoldItalic", "Courier,BoldItalic",
        "CourierNewPS-BoldItalicMT", "CourierNew-BoldItalic", NULL },
    { "Symbol", "Symbol,Italic", "Symbol,Bold", "Symbol,BoldItalic",
        "SymbolMT", "SymbolMT,Italic", "SymbolMT,Bold", "SymbolMT,BoldItalic", NULL },
    { "ZapfDingbats", NULL }
};

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
PdfeFont::PdfeFont( PdfeFont14Standard::Enum stdFontType, FT_Library ftLibrary ) :
    m_ftLibrary( NULL ), m_ftFace( NULL ),
    m_pEncoding( NULL ), m_encodingOwned( false )
{
    this->init();
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
    m_ftCharmapsIdx.resize( 3, -1 );

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
    // Delete encoding object if necessary.
    if( m_encodingOwned ) {
        delete m_pEncoding;
    }
}

PdfeVector PdfeFont::advance( const PdfeCIDString& str ) const
{
    PdfeVector advance;
    for( size_t i = 0 ; i < str.length() ; ++i ) {
        pdfe_cid c = str[i];
        advance += this->advance( c, false );

        // Space character 32: add word spacing.
        if( this->isSpace( c ) == PdfeFontSpace::Code32 ) {
            advance(0) += m_wordSpace / m_fontSize;
            // TODO: vertical fonts.
            //advance(1) += m_wordSpace / m_fontSize ;
        }
    }
    // Adjust using font parameters.
    advance(0) = advance(0) * m_fontSize * ( m_hScale / 100. );
    advance(0) += m_charSpace * str.length() * ( m_hScale / 100. );

    // TODO: vertical fonts.
    //advance(1) = advance(1) * m_fontSize;
    //advance(1) += m_charSpace * str.length();

    return advance;
}
PdfeVector PdfeFont::advance( const PdfString& str ) const
{
    return this->advance( this->toCIDString( str ) );
}

PdfRect PdfeFont::bbox( const PdfeCIDString& str ) const
{
    // Bounding box coordinates.
    double left = std::numeric_limits<double>::max();
    double bottom = std::numeric_limits<double>::max();
    double right = -std::numeric_limits<double>::max();
    double top = -std::numeric_limits<double>::max();

    PdfeVector advance;
    PdfRect cbbox;

    for( size_t i = 0 ; i < str.length() ; ++i ) {
        pdfe_cid c = str[i];
        // Glyph bounding box.
        cbbox = this->bbox( c, true );

        // Update string bounding box coordinates.
        left = std::min( left, advance(0) + cbbox.GetLeft() );
        bottom = std::min( bottom, advance(1) + cbbox.GetBottom() );
        right = std::max( right, advance(0) + cbbox.GetLeft() + cbbox.GetWidth() );
        top = std::max( top, advance(1) + cbbox.GetBottom() + cbbox.GetHeight() );

        // Update advance vector.
        advance = advance + this->advance( c, true );
    }
    // Got a problem! Default empty bounding box.
    if( left > right || bottom > top ) {
        return PdfRect( 0,0,0,0 );
    }
    return PdfRect( left, bottom, right-left, top-bottom );
}
PdfRect PdfeFont::bbox( const PdfString& str ) const
{
    return this->bbox( this->toCIDString( str ) );
}

// Default simple implementation using font bounding box.
PoDoFo::PdfRect PdfeFont::bbox( pdfe_cid c, bool useFParams ) const
{
    // Font BBox for default height.
    PdfRect fontBBox = this->fontBBox();

    // CID width.
    double width = this->advance( c, false )(0);

    // Default bottom and height.
    double bottom = fontBBox.GetBottom();
    double height = fontBBox.GetHeight();

    // Apply font parameters.
    PdfRect cbbox( 0.0, 0.0, width, height-bottom );
    if( useFParams ) {
        this->applyFontParameters( cbbox, this->isSpace( c ) == PdfeFontSpace::Code32 );
    }
    return cbbox;
}

QString PdfeFont::toUnicode(const PdfeCIDString& str, bool useUCMap, bool firstTryEncoding ) const
{
    // Default implementation: get the unicode string of every CID in the string..
    QString ustr;
    ustr.reserve( str.length() );
    for( size_t i = 0 ; i < str.length() ; ++i ) {
        ustr += this->toUnicode( str[i], useUCMap, firstTryEncoding );
    }
    return ustr;
}
QString PdfeFont::toUnicode(const PoDoFo::PdfString& str, bool useUCMap , bool firstTryEncoding ) const
{
    QString ustr;

    // Not empty unicode CMap : directly try this way (if allowed).
    if( !m_unicodeCMap.emptyCodeSpaceRange() && useUCMap && !firstTryEncoding ) {
         ustr = m_unicodeCMap.toUnicode( str );
    }
    // No result: try using Pdf encoding (not for Type 0 fonts). No need to retry CMap.
    if( !ustr.length() && this->type() != PdfeFontType::Type0 ) {
        ustr = this->toUnicode( this->toCIDString( str ), useUCMap, firstTryEncoding );
    }
    return ustr;
}
// Default implementation for simple fonts (Type 1, TrueType and Type 3).
QString PdfeFont::toUnicode( pdfe_cid c, bool useUCMap, bool firstTryEncoding ) const
{
    QString ustr;

    // Not empty unicode CMap : directly try this way (if allowed).
    if( !m_unicodeCMap.emptyCodeSpaceRange() && useUCMap && !firstTryEncoding ) {
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
    // Unicode CMap at the end...
    if( !m_unicodeCMap.emptyCodeSpaceRange() && !ustr.length() && useUCMap && firstTryEncoding ) {
        pdf_uint8 uint8_c( c ) ;
        PdfeCMap::CharCode charCode( uint8_c );
        ustr = m_unicodeCMap.toUnicode( charCode );
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
// Default implementation.
double PdfeFont::spaceWidth() const
{
    // Default value for Type 1, TrueType and Type 0 fonts.
    return 0.5;
}
// Default implementation.
double PdfeFont::spaceHeight() const
{
    // Default value for Type 1, TrueType and Type 0 fonts.
    return 0.5;
}
// Default implementation.
PdfeFont::Statistics PdfeFont::statistics( bool defaultValue ) const
{
    PdfeFont::Statistics stats;

    // Default values computed from standard 14 fonts.
    if( defaultValue ) {
        stats.meanAdvance = 0.5;
        stats.meanBBox = PdfRect( 0.0, 0.0, 0.45, 0.57 );
    }
    else {
        // TODO: case of Type 0 fonts.
        // Compute mean values.
        PdfeVector advance;
        double left = 0.0;
        double bottom = 0.0;
        double width = 0.0;
        double height = 0.0;
        size_t nbChars = 0;

        for( pdfe_cid c = 0 ; c <= 255 ; ++c ) {
            // Consider the character if GID not null and not a space char.
            pdfe_gid gid = this->fromCIDToGID( c );
            if( gid && ( this->isSpace( c ) == PdfeFontSpace::None ) ) {
                PdfRect bbox = this->bbox( c, false );
                if( bbox.GetWidth() > 0 && bbox.GetHeight() > 0 ) {
                    advance = advance + this->advance( c, false );

                    left += bbox.GetLeft();
                    bottom += bbox.GetBottom();
                    width += bbox.GetWidth();
                    height += bbox.GetHeight();

                    nbChars++;
                }
            }
        }
        stats.meanAdvance = advance.norm2() / nbChars;
        stats.meanBBox.SetLeft( left / nbChars );
        stats.meanBBox.SetBottom( bottom / nbChars );
        stats.meanBBox.SetWidth( width / nbChars );
        stats.meanBBox.SetHeight( height / nbChars );
//        stats.meanBBox = this->fontBBox();
//        stats.meanAdvance = stats.meanBBox.GetWidth();
    }
    return stats;
}

PdfName PdfeFont::fromCIDToName( pdfe_cid c , bool useEncoding, bool useDiffEncoding, bool useUnicodeCMap ) const
{
    PdfName cname;
    pdf_utf16be ucode;

    // Is the font encoding a difference encoding?
    PdfDifferenceEncoding* pDiffEncoding = dynamic_cast<PdfDifferenceEncoding*>( m_pEncoding );
    if( pDiffEncoding && useDiffEncoding ) {
        const PdfEncodingDifference& differences = pDiffEncoding->GetDifferences();
        if( differences.Contains( c, cname, ucode ) ) {
            // Name found!
            return cname;
        }
    }
    // Try using Pdf encoding and the UnicodeToName map.
    if( m_pEncoding && useEncoding ) {
        pdf_utf16be ucode = m_pEncoding->GetCharCode( c );
        cname = PdfDifferenceEncoding::UnicodeIDToName( ucode );

        // Check the name does no correspond to default PoDoFo construction.
        QString defName = QString("uni%1").arg( ucode, 4, 16, QLatin1Char('0') );
        if( defName.toStdString() != cname.GetName() ) {
            return cname;
        }
    }
    // Try using unicode CMap.
    if( !m_unicodeCMap.emptyCodeSpaceRange() && useUnicodeCMap ) {
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
    }
    // No name found...
    return PdfName();
}

void PdfeFont::initEncoding( PoDoFo::PdfObject* pEncodingObj )
{
    // Create encoding if necessary.
    if( pEncodingObj ) {
        m_pEncoding = const_cast<PdfEncoding*>( PdfEncodingObjectFactory::CreateEncoding( pEncodingObj ) );

        // According to PoDoFo implementation.
        m_encodingOwned = !pEncodingObj->IsName() || ( pEncodingObj->IsName() && (pEncodingObj->GetName() == PdfName("Identity-H")) );
    }
    else {
        m_pEncoding = NULL;
        m_encodingOwned = false;
    }
}
void PdfeFont::initEncoding( PdfEncoding* pEncoding, bool owned )
{
    m_pEncoding = pEncoding;
    m_encodingOwned = owned && pEncoding;
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
    PdfeFont14Standard::Enum stdFont14 = PdfeFont::isStandard14Font( fontDescriptor.fontName( false ).GetName() );

    // Embedded font program.
    if( fontDescriptor.fontEmbedded().fontFile() ) {
        // Copy font program in a buffer.
        char* pBuffer;
        long length;
        fontDescriptor.fontEmbedded().copyFontProgram( &pBuffer, &length );
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
    }
    // Standard font.
    else if( stdFont14 != PdfeFont14Standard::None ) {
        m_ftFaceData = PdfeFont::standard14FontData( stdFont14 );
    }
    // Font program not embedded in the PDF: try to load on the host system.
    else {
        // TODO.
        // See: http://lists.trolltech.com/qt-interest/2008-03/thread00445-0.html
        m_ftFace = NULL;
        m_ftFaceData.clear();

//#ifdef Q_WS_X11
//        std::string sfontname = fontDescriptor.fontName( false ).GetName();
//        QFont qfont( sfontname.c_str() );
//        qDebug() << sfontname.c_str() << ":"
//                 << qfont.exactMatch() << "/"
//                 << qfont.key();

//        std::string sfontname2 = "Times";
//        QFont qfont2( sfontname2.c_str(),  10, QFont::Bold);
//        qDebug() << sfontname2.c_str() << ":"
//                 << qfont2.exactMatch() << "/"
//                 << qfont2.key();
//#endif
    }
    // Load FreeType face from data buffer.
    int error;
    unsigned char* pData = reinterpret_cast<unsigned char*>( const_cast<char*>( m_ftFaceData.constData() ) );
    error = FT_New_Memory_Face( m_ftLibrary,
                                pData,
                                m_ftFaceData.size(), 0,
                                &m_ftFace );
    if( error ) {
        // Can not load: return...
        m_ftFace = NULL;
        m_ftFaceData.clear();
        this->initFTFaceCharmaps();
        return;
    }
    // Find charmaps.
    this->initFTFaceCharmaps();
}
void PdfeFont::initFTFace( QString filename )
{
    // Load FreeType face from data buffer.
    int error;
    m_ftFaceData.clear();
    error = FT_New_Face( m_ftLibrary,
                         filename.toLocal8Bit().constData(),
                         0,
                         &m_ftFace );

    if( error ) {
        // Can not load: return...
        m_ftFace = NULL;
        this->initFTFaceCharmaps();
        return;
    }
    this->initFTFaceCharmaps();
}
void PdfeFont::initFTFaceCharmaps()
{
    m_ftCharmapsIdx.resize( 3, -1 );
    if( !m_ftFace ) {
        return;
    }

    // Find charmaps...
    for( int i = 0 ; i < m_ftFace->num_charmaps ; i++ ) {
        FT_CharMap charmap = m_ftFace->charmaps[i];
        if( charmap->platform_id == 1 && charmap->encoding_id == 0 ) {
            m_ftCharmapsIdx[ FTCharmap10 ] = i;
        }
        else if( charmap->platform_id == 3 && charmap->encoding_id == 0 ) {
            m_ftCharmapsIdx[ FTCharmap30 ] = i;
        }
        else if( charmap->platform_id == 3 && charmap->encoding_id == 1 ) {
            m_ftCharmapsIdx[ FTCharmap31 ] = i;
        }
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
    size_t MaxNameLength = 25;

    QLOG_INFO() << QString( "<PdfeFont> %1 ;" ).arg( this->fontDescriptor().fontName( false ).GetName().c_str(), -MaxNameLength )
                   .toAscii().constData()
                << QString( "Subset (%1) ;" ).arg( this->fontDescriptor().isSubsetFont() )
                   .toAscii().constData()
                << QString( "Type (%1,%2) ;" ).arg( this->type() ).arg( this->subtype() )
                   .toAscii().constData()
                << QString( "Encoding (%1) ;" ).arg( bool( m_pEncoding ) )
                   .toAscii().constData()
                << QString( "UCMap (%1) ;" ).arg( bool( !m_unicodeCMap.emptyCodeSpaceRange() ) )
                   .toAscii().constData()
                << QString( "Symbolic (%1) ;" ).arg( bool( this->fontDescriptor().flags() & PdfeFontDescriptor::FlagSymbolic ) )
                   .toAscii().constData()
                << QString( "Font program (%1,%2)." ).arg( bool( m_ftFace ) || this->type() == PdfeFontType::Type3 )
                   .arg( bool( this->fontDescriptor().fontEmbedded().fontFile() ) )
                   .toAscii().constData();

//    if( this->type() == PdfeFontType::Type3 ) {
//        PdfeFont::Statistics stats = this->statistics();
//        qDebug() << QString( "Mean BBox : [ %1 ; %2 ; %3 ; %4 ]" )
//                    .arg( stats.meanBBox.GetLeft() )
//                    .arg( stats.meanBBox.GetBottom() )
//                    .arg( stats.meanBBox.GetWidth() )
//                    .arg( stats.meanBBox.GetHeight() )
//                 << QString( "Font BBox : [ %1 ; %2 ; %3 ; %4 ]" )
//                    .arg( this->fontBBox().GetLeft() )
//                    .arg( this->fontBBox().GetBottom() )
//                    .arg( this->fontBBox().GetWidth() )
//                    .arg( this->fontBBox().GetHeight() );
//    }
}

void PdfeFont::applyFontParameters( double& width, bool space32 ) const
{
    // Apply font parameters.
    width = ( width * m_fontSize + m_charSpace ) * ( m_hScale / 100. );
    if( space32 ) {
        width += m_wordSpace * ( m_hScale / 100. );
    }
}
void PdfeFont::applyFontParameters( PdfeVector& advance, bool space32 ) const
{
    // Apply font parameters.
    advance(0) = ( advance(0) * m_fontSize + m_charSpace ) * ( m_hScale / 100. );
    //advance(1) = advance(1) * m_fontSize + m_charSpace;
    if( space32 ) {
        advance(0) += m_wordSpace * ( m_hScale / 100. );
        //advance(1) += m_wordSpace;
    }
    // TODO: vertical fonts.
}
void PdfeFont::applyFontParameters( PdfRect& bbox, bool space32 ) const
{
    // On characters bounding box: does no use the char space.
    // Word space is used since it only applies on space characters.

    double width = bbox.GetWidth();
    width = width * m_fontSize * ( m_hScale / 100. );
    if( space32 ) {
        width += m_wordSpace * ( m_hScale / 100. );
    }
    bbox.SetWidth( width );

    bbox.SetLeft( bbox.GetLeft() * m_fontSize * ( m_hScale / 100. ) );
    bbox.SetBottom( bbox.GetBottom() * m_fontSize );
    bbox.SetHeight( bbox.GetHeight() * m_fontSize );
}

// Static functions used as interface with FreeType library.
PdfRect PdfeFont::ftGlyphBBox(FT_Face ftFace, pdfe_gid gid)
{
    // Glyph bounding box.
    PdfRect glyphBBox( 0, 0, 0, 0 );

    // Try to load the glyph.
    int error = FT_Load_Glyph( ftFace, gid, FT_LOAD_NO_SCALE );
    if( error ) {
        return glyphBBox;
    }
    // Scaling factor due to face units per EM.
    double scaling = 1000. / double( ftFace->units_per_EM );

    // Get bounding box using glyph metrics, in 1000 scale font units.
    FT_Glyph_Metrics metrics = ftFace->glyph->metrics;
    double left = double( metrics.horiBearingX ) * scaling;
    double right = double( metrics.horiBearingX + metrics.width ) * scaling;
    double bottom = double( metrics.horiBearingY - metrics.height ) * scaling;
    double top = double( metrics.horiBearingY ) * scaling;
//    double advance = double( metrics.horiAdvance ) * scaling;

    // Get bounding box, computed using the outline of the glyph.
//    FT_BBox glyph_bbox;
//    error = FT_Outline_Get_BBox( &ftFace->glyph->outline, &glyph_bbox );
//    double left = double( glyph_bbox.xMin ) * scaling;
//    double right = double( glyph_bbox.xMax ) * scaling;
//    double bottom = double( glyph_bbox.yMin ) * scaling;
//    double top = double( glyph_bbox.yMax ) * scaling;

    // Set glyph bounding box.
    glyphBBox.SetLeft( left );
    glyphBBox.SetWidth( right-left );
    glyphBBox.SetBottom( bottom );
    glyphBBox.SetHeight( top-bottom );

    return glyphBBox;
}
PdfRect PdfeFont::ftGlyphBBox( pdfe_gid gid )
{
    return PdfeFont::ftGlyphBBox( m_ftFace, gid );
}
PdfeFont::GlyphImage PdfeFont::ftGlyphRender( pdfe_gid gid, unsigned int charHeight, long resolution )
{
    // Static color table (always the same!).
    static QVector<QRgb> colorTable;
    if( !colorTable.size() ) {
        for( int i = 0 ; i < 256 ; ++i ) {
            colorTable << qRgba(0, 0, 0, i);
        }
    }
    // Set character size.
    int error = FT_Set_Char_Size( m_ftFace,
                                  0, charHeight * 64,
                                  0, resolution );
    // Error : return empty image.
    if( error ) {
        return GlyphImage();
    }
    // Load glyph and render it.
    error = FT_Load_Glyph( m_ftFace, gid, FT_LOAD_DEFAULT);
    if( error ) {
        return GlyphImage();
    }
    error = FT_Render_Glyph( m_ftFace->glyph, FT_RENDER_MODE_NORMAL);
    if( error ) {
        return GlyphImage();
    }
    // Create QImage from the glyph bitmap.
    GlyphImage glyph;
    glyph.image = QImage( m_ftFace->glyph->bitmap.buffer,
                          m_ftFace->glyph->bitmap.width,
                          m_ftFace->glyph->bitmap.rows,
                          m_ftFace->glyph->bitmap.pitch,
                          QImage::Format_Indexed8 );
    glyph.image.setColorTable(colorTable);
    // TODO: set transformation matrix.
    // pixel_size = point_size * resolution / 72
    // pixel_coord = grid_coord * pixel_size / EM_size

    return glyph;
}

pdfe_gid PdfeFont::ftGIDFromCharCode( pdfe_cid charCode, bool charmap30 ) const
{
    pdfe_gid gid( 0 );
    gid = FT_Get_Char_Index( m_ftFace , charCode );

    // In case of TrueType font + (3,0) CMap.
    if( !gid && charmap30 ) {
        gid = FT_Get_Char_Index( m_ftFace, 0xf000 + charCode );
        if( !gid ) {
            gid = FT_Get_Char_Index( m_ftFace, 0xf100 + charCode );
        }
        if( !gid ) {
            gid = FT_Get_Char_Index( m_ftFace, 0xf200 + charCode );
        }
    }
    // Tweak from MuPDF...
    // some chinese fonts only ship the similarly looking 0x2026.
    if( !gid && charCode == 0x22ef ) {
        gid = FT_Get_Char_Index( m_ftFace, 0x2026 );
    }
    return gid;
}
pdfe_gid PdfeFont::ftGIDFromName( const PdfName& charName ) const
{
    pdfe_gid gid( 0 );
    if( charName.GetLength() ) {
        gid = FT_Get_Name_Index( m_ftFace, const_cast<char*>( charName.GetName().c_str() ) );
    }
    return gid;
}

PdfeFont14Standard::Enum PdfeFont::isStandard14Font( const std::string& fontName )
{
    // Find the font name in the list.
    for( int i = 0 ; i < 14 ; ++i ) {
        for( int j = 0 ; Standard14FontNames[i][j] ; ++j ) {
            if( fontName == Standard14FontNames[i][j] ) {
                return PdfeFont14Standard::Enum( i );
            }
        }
    }
    return PdfeFont14Standard::None;
}
std::string PdfeFont::standard14FontName( PdfeFont14Standard::Enum stdFontType )
{
    return Standard14FontNames[ stdFontType ][0];
}
QString PdfeFont::standard14FontPath( PdfeFont14Standard::Enum stdFontType )
{
    // Filename.
    QString filename;
    switch( stdFontType )
    {
    case PdfeFont14Standard::TimesRoman:
        filename = "NimbusRomNo9L-Regu.cff";
        break;
    case PdfeFont14Standard::TimesBold:
        filename = "NimbusRomNo9L-Medi.cff";
        break;
    case PdfeFont14Standard::TimesItalic:
        filename = "NimbusRomNo9L-ReguItal.cff";
        break;
    case PdfeFont14Standard::TimesBoldItalic:
        filename = "NimbusRomNo9L-MediItal.cff";
        break;
    case PdfeFont14Standard::Helvetica:
        filename = "NimbusSanL-Regu.cff";
        break;
    case PdfeFont14Standard::HelveticaBold:
        filename = "NimbusSanL-Bold.cff";
        break;
    case PdfeFont14Standard::HelveticaOblique:
        filename = "NimbusSanL-ReguItal.cff";
        break;
    case PdfeFont14Standard::HelveticaBoldOblique:
        filename = "NimbusSanL-BoldItal.cff";
        break;
    case PdfeFont14Standard::Courier:
        filename = "NimbusMonL-Regu.cff";
        break;
    case PdfeFont14Standard::CourierBold:
        filename = "NimbusMonL-Bold.cff";
        break;
    case PdfeFont14Standard::CourierOblique:
        filename = "NimbusMonL-ReguObli.cff";
        break;
    case PdfeFont14Standard::CourierBoldOblique:
        filename = "NimbusMonL-BoldObli.cff";
        break;
    case PdfeFont14Standard::Symbol:
        filename = "StandardSymL.cff";
        break;
    case PdfeFont14Standard::ZapfDingbats:
        filename = "Dingbats.cff";
        break;
    case PdfeFont14Standard::None:
        return QString();
    }
    return Standard14FontsDir.absoluteFilePath( filename );
}
QByteArray PdfeFont::standard14FontData( PdfeFont14Standard::Enum stdFontType )
{
    // Static data vector.
    static std::vector<QByteArray> stdFontsData( 14, QByteArray() );

    // Data already loaded...
    if( stdFontsData[ stdFontType ].size() ) {
        return stdFontsData[ stdFontType ];
    }
    // Else: load file.
    QString fontpath = PdfeFont::standard14FontPath( stdFontType );
    QFile fontfile( fontpath );
    if (!fontfile.open( QIODevice::ReadOnly ) ) {
        return QByteArray();
    }
    stdFontsData[ stdFontType ] = fontfile.readAll();
    fontfile.close();
    return stdFontsData[ stdFontType ];
}

const std::vector<QChar>& PdfeFont::spaceCharacters()
{
    // Static variable containing space elements.
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
