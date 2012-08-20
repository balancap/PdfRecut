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

#ifndef PDFEFONTTYPE0_H
#define PDFEFONTTYPE0_H

#include "PdfeFont.h"
#include "PdfeCMap.h"
#include "PdfeFontDescriptor.h"

namespace PoDoFoExtended {

class PdfeFontCID;

//**********************************************************//
//                          PdfeFont0                       //
//**********************************************************//
/** Class that represents a PDF Font of type 0 (c.f. PDF Reference).
 */
class PdfeFontType0 : public PdfeFont
{
public:
    /** Create a PdfeFontType0 from a PdfObject.
     * \param pFont Pointer to the object where is defined the type 0 font.
     */
    PdfeFontType0( PoDoFo::PdfObject* pFont, FT_Library* ftLibrary );

    /** Initialize the object to default parameters.
     */
    void init();

    /** Virtual destructor.
     */
    virtual ~PdfeFontType0();

protected:
    /** No default constructor.
     */
    PdfeFontType0() { }

    /** Initialize the vector of space characters.
     */
    void initSpaceCharacters();

public:
    // Virtual functions reimplemented.
    /** Get the descriptor object corresponding to the font.
     * \return Constant reference to a PdfeFontDescriptor object.
     */
    virtual const PdfeFontDescriptor& fontDescriptor() const;

    /** Get the font bounding box.
     * \return PoDoFo::PdfArray containing the bounding box.
     */
    virtual PoDoFo::PdfArray fontBBox() const;

    /** Convert a simple PDF string to a CID string (only perform a copy for simple fonts).
     * \param str PoDoFo::PdfString to convert (can contain 0 characters !).
     * \return CID String corresponding.
     */
    virtual PdfeCIDString toCIDString( const PoDoFo::PdfString& str ) const;

    /** Get the width of a character.
     * \param c Character identifier (CID).
     * \param useFParams Use font parameters (char and word space, font size, ...).
     * \return Width of the character.
     */
    virtual double width( pdf_cid c, bool useFParams ) const;

    /** Convert a character to its unicode equivalent (QChar).
     * \param  c Character identifier (CID).
     * \return Unicode QChar corresponding.
     */
    virtual QChar toUnicode( pdf_cid c ) const;

    /** Is a CID character a white space character.
     * \param  c Character identifier (CID).
     * \return Classification of the character.
     */
    virtual PdfeFontSpace::Enum isSpace( pdf_cid c ) const;

protected:
    // Members.
    /// The PostScript name of the font.
    PoDoFo::PdfName  m_baseFont;
    /// CMap encoding of the font.
    PdfeCMap  m_encodingCMap;

    /// Descendant CID font.
    PdfeFontCID*  m_fontCID;

    /// Vector of space characters.
    std::vector<pdf_cid>  m_spaceCharacters;
};

//**********************************************************//
//                        PdfeFontCID                       //
//**********************************************************//
/** Class that represents a PDF CID Font (c.f. PDF Reference).
 */
class PdfeFontCID
{
public:
    /** Default constructor.
     */
    PdfeFontCID();

    /** Initialize the object to default parameters.
     */
    void init();

    /** Initialize a PdfeFontCID from a PdfObject.
     * \param pFont Pointer to the object where is defined the CID font.
     */
    void init( PoDoFo::PdfObject* pFont );

public:
    /** Get the descriptor object corresponding to the font.
     * \return Constant reference to a PdfeFontDescriptor object.
     */
    const PdfeFontDescriptor& fontDescriptor() const;

    /** Get the width of a character.
     * \param c Character identifier (CID).
     * \return Width of the character.
     */
    double width( pdf_cid c ) const;

protected:
    /** Private class that represents an array of glyph's horizontal width.
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
        void init( const PoDoFo::PdfArray& widths );

        /** Set Default width.
         * \param defWidth Default width of CID glyphs.
         */
        void setDefaultWidth( double defWidth );
        /** Get default width.
         * \return Default width of CID glyphs.
         */
        double defaultWidth() const;

        /** Get the width of a character.
         * \param c Character identifier (CID).
         * \return Width of the character.
         */
        double width( pdf_cid c ) const;

    private:
        /// Vector containing first CID of each group.
        std::vector<pdf_cid>  m_firstCID;
        /// Vector containing last CID of each group.
        std::vector<pdf_cid>  m_lastCID;
        /// Vector containing widths of each group.
        std::vector< std::vector<double> >  m_widthsCID;

        /// Default horizontal width.
        double  m_defaultWidth;
    };

protected:
    // Members for a CID font.
    /// Font type.
    PdfeFontType::Enum  m_type;
    /// Font subtype.
    PdfeFontSubType::Enum  m_subtype;
    /// The PostScript name of the font.
    PoDoFo::PdfName  m_baseFont;

    /// CIDSystemInfo (character collection).
    PdfeCIDSystemInfo  m_cidSystemInfo;
    /// Font descriptor.
    PdfeFontDescriptor  m_fontDescriptor;
    /// Horizontal widths of characters.
    HWidthsArray  m_hWidths;
};


//**********************************************************//
//                        PdfeFontCID                       //
//**********************************************************//
inline const PdfeFontDescriptor& PdfeFontCID::fontDescriptor() const
{
    return m_fontDescriptor;
}
inline double PdfeFontCID::width( pdf_cid c ) const
{
    return m_hWidths.width( c ) / 1000.;
}

//**********************************************************//
//                 PdfeFontCID::HWidthsArray                //
//**********************************************************//
inline double PdfeFontCID::HWidthsArray::width( pdf_cid c ) const
{
    // Find the group of CID it belongs to.
    for( size_t i = 0 ; i < m_widthsCID.size() ; ++i ) {
        if( c >= m_firstCID[i] && c <= m_lastCID[i] ) {
            // Only one element: same size for the group.
            if( m_widthsCID[i].size() == 1 ) {
                return m_widthsCID[i][0];
            }
            else {
                return m_widthsCID[i][c - m_firstCID[i]];
            }
        }
    }
    // Return default width.
    return m_defaultWidth;
}
inline void PdfeFontCID::HWidthsArray::setDefaultWidth( double defWidth )
{
    m_defaultWidth = defWidth;
}
inline double PdfeFontCID::HWidthsArray::defaultWidth() const
{
    return m_defaultWidth;
}


}

#endif // PDFEFONTTYPE0_H
