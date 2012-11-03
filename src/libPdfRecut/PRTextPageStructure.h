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

#ifndef PRTEXTPAGESTRUCTURE_H
#define PRTEXTPAGESTRUCTURE_H

#include "PRDocument.h"
#include "PRTextWords.h"
#include "PRTextLine.h"

#include "PdfeCanvasAnalysis.h"

namespace PoDoFo {
class PdfPage;
class PdfRect;
class PdfMemDocument;
class PdfString;
class PdfVariant;
}

namespace PdfRecut {

class PRRenderPage;

/** Class that analyse the text structure of a PDF page.
 */
class PRTextPageStructure : public PoDoFoExtended::PdfeCanvasAnalysis
{
public:
    /** Default constructor
     * \param document Input document.
     * \param pageIndex Index of the page to render.
     */
    PRTextPageStructure( PRDocument* document,
                         long pageIndex );

    /** Destructor.
     */
    virtual ~PRTextPageStructure();

protected:
    /** Clear content of the object (i.e. different vectors).
     */
    void clearContent();

public:
    /** Detect the groups of words in the page.
     */
    void detectGroupsWords();
    /** Detect text lines structure of the page.
     */
    void detectLines();

private:
    /** Create a line for a group of words. Try to gather other groups inside the same line.
     * Basic algorithm used.
     * \param idxGroupWords Index of the group of words.
     * \return Pointer to the line object.
     */
    PRTextLine* createLine_Basic( size_t idxGroupWords );

    /** Try to merge existing lines.
     * Enlarge inside algorithm: detect elements inside a line and check if they belong to it.
     * \param pLine Pointer of the line to consider.
     * \param minBaseHeight Minimum height used in the enlargement algorithm.
     * \param maxBaseHeight Maximum height used in the enlargement algorithm.
     * \param minLineWidth Minimum length of lines.
     * \return Pointer the merged line.
     */
    PRTextLine* mergeLines_EnlargeInside( PRTextLine* pBaseLine,
                                          double minBaseHeight,
                                          double maxBaseHeight,
                                          double minLineWidth );

    /** Try to merge existing lines. Depreciated.
     * Enlarge outside algorithm: detect elements in the neighbourhood of a line and check if they belong to it.
     * \param pLine Pointer of the line to consider.
     * \return Pointer the merged line.
     */
    PRTextLine* mergeLines_EnlargeOutside( PRTextLine* pBaseLine,
                                           double scaleXEnlarge,
                                           double scaleYEnlarge,
                                           double maxLineWidth );

    /** Try to merge existing lines. Depreciated.
     * Inside algorithm: detect elements inside a line and check if they belong to it.
     * \param pLine Pointer of the line to consider.
     * \return Pointer the merged line.
     */
    PRTextLine* mergeLines_Inside( PRTextLine* pLine );

    /** Try to merge existing lines. Depreciated.
     * Small elements algorithm: detect small elements close to a line and check if they belong to it.
     * \param pLine Pointer of the line to consider.
     * \return Pointer the merged line.
     */
    PRTextLine* mergeLines_Small( PRTextLine* pLine );

    /** Split lines using horizontal blocks.
     * \param pLine Pointer of the line to consider.
     * \return Vector of new lines (the first element corresponds to pLine).
     */
    std::vector<PRTextLine*> splitLines_hBlocks( PRTextLine* pLine );

    /** Merge a vector of lines into a single one.
     * Use the first element as base (other lines are deleted).
     * \param pLines Vector of pointer of lines to merge.
     * \return Pointer to the resulting line.
     */
    PRTextLine* mergeVectorLines( const std::vector<PRTextLine*>& pLines );

    /** No copy constructor allowed.
     */
    PRTextPageStructure( const PRTextPageStructure& rhs );

protected:
    // Reimplement PdfeCanvasAnalysis interface.
    virtual void fGeneralGState( const PoDoFoExtended::PdfeStreamState& streamState ) { }

    virtual void fSpecialGState( const PoDoFoExtended::PdfeStreamState& streamState ) { }

    virtual void fPathConstruction( const PoDoFoExtended::PdfeStreamState& streamState,
                                    const PoDoFoExtended::PdfePath& currentPath ) { }

    virtual void fPathPainting( const PoDoFoExtended::PdfeStreamState& streamState,
                                const PoDoFoExtended::PdfePath& currentPath ) { }

    virtual void fClippingPath( const PoDoFoExtended::PdfeStreamState& streamState,
                                const PoDoFoExtended::PdfePath& currentPath ) { }

    virtual void fTextObjects( const PoDoFoExtended::PdfeStreamState& streamState ) { }

    virtual void fTextState( const PoDoFoExtended::PdfeStreamState& streamState ) { }

    virtual void fTextPositioning( const PoDoFoExtended::PdfeStreamState& streamState ) { }

    /** Reimplementation of text showing function from PRRenderPage.
     * Used to read text groups of words.
     */
    virtual PdfeVector fTextShowing( const PoDoFoExtended::PdfeStreamState& streamState );

    virtual void fType3Fonts( const PoDoFoExtended::PdfeStreamState& streamState ) { }

    virtual void fColor( const PoDoFoExtended::PdfeStreamState& streamState ) { }

    virtual void fShadingPatterns( const PoDoFoExtended::PdfeStreamState& streamState ) { }

    virtual void fInlineImages( const PoDoFoExtended::PdfeStreamState& streamState ) { }

    virtual void fXObjects( const PoDoFoExtended::PdfeStreamState& streamState ) { }

    virtual void fMarkedContents( const PoDoFoExtended::PdfeStreamState& streamState ) { }

    virtual void fCompatibility( const PoDoFoExtended::PdfeStreamState& streamState ) { }

    virtual void fUnknown( const PoDoFoExtended::PdfeStreamState& streamState ) { }

    virtual void fFormBegin( const PoDoFoExtended::PdfeStreamState& streamState,
                             PoDoFo::PdfXObject* form ) { }

    virtual void fFormEnd( const PoDoFoExtended::PdfeStreamState& streamState,
                           PoDoFo::PdfXObject* form ) { }

public:
    // Rendering routines.
    /** Render the groups of words present in the page.
     * \param renderPage PRRenderPage on which words are rendered.
     */
    void renderTextGroupsWords( PRRenderPage& renderPage );
    /** Render lines of text present in the page.
     * \param renderPage PRRenderPage on which lines are rendered.
     */
    void renderTextLines( PRRenderPage& renderPage );

private:
    /// Pointer to PRDocument object.
    PRDocument*  m_document;
    /// Page pointer.
    PoDoFo::PdfPage*  m_page;
    /// Page index.
    long  m_pageIndex;

    /// Number of text groups read.
    long  m_nbTextGroups;

    /// Groups of words that belong to the page (vector of pointers).
    std::vector<PRTextGroupWords*>  m_pGroupsWords;
    /// Text lines detected inside the page (vector of pointers).
    std::vector<PRTextLine*>  m_pTextLines;
};

}

#endif // PRTEXTPAGESTRUCTURE_H
