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

#ifndef PRSUBDOCUMENT_H
#define PRSUBDOCUMENT_H

#include <QObject>
#include "PRDocument.h"

namespace PdfRecut {

class PRPage;

/** Class that represents a sub-document, i.e. a range of pages,
 * usually homogeneous in certain way, from a parent PRDocument.
 */
class PRSubDocument : public QObject
{
    Q_OBJECT

public:
    /** Default constructor: initialize pointers and filename.
     * \param parent Parent PRDocument.
     * \param firstPageIndex First page of the sub-document.
     * \param lastPageIndex Last page of the sub-document.
     */
    explicit PRSubDocument( PRDocument* parent,
                            size_t firstPageIndex,
                            size_t lastPageIndex );
    /** Destructor.
     */
    virtual ~PRSubDocument();

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

public:
    /** Reimplement QObject parent function.
     * \return Pointer to the parent PRDocument.
     */
    PRDocument* parent() const;

public:
    // Getters...
    /// Number of pages in the sub-document.
    size_t nbPages() const  {   return (m_lastPageIndex-m_firstPageIndex+1);    }
    /// Get a page object.
    PRPage* page( size_t idx );
    const PRPage* page( size_t idx ) const;
    /// First page index in the sub-document.
    size_t firstPageIndex() const   {   return m_firstPageIndex;    }
    /// Last page index in the sub-document.
    size_t lastPageIndex() const    {   return m_lastPageIndex;    }

private:
    // No copy constructor and operator= allowed.
    Q_DISABLE_COPY(PRSubDocument)

private:
    /// Index of the first page of the sub-document.
    size_t  m_firstPageIndex;
    /// Index of the last page of the sub-document.
    size_t  m_lastPageIndex;
    
    /// Vector of PRPage (pointers) corresponding the page range.
    std::vector<PRPage*>  m_pages;
};

}

#endif // PRSUBDOCUMENT_H
