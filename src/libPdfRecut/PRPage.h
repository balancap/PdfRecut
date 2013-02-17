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

#include "podofo/base/PdfRect.h"

#include "PdfeTypes.h"
#include "PdfeContentsStream.h"
#include "PdfeResources.h"

#include <QObject>

namespace PoDoFo {
class PdfRect;
class PdfPage;
}

namespace PdfRecut {

class PRDocument;

//************************************************************//
//                           PRPage                           //
//************************************************************//
/** Class that represent a page in a PDF document.
 * It enhances PoDoFo::PdfPage, e.g. using PdfeContentsStream
 * to store the content stream of the page. It supports two modes
 * of synchronisation with a PDF document:
 *  - None, i.e. the page is not linked to a document, meaning modifications
 *    are not backported to a real PDF objects;
 *  - Complete, i.e. the page corresponds to an actual page in a given PDF
 *    document, implying that any modification to the PRPage is synchronised
 *    with the PDF content.
 * Page contents stream can be cached/uncached depending on the willing of the user.
 */
class PRPage : public QObject
{
    Q_OBJECT

    friend class PRDocument;
public:
    /** Create an empty page with a given media box.
     * \param mediaBox Media box of the new page.
     */
    PRPage( const PoDoFo::PdfRect& mediaBox );
    /** Create a page from data of an existing PoDoFo::PdfPage.
     * The PRPage is not attached to a document when created, the
     * page data is only copied.
     * \param page Pointer to the PoDoFo::PdfPage.
     * \param loadPageContents Is page's content also loaded?
     */
    PRPage( PoDoFo::PdfPage* page, bool loadPageContents );
    /** (Deep) Copy constructor. The resulting page is not attached to any
     * document by default.
     * \param rhs Page to copy.
     */
    PRPage( const PRPage& rhs );
    /** Assignment operator. The resulting page is not attached to any
     * document by default.
     * \param rhs Page to copy.
     */
    PRPage& operator=( const PRPage& rhs );
    /** Default destructor.
     */
    virtual ~PRPage();

    /** Initialize the page to an empty page.
     * \param mediaBox Media box of the new page.
     */
    void init( const PoDoFo::PdfRect& mediaBox );
    /** Initialize the page data from an existing PoDoFo::PdfPage.
     * \param page Pointer to the PoDoFo::PdfPage.
     * \param loadPageContents Is page's content also loaded?
     */
    void init( PoDoFo::PdfPage* page, bool loadPageContents );
    /** Clear page. Reinitialized completely to an empty one.
     */
    void clear();

public:
    // Load/Clear (cached) page contents. Should only be used if attached to a page.
    /** Load document contents from the attached PoDoFo::PdfPage.
     * Does nothing if the page is not attached to an existing object.
     */
    void loadContents() const;
    /** Clear page contents. Use it at your own risk !
     */
    void clearContents() const;
    /** Is page contents loaded?
     * \return F***ing answer!
     */
    bool isContentsLoaded() const;
signals:
    /** Page contents have been loaded, or modified.
     */
    void contentsLoaded() const;
    /** Page contents have been cleared.
     */
    void contentsCleared() const;

private:
    /** Clean PoDoFo page contents, i.e. clear existing contents and
     * remove unnecessary objects.
     * \return Object containing the stream to which append.
     */
    PoDoFo::PdfObject* cleanPoDoFoContents();

public:
    // Synchronisation with the PoDoFo::Page.
    /** Push modification to the associated PoDoFo::Page.
     * \param incContents Include contents (optimization purpose).
     */
    void push( bool incContents = true );
    /** Pull data and contents from the associated PoDoFo::Page.
     * Equivalent to initialization with the associated page.
     */
    void pull();
private:
    /** Attach the page to a document. Does not modify the
     * PoDoFo::PdfPage. See pull/push for that.
     * Should only be used if you know what you're doing!
     * \param document Pointer to the parent document.
     * \param page Pointer to the associated page.
     */
    void attach( PRDocument* document, PoDoFo::PdfPage* page );
    /** Detach the page from a document.
     */
    void detach();

public:
    // Getters...
    /// Get page contents stream. Loaded from PoDoFo::PdfPage if necessary.
    const PoDoFoExtended::PdfeContentsStream& contents() const;

    /// Get parent document. NULL if not attached to any document.
    PRDocument* document() const        {   return m_pDocument; }
    /// Get attached podofo page. NULL if nothing attached.
    PoDoFo::PdfPage* podofoPage() const {   return m_pPage;     }
    /// Get page index in a document. Beginning at zero (default: 0).
    size_t pageIndex() const    {   return  m_pageIndex;    }

