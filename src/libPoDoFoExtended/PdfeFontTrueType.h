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

#ifndef PDFEFONTTRUETYPE_H
#define PDFEFONTTRUETYPE_H

#include "PdfeFont.h"

namespace PoDoFoExtended {

/** Class that represents a PDF TrueType Font (c.f. PDF Reference).
 */
class PdfeFontTrueType : public PdfeFont
{
public:
    /** Create a PdfeFontTrueType from a PdfObject.
     * \param pFont Pointer to the object where is defined the TrueType font.
     */
    PdfeFontTrueType( PoDoFo::PdfObject* pFont, FT_Library ftLibrary );
    /** Initialize the object to default parameters.
     */
    void init();
    /** Virtual destructor.
     */
    virtual ~PdfeFontTrueType();

private:
    /** No default constructor.
     */
    PdfeFontTrueType() { }
    /** Initialize the size of characters according to the font object.
     * \param pFont Pointer to the object where is defined the TrueType font.
     */
    void initCharactersBBox( const PoDoFo::PdfObject* pFont );

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
    /** Get default width used for space characters of the font.
     * \return Space width.
     */
    virtual double spaceWidth() const {
        return this->PdfeFont::spaceWidth();
    }
    /** Get default height used for space characters of the font.
     * \return Space height.
     */
    virtual double spaceHeight() const {
        return this->PdfeFont::spaceHeight();
    }
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
    /// The PostScript name of the font (retrieved from font descriptor).
    PoDoFo::PdfName  m_baseFont;

    /// First character defined in font's width array.
    pdfe_cid  m_firstCID;
    /// Last character defined in font's width array.
    pdfe_cid  m_lastCID;

    /// Array of advance vectors (horizontal for TrueType fonts).
    std::vector<PdfeVector>  m_advanceCID;
    /// Array storing the bounding box of characters.
    std::vector<PoDoFo::PdfRect>  m_bboxCID;

    /// Font descriptor.
    PdfeFontDescriptor  m_fontDescriptor;
};

}

#endif // PDFEFONTTRUETYPE_H
