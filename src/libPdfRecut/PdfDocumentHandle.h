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

#ifndef PDFDOCUMENTHANDLE_H
#define PDFDOCUMENTHANDLE_H

#include <QString>
#include <QMutex>
#include <QtCore/QObject>

namespace PoDoFo {
    class PdfMemDocument;
}
namespace Poppler {
    class Document;
}

namespace PdfeBooker {

/** Class that handles Pdf document objects from PoDoFo and Poppler libraries.
 * Also has two mutex for these objects.
 */
class PdfDocumentHandle : public QObject
{
    Q_OBJECT

public:
    /** Default constructor: initialize pointers and filename.
     * \param filename Filename of the Pdf document.
     */
    PdfDocumentHandle( QObject* parent = 0, const QString& filename = "" );

    /** Destructor: delete PoDoFo and Poppler document objects.
     */
    ~PdfDocumentHandle();

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
    /** Load PoDoFo and Poppler documents.
     * \param loadPoDoFo Load PoDoFo document.
     * \param loadPoppler Load Poppler document.
     */
    void loadDocuments( bool loadPoDoFo = true, bool loadPoppler = true );

public:
    /** (Re)Load PoDoFo document from the defined filename. Need PoDoFo mutex.
     * \return Pointer to a PdfMemDocument object if loaded correctly.
     */
    PoDoFo::PdfMemDocument* loadPoDoFoDocument();

    /** (Re)Load Poppler document from the defined filename.  Need Poppler mutex.
     * \return Pointer to a Document object if loaded correctly.
     */
    Poppler::Document* loadPopplerDocument();

    /** Write PoDoFo document to a file.  Need PoDoFo mutex.
     * \param filename Filename of the output Pdf document. Modified if equal to
     * the member filename (to avoid PoDoFo problem).
     */
    void writePoDoFoDocument( const QString& filename );

    /** Is PoDoFo document loaded.
     */
    bool isPoDoFoDocumentLoaded() const;

    /** Is Poppler document loaded.
     */
    bool isPopplerDocumentLoaded() const;

    /** Free PoDoFo document. Need PoDoFo mutex to free memory.
     */
    void freePoDoFoDocument();

    /** Free Poppler document. Need Poppler mutex to free memory.
     */
    void freePopplerDocument();

    /** Get PoDoFo document pointer.
     * \return Pointer to a PdfMemDocument object (can be NULL if not loaded).
     */
    PoDoFo::PdfMemDocument* getPoDoFoDocument() const;

    /** Get Poppler document pointer.
     * \return Pointer to a Document object (can be NULL if not loaded).
     */
    Poppler::Document* getPopplerDocument() const;

    /** Get PoDoFo document mutex.
     * \return QMutex used for PoDoFo document object.
     */
    QMutex* getPoDoFoMutex();

    /** Get Poppler document mutex.
     * \return QMutex used for Poppler document object.
     */
    QMutex* getPopplerMutex();

    /** Get filename of the Pdf document.
     * \return Filename.
     */
    QString getFilename() const;

    /** Set filename of the Pdf document. Document loaded  with the
     * previous filename are released. Need PoDoFo and Poppler mutex.
     * \param filename New filename of the document.
     */
    void setFilename( const QString& filename );

private:
    // No copy constructor and operator= allowed.
    PdfDocumentHandle( const PdfDocumentHandle& );
    PdfDocumentHandle& operator=( const PdfDocumentHandle& );

private:
    /** Document filename.
     */
    QString m_filename;

    /** PoDoFo document.
     */
    PoDoFo::PdfMemDocument* m_podofoDocument;

    /** PoDoFo document mutex.
     */
    QMutex m_podofoMutex;

    /** Poppler document.
     */
    Poppler::Document* m_popplerDocument;

    /** Poppler document mutex.
     */
    QMutex m_popplerMutex;
};

//************************************************************//
//                      Inline functions                      //
//************************************************************//
inline bool PdfDocumentHandle::isPoDoFoDocumentLoaded() const
{
    return ( m_podofoDocument != NULL );
}
inline bool PdfDocumentHandle::isPopplerDocumentLoaded() const
{
    return ( m_popplerDocument != NULL );
}

inline PoDoFo::PdfMemDocument* PdfDocumentHandle::getPoDoFoDocument() const
{
    return m_podofoDocument;
}
inline Poppler::Document* PdfDocumentHandle::getPopplerDocument() const
{
    return m_popplerDocument;
}

inline QMutex* PdfDocumentHandle::getPoDoFoMutex()
{
    return &m_podofoMutex;
}
inline QMutex* PdfDocumentHandle::getPopplerMutex()
{
    return &m_popplerMutex;
}

inline QString PdfDocumentHandle::getFilename() const
{
    return m_filename;
}

}

#endif // PDFDOCUMENTHANDLE_H
