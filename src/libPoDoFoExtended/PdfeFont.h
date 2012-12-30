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
#include <QDir>

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
namespace PdfeFont14Standard {
/** Enumeration of the 14 standard fonts.
 */
enum Enum {
    TimesRoman = 0,
    TimesBold,
    TimesItalic,
    TimesBoldItalic,
    Helvetica,
    HelveticaBold,
    HelveticaOblique,
    HelveticaBoldOblique,
    Courier,
    CourierBold,
    CourierOblique,
    CourierBoldOblique,
    Symbol,
    ZapfDingbats,
    None
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
    /** Create a PdfeFont corresponding to a standard 14 font.
     * \param stdFontType Type of the standard font.
     * \param ftLibrary FreeType library.
     */
    PdfeFont( PdfeFont14Standard::Enum stdFontType, FT_Library ftLibrary );
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
    /** Get the advance vector of a CID string.  Font parameters are used in the computation.
     * \param str CID string to consider.
     * \return Advance vector of the string.
     */
    PdfeVector advance( const PdfeCIDString& str ) const;
    /** Get the advance vector of a string (first converted to a CID string).
     * Font parameters are used in the computation.
     * \param str PoDoFo::PdfString to consider (can contain 0 characters !).
     * \return Advance vector of the string.
     */
    PdfeVector advance( const PoDoFo::PdfString& str ) const;

    /** Compute the bounding box of a CID string. Refer to the coordinates of the first character.
     * Font parameters are used in the computation.
     * \param str CID string to consider.
     * \return Bounding box of the string.
     */
    PoDoFo::PdfRect bbox( const PdfeCIDString& str ) const;
    /** Compute the bounding box of a string (first converted to a CID string).
     * Font parameters are used in the computation.
     * \param str PoDoFo::PdfString to consider (can contain 0 characters !).
     * \return Bounding box of the string.
     */
    PoDoFo::PdfRect bbox( const PoDoFo::PdfString& str ) const;

    /** Convert a CID string to unicode.
     * \param str CID string to convert.
     * \param useUCMap Try to use the unicode CMap to convert.
     * \return Unicode QString corresponding.
     */
    virtual QString toUnicode( const PdfeCIDString& str,
                               bool useUCMap = true,
                               bool firstTryEncoding = false ) const;
    /** Convert a simple string to unicode.
     * \param str PoDoFo::PdfString to convert (can contain 0 characters !).
     * \param useUCMap Try to use the unicode CMap to convert.
     * \return Unicode QString corresponding.
     */
    virtual QString toUnicode( const PoDoFo::PdfString& str,
                               bool useUCMap = true,
                               bool firstTryEncoding = false ) const;

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
    /** Get the advance vector of a character (horizontal or vertical usually).
     * \param c Character identifier (CID).
     * \param useFParams Use font parameters (char and word space, font size, ...).
     * \return Advance vector.
     */
    virtual PdfeVector advance( pdfe_cid c, bool useFParams ) const = 0;
    /** Get the bounding box of a character.
     * \param c Character identifier (CID).
     * \param useFParams Use font parameters (char and word space, font size, ...).
     * \return Bounding box of the character.
     */
    virtual PoDoFo::PdfRect bbox( pdfe_cid c, bool useFParams ) const = 0;
    /** Convert a simple PDF string to a CID string (only perform a copy for simple fonts).
     * \param str PoDoFo::PdfString to convert (can contain 0 characters !).
     * \return CID String corresponding.
     */
    virtual PdfeCIDString toCIDString( const PoDoFo::PdfString& str ) const = 0;
    /** Convert a character to a unicode QString.
     * \param  c Character identifier (CID).
     * \param useUCMap Try to use the unicode CMap to convert.
     * \param firstTryEncoding First try to use the Pdf encoding.
     * \return Unicode QString representing the character.
     */
    virtual QString toUnicode( pdfe_cid c,
                               bool useUCMap = true,
                               bool firstTryEncoding = false ) const = 0;
    /** Is a CID character a white space character.
     * \param  c Character identifier (CID).
     * \return Classification of the character.
     */
    virtual PdfeFontSpace::Enum isSpace( pdfe_cid c ) const = 0;
    /** Get default width used for space characters of the font.
     * \return Space width.
     */
    virtual double spaceWidth() const = 0;
    /** Get default height used for space characters of the font.
     * \return Space height.
     */
    virtual double spaceHeight() const = 0;
    /** Convert a character CID to the corresponding glyph GID.
     * Need the freetype face, unicode CMap and encoding to be initialized.
     * \param c Character CID;
     * \return Glyph GID. 0 if not found.
     */
    virtual pdfe_gid fromCIDToGID( pdfe_cid c ) const = 0;

public:
    /// Embedded structure containing some font statistics.
    struct Statistics
    {
        /// Mean advance value.
        double meanAdvance;
        /// Mean bounding box (width, bottom and top computed).
        PoDoFo::PdfRect meanBBox;
    };

