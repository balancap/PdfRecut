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

#include <podofo/base/PdfDefines.h>
#include <podofo/doc/PdfFontCache.h>
#include <podofo/base/PdfRect.h>

#include "PdfeTypes.h"
#include "PdfeCMap.h"
#include "PdfeFontDescriptor.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <QByteArray>
#include <QString>
#include <QImage>

#include <QDebug>

#include <iostream>

namespace PoDoFo {
class PdfObject;
class PdfEncoding;
class PdfDifferenceEncoding;
class PdfString;
class PdfArray;
}

namespace PoDoFoExtended {

namespace PdfeFontType {
/** Enumeration of the different types of font allowed in the PDF Reference.
 */
enum Enum {
    Type0 = 0,
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
    Type0 = 0,
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
    PdfeFont( PoDoFo::PdfObject* pFont, FT_Library ftLibrary );

    /** Initialize the object to default parameters.
     */
    void init();

    /** Virtual destructor.
     */
    virtual ~PdfeFont();

protected:
    /** No default constructor.
     */
    PdfeFont();

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
     * \param useUCMap Try to use the unicode CMap to convert.
     * \return Unicode QString corresponding.
     */
    virtual QString toUnicode( const PdfeCIDString& str, bool useUCMap = true ) const;
    /** Convert a simple string to unicode.
     * \param str PoDoFo::PdfString to convert (can contain 0 characters !).
     * \param useUCMap Try to use the unicode CMap to convert.
     * \return Unicode QString corresponding.
     */
    virtual QString toUnicode( const PoDoFo::PdfString& str, bool useUCMap = true ) const;

public:
    // Virtual functions to reimplement in derived classes.
    /** Get the descriptor object corresponding to the font.
     * \return Constant reference to a PdfeFontDescriptor object.
     */
    virtual const PdfeFontDescriptor& fontDescriptor() const = 0;

    /** Get the font bounding box.
     * \return PoDoFo::PdfRect containing the font bounding box.
     */
    virtual PoDoFo::PdfRect fontBBox() const = 0;

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
    virtual double width( pdfe_cid c, bool useFParams ) const = 0;

    /** Get the bounding box of a character.
     * \param c Character identifier (CID).
     * \param useFParams Use font parameters (char and word space, font size, ...).
     * \return Bounding box of the character.
     */
    virtual PoDoFo::PdfRect bbox( pdfe_cid c, bool useFParams ) const = 0;

    /** Convert a character to a unicode QString.
     * \param  c Character identifier (CID).
     * \param useUCMap Try to use the unicode CMap to convert.
     * \return Unicode QString representing the character.
     */
    virtual QString toUnicode( pdfe_cid c, bool useUCMap = true ) const = 0;

    /** Is a CID character a white space character.
     * \param  c Character identifier (CID).
     * \return Classification of the character.
     */
    virtual PdfeFontSpace::Enum isSpace( pdfe_cid c ) const = 0;

    /** Get default height used for space characters of the font.
     * \return Space height.
     */
    virtual double spaceHeight() const = 0;

protected:
    /** Initialize PdfEncoding of a PdfeFont.
     * \param Pointer to the object where is defined the encoding.
     */
    void initEncoding( PoDoFo::PdfObject* pEncodingObj );
    /** Initialize unicode CMap of a PdfeFont.
     * \param Pointer to the object where is defined the CMap.
     */
    void initUnicodeCMap( PoDoFo::PdfObject* pUCMapObj );
    /** Initialize FreeType face object.
     * \param fontDescriptor Font descriptor containing font name and/or embedded font program.
     */
    void initFTFace( const PdfeFontDescriptor& fontDescriptor );
    /** Initialize the list of space characters for the font.
     * \param firstCID First CID to consider.
     * \param lastCID Last CID to consider.
     * \param clearContents Clear existing contents.
     */
    void initSpaceCharacters( pdfe_cid firstCID, pdfe_cid lastCID, bool clearContents = true );

protected:
    /** Get a character name from its CID. First try using the font encoding and then the unicode code.
     * Needs the encoding object to be initialized.
     * \param c CID of the character.
     * \return Name of the character. Empty if no name found.
     */
    PoDoFo::PdfName fromCIDToName( pdfe_cid c ) const;

    /** Convert a character CID to the corresponding glyph GID.
     * Need the freetype face, unicode CMap and encoding to be initialized.
     * \param c Character CID;
     * \return Glyph GID. 0 if not found.
     */
    pdfe_gid fromCIDToGID( pdfe_cid c ) const;

    /** Apply font parameter to a character width.
     * \param Reference to the width to modify.
     * \param space32 Is the character a 32 type space?
     */
    void applyFontParameters( double& width,
                              bool space32 ) const;

    /** Apply font parameter to a character bounding box.
     * \param Reference to the bbox to modify.
     * \param space32 Is the character a 32 type space?
     */
    void applyFontParameters( PoDoFo::PdfRect& bbox,
                              bool space32 ) const;

public:
    /** Simple structure that gathers data of a rendered glyph.
     */
    struct GlyphImage
    {
        /// QIMage containing rendered glyph.
        QImage  image;
        /// Transformation matrix from bitmap coordinates to font coordinates.
        PdfeMatrix  transMatrix;
    };

public:
    // Static functions used as interface with FreeType library.
    /** Obtain the bounding box of a glyph, using FreeType.
     * \param ftFace Freetype face object.
     * \param glyph_idx Glyph index.
     * \param fontBBox Font bounding box.
     * \return Bounding box of the glyph (in 1000 units scale).
     * Set to zero if anything wrong happened.
     */
    static PoDoFo::PdfRect ftGlyphBBox( FT_Face ftFace,
                                        pdfe_gid glyph_idx,
                                        const PoDoFo::PdfRect& fontBBox );

    /** Render a glyph using FreeType.
     * \param ftFace Freetype face object.
     * \param glyph_idx Glyph index.
     * \param charHeight Size chosen for the glyph.
     * \param resolution Resolution in dpi.
     * \return GlyphImage containing the rendered glyph.
     */
    static GlyphImage ftGlyphRender( FT_Face ftFace,
                                     pdfe_gid glyph_idx ,
                                     unsigned int charHeight,
                                     long resolution );

protected:
    /** Static function that return what are considered as space characters.
     * \return Constant reference to a vector of QChar containing space characters.
     * By convention the first one is the classic space.
     */
    static const std::vector<QChar>& spaceCharacters();

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

    // Common members shared by all fonts.
    /// FreeType library.
    FT_Library  m_ftLibrary;
    /// FreeType Face object (represent a font).
    FT_Face  m_ftFace;
    /// FreeType Face data.
    QByteArray  m_ftFaceData;

    /// Font encoding.
    PoDoFo::PdfEncoding*  m_pEncoding;
    /// Does the object owns the encoding ?
    bool  m_encodingOwned;
    /// Unicode CMap.
    PdfeCMap  m_unicodeCMap;

    /// Vector of space characters (pair of CID and space type).
    std::vector< std::pair<pdfe_cid,PdfeFontSpace::Enum> >  m_spaceCharacters;

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
