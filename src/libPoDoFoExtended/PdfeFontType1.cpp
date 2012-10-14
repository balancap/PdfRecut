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

#include FT_BBOX_H
#include FT_GLYPH_H

using namespace PoDoFo;

namespace PoDoFoExtended {

QDir PdfeFontType1::Standard14FontsPath;

PdfeFontType1::PdfeFontType1( PoDoFo::PdfObject* pFont, FT_Library ftLibrary ) :
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

    // Standard 14 font.
    if( PdfeFontType1::IsStandard14Font( pFont ) ) {
        this->initStandard14Font( pFont );
        return;
    }

    // Need the following entries in the dictionary.
    PdfObject* pFChar = pFont->GetIndirectKey( "FirstChar" );
    PdfObject* pLChar = pFont->GetIndirectKey( "LastChar" );
    PdfObject* pWidths = pFont->GetIndirectKey( "Widths" );
    PdfObject* pDescriptor = pFont->GetIndirectKey( "FontDescriptor" );

    // If does not exist: must be a 14 standard font.
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
    // Space characters.
    this->initSpaceCharacters( m_firstCID, m_lastCID, true );

    // FreeType font face.
    this->initFTFace( m_fontDescriptor );
    // Characters bounding box.
    this->initCharactersBBox( pFont );

