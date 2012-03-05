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
