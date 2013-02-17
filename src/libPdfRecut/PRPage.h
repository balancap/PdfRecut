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
class PdfObject;
class PdfPage;
}

namespace PdfRecut {

class PRDocument;

//************************************************************//
//                           PRPage                           //
//************************************************************//
/** Class that represent a page in a PDF document.
 * It enhances PoDoFo::PdfPage, e.g. using PdfeContentsStream
 * to store the content stream of the page and other attributes in
 * members. It supports two modes of synchronisation with a PDF document:
 *  - None, i.e. the page is not linked to a document, meaning modifications
 *    are not backported to a real PDF objects;
 *  - Complete, i.e. the page corresponds to an actual page in a given PDF
 *    document, implying that any modification to the PRPage is synchronised
 *    with the PDF content. It is up to a PRDocument to set this link.
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
    PRPage( const PoDoFo::PdfRect& mediaBox = PoDoFo::PdfRect() );
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
    /** Clear page. Reinitialized completely to an empty one.
     */
    void clear();

private:
    /** Initialize page attributes.
     * \param mediaBox Default media box to use.
     */
    void initAttributes( const PoDoFo::PdfRect& mediaBox );
    /** Copy attributes from another page.
     * \param rhs Input page.
     */
    void copyAttributes( const PRPage& rhs );
    /** Copy contents from another page.
     * \param rhs Input page.
     */
    void copyContents( const PRPage& rhs );

public:
    // Load/Save page into a PoDoFo::PdfPage.
    /** Load a PoDoFo::PdfPage.
     * \param page Page to load.
     * \param incContents Include loading page's contents stream.
     * \param incAttributes Include loading page's attributes.
     */
    void load( const PoDoFo::PdfPage* page,
               bool incContents = true,
               bool incAttributes = true );
    /** Save back thepage into a PoDoFo::PdfPage.
     * \param page Page where to save back.
     * \param incContents Include saving page's contents stream.
     * \param incAttributes Include saving page's attributes.
     */
    void save( PoDoFo::PdfPage* page,
               bool incContents = true,
               bool incAttributes = true );

signals:
    /** Page has been loaded.
     * \param Page index of the present object.
     * \param incContents Contents stream loaded?
     * \param incAttributes Attributes loaded?
     */
    void loaded( size_t pageIndex, bool incContents, bool incAttributes );
    /** Page has been loaded. It is up to a parent document
     * to synchronised contents when a signal is received.
     * \param pageIndex Page index of the present object.
     * \param incContents Modifications on contents?
     * \param incAttributes Modifications on attributes?
     */
    void modified( size_t pageIndex, bool incContents, bool incAttributes );

public:
    // Cache/Uncache page contents. Only works if attached to a PoDoFo page.
    /** Cache page contents from the attached PoDoFo::PdfPage.
     * Does nothing if page contents is already cached (use load instead
     * if you want to force page contents loading).
     */
    void cacheContents() const;
    /** Uncache page contents. Only works if a parent PRDocument exists.
     */
    void uncacheContents() const;
    /** Is page contents cached.
     * \return F***ing answer!
     */
    bool isContentsCached() const;

signals:
    /** Page contents has been cached.
     * \param Page index of the present object.
     */
    void contentsCached( size_t pageIndex ) const;
    /** Page contents have been uncached (cleared).
     * \param Page index of the present object.
     */
    void contentsUncached( size_t pageIndex ) const;

private:
    /** Push modifications to a PoDoFo page. Only works when the page is attached
     * to a document. Emits modified signal.
     * \param incContents Modifications on contents?
     * \param incAttributes Modifications on attributes?
     */
    void pushModifications( bool incContents, bool incAttributes );
    /** Clean PoDoFo page stream contents, i.e. clear existing contents and
     * remove unnecessary objects.
     * \param Page concerned.
     */
    void cleanPoDoFoPageStreams( PoDoFo::PdfPage* page );

public:
    // Getters...
    /// Get parent document. NULL if not attached to any document.
    PRDocument* document() const;
    /// Get attached podofo page. NULL if nothing attached.
    PoDoFo::PdfPage* podofoPage() const;
    /// Get page index in a document. Beginning at zero (default: 0).
    size_t pageIndex() const;

    /// Get page contents stream. Cache it from PoDoFo::PdfPage if necessary.
    const PoDoFoExtended::PdfeContentsStream& contents() const;

    // Setters...
    /// Set parent document. Reimplement QObject function.
    void setParent( PRDocument* document );
    /** Set page contents. If the page is attached to a PoDoFo page, the last
     * is automatically updated (can be costly...).
     * \param contents New page contents. The input object is entirely copied.
     */
    void setContents( const PoDoFoExtended::PdfeContentsStream& contents );

private:
    // Private getters and setters...
    /// Get pointer to page contents. Create object if necessary.
    PoDoFoExtended::PdfeContentsStream* pContents() const;
    /// Set page index in the document. Take care of not messing up the order!
    void setPageIndex( size_t pageIndex )   {   m_pageIndex = pageIndex;    }


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
    /// Page index.
    size_t  m_pageIndex;
    /// Page stream contents.
    mutable PoDoFoExtended::PdfeContentsStream*  m_pContentsStream;
    /// Do we own page stream contents object?
    bool  m_ownPageContentsObj;

    // Page attributes. At least the important ones.
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

}

#endif // PRPAGE_H
