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

#ifndef PRGSUBDOCUMENT_H
#define PRGSUBDOCUMENT_H

#include <QObject>
#include "PRGDocument.h"

namespace PdfRecut {

class PRGPage;

/** Class that represents a sub-document, i.e. a range of pages that
 * should be homogeneous from a geometrical point of view.
 */
class PRGSubDocument : public QObject
{
    Q_OBJECT

public:
    /** Default constructor: initialize parent PRGDocument and page range.
     * \param parent Parent PRGDocument.
     * \param firstPageIndex First page of the sub-document.
     * \param lastPageIndex Last page of the sub-document.
     */
    explicit PRGSubDocument( PRGDocument* parent,
                             size_t firstPageIndex,
                             size_t lastPageIndex );
    /** Destructor.
     */
    virtual ~PRGSubDocument();
    /** Reimplement QObject parent function.
     * \return Pointer to the parent PRGDocument.
     */
    PRGDocument* parent() const;

public:
    // Contents related member functions.
    /** Analyse the geometry of the sub-document
     * Create an internal structure (based on PRGPage,...) that
     * describes the geometry of the PDF content.
     * \param params Parameters used for the analysis.
     */
    void analyse( const PRGDocument::GParameters& params );
    /** Clear the internal structure which describes the sub-document geometry.
     */
    void clear();

public:
    // Getters...
    /// Number of pages in the sub-document.
    size_t nbPages() const  {   return (m_lastPageIndex-m_firstPageIndex+1);    }
    /// Get a page object.
    PRGPage* page( size_t idx );
    const PRGPage* page( size_t idx ) const;
    /// First page index in the sub-document.
    size_t firstPageIndex() const   {   return m_firstPageIndex;    }
    /// Last page index in the sub-document.
    size_t lastPageIndex() const    {   return m_lastPageIndex;    }

private:
    // No copy constructor and operator= allowed.
    Q_DISABLE_COPY(PRGSubDocument)

private:
    /// Index of the first page of the sub-document.
    size_t  m_firstPageIndex;
    /// Index of the last page of the sub-document.
    size_t  m_lastPageIndex;
    
    /// Vector of PRGPage (pointers) corresponding the page range.
    std::vector<PRGPage*>  m_pages;
};

}

#endif // PRGSUBDOCUMENT_H
