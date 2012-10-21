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

#ifndef PDFEFONTTYPE1_H
#define PDFEFONTTYPE1_H

#include "PdfeFont.h"

#include <QDir>

namespace PoDoFo {
struct PODOFO_CharData;
}

namespace PoDoFoExtended {

/** Class that represents a PDF Font of type 1 (c.f. PDF Reference).
 */
class PdfeFontType1 : public PdfeFont
{
public:
    /** Create a PdfeFontType1 from a PdfObject.
     * \param pFont Pointer to the object where is defined the type 1 font.
     */
    PdfeFontType1( PoDoFo::PdfObject* pFont, FT_Library ftLibrary );

    /** Initialize the object to default parameters.
     */
    void init();

    /** Virtual destructor.
     */
    virtual ~PdfeFontType1();

protected:
    /** No default constructor.
     */
    PdfeFontType1() { }

    /** Initialize the size of characters according to the font object.
     * \param pFont Pointer to the object where is defined the type 1 font.
     */
    void initCharactersBBox( const PoDoFo::PdfObject* pFont );

    /** Initialize the font as one of the 14 standard font.
     * \param pFont Pointer to the object where is defined the standard type 1 font.
     */
    void initStandard14Font( const PoDoFo::PdfObject* pFont );

public:
    // Implementation of PdfeFont interface.
    /** Get the descriptor object corresponding to the font.
     * \return Constant reference to a PdfeFontDescriptor object.
     */
    virtual const PdfeFontDescriptor& fontDescriptor() const;

    /** Get the font bounding box.
     * \return PoDoFo::PdfRect containing the font bounding box.
     */
    virtual PoDoFo::PdfRect fontBBox() const;

    /** Get the width of a character.
     * \param c Character identifier (CID).
     * \param useFParams Use font parameters (char and word space, font size, ...).
     * \return Width of the character.
     */
    virtual double width( pdfe_cid c, bool useFParams ) const;

    /** Get the advance vector of a character (horizontal or vertical usually).
     * \param c Character identifier (CID).
     * \param useFParams Use font parameters (char and word space, font size, ...).
     * \return Advance vector.
     */
    virtual PdfeVector advance( pdfe_cid c, bool useFParams ) const;

    /** Get the bounding box of a character.
     * \param c Character identifier (CID).
     * \param useFParams Use font parameters (char and word space, font size, ...).
     * \return Bounding box of the character.
     */
    virtual PoDoFo::PdfRect bbox( pdfe_cid c, bool useFParams ) const;

    /** Convert a simple PDF string to a CID string (only perform a copy for simple fonts).
     * \param str PoDoFo::PdfString to convert (can contain 0 characters !).
     * \return CID String corresponding.
     */
    virtual PdfeCIDString toCIDString( const PoDoFo::PdfString& str ) const {
        return this->PdfeFont::toCIDString( str );
    }

    /** Convert a character to a unicode QString.
     * \param  c Character identifier (CID).
     * \param useUCMap Try to use the unicode CMap to convert.
     * \param firstTryEncoding First try to use the Pdf encoding.
     * \return Unicode QString representing the character.
     */
    virtual QString toUnicode( pdfe_cid c,
                               bool useUCMap = true,
                               bool firstTryEncoding = false ) const {
        return this->PdfeFont::toUnicode( c, useUCMap, firstTryEncoding );
    }

    /** Is a CID character a white space character.
     * \param  c Character identifier (CID).
     * \return Classification of the character.
     */
    virtual PdfeFontSpace::Enum isSpace( pdfe_cid c ) const {
        return this->PdfeFont::isSpace( c );
    }

    /** Get default height used for space characters of the font.
     * \return Space height.
     */
    virtual double spaceHeight() const {
        return 500. / 1000.;    // Default chose for Type 1 font.
    }

    /** Convert a character CID to the corresponding glyph GID.
     * Need the freetype face, unicode CMap and encoding to be initialized.
     * \param c Character CID;
     * \return Glyph GID. 0 if not found.
     */
    virtual pdfe_gid fromCIDToGID( pdfe_cid c ) const;

private:
    // Members.
    /// The PostScript name of the font (retrieved from font descriptor).
    PoDoFo::PdfName  m_baseFont;

    /// First character defined in font's width array.
    pdfe_cid  m_firstCID;
    /// Last character defined in font's width array.
    pdfe_cid  m_lastCID;

    /// Array of advance vectors (horizontal for Type 1 fonts).
    std::vector<PdfeVector>  m_advanceCID;
    /// Array storing the bounding box of characters.
    std::vector<PoDoFo::PdfRect>  m_bboxCID;

    /// Font descriptor.
    PdfeFontDescriptor  m_fontDescriptor;
};

}

#endif // PDFEFONTTYPE1_H
