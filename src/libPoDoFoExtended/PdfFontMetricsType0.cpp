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

#include "PdfFontMetricsType0.h"
#include "podofo/podofo.h"

using namespace PoDoFoExtended;

namespace PoDoFo {

//**********************************************************//
//             PdfFontMetricsType0::HWidthsArray            //
//**********************************************************//
PdfFontMetricsType0::HWidthsArray::HWidthsArray()
{
    this->init();
}
void PdfFontMetricsType0::HWidthsArray::init()
{
    m_firstCID.clear();
    m_lastCID.clear();
    m_widthsCID.clear();
    m_defaultWidth = 1000.;
}
void PdfFontMetricsType0::HWidthsArray::init( const PdfArray& widths )
{
    this->init();

    size_t i = 0;
    while( i < widths.size() ) {
        // Increase size of vectors.
        m_firstCID.push_back( 0 );
        m_lastCID.push_back( 0 );
        m_widthsCID.push_back( std::vector<double>() );

        // First CID value.
        m_firstCID.back() = static_cast<pdf_cid>( widths[i].GetNumber() );
        ++i;

        // Read array of widths.
        if( widths[i].IsArray() ) {
            PdfArray widthsCID = widths[i].GetArray();
            m_widthsCID.back().resize( widthsCID.size() );
            for( size_t j = 0 ; j < widthsCID.size() ; ++j ) {
                m_widthsCID.back()[j] = static_cast<double>( widthsCID[j].GetNumber() );
            }
            m_lastCID.back() = m_firstCID.back() + widthsCID.size() - 1;
            ++i;
        }
        // Read width for a range of CIDs.
        else {
            m_lastCID.back() = static_cast<pdf_cid>( widths[i].GetNumber() );
            ++i;
            m_widthsCID.back().push_back( static_cast<double>( widths[i].GetNumber() )  );
            ++i;
        }
    }
}
void PdfFontMetricsType0::HWidthsArray::setDefaultWidth( double defWidth )
{
    m_defaultWidth = defWidth;
}
double PdfFontMetricsType0::HWidthsArray::defaultWidth() const
{
    return m_defaultWidth;
}

//**********************************************************//
//                    PdfFontMetricsType0                   //
//**********************************************************//
PdfFontMetricsType0::PdfFontMetricsType0( PdfObject* pFont )
    : PdfFontMetrics( EPdfFontType( ePdfFontTypeMetrics_Type0 ), "", NULL )
{
    // Get subtype of the font and check if it is Type0.
    const PdfName& rSubType = pFont->GetDictionary().GetKey( PdfName::KeySubtype )->GetName();
    if ( rSubType != PdfName("Type0") ) {
        PODOFO_RAISE_ERROR( ePdfError_InvalidDataType );
    }

    // Read CMap encoding.
    PdfObject* pCMapEncoding = pFont->GetIndirectKey( "Encoding" );
    if( pCMapEncoding->IsName() ) {
        m_cmapEncoding.init( pCMapEncoding->GetName() );
    }
    else {
        m_cmapEncoding.init( pCMapEncoding );
    }

    // TODO: implement to unicode.

    // Get descendant CID font.
    const PdfArray& descendantFonts  = pFont->GetIndirectKey( "DescendantFonts" )->GetArray();
    PdfObject* pDFont = pFont->GetOwner()->GetObject( descendantFonts[0].GetReference() );

    // Read CID system info.
    m_cidSystemInfo.init( pDFont->GetIndirectKey( "CIDSystemInfo" ) );

    // Get font descriptor and information from it.
    PdfObject* pDescriptor = pDFont->GetIndirectKey( "FontDescriptor" );
    if( pDescriptor )
    {
        PdfObject *pObj = pDescriptor->GetIndirectKey( "FontBBox" );
        if( pObj ) {
            m_bbox = pObj->GetArray();
        }

        m_sName = pDescriptor->GetIndirectKey( "FontName" )->GetName();

        m_nWeight = static_cast<unsigned int>(pDescriptor->GetDictionary().GetKeyAsLong( "FontWeight", 400L ));
        m_nItalicAngle = static_cast<int>(pDescriptor->GetDictionary().GetKeyAsLong( "ItalicAngle", 0L ));

        m_dPdfAscent = pDescriptor->GetDictionary().GetKeyAsReal( "Ascent", 0.0 );
        m_dPdfDescent = pDescriptor->GetDictionary().GetKeyAsReal( "Descent", 0.0 );
    }
    else
    {
        // Use some default values...
        m_nWeight = 400;
        m_nItalicAngle = 0;
        m_dPdfAscent = 0.0;
        m_dPdfDescent = 0.0;
    }
    m_dAscent      = m_dPdfAscent / 1000.0;
    m_dDescent     = m_dPdfDescent / 1000.0;
    m_dLineSpacing = m_dAscent + m_dDescent;

    // Width of glyphs CID.
    m_widths.setDefaultWidth( static_cast<double>(pDFont->GetDictionary().GetKeyAsLong( "DW", 1000L )) );
    PdfObject * pWidths = pDFont->GetIndirectKey( "W" );
    if( pWidths ) {
        m_widths.init( pWidths->GetArray() );
    }

    // Try to fine some sensible values.
    m_dUnderlineThickness = 1.0;
    m_dUnderlinePosition  = 0.0;
    m_dStrikeOutThickness = m_dUnderlinePosition;
    m_dStrikeOutPosition  = m_dAscent / 2.0;

    m_bSymbol = false; // TODO
}
void PdfFontMetricsType0::init()
{
}

PdfFontMetricsType0::~PdfFontMetricsType0()
{
}

const char* PdfFontMetricsType0::GetFontname() const
{
    return m_sName.GetName().c_str();
}

void PdfFontMetricsType0::GetBoundingBox( PdfArray& array ) const
{
    array = m_bbox;
}

double PdfFontMetricsType0::CharWidth( unsigned char c ) const
{
    // Useless in the case of Type 0 fonts: return default width.
    double width = m_widths.defaultWidth();
    return width * static_cast<double>( this->GetFontSize() * this->GetFontScale() / 100.0 ) / 1000.0 +
                   static_cast<double>( this->GetFontSize() * this->GetFontScale() / 100.0 * this->GetFontCharSpace() / 100.0);

}
double PdfFontMetricsType0::GetCIDWidth( pdf_cid c )
{
    return m_widths.getCIDWidth( c );
}

double PdfFontMetricsType0::StringWidthCID( const char *pszText, pdf_long nLength ) const
{
    // Get CID vector corresponding to the string.
    std::vector<pdf_cid> strCIDs( m_cmapEncoding.getCID( pszText, nLength ) );

    // Compute width.
    double width = 0.0;
    for( size_t i = 0 ; i < strCIDs.size() ; ++i ) {
        width += m_widths.getCIDWidth( strCIDs[i] );
    }

    // Normalize using font parameters.
    return width * static_cast<double>( this->GetFontSize() * this->GetFontScale() / 100.0 ) / 1000.0 +
           strCIDs.size() * static_cast<double>( this->GetFontSize() * this->GetFontScale() / 100.0 * this->GetFontCharSpace() / 100.0);
}

double PdfFontMetricsType0::UnicodeCharWidth( unsigned short c ) const
{
    // TODO
    return 0.0;
}

void PdfFontMetricsType0::GetWidthArray( PdfVariant & var, unsigned int nFirst, unsigned int nLast ) const
{
    // TODO
    //var = m_width;
}

double PdfFontMetricsType0::GetGlyphWidth( int nGlyphId ) const
{
    // TODO
    return 0.0; // OC 13.08.2010 BugFix: Avoid microsoft compiler error
}

double PdfFontMetricsType0::GetGlyphWidth( const char* pszGlyphname ) const
{
    // TODO
    return 0.0;
}

long PdfFontMetricsType0::GetGlyphId( long lUnicode ) const
{
    // TODO
    return 0; // OC 13.08.2010 BugFix: Avoid microsoft compiler error
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetricsType0::GetLineSpacing() const
{
    return m_dLineSpacing * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetricsType0::GetUnderlinePosition() const
{
    return m_dUnderlinePosition * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetricsType0::GetStrikeOutPosition() const
{
    return m_dStrikeOutPosition * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetricsType0::GetUnderlineThickness() const
{
    return m_dUnderlineThickness * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetricsType0::GetStrikeoutThickness() const
{
    return m_dStrikeOutThickness * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
const char* PdfFontMetricsType0::GetFontData() const
{
    return NULL;
}

// -----------------------------------------------------
//
// -----------------------------------------------------
pdf_long PdfFontMetricsType0::GetFontDataLen() const
{
    return 0;
}

// -----------------------------------------------------
//
// -----------------------------------------------------
unsigned int PdfFontMetricsType0::GetWeight() const
{
    return m_nWeight;
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetricsType0::GetAscent() const
{
    return m_dAscent * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetricsType0::GetPdfAscent() const
{
    return m_dPdfAscent;
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetricsType0::GetDescent() const
{
    return m_dDescent * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetricsType0::GetPdfDescent() const
{
    return m_dPdfDescent;
}

// -----------------------------------------------------
//
// -----------------------------------------------------
int PdfFontMetricsType0::GetItalicAngle() const
{
    return m_nItalicAngle;
}

// -----------------------------------------------------
//
// -----------------------------------------------------
bool PdfFontMetricsType0::IsSymbol() const
{
    return m_bSymbol;
}

};
