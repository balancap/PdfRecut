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

#ifndef PRGDOCUMENT_H
#define PRGDOCUMENT_H

#include <vector>
#include <list>
#include <QObject>

namespace PdfRecut {

class PRDocument;
class PRGSubDocument;
class PRGPage;

//************************************************************//
//                         PRGDocument                        //
//************************************************************//
/** Class that describes the basic geometry of PDF document.
 * The description gathers text elements, paths and images. Its goal
 * is to provide an efficient interface for the deduction of a
 * higher level structure of a text document.
 */
class PRGDocument : public QObject
{
    Q_OBJECT

public:
    /** Default constructor: initialize using a PRDocument parent
     * object (must be provided).
     * \param parent Parent PRDocument.
     * \param subDocumentTolerance Sub-document (relative) tolerance for page size.
     */
    explicit PRGDocument( PRDocument* parent,
                          double subDocumentTol = 0.05 );
    /** Destructor. Clear inside structures.
     */
    virtual ~PRGDocument();

public:
    /// Structure which describes geometry analysis parameters.
    struct GParameters;

    // Contents related member functions.
   /** Analyse the geomtry of the PDF file.
     * Create an internal structure (based on PRGSubDocument, PRGPage, ...) that
     * describes the geometry of the PDF content.
     * \param params Parameters used for the analysis.
     */
    void analyse( const PRGDocument::GParameters& params );

public slots:
    // Page cache.
    /** Add a page to the cache. The data from the page
     * is assumed to be loaded by the caller.
     * \param page Pointer to the page to add.
     */
    void cacheAddPage( PRGPage* gpage );
    /** Remove a page from the cache. PRGPage::clearData
     * is automatically called.
     * \param page Pointer to the page to remove.
     */
    void cacheRmPage( PRGPage* gpage );

private:
    /** Create sub-documents inside the PDF.
     * \param tolerance Tolerance used for page size.
     */
    void createSubDocuments( double tolerance );
    /** Clear the internal structure which describes the PDF geometry.
     */
    void clear();

public:
    // Getters...
    /// Number of sub-documents detected.
    size_t nbSubDocuments() const {     return m_subDocuments.size();   }
    /// Get a sub-document object.
    PRGSubDocument* subDocument( size_t idx )                {  return m_subDocuments.at( idx );    }
    const PRGSubDocument* subDocument( size_t idx ) const    {  return m_subDocuments.at( idx );    }
    /// Reimplement QObject parent function.
    PRDocument* parent() const;
    /// Number of pages in the document.
    size_t nbPages() const;
    /// Get a geometry page object.
    PRGPage* page( size_t idx );
    const PRGPage* page( size_t idx ) const;
    /// Size of the page cache.
    size_t cachePagesSize() const   {   return m_cachePagesSize;    }

    // and setters.
    /// Size of the page cache.
    void setCachePagesSize( size_t size )   {   m_cachePagesSize = size;    }

private:
    // No copy constructor and operator= allowed.
    Q_DISABLE_COPY(PRGDocument)

private:
    // PDF geometrical content.
    /// Vector of sub-documents.
    std::vector<PRGSubDocument*>  m_subDocuments;

    /// Page cache.
    std::list<PRGPage*>  m_cachePages;
    /// Size of the page cache (maximum number of elements : 10 by default).
    size_t  m_cachePagesSize;
};

//************************************************************//
//                   PRGDocument::GParameters                 //
//************************************************************//
/** Structure that describes PRGDocument geometry analysis parameters.
 */
struct PRGDocument::GParameters
{
    /// First page to analyse (default: 0).
    size_t  firstPageIndex;
    /// Last page to analyse (default: size_t::max).
    size_t  lastPageIndex;
    /// Perform the text line detection (default: true).
    bool  textLineDetection;

    /// Default constructor.
    GParameters();
    /// Initialize to default values.
    void init();
};


}

#endif // PRGDOCUMENT_H
