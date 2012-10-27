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
    PdfeFontType0( PoDoFo::PdfObject* pFont, FT_Library ftLibrary );

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
    // Implementation of PdfeFont interface.
    /** Get the descriptor object corresponding to the font.
     * \return Constant reference to a PdfeFontDescriptor object.
     */
    virtual const PdfeFontDescriptor& fontDescriptor() const;

    /** Get the font bounding box.
     * \return PoDoFo::PdfRect containing the font bounding box.
     */
    virtual PoDoFo::PdfRect fontBBox() const;

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
    virtual PdfeCIDString toCIDString( const PoDoFo::PdfString& str ) const;
    /** Convert a character to a unicode QString.
     * \param  c Character identifier (CID).
     * \param useUCMap Try to use the unicode CMap to convert.
     * \return Unicode QString representing the character.
     */
    virtual QString toUnicode( pdfe_cid c,
                               bool useUCMap = true,
                               bool firstTryEncoding = false ) const;
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
    virtual double spaceHeight() const;
    /** Compute some font statistics.
     * \param defaultValue Use some predefined default values (for Type 0/1 and TrueType fonts).
     * \return PdfeFont::Statistics object containing data.
     */
    virtual Statistics statistics( bool defaultValue = false ) const {
        return this->PdfeFont::statistics( defaultValue );
    }

    /** Convert a character CID to the corresponding glyph GID.
     * Need the freetype face, unicode CMap and encoding to be initialized.
     * \param c Character CID;
     * \return Glyph GID. 0 if not found.
     */
    virtual pdfe_gid fromCIDToGID( pdfe_cid c ) const;

private:
    // Members.
    /// The PostScript name of the font.
    PoDoFo::PdfName  m_baseFont;
    /// CMap encoding of the font.
    PdfeCMap  m_encodingCMap;

    /// Descendant CID font.
    PdfeFontCID*  m_fontCID;
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

    /** Initialize characters bounding box using a FreeType face.
     * \param FreeType face use to retrieve glyph information.
     */
    void initCharactersBBox( FT_Face ftFace );

public:
    /** Get the descriptor object corresponding to the font.
     * \return Constant reference to a PdfeFontDescriptor object.
     */
    const PdfeFontDescriptor& fontDescriptor() const;

    /** Get the width of a character.
     * \param c Character identifier (CID).
     * \return Width of the character.
     */
    double width( pdfe_cid c ) const;
    /** Get the advance vector of a character (horizontal or vertical usually).
     * \param c Character identifier (CID).
     * \return Advance vector.
     */
    virtual PdfeVector advance( pdfe_cid c ) const;
    /** Get the bounding box of a character.
     * \param c Character identifier (CID).
     * \return Bounding box of the character.
     */
    PoDoFo::PdfRect bbox( pdfe_cid c ) const;

    /** The vector that define first CID of each group.
     * \return Constant reference to a vector of CID.
     */
    const std::vector<pdfe_cid>& firstCIDs() const;
    /** The vector that define last CID of each group.
     * \return Constant reference to a vector of CID.
     */
    const std::vector<pdfe_cid>& lastCIDs() const;

    /** Get default height used for space characters of the font.
     * \return Space height.
     */
    double spaceHeight() const;

protected:
    /** Private embedded class that represents an array of glyph's horizontal
     * bounding boxes.
     */
    class HBBoxArray
    {
    public:
        /** Default constructor.
         */
        HBBoxArray();

        /** Initialize to a default object, with DW=1000.
         */
        void init();
        /** Initialize given a PdfArray from a PdfFont object,
         * a FreeType font face and the default font bounding box.
         * \param widths PdfArray containing CID and widths informations.
         * \param defWidth Default width.
         * \param fontBBox Font bounding box, used for default height.
         */
        void init( const PoDoFo::PdfArray& widths,
                   double defaultWidth,
                   const PoDoFo::PdfRect& fontBBox );

        /** Initialize characters bounding box.
         * \param FreeType face use to retrieve glyph information.
         */
        void initCharactersBBox(FT_Face ftFace );

        /** Get default width.
         * \return Default width of CID glyphs.
         */
        double defaultWidth() const;
        /** Get default bounding box for a character.
         * \return Default bounding box of CID glyphs.
         */
        PoDoFo::PdfRect defaultBBox() const;

        /** Get the width of a character.
         * \param c Character identifier (CID).
         * \return Width of the character.
         */
        double width( pdfe_cid c ) const;
        /** Get the advance vector of a character (horizontal).
         * \param c Character identifier (CID).
         * \return Advance vector.
         */
        virtual PdfeVector advance( pdfe_cid c ) const;
        /** Get the bounding box of a character.
         * \param c Character identifier (CID).
         * \return Bounding box of the character.
         */
        PoDoFo::PdfRect bbox( pdfe_cid c ) const;

        /** The vector that define first CID of each group.
         * \return Constant reference to a vector of CID.
         */
        const std::vector<pdfe_cid>& firstCIDs() const;
        /** The vector that define last CID of each group.
         * \return Constant reference to a vector of CID.
         */
        const std::vector<pdfe_cid>& lastCIDs() const;

    private:
        /// Vector containing first CID of each group.
        std::vector<pdfe_cid>  m_firstCID;
        /// Vector containing last CID of each group.
        std::vector<pdfe_cid>  m_lastCID;

        /// Vector containing the advance vector of CID of each group.
        std::vector< std::vector<PdfeVector> >  m_advanceCID;
        /// Vector containing bounding box of CID of each group.
        std::vector< std::vector<PoDoFo::PdfRect> >  m_bboxCID;

        /// Default horizontal advance vector.
        PdfeVector  m_defaultAdvance;
        /// Default horizontal bounding box.
        PoDoFo::PdfRect  m_defaultBBox;
    };

