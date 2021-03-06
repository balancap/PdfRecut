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
#include "PRPage.h"
#include "PdfeContentsStream.h"

namespace PoDoFo {
class PdfPage;
class PdfRect;
}

namespace PdfRecut {

class PRGDocument;
class PRGSubDocument;
class PRGTextPage;
class PRGPathPage;
class PRGImagePage;

/** Class that represent a page in a PDF document.
 * It describes page's basic objects (text, paths and images),
 * and the geometrical relationship between them.
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

public:
    // Contents and cached data.
    /** Load page data: text, paths and images.
     */
    void loadData();
    /** Clear page cached data. Only keep basic
     * skeleton of page organization.
     */
    void clearData();

signals:
    /** Qt signal: page stream contents has been loaded!
     * \param page Pointer of the sender page.
     */
    void contentsLoaded( const PRGPage* page ) const;
    /** Qt signal: page data has been (partially?) loaded!
     * \param page Pointer of the sender page.
     */
    void dataLoaded( PRGPage* page );

public:
    // Contents related member functions.
    /** Analyse the geometry of the page content.
     * Create an internal structure (based on PRGTextPage,...) that
     * describes the precise internal geometry of the PDF page.
     * \param params Parameters used for the analysis.
     */
    void analyse( const PRGDocument::GParameters& params );

public:
    // Some static functions that can be useful.
    /// Get the media box of a PoDoFo page.
    static PoDoFo::PdfRect PageMediaBox( PoDoFo::PdfPage* pPage );
    /// Get the crop box of a PoDoFo page (check the coherence with media box).
    static PoDoFo::PdfRect PageCropBox( PoDoFo::PdfPage* pPage );

public:
    // Getters...
    /// Get the PRPage object corresponding to the page.
    PRPage* page()              {   return m_pPage;     }
    const PRPage* page() const  {   return m_pPage;     }
    /// Reimplement QObject parent function with PRGSubDocument.
    PRGSubDocument* parent() const;
    /// Parent PRGDocument.
    PRGDocument* gdocument() const;

    /// Get text component of the page.
    PRGTextPage* text() const           {   return m_textPage;  }


/*    /// Get the PoDoFo page object corresponding to the page.
    PoDoFo::PdfPage* podofoPage() const {   return m_page;      }
    /// Get page index.
    size_t pageIndex() const            {   return m_pageIndex; }

    /// Is page contents loaded?
    bool isContentsLoaded() const       {   return m_pContentsStream;   }
    /// Get a constant reference to page contents stream.
    const PoDoFoExtended::PdfeContentsStream& contents() const {
        if( !m_pContentsStream ) {
            this->loadContents();
        }
        return *m_pContentsStream;
    }
*/
private:
    // No copy constructor and operator= allowed.
    Q_DISABLE_COPY(PRGPage)

private:
    /// PRPage object corresponding to this page.
    PRPage*  m_pPage;


    /*/// Page index.
    size_t  m_pageIndex;
    /// Pointer to the PoDoFo page;
    PoDoFo::PdfPage*  m_page;
    /// Page contents stream (mutable since considered as cached data).
    mutable PoDoFoExtended::PdfeContentsStream*  m_pContentsStream;
*/

    /// Text elements in the page.
    PRGTextPage*  m_textPage;
    /// Paths in the page.
    PRGPathPage*  m_pathPage;
    /// Images in the page.
    PRGTextPage*  m_imagePage;
};

}

#endif // PRGPAGE_H
