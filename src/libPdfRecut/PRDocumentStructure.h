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

#ifndef PRDOCUMENTSTRUCTURE_H
#define PRDOCUMENTSTRUCTURE_H

#include <QtCore/QObject>
#include <poppler/qt4/poppler-qt4.h>

#include "PRDocumentLayout.h"

namespace PdfRecut {

/** Class which represents the structure of a Pdf document:
 * body, headers, footers, margins, ...
 * This class is virtual and has to be implemented to support a particular
 * document structure (two sided book, two columns article, ...).
 */
class PRDocumentStructure : public QObject
{
    Q_OBJECT

public:
    /** Default constructor... Not a big deal.
     */
    PRDocumentStructure( QObject* parent = 0 );

    /** Abort current operation. Used in a context of multithread program where
     * slots are executed on another thread.
     * \param abort True to abort current method.
     */
    void setAbortOperation( bool abort = true );

public slots:
    /** Analyse structure of a Pdf document.
     * \param documentHandle Document to analyse.
     */
    virtual void analyseDocument( PRDocument* documentHandle ) = 0;

    /** Generate a document layout based on the document structure.
     * \param layout Layout to overwrite.
     */
    virtual void generateLayout( PRDocumentLayout* layout ) = 0;

protected:
    /** Boolean used to indicate if a current operation should be aborted.
     */
    bool m_abortOperation;
};

//**********************************************************//
//                      Inline methods                      //
//**********************************************************//
inline void PRDocumentStructure::setAbortOperation( bool abort )
{
    this->m_abortOperation = abort;
}

}

#endif // PRDOCUMENTSTRUCTURE_H
