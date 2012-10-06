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


std::vector<pdfe_utf16> UTF16BEStrToUTF16Vec( const char* pstr, size_t length )
{
    std::vector<pdfe_utf16>  utf16vec;
    utf16vec.reserve( length / 2 );

    const pdfe_utf16* pchar = reinterpret_cast<const pdfe_utf16*>( pstr );
    for( size_t i = 0 ; i < length; i+=2 ) {
        utf16vec.push_back( PDFE_UTF16BE_TO_HBO( *pchar ) );
        ++pchar;
    }
    return utf16vec;
}
QString UTF16VecToQString( const std::vector<pdfe_utf16>& utf16vec )
{
    const ushort* putf16 = reinterpret_cast<const ushort*>( &utf16vec[0] );
    return QString::fromUtf16( putf16, utf16vec.size() );
}


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

    m_ftLibrary = NULL;
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

QString PdfeFont::toUnicode( const PdfeCIDString& str ) const
{
    QString ustr;
    ustr.reserve( str.length() );
    for( size_t i = 0 ; i < str.length() ; ++i ) {
        ustr.push_back( QChar( this->toUnicode( str[i] ) ) );
    }
    return ustr;
}
QString PdfeFont::toUnicode( const PoDoFo::PdfString& str ) const
{
    return this->toUnicode( this->toCIDString( str ) );
}

// Default implementation for character bounding box.
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
    if( useFParams ) {
        width = ( width * m_fontSize + m_charSpace ) * ( m_hScale / 100. );

        bottom = bottom * m_fontSize;
        height = height * m_fontSize;

        if( this->isSpace( c ) == PdfeFontSpace::Code32 ) {
            width += m_wordSpace * ( m_hScale / 100. );
        }
    }
    PdfRect cbbox( 0.0, bottom, width, height );
    return cbbox;
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
        // Try to obtain the CID name from its unicode code.
        ucode = this->toUnicode( c );
        cname = PdfDifferenceEncoding::UnicodeIDToName( PDFE_UTF16BE_TO_HBO( ucode ) );

        // Glyph index from the glyph name.
        glyph_idx = FT_Get_Name_Index( face, const_cast<char*>( cname.GetName().c_str() ) );

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
    // Tru to load the glyph.
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
