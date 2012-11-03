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

#include <map>

#include <QString>
#include <QMutex>
#include <QtCore/QObject>

#include <podofo/doc/PdfFontCache.h>

#include "PdfeFont.h"

namespace PoDoFo {
    class PdfMemDocument;
}

namespace PdfRecut {

/** Class that handles a document object from PoDoFo library.
 * It owns a mutex for the access to this object.
 */
class PRDocument : public QObject
{
    Q_OBJECT

public:
    /** Default constructor: initialize pointers and filename.
     * \param filename Filename of the Pdf document.
     */
    PRDocument( QObject* parent = 0, const QString& filename = "" );

    /** Destructor: delete PoDoFo and Poppler document objects.
     */
    ~PRDocument();

signals:
    /** Progress signal sent by methods.
     * \param title Title of the current operation.
     * \param progress Progress, between 0 and 1.
     */
    void methodProgress( const QString& title, double progress ) const;

    /** Error signal.
     * \param title Title of the error.
     * \param description Description of the error.
     */
    void methodError( const QString& title, const QString& description ) const;

public slots:
    /** Load the PoDoFo document using the filename given.
     */
    void loadDocument();

public:
    /** (Re)Load PoDoFo document from the defined filename. Need PoDoFo mutex.
     * \return Pointer to a PdfMemDocument object if loaded correctly.
     */
    PoDoFo::PdfMemDocument* loadPoDoFoDocument();
    /** Write PoDoFo document to a file.  Need PoDoFo mutex.
     * \param filename Filename of the output Pdf document. Modified if equal to
     * the member filename (to avoid PoDoFo problem).
     */
    void writePoDoFoDocument( const QString& filename );
    /** Free PoDoFo document. Need PoDoFo mutex to free memory.
     */
    void freePoDoFoDocument();

    /** Get the font object corresponding to a font reference.
     * \param fontRef Reference to the PoDoFo font object.
     * \return PdfeFont pointer, owned by the PRDocument object.
     */
    PoDoFoExtended::PdfeFont* fontCache( const PoDoFo::PdfReference& fontRef );
    /** Free objects stored in font cache.
     */
    void freeFontCache();

private:
    /** Add a font object into the cache.
     * \param fontRef Reference to the PoDoFo font object.
     * \return PdfeFont pointer, owned by the PRDocument object.
     */
    PoDoFoExtended::PdfeFont* addFontToCache( const PoDoFo::PdfReference& fontRef );

public:
    // Getters...
    /** Is PoDoFo document loaded.
     * \return Answer!
     */
    bool isPoDoFoDocumentLoaded() const {
        return ( m_podofoDocument != NULL );
    }
    /** Get PoDoFo document pointer.
     * \return Pointer to a PdfMemDocument object (can be NULL if not loaded).
     */
    PoDoFo::PdfMemDocument* getPoDoFoDocument() const {
        return m_podofoDocument;
    }
    /** Get PoDoFo document mutex.
     * \return QMutex used for PoDoFo document object.
     */
    QMutex* getPoDoFoMutex() {
        return &m_podofoMutex;
    }
    /** Get filename of the Pdf document.
     * \return Filename.
     */
    QString filename() const {
        return m_filename;
    }
    /** Get the FreeType library object.
     * \return FT_Library object.
     */
    FT_Library ftLibrary() const {
        return m_ftLibrary;
    }

    // and setters.
    /** Set filename of the Pdf document. Document loaded  with the
     * previous filename are released. Need PoDoFo and Poppler mutex.
     * \param filename New filename of the document.
     */
    void setFilename( const QString& filename );

private:
    // No copy constructor and operator= allowed.
    PRDocument( const PRDocument& );
    PRDocument& operator=( const PRDocument& );

private:
    /// Document filename.
    QString  m_filename;
    /// PoDoFo document.
    PoDoFo::PdfMemDocument*  m_podofoDocument;
    /// PoDoFo document mutex.
    QMutex  m_podofoMutex;

    /// Map containing font cache. Each key corresponds to the reference of the font object.
    std::map< PoDoFo::PdfReference, PoDoFoExtended::PdfeFont* >  m_fontCache;

    /// FreeType library.
    FT_Library  m_ftLibrary;
};

//************************************************************//
//                      Inline functions                      //
//************************************************************//


}

#endif // PRDOCUMENT_H
