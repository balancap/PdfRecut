/***************************************************************************
 * Copyright (C) Paul Balança - All Rights Reserved                        *
 *                                                                         *
 * NOTICE:  All information contained herein is, and remains               *
 * the property of Paul Balança. Dissemination of this information or      *
 * reproduction of this material is strictly forbidden unless prior        *
 * written permission is obtained from Paul Balança.                       *
 *                                                                         *
 * Written by Paul Balança <paul.balanca@gmail.com>, 2012                  *
 ***************************************************************************/

#ifndef PDFEFONTDESCRIPTOR_H
#define PDFEFONTDESCRIPTOR_H

#include "podofo/base/PdfDefines.h"
#include "podofo/base/PdfName.h"
#include "podofo/base/PdfString.h"
#include "podofo/base/PdfRect.h"

namespace PoDoFo {
class PdfObject;
class PdfName;
class PdfString;
}

namespace PoDoFoExtended {

/** Simple class that gathers information on an embedded font.
 */
class PdfeFontEmbedded
{
public:
    /** Default constructor.
     */
    PdfeFontEmbedded() {
        this->init();
    }

    /** Initialize to default values.
     */
    void init() {
        m_fontFile = m_fontFile2 = m_fontFile3 = NULL;
    }

    /** Get the font file object. If no index provided, return first pointer found not equal to zero.
     * \param idx Index (1, 2 or 3).
     * \return Pointer to the PoDoFo object.
     */
    PoDoFo::PdfObject* fontFile( size_t idx = 0 ) const;

    /** Set font file objects (as set in a font descriptor).
     */
    void setFontFiles( PoDoFo::PdfObject* fontFile,
                       PoDoFo::PdfObject* fontFile2,
                       PoDoFo::PdfObject* fontFile3 );

    /** Get a copy of the font program, stored in a buffer.
     * \param pBuffer Pointer of the buffer which will contain the font program. Owned by the user.
     * \param pLength Pointer to the length of the buffer.
     */
    void copyFontProgram( char** pBuffer, long* pLength ) const;

protected:
    /// FontFile object no1.
    PoDoFo::PdfObject*  m_fontFile;
    /// FontFile object no2.
    PoDoFo::PdfObject*  m_fontFile2;
    /// FontFile object no3.
    PoDoFo::PdfObject*  m_fontFile3;
};


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
    static const PoDoFo::pdf_uint32  FlagPixedPitch = ( 1 << 0 );
    static const PoDoFo::pdf_uint32  FlagSerif = ( 1 << 1 );
    static const PoDoFo::pdf_uint32  FlagSymbolic = ( 1 << 2 );
    static const PoDoFo::pdf_uint32  FlagScript = ( 1 << 3 );
    static const PoDoFo::pdf_uint32  FlagNonsymbolic = ( 1 << 5 );
    static const PoDoFo::pdf_uint32  FlagItalic = ( 1 << 6 );
    static const PoDoFo::pdf_uint32  FlagAllCap = ( 1 << 16 );
    static const PoDoFo::pdf_uint32  FlagSmallCap = ( 1 << 17 );
    static const PoDoFo::pdf_uint32  FlagForceBold = ( 1 << 18 );
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

private:
    // Members corresponding to keys.
    /// Font full name.
    PoDoFo::PdfName     m_fontName;
    /// Font short name (without subset prefix).
    PoDoFo::PdfName     m_fontNameShort;
    /// Is it a subset font?
    bool     m_subsetFont;

    PoDoFo::PdfString   m_fontFamily;
    PoDoFo::PdfName     m_fontStretch;
    PoDoFo::PdfRect     m_fontBBox;

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

    PdfeFontEmbedded  m_fontEmbedded;
    PoDoFo::PdfString   m_charSet;

public:
    //**********************************************************//
    //                     Getters / Setters                    //
    //**********************************************************//
    PoDoFo::PdfName fontName( bool subsetPrefix = true ) const;
    void setFontName( const PoDoFo::PdfName& fontName );

    bool isSubsetFont() const       {   return m_subsetFont; }

    PoDoFo::PdfString fontFamily() const                        {  return m_fontFamily;  }
    void fontFamily( const PoDoFo::PdfString& fontFamily )      {  m_fontFamily = fontFamily;  }
    PoDoFo::PdfName fontStretch() const                         {  return m_fontStretch;  }
    void setFontStretch( const PoDoFo::PdfName& fontStretch )   {  m_fontStretch = fontStretch;  }
    PoDoFo::PdfRect fontBBox() const                            {  return m_fontBBox;  }
    void setFontBBox( const PoDoFo::PdfRect& fontBBox )         {  m_fontBBox = fontBBox;  }

    PoDoFo::pdf_uint32 fontWeight() const                   {  return m_fontWeight;  }
    void setFontWeight( PoDoFo::pdf_uint32 fontWeight )     {  m_fontWeight = fontWeight;  }
    PoDoFo::pdf_uint32 flags() const                        {  return m_flags;  }
    void setFlags( PoDoFo::pdf_uint32 flags )               {  m_flags = flags;  }
    PoDoFo::pdf_int32 italicAngle() const                   {  return m_italicAngle;  }
    void setItalicAngle( PoDoFo::pdf_int32 italicAngle )    {  m_italicAngle = italicAngle;  }

    double ascent() const               {  return m_ascent;  }
    void setAscent( double ascent )     {  m_ascent = ascent;  }
    double descent() const              {  return m_descent;  }
    void setDescent( double descent )   {  m_descent = descent;  }
    double leading() const              {  return m_leading;  }
    void setLeading( double leading )   {  m_leading = leading;  }
    double capHeight() const                {  return m_capHeight;  }
    void setCapHeight( double capHeight )   {  m_capHeight = capHeight;  }
    double xHeight() const              {  return m_xHeight;  }
    void setXHeight( double xHeight )   {  m_xHeight = xHeight;  }
    double stemV() const                {  return m_stemV;  }
    void setStemV( double stemV )       {  m_stemV = stemV;  }
    double stemH() const                {  return m_stemH;  }
    void setStemH( double stemH )       {  m_stemH = stemH;  }
    double avgWidth() const             {  return m_avgWidth;  }
    void setAvgWidth( double avgWidth ) {  m_avgWidth = avgWidth;  }
    double maxWidth() const             {  return m_maxWidth;  }
    void setMaxWidth( double maxWidth ) {  m_maxWidth = maxWidth;  }
    double missingWidth() const                 {  return m_missingWidth;  }
    void setMissingWidth( double missingWidth ) {  m_missingWidth = missingWidth;  }

    const PdfeFontEmbedded& fontEmbedded() const    {  return m_fontEmbedded;  }
};

//**********************************************************//
//                     Getters / Setters                    //
//**********************************************************//

}

#endif // PDFEFONTDESCRIPTOR_H