private:
    // Members for a CID font.
    /// Font type.
    PdfeFontType::Enum  m_type;
    /// Font subtype.
    PdfeFontSubType::Enum  m_subtype;
    /// The PostScript name of the font (retrieved from font descriptor).
    PoDoFo::PdfName  m_baseFont;

    /// CIDSystemInfo (character collection).
    PdfeCIDSystemInfo  m_cidSystemInfo;
    /// Font descriptor.
    PdfeFontDescriptor  m_fontDescriptor;
    /// Horizontal bounding boxes of characters.
    HBBoxArray  m_hBBoxes;
};


//**********************************************************//
//                    Inline PdfeFontCID                    //
//**********************************************************//
inline const PdfeFontDescriptor& PdfeFontCID::fontDescriptor() const
{
    return m_fontDescriptor;
}
inline double PdfeFontCID::width( pdfe_cid c ) const
{
    return m_hBBoxes.width( c ) * 0.001;
}
inline const std::vector<pdfe_cid>& PdfeFontCID::firstCIDs() const
{
    return m_hBBoxes.firstCIDs();
}
inline const std::vector<pdfe_cid>& PdfeFontCID::lastCIDs() const
{
    return m_hBBoxes.lastCIDs();
}
inline double PdfeFontCID::spaceHeight() const
{
    // Default value
    return 0.5;
}

//**********************************************************//
//               Inline PdfeFontCID::HBBoxArray             //
//**********************************************************//
inline double PdfeFontCID::HBBoxArray::width( pdfe_cid c ) const
{
    // Find the group of CID it belongs to.
    for( size_t i = 0 ; i < m_bboxCID.size() ; ++i ) {
        if( c >= m_firstCID[i] && c <= m_lastCID[i] ) {
            return m_bboxCID[i][c - m_firstCID[i]].GetWidth();
        }
    }
    // Return default width.
    return m_defaultBBox.GetWidth();
}
inline PdfeVector PdfeFontCID::HBBoxArray::advance( pdfe_cid c ) const
{
    // Find the group of CID it belongs to.
    for( size_t i = 0 ; i < m_advanceCID.size() ; ++i ) {
        if( c >= m_firstCID[i] && c <= m_lastCID[i] ) {
            return m_advanceCID[i][c - m_firstCID[i]];
        }
    }
    // Return default advance vector.
    return m_defaultAdvance;
}
inline PoDoFo::PdfRect PdfeFontCID::HBBoxArray::bbox( pdfe_cid c ) const
{
    // Find the group of CID it belongs to.
    for( size_t i = 0 ; i < m_bboxCID.size() ; ++i ) {
        if( c >= m_firstCID[i] && c <= m_lastCID[i] ) {
            return m_bboxCID[i][c - m_firstCID[i]];
        }
    }
    // Return default width.
    return m_defaultBBox;
}
inline double PdfeFontCID::HBBoxArray::defaultWidth() const
{
    return m_defaultBBox.GetWidth();
}
inline PoDoFo::PdfRect PdfeFontCID::HBBoxArray::defaultBBox() const
{
    return m_defaultBBox;
}
inline const std::vector<pdfe_cid>& PdfeFontCID::HBBoxArray::firstCIDs() const
{
    return m_firstCID;
}
inline const std::vector<pdfe_cid>& PdfeFontCID::HBBoxArray::lastCIDs() const
{
    return m_lastCID;
}

}

#endif // PDFEFONTTYPE0_H
