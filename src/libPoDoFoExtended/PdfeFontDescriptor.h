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

#ifndef PDFEFONTDESCRIPTOR_H
#define PDFEFONTDESCRIPTOR_H

#include "podofo/base/PdfDefines.h"
#include "podofo/base/PdfName.h"
#include "podofo/base/PdfString.h"
#include "podofo/base/PdfArray.h"

namespace PoDoFo {
class PdfObject;
class PdfName;
class PdfString;
}

namespace PoDoFoExtended {

/** Class that represents a Font Descriptor, as defined in the PDF Reference.
 */
class PdfeFontDescriptor
{
public:
    /// Enumerate the different keys that can exist in a Font Descriptor.
    enum Key {
        FontName = 0,
        FontFamily,
        FontStretch,
        FontWeight,
        Flags,
        FontBBox,
        ItalicAngle,
        Ascent,
        Descent,
        Leading,
        CapHeight,
        XHeight,
        StemV,
        StemH,
        AvgWidth,
        MaxWidth,
        MissingWidth,
        FontFile,
        FontFile2,
        FontFile3,
        CharSet,
        Style,      // CID Font
        Lang,       // CID Font
        FD,         // CID Font
        CIDSet      // CID Font
    };

    // Font Descriptor flags.
    static const PoDoFo::pdf_uint32  FlagPixedPitch = ( 0x0001 << 0 );
    static const PoDoFo::pdf_uint32  FlagSerif = ( 0x0001 << 1 );
    static const PoDoFo::pdf_uint32  FlagSymbolic = ( 0x0001 << 2 );
    static const PoDoFo::pdf_uint32  FlagScript = ( 0x0001 << 3 );
    static const PoDoFo::pdf_uint32  FlagNonsymbolic = ( 0x0001 << 5 );
    static const PoDoFo::pdf_uint32  FlagItalic = ( 0x0001 << 6 );
    static const PoDoFo::pdf_uint32  FlagAllCap = ( 0x0001 << 16 );
    static const PoDoFo::pdf_uint32  FlagSmallCap = ( 0x0001 << 17 );
    static const PoDoFo::pdf_uint32  FlagForceBold = ( 0x0001 << 18 );
public:
    /** Default constructor.
     */
    PdfeFontDescriptor();

    /** Create a Font Descriptor from a PdfObject.
     * \param pFontDesc Pointer to the object where is defined the font descriptor.
     */
    PdfeFontDescriptor( PoDoFo::PdfObject* pFontDesc );

    /** Copy constructor.
     * \param fontDesc Font descriptor to copy.
     */
    PdfeFontDescriptor( const PdfeFontDescriptor& fontDesc );

    /** Initialize the Font Descriptor to default.
     */
    void init();

    /** Initialize the Font Descriptor according to a PdfObject.
     * \param pFontDesc Pointer to the object where is defined the font descriptor.
     */
    void init( PoDoFo::PdfObject* pFontDesc );

    /** Set a parameter to default value.
     * \param key Key of the font descriptor to reset.
     */
    void resetKey( PdfeFontDescriptor::Key key );


protected:
    // Members corresponding to keys.
    PoDoFo::PdfName     m_fontName;
    PoDoFo::PdfString   m_fontFamily;
    PoDoFo::PdfName     m_fontStretch;
    PoDoFo::PdfArray    m_fontBBox;

    PoDoFo::pdf_uint32  m_fontWeight;
    PoDoFo::pdf_uint32  m_flags;
    PoDoFo::pdf_int32   m_italicAngle;

    double  m_ascent;
    double  m_descent;
    double  m_leading;
    double  m_capHeight;
    double  m_xHeight;
    double  m_stemV;
    double  m_stemH;
    double  m_avgWidth;
    double  m_maxWidth;
    double  m_missingWidth;

    PoDoFo::PdfObject*  m_fontFile;
    PoDoFo::PdfObject*  m_fontFile2;
    PoDoFo::PdfObject*  m_fontFile3;

    PoDoFo::PdfString   m_charSet;

public:
    //**********************************************************//
    //                     Getters / Setters                    //
    //**********************************************************//
    PoDoFo::PdfName fontName() const                    {  return m_fontName;  }
    void setFontName( const PoDoFo::PdfName& fontName ) {  m_fontName = fontName;  }
    PoDoFo::PdfString fontFamily() const                    {  return m_fontFamily;  }
    void fontFamily( const PoDoFo::PdfString& fontFamily )  {  m_fontFamily = fontFamily;  }
    PoDoFo::PdfName fontStretch() const                         {  return m_fontStretch;  }
    void setFontStretch( const PoDoFo::PdfName& fontStretch )   {  m_fontStretch = fontStretch;  }
    PoDoFo::PdfArray fontBBox() const                       {  return m_fontBBox;  }
    void setFontBBox( const PoDoFo::PdfArray& fontBBox )    {  m_fontBBox = fontBBox;  }

    PoDoFo::pdf_uint32 fontWeight() const               {  return m_fontWeight;  }
    void setFontWeight( PoDoFo::pdf_uint32 fontWeight ) {  m_fontWeight = fontWeight;  }
    PoDoFo::pdf_uint32 flags() const            {  return m_flags;  }
    void setFlags( PoDoFo::pdf_uint32 flags )   {  m_flags = flags;  }
    PoDoFo::pdf_int32 italicAngle() const                   {  return m_italicAngle;  }
    void setItalicAngle( PoDoFo::pdf_int32 italicAngle )    {  m_italicAngle = italicAngle;  }

    double ascent() const           {  return m_ascent;  }
    void setAscent( double ascent ) {  m_ascent = ascent;  }
    double descent() const              {  return m_descent;  }
    void setDescent( double descent )   {  m_descent = descent;  }
    double leading() const              {  return m_leading;  }
    void setLeading( double leading )   {  m_leading = leading;  }
    double capHeight() const                {  return m_capHeight;  }
    void setCapHeight( double capHeight )   {  m_capHeight = capHeight;  }
    double xHeight() const              {  return m_xHeight;  }
    void setXHeight( double xHeight )   {  m_xHeight = xHeight;  }
    double stemV() const            {  return m_stemV;  }
    void setStemV( double stemV )   {  m_stemV = stemV;  }
    double stemH() const            {  return m_stemH;  }
    void setStemH( double stemH )   {  m_stemH = stemH;  }
    double avgWidth() const             {  return m_avgWidth;  }
    void setAvgWidth( double avgWidth ) {  m_avgWidth = avgWidth;  }
    double maxWidth() const             {  return m_maxWidth;  }
    void setMaxWidth( double maxWidth ) {  m_maxWidth = maxWidth;  }
    double missingWidth() const                 {  return m_missingWidth;  }
    void setMissingWidth( double missingWidth ) {  m_missingWidth = missingWidth;  }


};

//**********************************************************//
//                     Getters / Setters                    //
//**********************************************************//

}

#endif // PDFEFONTDESCRIPTOR_H