    // Log font information.
    this->initLogInformation();
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
    m_bboxCID.clear();
}
void PdfeFontType1::initCharactersBBox( const PdfObject* pFont )
{
    // Font bounding box used for default height.
    PdfRect fontBBox = m_fontDescriptor.fontBBox();

    // First read characters widths given in font object and set default bbox.
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

     // Get glyph bounding box.
    for( pdfe_cid c = m_firstCID ; c <= m_lastCID ; ++c ) {
        // Not a space character.
        if( this->isSpace( c ) == PdfeFontSpace::None ) {
            // Glyph ID.
            pdfe_gid gid = this->fromCIDToGID( c );
            if( gid ) {
                PdfRect glyphBBox = this->ftGlyphBBox( m_ftFace, gid, fontBBox );
                if( glyphBBox.GetWidth() > 0 && glyphBBox.GetHeight() > 0 ) {
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
void PdfeFontType1::initStandard14Font( const PoDoFo::PdfObject* pFont )
{
    // Read base font (required for standard font!).
    m_baseFont = pFont->GetIndirectKey( "BaseFont" )->GetName();

    // Get PoDoFo Metrics object and set metrics paramters.
    PdfFontMetricsBase14* pMetrics = PODOFO_Base14FontDef_FindBuiltinData( m_baseFont.GetName().c_str() );
    pMetrics->SetFontSize( 1.0 );
    pMetrics->SetFontScale( 100.0 );
    pMetrics->SetFontCharSpace( 0.0 );

    // Can retrieve: widths, symbol, ascent, descent, xHeight, capHeight, BBox.
    m_fontDescriptor.setFontName( m_baseFont );
    m_fontDescriptor.setAscent( pMetrics->GetAscent() );
    m_fontDescriptor.setDescent( pMetrics->GetDescent() );
    m_fontDescriptor.setCapHeight( pMetrics->GetCapHeight() );

    PdfArray fontBBox;
    pMetrics->GetBoundingBox( fontBBox );
    m_fontDescriptor.setFontBBox( fontBBox );

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
        this->initEncoding( pEncoding );
    }
    else if( !pMetrics->IsSymbol() ) {
        m_pEncoding = const_cast<PdfEncoding*>( PdfEncodingFactory::GlobalStandardEncodingInstance() );
        m_encodingOwned = false;
    }
    else if( strcmp( m_baseFont.GetName().c_str(), "Symbol" ) == 0 ) {
        m_pEncoding = const_cast<PdfEncoding*>( PdfEncodingFactory::GlobalSymbolEncodingInstance() );
        m_encodingOwned = false;
    }
    else if( strcmp( m_baseFont.GetName().c_str(), "ZapfDingbats" ) == 0 ) {
        m_pEncoding = const_cast<PdfEncoding*>( PdfEncodingFactory::GlobalZapfDingbatsEncodingInstance() );
        m_encodingOwned = false;
    }
    // Unicode CMap.
    PdfObject* pUnicodeCMap = pFont->GetIndirectKey( "ToUnicode" );
    this->initUnicodeCMap( pUnicodeCMap );

    // FreeType font face.
    QString filename = filenameStandard14Font( m_baseFont.GetName() );
    this->initFTFace( filename );

    // Construct widths array using the font encoding.
    m_firstCID = m_pEncoding->GetFirstChar();
    m_lastCID = m_pEncoding->GetLastChar();

    pdf_utf16be ucode;
    double widthCID;
    for( pdfe_cid c = m_firstCID ; c <= m_lastCID ; ++c ) {
        ucode = m_pEncoding->GetCharCode( c );

        // Dumb bug in PoDoFo: why bytes are inverted in GetCharCode but not UnicodeCharWidth ???
        ucode = PDFE_UTF16BE_HBO( ucode );

        widthCID = pMetrics->UnicodeCharWidth( ucode ) * 1000.0;
        m_widthsCID.push_back( widthCID );
        m_bboxCID.push_back( PdfRect( 0, 0, widthCID, fontBBox[3].GetReal() ) );

        // Get bounding box from FTFace.
        if( this->isSpace( c ) == PdfeFontSpace::None ) {
            // Glyph ID.
            pdfe_gid gid = this->fromCIDToGID( c );
            if( gid ) {
                PdfRect glyphBBox = this->ftGlyphBBox( m_ftFace, gid, fontBBox );
                if( glyphBBox.GetWidth() > 0 && glyphBBox.GetHeight() > 0 ) {
                    m_bboxCID.back().SetBottom( glyphBBox.GetBottom() );
                    m_bboxCID.back().SetHeight( glyphBBox.GetHeight() );
                }
            }
        }
        else {
            m_bboxCID[c - m_firstCID].SetBottom( 0.0 );
            m_bboxCID[c - m_firstCID].SetHeight( this->spaceHeight() );
        }
    }

    // Space characters.
    this->initSpaceCharacters( m_firstCID, m_lastCID, true );

    // Log font information.
    this->initLogInformation();
}

PdfeFontType1::~PdfeFontType1()
{
    // Delete encoding object if necessary.
    if( m_encodingOwned ) {
        delete m_pEncoding;
    }
}

const PdfeFontDescriptor& PdfeFontType1::fontDescriptor() const
{
    return m_fontDescriptor;
}
PdfRect PdfeFontType1::fontBBox() const
{
    // Font bbox rescaled.
    PdfRect fontBBox = m_fontDescriptor.fontBBox();
    fontBBox.SetLeft( fontBBox.GetLeft() / 1000. );
    fontBBox.SetBottom( fontBBox.GetBottom() / 1000. );
    fontBBox.SetWidth( fontBBox.GetWidth() / 1000. );
    fontBBox.SetHeight( fontBBox.GetHeight() / 1000. );

    return fontBBox;
}
double PdfeFontType1::width( pdfe_cid c, bool useFParams ) const
{
    double width;
    if( c >= m_firstCID && c <= m_lastCID ) {
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
PdfRect PdfeFontType1::bbox( pdfe_cid c, bool useFParams ) const
{
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

bool PdfeFontType1::IsStandard14Font( PdfObject* pFont )
{
    // No font name...
    if( !pFont || !pFont->GetIndirectKey( "BaseFont" ) ) {
        return false;
    }

    // Read font name.
    std::string fontName = pFont->GetIndirectKey( "BaseFont" )->GetName().GetName();
    bool standard14font =
            fontName == "Times-Roman" ||
            fontName == "Times-Bold" ||
            fontName == "Times-Italic" ||
            fontName == "Times-BoldItalic" ||
            fontName == "Helvetica" ||
            fontName == "Helvetica-Bold" ||
            fontName == "Helvetica-Oblique" ||
            fontName == "Helvetica-BoldOblique" ||
            fontName == "Courier" ||
            fontName == "Courier-Bold" ||
            fontName == "Courier-Oblique" ||
            fontName == "Courier-BoldOblique" ||
            fontName == "Symbol" ||
            fontName == "ZapfDingbats";

    return standard14font;
}
QString PdfeFontType1::filenameStandard14Font( const std::string& fontName )
{
    // Standard font filename.
    QString filename;
    if( fontName == "Times-Roman" ) {
        filename = "NimbusRomNo9L-Regu.cff";
    }
    else if( fontName == "Times-Bold" ) {
        filename = "NimbusRomNo9L-Medi.cff";
    }
    else if( fontName == "Times-Italic" ) {
        filename = "NimbusRomNo9L-ReguItal.cff";
    }
    else if( fontName == "Times-BoldItalic" ) {
        filename = "NimbusRomNo9L-MediItal.cff";
    }
    else if( fontName == "Helvetica" ) {
        filename = "NimbusSanL-Regu.cff";
    }
    else if( fontName == "Helvetica-Bold" ) {
        filename = "NimbusSanL-Bold.cff";
    }
    else if( fontName == "Helvetica-Oblique" ) {
        filename = "NimbusSanL-ReguItal.cff";
    }
    else if( fontName == "Helvetica-BoldOblique" ) {
        filename = "NimbusSanL-BoldItal.cff";
    }
    else if( fontName == "Courier" ) {
        filename = "NimbusMonL-Regu.cff";
    }
    else if( fontName == "Courier-Bold" ) {
        filename = "NimbusMonL-Bold.cff";
    }
    else if( fontName == "Courier-Oblique" ) {
        filename = "NimbusMonL-ReguObli.cff";
    }
    else if( fontName == "Courier-BoldOblique" ) {
        filename = "NimbusMonL-BoldObli.cff";
    }
    else if( fontName == "Symbol" ) {
        filename = "StandardSymL.cff";
    }
    else if( fontName == "ZapfDingbats" ) {
        filename = "Dingbats.cff";
    }
    return Standard14FontsPath.absoluteFilePath( filename );
}

}
