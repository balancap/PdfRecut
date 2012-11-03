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

#ifndef PDFEFONTTYPE3_H
#define PDFEFONTTYPE3_H

#include "PdfeFont.h"
#include "PdfeCanvasAnalysis.h"

#include "podofo/base/PdfCanvas.h"

namespace PoDoFoExtended {

class PdfeGlyphType3;
//**********************************************************//
//                       PdfeFontType3                      //
//**********************************************************//
/** Class that represents a PDF Font of type 3 (c.f. PDF Reference).
 */
class PdfeFontType3 : public PdfeFont
{
public:
    /** Create a PdfeFontType3 from a PdfObject.
     * \param pFont Pointer to the object where is defined the type 3 font.
     */
    PdfeFontType3( PoDoFo::PdfObject* pFont, FT_Library ftLibrary );

    /** Initialize the object to default parameters.
     */
    void init();

    /** Virtual destructor.
     */
    virtual ~PdfeFontType3();

protected:
    /** Private default constructor.
     */
    PdfeFontType3();

   /** Initialize the vector of glyphs and their bounding box.
     * \param pFont Pointer to the object where is defined the type 3 font.
     */
    void initGlyphs( const PoDoFo::PdfObject* pFont );

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

    /** Get default height used for space characters of the font.
     * \return Space height.
     */
    virtual double spaceHeight() const;
    /** Compute some font statistics.
     * \param defaultValue Use some predefined default values (for Type 0/1 and TrueType fonts).
     * \return PdfeFont::Statistics object containing data.
     */
    virtual Statistics statistics( bool defaultValue = false ) const {
        // No default value for Type 3 fonts.
        return this->PdfeFont::statistics( false );
    }

    /** Convert a character CID to the corresponding glyph GID.
     * Need the freetype face, unicode CMap and encoding to be initialized.
     * \param c Character CID;
     * \return Glyph GID. 0 if not found.
     */
    virtual pdfe_gid fromCIDToGID( pdfe_cid c ) const;

private:
    // Members.
    /// Font BBox.
    PoDoFo::PdfRect  m_fontBBox;
    /// Font Matrix.
    PdfeMatrix  m_fontMatrix;

    /// First character defined in font's width array.
    pdfe_cid  m_firstCID;
    /// Last character defined in font's width array.
    pdfe_cid  m_lastCID;

    /// Array of advance vectors.
    std::vector<PdfeVector>  m_advanceCID;

    /// Font descriptor.
    PdfeFontDescriptor  m_fontDescriptor;

    /// Vector of type 3 glyphs.
    std::vector<PdfeGlyphType3>  m_glyphs;
    /// CID to GID map.
    std::vector<pdfe_gid>  m_mapCIDToGID;
};

//**********************************************************//
//                      PdfeGlyphType3                      //
//**********************************************************//
/** Class used to represent a glyph from a Type 3 font.
 * Inherit from the interface PdfeCanvasAnalysis and the class PdfCanvas.
 */
class PdfeGlyphType3 : public PdfeCanvasAnalysis, public PoDoFo::PdfCanvas
{
public:
    /** Default constructor.
     */
    PdfeGlyphType3();

    /** Main constructor.
     * \param glyphName Name of the glyph used in CharProcs.
     * \param glyphStream Object where is defined the glyph stream.
     * \param fontResources Object containing font resources.
     */
    PdfeGlyphType3( const PoDoFo::PdfName& glyphName,
                    PoDoFo::PdfObject* glyphStream,
                    PoDoFo::PdfObject* fontResources );

    /** Get glyph bounding box.
     * \return PdfRect containing the bbox.
     */
    PoDoFo::PdfRect bbox() const;

protected:
    /** Compute bounding box.
     */
    void computeBBox();

protected:
    // Reimplement PdfeCanvasAnalysis interface.
    virtual void fGeneralGState( const PdfeStreamState& streamState ) { }

    virtual void fSpecialGState( const PdfeStreamState& streamState ) { }

    virtual void fPathConstruction( const PdfeStreamState& streamState,
                                    const PdfePath& currentPath ) { }

    virtual void fPathPainting( const PdfeStreamState& streamState,
                                const PdfePath& currentPath );

    virtual void fClippingPath( const PdfeStreamState& streamState,
                                const PdfePath& currentPath ) { }

    virtual void fTextObjects( const PdfeStreamState& streamState ) { }

    virtual void fTextState( const PdfeStreamState& streamState ) { }

    virtual void fTextPositioning( const PdfeStreamState& streamState ) { }

    virtual PdfeVector fTextShowing( const PdfeStreamState& streamState ) { return PdfeVector(); }

    virtual void fType3Fonts( const PdfeStreamState& streamState );

    virtual void fColor( const PdfeStreamState& streamState ) { }

    virtual void fShadingPatterns( const PdfeStreamState& streamState ) { }

    virtual void fInlineImages( const PdfeStreamState& streamState ) { }

    virtual void fXObjects( const PdfeStreamState& streamState ) { }

    virtual void fMarkedContents( const PdfeStreamState& streamState ) { }

    virtual void fCompatibility( const PdfeStreamState& streamState ) { }

    virtual void fUnknown( const PdfeStreamState& streamState );

    virtual void fFormBegin( const PdfeStreamState& streamState,
                             PoDoFo::PdfXObject* form ) { }

    virtual void fFormEnd( const PdfeStreamState& streamState,
                           PoDoFo::PdfXObject* form ) { }

public:
    // Reimplement PdfCanvas interface.
    virtual PoDoFo::PdfObject* GetContents() const;
    virtual PoDoFo::PdfObject* GetContentsForAppending() const;
    virtual PoDoFo::PdfObject* GetResources() const;
    virtual const PoDoFo::PdfRect GetPageSize() const;

protected:
    /// Glyph name (c.f. CharProcs entry).
    PoDoFo::PdfName  m_name;
    /// Stream object that defines the glyph.
    PoDoFo::PdfObject*  m_pStream;
    /// Font resources object.
    PoDoFo::PdfObject*  m_pResources;

    // Cache data.
    /// Has the bbox been computed?
    bool  m_isBBoxComputed;
    /// BBox defined by the operator d1 (=0.0 if not defined).
    PoDoFo::PdfRect  m_bboxD1;
    /// CBox corresponding computed using glyph stream.
    PoDoFo::PdfRect  m_cbox;
};

}

#endif // PDFEFONTTYPE3_H
