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

#ifndef PRPAGE_H
#define PRPAGE_H

#include <QObject>
#include "PRDocument.h"

namespace PoDoFo {
class PdfPage;
}

namespace PdfRecut {

class PRSubDocument;

/** Class that represent a page in a PDF document.
 * It describes its basic geometry (text, paths and images).
 */
class PRPage : public QObject
{
    Q_OBJECT

public:
    /** Default constructor. The page is related to a parent
     * PRSubDocument and has a given index in the PDF.
     * \param parent Pointer to the parent PRSubDocument.
     * \param pageIndex Index of the page.
     */
    explicit PRPage( PRSubDocument* parent,
                     size_t pageIndex );
    /** Destructor.
     */
    virtual ~PRPage();
    
public:
    // Contents related member functions.
    /** Analyse the content of the sub-document
     * Create an internal structure (based on PRPage,...) that
     * describes the geometry of the PDF content.
     * \param params Parameters used for the analysis.
     */
    void analyseContent( const PRDocument::ContentParameters& params );
    /** Clear the internal structure which describes the sub-document content.
     */
    void clearContent();

    /** Reimplement QObject parent function.
     * \return Pointer to the parent PRSubDocument.
     */
    PRSubDocument* parent() const;

private:
    // No copy constructor and operator= allowed.
    Q_DISABLE_COPY(PRPage)

private:
    /// Page index.
    size_t  m_pageIndex;
    /// Pointet to the PoDoFo page;
    PoDoFo::PdfPage*  m_page;
    
};

}

#endif // PRPAGE_H
