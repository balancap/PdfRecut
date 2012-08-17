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
};


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
    PdfeFontType0( PoDoFo::PdfObject* pFont );

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

public:
    // Virtual functions reimplemented.
    /** Get the descriptor object corresponding to the font.
     * \return Constant reference to a PdfeFontDescriptor object.
     */
    virtual const PdfeFontDescriptor& fontDescriptor() const;

    /** Convert a simple string to a CID string (only perform a copy for simple fonts).
     * \param str Std::string to convert.
     * \return CID String corresponding.
     */
    virtual PdfeCIDString toCIDString( const std::string& str ) const;

    /** Get the width of a character.
     * \param c Character identifier (CID).
     * \return Width of the character.
     */
    virtual double width( pdf_cid c ) const;

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
    PdfeFontCID  m_fontCID;


};

}

#endif // PDFEFONTTYPE0_H
