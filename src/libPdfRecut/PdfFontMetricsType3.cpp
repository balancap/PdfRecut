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

#include "PdfFontMetricsType3.h"
#include "podofo/podofo.h"

namespace PoDoFo {

PdfFontMetricsType3::PdfFontMetricsType3( PdfObject* pFont )
    : PdfFontMetrics( ePdfFontType_Unknown, "", NULL ), m_dDefWidth(0.0)
{
    // Get subtype of the font and check if it is Type3.
    const PdfName& rSubType = pFont->GetDictionary().GetKey( PdfName::KeySubtype )->GetName();
    if ( rSubType != PdfName("Type3") ) {
        PODOFO_RAISE_ERROR( ePdfError_InvalidDataType );
    }

    // Get font descriptor and information from it.
    PdfObject* pDescriptor = pFont->GetIndirectKey( "FontDescriptor" );
    if( pDescriptor )
    {
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

    // Read font parameters.
    m_bbox = pFont->GetIndirectKey( "FontBBox" )->GetArray();
    m_nFirst = static_cast<int>(pFont->GetDictionary().GetKeyAsLong( "FirstChar", 0L ));
    m_nLast = static_cast<int>(pFont->GetDictionary().GetKeyAsLong( "LastChar", 0L ));

    // Characters widths and font matrix.
    PdfObject* widths = pFont->GetIndirectKey( "Widths" );
    if( widths ) {
        m_width = widths->GetArray();
    }
    else {
        PODOFO_RAISE_ERROR_INFO( ePdfError_NoObject, "Font object does not define Widths!" );
    }
    PdfObject* fontMatrix = pFont->GetIndirectKey( "FontMatrix" );
    if( fontMatrix ) {
        m_fontMatrix = fontMatrix->GetArray();
    }
    else {
        PODOFO_RAISE_ERROR_INFO( ePdfError_NoObject, "Font object does not define font matrix!" );
    }

    // Try to fine some sensible values.
    m_dUnderlineThickness = 1.0;
    m_dUnderlinePosition  = 0.0;
    m_dStrikeOutThickness = m_dUnderlinePosition;
    m_dStrikeOutPosition  = m_dAscent / 2.0;

    m_bSymbol = false; // TODO
}

PdfFontMetricsType3::~PdfFontMetricsType3()
{
}

const char* PdfFontMetricsType3::GetFontname() const
{
    return m_sName.GetName().c_str();
}

void PdfFontMetricsType3::GetBoundingBox( PdfArray& array ) const
{
    array = m_bbox;

    // Awful hack: transform the bounding box so that it looks like
    // a bounding box in a classic glyph coordinate system.
    array[0] = ( array[0].GetReal() * m_fontMatrix[0].GetReal() + m_fontMatrix[4].GetReal() ) * 1000.;
    array[1] = ( array[1].GetReal() * m_fontMatrix[3].GetReal() + m_fontMatrix[5].GetReal() ) * 1000.;
    array[2] = ( array[2].GetReal() * m_fontMatrix[0].GetReal() ) * 1000.;
    array[3] = ( array[3].GetReal() * m_fontMatrix[3].GetReal() ) * 1000.;
}

double PdfFontMetricsType3::CharWidth( unsigned char c ) const
{
    double dWidth = m_dDefWidth;
    if( c >= m_nFirst && c <= m_nLast && c - m_nFirst < static_cast<int>(m_width.GetSize()) ) {
        dWidth = m_width[c - m_nFirst].GetReal();
    }

    // Simple approximation of the font matrix.
    dWidth = dWidth * m_fontMatrix[0].GetReal();
    dWidth = dWidth * static_cast<double>(this->GetFontSize() * this->GetFontScale() / 100.0) +
            static_cast<double>( this->GetFontSize() * this->GetFontScale() / 100.0 * this->GetFontCharSpace() / 100.0);
    return dWidth;
}

double PdfFontMetricsType3::UnicodeCharWidth( unsigned short c ) const
{
    // TODO
    return 0.0;
}

void PdfFontMetricsType3::GetWidthArray( PdfVariant & var, unsigned int nFirst, unsigned int nLast ) const
{
    var = m_width;
}

double PdfFontMetricsType3::GetGlyphWidth( int nGlyphId ) const
{
    // TODO
    return 0.0; // OC 13.08.2010 BugFix: Avoid microsoft compiler error
}

double PdfFontMetricsType3::GetGlyphWidth( const char* pszGlyphname ) const
{
    // TODO
    return 0.0;
}

long PdfFontMetricsType3::GetGlyphId( long lUnicode ) const
{
    // TODO
    return 0; // OC 13.08.2010 BugFix: Avoid microsoft compiler error
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetricsType3::GetLineSpacing() const
{
    return m_dLineSpacing * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetricsType3::GetUnderlinePosition() const
{
    return m_dUnderlinePosition * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetricsType3::GetStrikeOutPosition() const
{
    return m_dStrikeOutPosition * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetricsType3::GetUnderlineThickness() const
{
    return m_dUnderlineThickness * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetricsType3::GetStrikeoutThickness() const
{
    return m_dStrikeOutThickness * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
const char* PdfFontMetricsType3::GetFontData() const
{
    return NULL;
}

// -----------------------------------------------------
//
// -----------------------------------------------------
pdf_long PdfFontMetricsType3::GetFontDataLen() const
{
    return 0;
}

// -----------------------------------------------------
//
// -----------------------------------------------------
unsigned int PdfFontMetricsType3::GetWeight() const
{
    return m_nWeight;
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetricsType3::GetAscent() const
{
    return m_dAscent * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetricsType3::GetPdfAscent() const
{
    return m_dPdfAscent;
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetricsType3::GetDescent() const
{
    return m_dDescent * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetricsType3::GetPdfDescent() const
{
    return m_dPdfDescent;
}

// -----------------------------------------------------
//
// -----------------------------------------------------
int PdfFontMetricsType3::GetItalicAngle() const
{
    return m_nItalicAngle;
}

// -----------------------------------------------------
//
// -----------------------------------------------------
bool PdfFontMetricsType3::IsSymbol() const
{
    return m_bSymbol;
}

};
