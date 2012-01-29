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

#include "PdfFontMetrics14.h"
#include "podofo/podofo.h"

namespace PoDoFo {

PdfFontMetrics14::PdfFontMetrics14( PdfFontMetricsBase14* pFontMetrics, const PdfEncoding* pPdfEncoding )
    : PdfFontMetrics( ePdfFontType_Unknown, "", NULL ), m_dDefWidth(0.0)
{
    // Get parameters from PdfFontMetricsBase14 object.
    m_sName = pFontMetrics->GetFontname();
    m_nWeight = pFontMetrics->GetWeight();
    m_nItalicAngle = pFontMetrics->GetItalicAngle();
    m_dPdfAscent = pFontMetrics->GetPdfAscent();
    m_dPdfDescent = pFontMetrics->GetPdfDescent();
    m_bSymbol = pFontMetrics->IsSymbol();
    pFontMetrics->GetBoundingBox( m_bbox );

    m_dAscent      = m_dPdfAscent / 1000.0;
    m_dDescent     = m_dPdfDescent / 1000.0;
    m_dLineSpacing = m_dAscent + m_dDescent;

    m_dUnderlineThickness = 0.0;
    m_dUnderlinePosition  = 0.0;
    m_dStrikeOutThickness = m_dUnderlinePosition;
    m_dStrikeOutPosition  = m_dAscent / 2.0;

    pFontMetrics->SetFontCharSpace( 0.0 );
    pFontMetrics->SetFontScale( 100.0 );
    pFontMetrics->SetFontSize( 1.0 );

    // Construct width array using the pdf encoding.
    m_nFirst = pPdfEncoding->GetFirstChar();
    m_nLast = pPdfEncoding->GetLastChar();

    pdf_utf16be ucode;
    double widthChar;
    for( int i = m_nFirst ; i <= m_nLast ; ++i) {
        ucode = pPdfEncoding->GetCharCode( i );

        // Dumb bug in PoDoFo: why bytes are inverted in GetCharCode but not UnicodeCharWidth ???
#ifdef PODOFO_IS_LITTLE_ENDIAN
        ucode = ((ucode & 0xff) << 8) | ((ucode & 0xff00) >> 8);
#endif
        widthChar = pFontMetrics->UnicodeCharWidth( ucode ) * 1000.0;
        m_width.push_back( widthChar );
    }
}

PdfFontMetrics14::~PdfFontMetrics14()
{
}

const char* PdfFontMetrics14::GetFontname() const
{
    return m_sName.GetName().c_str();
}

void PdfFontMetrics14::GetBoundingBox( PdfArray& array ) const
{
    array = m_bbox;
}

double PdfFontMetrics14::CharWidth( unsigned char c ) const
{
    if( c >= m_nFirst && c <= m_nLast && c - m_nFirst < static_cast<int>(m_width.GetSize()) )
    {
        double dWidth = m_width[c - m_nFirst].GetReal();

        return dWidth * static_cast<double>(this->GetFontSize() * this->GetFontScale() / 100.0) / 1000.0 +
            static_cast<double>( this->GetFontSize() * this->GetFontScale() / 100.0 * this->GetFontCharSpace() / 100.0);

    }
    else
    {
        return m_dDefWidth;
    }
}

double PdfFontMetrics14::UnicodeCharWidth( unsigned short c ) const
{
    // TODO
    return 0.0;
}

void PdfFontMetrics14::GetWidthArray( PdfVariant & var, unsigned int nFirst, unsigned int nLast ) const
{
    var = m_width;
}

double PdfFontMetrics14::GetGlyphWidth( int nGlyphId ) const
{
    // TODO
    return 0.0; // OC 13.08.2010 BugFix: Avoid microsoft compiler error
}

double PdfFontMetrics14::GetGlyphWidth( const char* pszGlyphname ) const
{
    // TODO
    return 0.0;
}

long PdfFontMetrics14::GetGlyphId( long lUnicode ) const
{
    // TODO
    return 0; // OC 13.08.2010 BugFix: Avoid microsoft compiler error
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetrics14::GetLineSpacing() const
{
    return m_dLineSpacing * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetrics14::GetUnderlinePosition() const
{
    return m_dUnderlinePosition * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetrics14::GetStrikeOutPosition() const
{
    return m_dStrikeOutPosition * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetrics14::GetUnderlineThickness() const
{
    return m_dUnderlineThickness * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetrics14::GetStrikeoutThickness() const
{
    return m_dStrikeOutThickness * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
const char* PdfFontMetrics14::GetFontData() const
{
    return NULL;
}

// -----------------------------------------------------
//
// -----------------------------------------------------
pdf_long PdfFontMetrics14::GetFontDataLen() const
{
    return 0;
}

// -----------------------------------------------------
//
// -----------------------------------------------------
unsigned int PdfFontMetrics14::GetWeight() const
{
    return m_nWeight;
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetrics14::GetAscent() const
{
    return m_dAscent * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetrics14::GetPdfAscent() const
{
    return m_dPdfAscent;
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetrics14::GetDescent() const
{
    return m_dDescent * this->GetFontSize();
}

// -----------------------------------------------------
//
// -----------------------------------------------------
double PdfFontMetrics14::GetPdfDescent() const
{
    return m_dPdfDescent;
}

// -----------------------------------------------------
//
// -----------------------------------------------------
int PdfFontMetrics14::GetItalicAngle() const
{
    return m_nItalicAngle;
}

// -----------------------------------------------------
//
// -----------------------------------------------------
bool PdfFontMetrics14::IsSymbol() const
{
    return m_bSymbol;
}

};
