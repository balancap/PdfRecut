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

using namespace PoDoFo;

namespace PoDoFoExtended {

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
        pdf_cid c = str[i];
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
PoDoFo::PdfRect PdfeFont::bbox( pdf_cid c, bool useFParams ) const
{
    // Font BBox for default height.
    PdfArray fontBBox = this->fontBBox();

    double width = this->width( c, false );
    double height = fontBBox[3].GetReal() / 1000.;

    // Apply font parameters.
    if( useFParams ) {
        width = ( width * m_fontSize + m_charSpace ) * ( m_hScale / 100. );
        height = height * m_fontSize;
        if( this->isSpace( c ) == PdfeFontSpace::Code32 ) {
            width += m_wordSpace * ( m_hScale / 100. );
        }
    }
    PdfRect cbbox( 0, fontBBox[1].GetReal() / 1000., width, height );
    return cbbox;
}

std::vector<pdf_gid> PdfeFont::mapCIDToGID( FT_Face face,
                                            pdf_cid firstCID,
                                            pdf_cid lastCID,
                                            PdfDifferenceEncoding* pDiffEncoding ) const
{
    // Initialize the vector.
    std::vector<pdf_gid> vectGID( lastCID - firstCID+1, 0 );

    // Set a CharMap: keep the default one if selected.
    if( !face->charmap ) {
        FT_Set_Charmap( face, face->charmaps[0] );
    }

    // Get the glyph index of every character.
    PdfName cname;
    pdf_gid glyph_idx;
    pdf_utf16be ucode;

    for( pdf_cid c = firstCID ; c <= lastCID ; ++c ) {
        // Try to obtain the CID name from its unicode code.
        ucode = this->toUnicode( c );
        cname = PdfDifferenceEncoding::UnicodeIDToName( PDF_UTF16_BE_LE( ucode ) );

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
int PdfeFont::glyphBBox( FT_Face face,
                         pdf_gid glyphIdx,
                         const PdfArray& fontBBox,
                         PdfRect* pGlyphBBox) const
{
    // Tru to load the glyph.
    int error = FT_Load_Glyph( face, glyphIdx, FT_LOAD_NO_SCALE );
    if( error ) {
        return error;
    }
    // Get bounding box, computed using the outline of the glyph.
    //FT_BBox glyph_bbox;
    //error = FT_Outline_Get_BBox( &face->glyph->outline, &glyph_bbox );

    // Compute bottom and top using glyph metrics.
    FT_Glyph_Metrics metrics = face->glyph->metrics;
    double bottom = static_cast<double>( metrics.horiBearingY - metrics.height );
    double top = static_cast<double>( metrics.horiBearingY );

    // Perform some corrections using font bounding box.
    bottom = std::max( fontBBox[1].GetReal(), bottom );
    top = std::min( fontBBox[3].GetReal(), top );

    // Set the bounding box of the glyph.
    pGlyphBBox->SetLeft( 0.0 );
    pGlyphBBox->SetWidth( metrics.horiAdvance );
    pGlyphBBox->SetBottom( bottom );
    pGlyphBBox->SetHeight( top-bottom );

    return 0;
}

}
