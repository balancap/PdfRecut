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

#ifndef PDFFONTMETRICSTYPE0_H
#define PDFFONTMETRICSTYPE0_H

#include "podofo/base/PdfDefines.h"
#include "podofo/base/PdfArray.h"
#include "podofo/base/PdfName.h"
#include "podofo/base/PdfString.h"
#include "podofo/doc/PdfFontMetrics.h"

#include "PdfCMap.h"

namespace PoDoFo {

enum EPdfFontTypeMetrics {
    ePdfFontTypeMetrics_Truetype,
    ePdfFontTypeMetrics_Type1 = ePdfFontType_Type1Base14,
    ePdfFontTypeMetrics_Type3,
    ePdfFontTypeMetrics_Type0
};

class PdfArray;
class PdfObject;
class PdfVariant;

class PODOFO_DOC_API PdfFontMetricsType0 : public PdfFontMetrics
{
public:

    /** Create a font metrics object based on an existing PdfObject.
     *  \param pObject an existing font descriptor object
     */
    PdfFontMetricsType0( PdfObject* pFont );
    virtual ~PdfFontMetricsType0();

    /** Create a width array for this font which is a required part of every font dictionary.
     *  \param var the final width array is written to this PdfVariant.
     *  \param nFirst first character to be in the array.
     *  \param nLast last character code to be in the array.
     */
    virtual void GetWidthArray( PdfVariant& var, unsigned int nFirst, unsigned int nLast ) const;

    /** Get the width of a single glyph id.
     *  \returns the width of a single glyph id.
     */
    virtual double GetGlyphWidth( int nGlyphId ) const;

    /** Get the width of a single named glyph.
     *  \param pszGlyphname name of the glyph.
     *  \returns the width of a single named glyph.
     */
    virtual double GetGlyphWidth( const char* pszGlyphname ) const;

    /** Create the bounding box array as required by the PDF reference
     *  so that it can be written directly to a PDF file.
     *  \param array write the bounding box to this array.
     */
    virtual void GetBoundingBox( PdfArray& array ) const;

    /** Retrieve the width of the given character in PDF units in the current font
     *  \param c character
     *  \returns the width in PDF units
     */
    virtual double CharWidth( unsigned char c ) const;

    /** Retrieve the width of the given CID in PDF units in the current font.
     *  \param c Character.
     *  \returns The width in PDF units
     */
    double GetCIDWidth( pdf_cid c );

    /** Retrieve the width of a given text string in PDF units when
     *  drawn with the current font (reimplementation).
     *  \param pszText a text string of which the width should be calculated
     *  \param nLength if != 0 only the width of the nLength first characters is calculated
     *  \returns the width in PDF units
     */
    double StringWidthCID( const char* pszText, pdf_long nLength = 0 ) const;

    // Peter Petrov 20 March 2009
    /** Retrieve the width of the given character in PDF units in the current font
     *  \param c character
     *  \returns the width in PDF unitsneither Widths
     */
    virtual double UnicodeCharWidth( unsigned short c ) const;

    /** Retrieve the line spacing for this font
     *  \returns the linespacing in PDF units
     */
    virtual double GetLineSpacing() const;

    /** Get the width of the underline for the current
     *  font size in PDF units
     *  \returns the thickness of the underline in PDF units
     */
    virtual double GetUnderlineThickness() const;

    /** Return the position of the underline for the current font
     *  size in PDF units
     *  \returns the underline position in PDF units
     */
    virtual double GetUnderlinePosition() const;

    /** Return the position of the strikeout for the current font
     *  size in PDF units
     *  \returns the underline position in PDF units
     */
    virtual double GetStrikeOutPosition() const;

    /** Get the width of the strikeout for the current
     *  font size in PDF units
     *  \returns the thickness of the strikeout in PDF units
     */
    virtual double GetStrikeoutThickness() const;

    /** Get a string with the postscript name of the font.
     *  \returns the postscript name of the font or NULL string if no postscript name is available.
     */
    virtual const char* GetFontname() const;

    /** Get the weight of this font.
     *  Used to build the font dictionay
     *  \returns the weight of this font (500 is normal).
     */
    virtual unsigned int GetWeight() const;

    /** Get the ascent of this font in PDF
     *  units for the current font size.
     *  \returns the ascender for this font
     *  \see GetPdfAscent
     */
    virtual double GetAscent() const;

    /** Get the ascent of this font
     *  Used to build the font dictionay
     *  \returns the ascender for this font
     *  \see GetAscent
     */
    virtual double GetPdfAscent() const;