    /** Compute some font statistics.
     * \param defaultValue Use some predefined default values (for Type 0/1 and TrueType fonts).
     * \return PdfeFont::Statistics object containing data.
     */
    virtual Statistics statistics( bool defaultValue = false ) const = 0;

public:
    /** Get a character name from its CID.
     * Default implementation first try using the font encoding and then the unicode code.
     * Needs the encoding object to be initialized.
     * \param c CID of the character.
     * \param useEncoding Use PDF encoding, if it exists?
     * \param useDiffEncoding Use difference encoding, if it exists?
     * \param useUnicodeCMap Use unicode CMap, if it exists?
     * \return Name of the character. Empty if no name found.
     */
    PoDoFo::PdfName fromCIDToName( pdfe_cid c,
                                   bool useEncoding = true,
                                   bool useDiffEncoding = true,
                                   bool useUnicodeCMap = true ) const;

protected:
    /** Initialize PdfEncoding of a PdfeFont.
     * \param Pointer to the object where is defined the encoding.
     */
    void initEncoding( PoDoFo::PdfObject* pEncodingObj );
    /** Initialize PdfEncoding.
     * \param Pointer to the PoDoFo encoding.
     * \param owned Is the encoding owned by the PdfeFont object.
     */
    void initEncoding( PoDoFo::PdfEncoding* pEncoding,
                       bool owned );
   /** Initialize unicode CMap of a PdfeFont.
     * \param Pointer to the object where is defined the CMap.
     */
    void initUnicodeCMap( PoDoFo::PdfObject* pUCMapObj );
    /** Initialize FreeType face object.
     * \param fontDescriptor Font descriptor containing font name and/or embedded font program.
     */
    void initFTFace( const PdfeFontDescriptor& fontDescriptor );
    /** Initialize FreeType face object.
     * \param filename Filename of the font to load..
     */
    void initFTFace( QString filename );
    /** Initialize FreeType face charmaps indexes.
     */
    void initFTFaceCharmaps();

