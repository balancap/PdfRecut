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

#ifndef PRGTEXTPAGE_H
#define PRGTEXTPAGE_H

#include <QObject>
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
class PRGPage;
class PRGTextLine;
class PRGTextGroupWords;

/** Class that describes the text organization in PDF page:
 * - Groups of words that belong to the page;
 * - Text lines that can be detected.
 */
class PRGTextPage : public QObject, public PoDoFoExtended::PdfeCanvasAnalysis
{
    Q_OBJECT

public:
    /** Default constructor. The text page object must linked to
     * a parent page PRGPage.
     * \param page Parent page object.
     * \param pageIndex Index of the page to render.
     */
    PRGTextPage( PRGPage* page );
    /** Destructor.
     */
    virtual ~PRGTextPage();

public:
    // Cached data.
    /** Load text page data: groups of words. It not existing, it
     * also create the basic structure that describes page data.
     */
    void loadData();
    /** Clear page cached data. It clears data that can be easily retrieve
     * using page content stream. Basic skeleton of page organisation is kept in memory.
     */
    void clearData();
signals:
    /** Qt signal: text page data has been loaded!
     * \param page Pointer to the parent page it belongs to.
     */
    void dataLoaded( PRGPage* page );

public:
    /** Detect text lines components inside the page.
     */
    void detectLines();

    /** Clear completely page content (i.e. groups of words, lines, ...).
     */
    void clear();

private:
    /** Create a line for a group of words. Try to gather other groups inside the same line.
     * Basic algorithm used.
     * \param idxGroupWords Index of the group of words.
     * \return Pointer to the line object.
     */
    PRGTextLine* createLine_Basic( size_t idxGroupWords );
    /** Try to merge existing lines.
     * Enlarge inside algorithm: detect elements inside a line and check if they belong to it.
     * \param pLine Pointer of the line to consider.
     * \param minBaseHeight Minimum height used in the enlargement algorithm.
     * \param maxBaseHeight Maximum height used in the enlargement algorithm.
     * \param minLineWidth Minimum length of lines.
     * \return Pointer the merged line.
     */
    PRGTextLine* mergeLines_EnlargeInside( PRGTextLine* pBaseLine,
                                           double minBaseHeight,
                                           double maxBaseHeight,
                                           double minLineWidth );
    /** Try to merge existing lines. Depreciated.
     * Enlarge outside algorithm: detect elements in the neighbourhood of a line and check if they belong to it.
     * \param pLine Pointer of the line to consider.
     * \return Pointer the merged line.
     */
    PRGTextLine* mergeLines_EnlargeOutside( PRGTextLine* pBaseLine,
                                            double scaleXEnlarge,
                                            double scaleYEnlarge,
                                            double maxLineWidth );

    /** Try to merge existing lines. Depreciated.
     * Inside algorithm: detect elements inside a line and check if they belong to it.
     * \param pLine Pointer of the line to consider.
     * \return Pointer the merged line.
     */
    PRGTextLine* mergeLines_Inside( PRGTextLine* pLine );
    /** Try to merge existing lines. Depreciated.
     * Small elements algorithm: detect small elements close to a line and check if they belong to it.
     * \param pLine Pointer of the line to consider.
     * \return Pointer the merged line.
     */
    PRGTextLine* mergeLines_Small( PRGTextLine* pLine );

    /** Split lines using horizontal blocks.
     * \param pLine Pointer of the line to consider.
     * \return Vector of new lines (the first element corresponds to pLine).
     */
    std::vector<PRGTextLine*> splitLines_hBlocks( PRGTextLine* pLine );
    /** Merge a vector of lines into a single one.
     * Use the first element as base (other lines are deleted).
     * \param pLines Vector of pointer of lines to merge.
     * \return Pointer to the resulting line.
     */
    PRGTextLine* mergeVectorLines( const std::vector<PRGTextLine*>& pLines );

protected:
    // Reimplement PdfeCanvasAnalysis interface.
    /** Reimplementation of text showing function of PdfeCanvasAnalysis.
     * Used to read text groups of words information.
     */
    virtual PdfeVector fTextShowing( const PoDoFoExtended::PdfeStreamState& streamState );

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
    void renderGroupsWords( PRRenderPage& renderPage ) const;
    /** Render lines of text present in the page.
     * \param renderPage PRRenderPage on which lines are rendered.
     */
    void renderLines( PRRenderPage& renderPage ) const;

public:
    // Getters.
    /// Parent page object.
    PRGPage* page() const   {   return m_page;  }
    /// Reimplement QObject parent function with PRGPage.
    PRGPage* parent() const;

private:
    // No copy constructor and operator= allowed.
    Q_DISABLE_COPY(PRGTextPage)

private:
    /// Pointer to the parent PRGPage object.
    PRGPage*  m_page;

    /// Number of text groups read in the page content stream.
    long  m_nbGroupsStream;
    /// Number of groups kept inside the page.
    long  m_nbGroupsPage;
    /// Groups of words that belong to the page (vector of pointers).
    std::vector<PRGTextGroupWords*>  m_pGroupsWords;
    /// Text lines detected inside the page (vector of pointers).
    std::vector<PRGTextLine*>  m_pTextLines;
};

}

#endif // PRGTEXTPAGE_H
