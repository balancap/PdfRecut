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

#ifndef PRDOCUMENT_H
#define PRDOCUMENT_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <vector>
#include <map>

#include <QObject>
#include <QString>
#include <QMutex>

namespace PoDoFo {
    class PdfMemDocument;
    class PdfReference;
    class PdfRect;
    class PdfPage;
}
namespace PoDoFoExtended {
    class PdfeFont;
}

namespace PdfRecut {

class PRPage;
//************************************************************//
//                         PRDocument                         //
//************************************************************//
/** Class improves the PoDoFo::PdfMemDocument class. It allows
 * to handle more easily some operations on PDF document, especially
 * modifications on pages.
 * Caution: one should NEVER try to perform on operation on the PoDoFo
 * document object if it is available in PRDocument interface. Otherwise,
 * unknown consequences might appear (Sarah Palin, Black holes,
 * Communism... who knows !)
 *
 * It owns a mutex for the access to this object.
 */
class PRDocument : public QObject
{
    Q_OBJECT

public:
    /** Default constructor: initialize parent QObject.
     * \param parent Parent QObject.
     */
    explicit PRDocument( QObject* parent = 0 );
    /** Destructor: delete PoDoFo document object, if loaded.
     */
    virtual ~PRDocument();

public:
    /** Load a PoDoFo document at a given filename.
     * \param filename Path the document to load.
     */
    void load( const QString& filename );
    /** Save the document in at a given place.
     * \param filename Path where to save the document. Modified if equal to
     * the member filename (to avoid PoDoFo writing issues).
     */
    void save( const QString& filename );
    /** Clear the document (free PoDoFo memory and internal objects).
     */
    void clear();

public:
    // Pages related member functions.



    /// Number of pages in the document.
    size_t nbPages() const          {   return m_pPages.size();     }
    /// Get a page (pointer to the obejct).
    PRPage* page( size_t idx )              {   return m_pPages.at( idx );  }
    const PRPage* page( size_t idx ) const  {   return m_pPages.at( idx );  }
    /// Set page contents cache size (minimum: 10).
    void setPagesCacheSize( size_t cacheSize );

private:
    /// Load pages from the PoDoFo document.
    void loadPages();
    /// Clear the vector of pages.
    void clearPages();
    /// Set pages index.
    void setPagesIndex();
    /** Attach a page to the document.
     * \param index Index of the page.
     */
    void attachPage( size_t index );

private slots:
    // Page contents cache.
    /** Cache a page of the document.
     * \param pageIndex Index of the page to cache.
     */
    void cachePageContents( size_t pageIndex );
    /** Uncache a page of the document.
     * \param pageIndex Index of the page to uncache.
     */
    void uncachePageContents( size_t pageIndex );
private:
    /** Clean page cache. i.e. uncached pages until
     * the cache size is respected.
     */
    void cleanCachePages();

public:
    // Font cache related member functions.
    /** Get the font object corresponding to a font reference.
     * \param fontRef Reference to the PoDoFo font object.
     * \return PdfeFont pointer, owned by the PRDocument object.
     */
    PoDoFoExtended::PdfeFont* fontCache( const PoDoFo::PdfReference& fontRef );
    /** Free font objects stored in the font cache.
     */
    void clearFontCache();
private:
    /** Add a font object into the cache.
     * \param fontRef Reference to the PoDoFo font object.
     * \return PdfeFont pointer, owned by the PRDocument object.
     */
    PoDoFoExtended::PdfeFont* addFontToCache( const PoDoFo::PdfReference& fontRef );

public:
    // Getters...
    /// Is PoDoFo document loaded.
    bool isDocumentLoaded() const   {   return ( m_podofoDocument != NULL );    }
    /// Get PoDoFo document pointer.
    PoDoFo::PdfMemDocument* podofoDocument() const {    return m_podofoDocument;    }
    /// Get PoDoFo document mutex.
    QMutex* podofoMutex()       {   return &m_podofoMutex;  }
    /// Get filename of the PDF document.
    QString filename() const    {   return m_filename;  }
    /// Get the FreeType library object linked to the document.
    FT_Library ftLibrary() const    {   return m_ftLibrary;   }

//signals:
//    /** Progress signal sent by methods.
//     * \param title Title of the current operation.
//     * \param progress Progress, between 0 and 1.
//     */
//    void methodProgress( const QString& title, double progress ) const;
//    /** Error signal.
//     * \param title Title of the error.
//     * \param description Description of the error.
//     */
//    void methodError( const QString& title, const QString& description ) const;

private:
    // PoDoFo document related member functions.
    /** (Re)Load PoDoFo document from the defined filename. Need PoDoFo mutex.
     * Throw an exception if an error occured during the loading operation.
     * \param filename Filename of the PDF document to load.
     * \return Pointer to a PdfMemDocument object if loaded correctly.
     */
    PoDoFo::PdfMemDocument* loadPoDoFoDocument( const QString& filename );
    /** Write PoDoFo document to a file.  Need PoDoFo mutex.
     * \param filename Filename of the output Pdf document. Modified if equal to
     * the member filename (to avoid PoDoFo writing issues).
     * \param suffix Suffix used to modified the filename, if necessary.
     */
    void writePoDoFoDocument( const QString& filename, const QString& suffix );
    /** Free PoDoFo document. Need PoDoFo mutex to free memory.
     * Reset the filename to empty.
     */
    void freePoDoFoDocument();

private:
    // No copy constructor and operator= allowed.
    Q_DISABLE_COPY(PRDocument)

private:
    /// Document filename.
    QString  m_filename;
    /// PoDoFo document.
    PoDoFo::PdfMemDocument*  m_podofoDocument;
    /// PoDoFo document mutex.
    QMutex  m_podofoMutex;

    /// Pages vector.
    std::vector<PRPage*>  m_pPages;
    /// Pages cache list.
    std::list<size_t>  m_pagesCacheList;
    /// Pages cache size (default: infinity).
    size_t  m_pagesCacheSize;

    /// Map containing font cache. Each key corresponds to the reference of the font object.
    std::map< PoDoFo::PdfReference, PoDoFoExtended::PdfeFont* >  m_fontCache;

    /// FreeType library instance associated to the document.
    FT_Library  m_ftLibrary;
};

}

#endif // PRDOCUMENT_H