    // Setters...
    /** Set page contents. If the page is attached to a PoDoFo page, the last
     * is automatically updated (can be costly...).
     * \param contents New page contents. The input object is entirely copied.
     */
    void setContents( const PoDoFoExtended::PdfeContentsStream& contents );

private:
    // Private getters and setters...
    /// Get page contents stream. Loaded from PoDoFo::PdfPage if necessary.
    PoDoFoExtended::PdfeContentsStream& contents( bool loadPageContents ) const;
    /// Set page index in the document. Take care of not messing up the order!
    void setPageIndex( size_t pageIndex ) {     m_pageIndex = pageIndex;    }


public:
    // Page's boxes getters...
    PoDoFo::PdfRect mediaBox() const;
    PoDoFo::PdfRect cropBox() const;
    PoDoFo::PdfRect bleedBox() const;
    PoDoFo::PdfRect trimBox() const;
    PoDoFo::PdfRect artBox() const;

    // Page's boxes setters...
    void setMediaBox( const PoDoFo::PdfRect& rhs );
    void setCropBox( const PoDoFo::PdfRect& rhs );
    void setBleedBox( const PoDoFo::PdfRect& rhs );
    void setTrimBox( const PoDoFo::PdfRect& rhs );
    void setArtBox( const PoDoFo::PdfRect& rhs );

private:
    /// PDF document the page is related to.
    PRDocument*  m_pDocument;
    /// PoDoFo::PdfPage it is connected to.
    PoDoFo::PdfPage*  m_pPage;
    /// Do we own page contents?
    bool  m_ownPageContents;

    /// Page index.
    size_t  m_pageIndex;
    /// Page stream contents.
    mutable PoDoFoExtended::PdfeContentsStream*  m_pContentsStream;

    // Different kinds of page bounding boxes.
    /// Media box.
    PoDoFo::PdfRect  m_mediaBox;
    /// Crop box.
    PoDoFo::PdfRect  m_cropBox;
    /// Bleed box.
    PoDoFo::PdfRect  m_bleedBox;
    /// Trim box.
    PoDoFo::PdfRect  m_trimBox;
    /// Art box.
    PoDoFo::PdfRect  m_artBox;
};

//************************************************************//
//                       Inline PRPage                        //
//************************************************************//
inline PoDoFo::PdfRect PRPage::mediaBox() const
{
    return m_mediaBox;
}
inline PoDoFo::PdfRect PRPage::cropBox() const
{
    if( m_cropBox.GetWidth() && m_cropBox.GetHeight() ) {
        return m_cropBox;
    }
    else {
        return m_mediaBox;
    }
}
inline PoDoFo::PdfRect PRPage::bleedBox() const
{
    if( m_bleedBox.GetWidth() && m_bleedBox.GetHeight() ) {
        return m_bleedBox;
    }
    else {
        return this->cropBox();
    }
}
inline PoDoFo::PdfRect PRPage::trimBox() const
{
    if( m_trimBox.GetWidth() && m_trimBox.GetHeight() ) {
        return m_trimBox;
    }
    else {
        return this->cropBox();
    }
}
inline PoDoFo::PdfRect PRPage::artBox() const
{
    if( m_artBox.GetWidth() && m_artBox.GetHeight() ) {
        return m_artBox;
    }
    else {
        return this->cropBox();
    }
}

inline void PRPage::setMediaBox( const PoDoFo::PdfRect& rhs )
{
    m_mediaBox = rhs;
    this->push( false );
}
inline void PRPage::setCropBox( const PoDoFo::PdfRect& rhs )
{
    m_cropBox = PdfeORect::intersection( m_mediaBox, rhs );
    this->push( false );
}
inline void PRPage::setBleedBox( const PoDoFo::PdfRect& rhs )
{
    m_bleedBox = PdfeORect::intersection( m_mediaBox, rhs );
    this->push( false );
}
inline void PRPage::setTrimBox( const PoDoFo::PdfRect& rhs )
{
    m_trimBox = PdfeORect::intersection( m_mediaBox, rhs );
    this->push( false );
}
inline void PRPage::setArtBox( const PoDoFo::PdfRect& rhs )
{
    m_artBox = PdfeORect::intersection( m_mediaBox, rhs );
    this->push( false );
}

}

#endif // PRPAGE_H
