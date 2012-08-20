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

#ifndef PDFEFONT_H
#define PDFEFONT_H

#include "podofo/base/PdfDefines.h"
#include "podofo/doc/PdfFontCache.h"
#include "PdfeFontDescriptor.h"

#include <QString>
#include <QDebug>

#include <iostream>

namespace PoDoFo {
class PdfObject;
class PdfEncoding;
class PdfString;
class PdfArray;
}

namespace PoDoFoExtended {

/** Pdf Character Identifier (CID) type.
 */
typedef PoDoFo::pdf_uint16  pdf_cid;

/** Macro that convert UTF16 Big Endian to Little Endian when necessary.
 */
#ifdef PODOFO_IS_LITTLE_ENDIAN
#define PDF_UTF16_BE_LE(c) ( ( (c & 0xff) << 8 ) | ( (c & 0xff00) >> 8 ) )
#else
#define PDF_UTF16_BE_LE(c) ( c )
#endif

/** Pdf CID String. To be improve ?
 */
typedef std::basic_string<pdf_cid>  PdfeCIDString;


namespace PdfeFontType {
/** Enumeration of the different types of font allowed in the PDF Reference.
 */
enum Enum {
    Type0,
    Type1,
    Type3,
    TrueType,
    CIDFont,
    Unknown
};
}
namespace PdfeFontSubType {
/** Enumeration of the different subtypes of font allowed in the PDF Reference.
 */
enum Enum {
    Type0,
    Type1,
    MMType1,
    Type3,
    TrueType,
    CIDFontType0,
    CIDFontType2,
    Unknown
};
}
namespace PdfeFontSpace {
/** Enumeration of the different of white space characters that can appear.
 */
enum Enum {
    None = 0,   // Not a space character.
    Code32,     // Single byte 32 space.
    Other       // Other kind of space character (not single byte, tabular, ...).
};

/// BE-UTF16 code for the classic space character.
static const PoDoFo::pdf_utf16be UTF16Code = 0x2000;
}

/** Class that represent a generic PDF Font. Extend and improve the class define in PoDoFo.
 */
class PdfeFont
{
public:
    /** Create a PdfeFont from a PdfObject.
     * \param pFont Pointer to the object which is defined the font.
     * \param ftLibrary FreeType library.
     */
    PdfeFont( PoDoFo::PdfObject* pFont, FT_Library* ftLibrary );

    /** Initialize the object to default parameters.
     */
    void init();

    /** Virtual destructor.
     */
    virtual ~PdfeFont();

protected:
    /** No default constructor.
     */
    PdfeFont() { }

public:
    /** Get the width of a CID string.
     * \param str CID string to consider.
     * \return Width of the string.
     */
    virtual double width( const PdfeCIDString& str ) const;
    /** Get the width of a string (first converted to a CID string).
     * \param str PoDoFo::PdfString to consider (can contain 0 characters !).
     * \return Width of the string.
     */
    virtual double width( const PoDoFo::PdfString& str ) const;

    /** Convert a CID string to unicode.
     * \param str CID string to convert.
     * \return Unicode QString corresponding.
     */
    virtual QString toUnicode( const PdfeCIDString& str ) const;
    /** Convert a simple string to unicode.
     * \param str PoDoFo::PdfString to convert (can contain 0 characters !).
     * \return Unicode QString corresponding.
     */
    virtual QString toUnicode( const PoDoFo::PdfString& str ) const;

public:
    // Virtual functions to reimplement in derived classes.
    /** Get the descriptor object corresponding to the font.
     * \return Constant reference to a PdfeFontDescriptor object.
     */
    virtual const PdfeFontDescriptor& fontDescriptor() const = 0;

    /** Get the font bounding box.
     * \return PoDoFo::PdfArray containing the bounding box.
     */
    virtual PoDoFo::PdfArray fontBBox() const = 0;

    /** Convert a simple PDF string to a CID string (only perform a copy for simple fonts).
     * \param str PoDoFo::PdfString to convert (can contain 0 characters !).
     * \return CID String corresponding.
     */
    virtual PdfeCIDString toCIDString( const PoDoFo::PdfString& str ) const = 0;

    /** Get the width of a character.
     * \param c Character identifier (CID).
     * \param useFParams Use font parameters (char and word space, font size, ...).
     * \return Width of the character.
     */
    virtual double width( pdf_cid c, bool useFParams ) const = 0;

    /** Convert a character to its unicode equivalent (QChar).
     * \param  c Character identifier (CID).
     * \return Unicode QChar corresponding.
     */
    virtual QChar toUnicode( pdf_cid c ) const = 0;

    /** Is a CID character a white space character.
     * \param  c Character identifier (CID).
     * \return Classification of the character.
     */
    virtual PdfeFontSpace::Enum isSpace( pdf_cid c ) const = 0;

protected:
    // Members
    /// Font type.
    PdfeFontType::Enum  m_type;
    /// Font subtype.
    PdfeFontSubType::Enum  m_subtype;

    /// Character spacing (default: 0).
    double  m_charSpace;
    /// Word spacing (default: 0).
    double  m_wordSpace;
    /// Horizontal scaling (default: 100).
    double  m_hScale;
    /// Font size (default: 1.0).
    double  m_fontSize;

    /// FreeType library.
    FT_Library*  m_ftLibrary;

public:
    //**********************************************************//
    //                     Getters / Setters                    //
    //**********************************************************//
    PdfeFontType::Enum type() const             {  return m_type;  }
    void setType( PdfeFontType::Enum type )     {  m_type = type;  }
    PdfeFontSubType::Enum subtype() const           {  return m_subtype;  }
    void setType( PdfeFontSubType::Enum subtype )   {  m_subtype = subtype;  }

    double charSpace() const                {  return m_charSpace;  }
    void setCharSpace( double charSpace )   {  m_charSpace = charSpace;  }
    double wordSpace() const                {  return m_wordSpace;  }
    void setWordSpace( double wordSpace )   {  m_wordSpace = wordSpace;  }
    double hScale() const               {  return m_hScale;  }
    void setHScale( double hScale )     {  m_hScale = hScale;  }
    double fontSize() const             {  return m_fontSize;  }
    void setFontSize( double fontSize ) {  m_fontSize = fontSize;  }
};

}

#endif // PDFEFONT_H
