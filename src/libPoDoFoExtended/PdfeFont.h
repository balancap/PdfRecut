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
#include "PdfeFontDescriptor.h"

namespace PoDoFo {
class PdfObject;
}

namespace PoDoFoExtended {

/** Pdf Character Identifier (CID) type.
 */
typedef PoDoFo::pdf_uint16  pdf_cid;

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
/** Enumeration of the different sub-types of font allowed in the PDF Reference.
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

/** Class that represent a generic PDF Font. Extend and improve the class define in PoDoFo.
 */
class PdfeFont
{
public:

    /** Create a PdfeFont from a PdfObject.
     * \param pFont Pointer to the object which is defined the font.
     */
    PdfeFont( PoDoFo::PdfObject* pFont );

    /** Initialize the object to default parameters.
     */
    void init();

    /** Virtual destructor.
     */
    virtual ~PdfeFont();

public:
    /** Get the descriptor object corresponding to the font.
     * \return Constant reference to a PdfeFontDescriptor object.
     */
    virtual const PdfeFontDescriptor& fontDescriptor() const = 0;

    /** Convert a simple string to a CID string (only a copy for simple fonts).
     *
     */
    virtual PdfeCIDString toCIDString( const char* str );

private:
    /** No default constructor.
     */
    PdfeFont() { }

protected:
    // Members
    /// Font type.
    PdfeFontType::Enum  m_type;
    /// Font subtype.
    PdfeFontSubType::Enum   m_subtype;

    /// Character spacing (default: 0).
    double  m_charSpace;
    /// Word spacing (default: 0).
    double  m_wordSpace;
    /// Horizontal scaling (default: 100).
    double  m_hScale;
    /// Font size (default: 1.0).
    double  m_fontSize;

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
