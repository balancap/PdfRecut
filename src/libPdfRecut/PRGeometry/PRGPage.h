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

#ifndef PRGPAGE_H
#define PRGPAGE_H

#include <QObject>
#include "PRGDocument.h"

namespace PoDoFo {
class PdfPage;
class PdfRect;
}

namespace PdfRecut {

class PRGSubDocument;
class PRGTextPage;

/** Class that represent a page in a PDF document.
 * It describes page's basic objects (text, paths and images),
 * and the geometrical links between them.
 */
class PRGPage : public QObject
{
    Q_OBJECT

public:
    /** Default constructor. The page must be related to a parent
     * PRGSubDocument and has a given index in the PDF document.
     * \param parent Pointer to the parent PRGSubDocument.
     * \param pageIndex Index of the page.
     */
    explicit PRGPage( PRGSubDocument* parent,
                      size_t pageIndex );
    /** Destructor.
     */
    virtual ~PRGPage();
    /** Reimplement QObject parent function.
     * \return Pointer to the parent PRGSubDocument.
     */
    PRGSubDocument* parent() const;

public:
    // Contents related member functions.
    /** Analyse the geometry of the page content.
     * Create an internal structure (based on PRGTextPage,...) that
     * describes the precise internal geometry of the PDF page.
     * \param params Parameters used for the analysis.
     */
    void analyse( const PRGDocument::GParameters& params );
    /** Clear the internal structure which describes the sub-document content.
     */
    void clear();


public:
    // Some static functions that can be useful.
    /// Get the media box of a PoDoFo page.
    static PoDoFo::PdfRect PageMediaBox( PoDoFo::PdfPage* pPage );
    /// Get the crop box of a PoDoFo page (check the coherence with media box).
    static PoDoFo::PdfRect PageCropBox( PoDoFo::PdfPage* pPage );


private:
    // No copy constructor and operator= allowed.
    Q_DISABLE_COPY(PRGPage)

private:
    /// Page index.
    size_t  m_pageIndex;
    /// Pointer to the PoDoFo page;
    PoDoFo::PdfPage*  m_page;

    /// Text elements in the page.
    PRGTextPage*  m_text;
    
};

}

#endif // PRGPAGE_H
