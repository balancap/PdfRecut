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

#include FT_BBOX_H

using namespace PoDoFo;

namespace PoDoFoExtended {

//**********************************************************//
//                          PdfeFont                        //
//**********************************************************//
PdfeFont::PdfeFont( PdfObject* pFont, FT_Library ftLibrary )
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
    m_pEncoding = NULL;
    m_encodingOwned = false;
    m_unicodeCMap.init();
    m_spaceCharacters.clear();
}
PdfeFont::~PdfeFont()
{
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
        // Create PdfeCMap::CharCode from CID.
        pdf_uint16 code = PDFE_UTF16BE_TO_HBO( c );
        PdfeCMap::CharCode charCode( code );
        // Convert to unicode.
        ustr = m_unicodeCMap.toUnicode( charCode );
    }
    // No result: Pdf encoding (not for Type 0 fonts).
    if( m_pEncoding && !ustr.length() && this->type() != PdfeFontType::Type0 ) {
        // Get UTF16 code from PdfEncoding object.
        pdf_utf16be ucode = m_pEncoding->GetCharCode( c );
        ucode = PDFE_UTF16BE_TO_HBO( ucode );
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
        std::string path( "./cmaps/" );
        path += this->fontDescriptor().fontName().GetName();
        path += ".txt";

        PdfOutputDevice outFile( path.c_str() );
        PdfMemStream* pStream = dynamic_cast<PdfMemStream*>( pUCMapObj->GetStream() );
        pStream->Uncompress();
        pStream->Write( &outFile );

        std::cout << this->fontDescriptor().fontName().GetName().length() << " : "
                  << this->fontDescriptor().fontName().GetName() << " / "
                  << this->type() << std::endl;

        if( ! this->fontDescriptor().fontName().GetName().length() ) {
            std::cout << "mmm" << std::endl;
        }
    }
}

void PdfeFont::cidToName( PdfEncoding* pEncoding, pdfe_cid c, PdfName& cname )
{
    // No encoding...
    if( !pEncoding ) {
        cname = PdfName();
    }
    pdf_utf16be ucode;

    // First try using difference encoding.
    PdfDifferenceEncoding* pDiffEncoding = dynamic_cast<PdfDifferenceEncoding*>( pEncoding );
    if( pDiffEncoding ) {
        const PdfEncodingDifference& differences = pDiffEncoding->GetDifferences();
        if( differences.Contains( c, cname, ucode ) ) {
            // Name found!
            return;
        }
    }

    // Classic encoding: try using the char code (UTF16) and the map unicode<->name.
    ucode = pEncoding->GetCharCode( c );
    cname = PdfDifferenceEncoding::UnicodeIDToName( ucode );
}

std::vector<pdfe_gid> PdfeFont::mapCIDToGID( FT_Face face,
                                             pdfe_cid firstCID,
                                             pdfe_cid lastCID,
                                             PdfDifferenceEncoding* pDiffEncoding ) const
{
    // Initialize the vector.
    std::vector<pdfe_gid> vectGID( lastCID - firstCID+1, 0 );

    // Set a CharMap: keep the default one if selected.
    if( !face->charmap ) {
        FT_Set_Charmap( face, face->charmaps[0] );
    }

    // Get the glyph index of every character.
    PdfName cname;
    pdfe_gid glyph_idx;
    pdf_utf16be ucode;

    for( pdfe_cid c = firstCID ; c <= lastCID ; ++c ) {
        glyph_idx = 0;

        // Try to obtain the CID name from its unicode code.
        QString ustr = this->toUnicode( c );
        if( ustr.length() == 1 ) {
            // Character UTF16BE
            ucode = ustr[0].unicode();
            cname = PdfDifferenceEncoding::UnicodeIDToName( PDFE_UTF16BE_TO_HBO( ucode ) );

            // Glyph index from the glyph name.
            glyph_idx = FT_Get_Name_Index( face, const_cast<char*>( cname.GetName().c_str() ) );
        }

        // Difference encoding: try to see if the character belongs to the difference map.
        if( pDiffEncoding ) {
            const PdfEncodingDifference& differences = pDiffEncoding->GetDifferences();

            if( differences.Contains( c, cname, ucode ) ) {
                // Find the glyph index from its name.
                glyph_idx = FT_Get_Name_Index( face, const_cast<char*>( cname.GetName().c_str() ) );
            }
        }

        // No glyph index yet: try using the character code and the different charmaps.
        if( !glyph_idx ) {
            for( int n = 0; n < face->num_charmaps && !glyph_idx ; n++ )
            {
                FT_Set_Charmap( face, face->charmaps[n] );

                if( ucode ) {
                    glyph_idx = FT_Get_Char_Index( face, ucode );
                }
                if( !glyph_idx ) {
                    glyph_idx = FT_Get_Char_Index( face, c );
                }
            }
        }

        // Set glyph index if found.
        if( glyph_idx ) {
            vectGID[ c - firstCID ] = glyph_idx;
        }
    }
    return vectGID;
}
int PdfeFont::glyphBBox(FT_Face face,
                        pdfe_gid glyphIdx,
                        const PdfRect& fontBBox,
                        PdfRect* pGlyphBBox) const
{
    // Try to load the glyph.
    int error = FT_Load_Glyph( face, glyphIdx, FT_LOAD_NO_SCALE );
    if( error ) {
        return error;
    }
    // Get bounding box, computed using the outline of the glyph.
//    FT_BBox glyph_bbox;
//    error = FT_Outline_Get_BBox( &face->glyph->outline, &glyph_bbox );
//    double bottom = glyph_bbox.yMin;
//    double top = glyph_bbox.yMax;

    // Compute bottom and top using glyph metrics.
    FT_Glyph_Metrics metrics = face->glyph->metrics;
    double bottom = static_cast<double>( metrics.horiBearingY - metrics.height );
    double top = static_cast<double>( metrics.horiBearingY );

    // Perform some corrections using font bounding box.
    bottom = std::max( fontBBox.GetBottom(), bottom );
    top = std::min( fontBBox.GetHeight()+fontBBox.GetBottom(), top );

    // Set the bounding box of the glyph.
    pGlyphBBox->SetLeft( 0.0 );
    pGlyphBBox->SetWidth( metrics.horiAdvance );
    pGlyphBBox->SetBottom( bottom );
    pGlyphBBox->SetHeight( top-bottom );

    return 0;
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


}