    /** Initialize the list of space characters for the font.
     * \param firstCID First CID to consider.
     * \param lastCID Last CID to consider.
     * \param clearContents Clear existing contents.
     */
    void initSpaceCharacters( pdfe_cid firstCID, pdfe_cid lastCID, bool clearContents = true );
    /** Log the initialization of PdfeFont object, and corresponding font information.
     */
    void initLogInformation();

protected:
    /** Apply font parameter to a character width.
     * \param Reference to the width to modify.
     * \param space32 Is the character a 32 type space?
     */
    void applyFontParameters( double& width,
                              bool space32 ) const;
    /** Apply font parameter to a character advance vector.
     * \param Reference to the advance vector to modify.
     * \param space32 Is the character a 32 type space?
     */
    void applyFontParameters( PdfeVector& advance,
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
    // Functions used as interface with FreeType library.
    /** Obtain the bounding box of a glyph, using FreeType.
     * \param ftFace Freetype face object.
     * \param gid Glyph index.
     * \return Bounding box of the glyph (in 1000 units scale).
     * Set to zero if anything wrong happened.
     */
    static PoDoFo::PdfRect ftGlyphBBox( FT_Face ftFace,
                                        pdfe_gid gid );
    /** Obtain the bounding box of a glyph, using FreeType (not static).
     * \param gid Glyph index.
     * \return Bounding box of the glyph (in 1000 units scale).
     * Set to zero if anything wrong happened.
     */
    PoDoFo::PdfRect ftGlyphBBox( pdfe_gid gid );
    /** Render a glyph using FreeType.
     * \param gid Glyph index.
     * \param charHeight Size chosen for the glyph.
     * \param resolution Resolution in dpi.
     * \return GlyphImage containing the rendered glyph.
     */
    GlyphImage ftGlyphRender( pdfe_gid gid,
                              unsigned int charHeight,
                              long resolution );
protected:
    //Interface with FreeType library.
    /** FreeType charmaps that can be present in a FT face.
     * List: (1,0), (3,0) and (3,1) charmaps.
     */
    enum FTCharmap {
        FTCharmap10 = 0,
        FTCharmap30,
        FTCharmap31
    };

    /** Retrieve the GID corresponding to a given character code.
     * Basically call FT_Get_Char_Index + few tweaks.
     * \param charCode Character code.
     * \param charmap30 Is charmap (3,0) used?
     * \return GID of the character. O if not found.
     */
    pdfe_gid ftGIDFromCharCode(pdfe_cid charCode , bool charmap30 = false ) const;
    /** Retrieve the GID corresponding to a given character code.
     * Basically call FT_Get_Name_Index.
     * \param charName Character name.
     * \return GID of the character. O if not found.
     */
    pdfe_gid ftGIDFromName( const PoDoFo::PdfName& charName ) const;


public:
    /// Directory where are stored standard 14 fonts files.
    static QDir Standard14FontsDir;

public:
    /** Does a font name corresponds to the name of a standard 14 font?
     * \param fontName Name of the font.
     * \return PdfeFont14Standard value. None if it is not.
     */
    static PdfeFont14Standard::Enum isStandard14Font( const std::string& fontName );
    /** Get the name (most used) of a standard 14 font.
     * \param stdFontType Type of the standard font.
     * \return Font name.
     */
    static std::string standard14FontName( PdfeFont14Standard::Enum stdFontType );
    /** Get the path (and filename) of a standard 14 font.
     * \param stdFontType Type of the standard font.
     * \return Path of the font file (empty if not found).
     */
protected:
    static QString standard14FontPath( PdfeFont14Standard::Enum stdFontType );
    /** Get the data of a standard 14 font.
     * \param stdFontType Type of the standard font.
     * \return QByteArray containing data (COW implies that no data is copied twice).
     */
    static QByteArray standard14FontData( PdfeFont14Standard::Enum stdFontType );

    /** Static function that return what are considered as space characters.
     * \return Constant reference to a vector of QChar containing space characters.
     * By convention the first one is the classic space 0x0020.
     */
    static const std::vector<QChar>& spaceCharacters();

private:
    /// Standard font names (multiple given for each font).
    static const char* Standard14FontNames[][10];

private:
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
    /// Index of the charmaps (1,0), (3,0) and (3,1) in FT face.
    /// -1 if it does exist in FreeType face.
    std::vector<int>  m_ftCharmapsIdx;

    /// Font encoding.
    PoDoFo::PdfEncoding*  m_pEncoding;
    /// Does the object owns the encoding ?
    bool  m_encodingOwned;
    /// Unicode CMap.
    PdfeCMap  m_unicodeCMap;

    /// Vector of space characters (pair of CID and space type).
    std::vector< std::pair<pdfe_cid,PdfeFontSpace::Enum> >  m_spaceCharacters;

protected:
    // Protected Getters.
    /// Get font face object.
    FT_Face ftFace() const                      {   return m_ftFace;   }
    /// Get charmap index.
    int ftCharmapIndex( FTCharmap cmapType ) const  {   return m_ftCharmapsIdx[cmapType];   }
    /// Get encoding object pointer.
    const PoDoFo::PdfEncoding* pEncoding() const    {   return m_pEncoding; }
    /// Get unicode CMap pointer.
    const PdfeCMap* pUnicodeCMap() const        {   return &m_unicodeCMap;  }

public:
    //**********************************************************//
    //                     Getters / Setters                    //
    //**********************************************************//
    PdfeFontType::Enum type() const             {  return m_type;       }
    PdfeFontSubType::Enum subtype() const       {  return m_subtype;    }

    void setType( PdfeFontType::Enum type )             {  m_type = type;  }
    void setSubtype( PdfeFontSubType::Enum subtype )    {  m_subtype = subtype;  }

    double charSpace() const    {  return m_charSpace;  }
    double wordSpace() const    {  return m_wordSpace;  }
    double hScale() const       {  return m_hScale;     }
    double fontSize() const     {  return m_fontSize;   }

    void setCharSpace( double charSpace )   {  m_charSpace = charSpace;  }
    void setWordSpace( double wordSpace )   {  m_wordSpace = wordSpace;  }
    void setHScale( double hScale )         {  m_hScale = hScale;  }
    void setFontSize( double fontSize )     {  m_fontSize = fontSize;  }
};

}

#endif // PDFEFONT_H