    /** Get the descent of this font in PDF
     *  units for the current font size.
     *  This value is usually negative!
     *  \returns the descender for this font
     *  \see GetPdfDescent
     */
    virtual double GetDescent() const;

    /** Get the descent of this font
     *  Used to build the font dictionay
     *  \returns the descender for this font
     *  \see GetDescent
     */
    virtual double GetPdfDescent() const;

    /** Get the italic angle of this font.
     *  Used to build the font dictionay
     *  \returns the italic angle of this font.
     */
    virtual int GetItalicAngle() const;

    /** Get the glyph id for a unicode character
     *  in the current font.
     *
     *  \param lUnicode the unicode character value
     *  \returns the glyhph id for the character or 0 if the glyph was not found.
     */
    virtual long GetGlyphId( long lUnicode ) const;

    /** Symbol fonts do need special treatment in a few cases.
     *  Use this method to check if the current font is a symbol
     *  font. Symbold fonts are detected by checking
     *  if they use FT_ENCODING_MS_SYMBOL as internal encoding.
     *
     * \returns true if this is a symbol font
     */
    virtual bool IsSymbol() const;

    /** Get a pointer to the actual font data - if it was loaded from memory.
     *  \returns a binary buffer of data containing the font data
     */
    virtual const char* GetFontData() const;

    /** Get the length of the actual font data - if it was loaded from memory.
     *  \returns a the length of the font data
     */
    virtual pdf_long GetFontDataLen() const;

private:
    /** Default constructor, not implemented.
     */
    PdfFontMetricsType0();

    /** Copy constructor, not implemented.
     */
    PdfFontMetricsType0( const PdfFontMetricsType0& metricsObj );

    /** Assignment operator, not implemented.
     */
    PdfFontMetricsType0& operator=( const PdfFontMetricsType0& metricsObj );

private:
    /** Initialize members to default values.
     */
    void init();

private:
    /** Private class that represents an array of glyphs horizontal widths.
     */
    class HWidthsArray
    {
    public:
        /** Default constructor.
         */
        HWidthsArray();

        /** Initialize to a default object, with DW=1000.
         */
        void init();
        /** Initialize given a PdfArray from a PdfFont object.
         */
        void init( const PdfArray& widths );

        /** Set Default width.
         * \param defWidth Default width of CID glyphs.
         */
        void setDefaultWidth( double defWidth );
        /** Get default width.
         * \return Default width of CID glyphs.
         */
        double defaultWidth() const;

        /** Obtain the width of CID glyph.
         */
        double getCIDWidth( pdf_cid cid ) const;

    private:
        /** Vector which contains first CID of groups.
         */
        std::vector<pdf_cid> m_firstCID;
        /** Vector which contains last CID of groups.
         */
        std::vector<pdf_cid> m_lastCID;
        /** Vector which contains widths of every group.
         */
        std::vector< std::vector<double> > m_widthsCID;

        /** Default horizontal width.
         */
        double m_defaultWidth;
    };

private:
    // Members related to the Type0 font.
    PdfName     m_baseFont;
    PdfCMap     m_cmapEncoding;

    // Members related to the CID font.
    PdfCIDSystemInfo    m_cidSystemInfo;

    PdfName       m_sName;
    PdfArray      m_bbox;

    HWidthsArray  m_widths;

    unsigned int  m_nWeight;
    int           m_nItalicAngle;

    double        m_dPdfAscent;
    double        m_dPdfDescent;
    double        m_dAscent;
    double        m_dDescent;
    double        m_dLineSpacing;

    double        m_dUnderlineThickness;
    double        m_dUnderlinePosition;
    double        m_dStrikeOutThickness;
    double        m_dStrikeOutPosition;

    bool          m_bSymbol;  ///< Internal member to singnal a symbol font
    double        m_dDefWidth; ///< default width
};

//**********************************************************//
//             PdfFontMetricsType0::HWidthsArray            //
//**********************************************************//
inline double PdfFontMetricsType0::HWidthsArray::getCIDWidth( pdf_cid cid ) const
{
    // Find the group of CID it belongs to.
    for( size_t i = 0 ; i < m_widthsCID.size() ; ++i ) {
        if( cid >= m_firstCID[i] && cid <= m_lastCID[i] ) {
            // Only one element: same size for the group.
            if( m_widthsCID[i].size() == 1 ) {
                return m_widthsCID[i][0];
            }
            else {
                return m_widthsCID[i][cid - m_firstCID[i]];
            }
        }
    }
    // Return default width.
    return m_defaultWidth;
}


}

#endif // PDFFONTMETRICSTYPE0_H
