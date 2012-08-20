/***************************************************************************
 *   Copyright (C) 2011 by Paul Balança                                    *
 *   paul.balanca@gmail.com                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
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

    /** Is PoDoFo document loaded.
     */
    bool isPoDoFoDocumentLoaded() const;

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
    // Getters and setters.

    /** Get PoDoFo document pointer.
     * \return Pointer to a PdfMemDocument object (can be NULL if not loaded).
     */
    PoDoFo::PdfMemDocument* getPoDoFoDocument() const;

    /** Get PoDoFo document mutex.
     * \return QMutex used for PoDoFo document object.
     */
    QMutex* getPoDoFoMutex();

    /** Get filename of the Pdf document.
     * \return Filename.
     */
    QString filename() const;

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
inline bool PRDocument::isPoDoFoDocumentLoaded() const
{
    return ( m_podofoDocument != NULL );
}
inline PoDoFo::PdfMemDocument* PRDocument::getPoDoFoDocument() const
{
    return m_podofoDocument;
}
inline QMutex* PRDocument::getPoDoFoMutex()
{
    return &m_podofoMutex;
}
inline QString PRDocument::filename() const
{
    return m_filename;
}

}

#endif // PRDOCUMENT_H
