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

#ifndef PRSUBDOCUMENT_H
#define PRSUBDOCUMENT_H

#include <QObject>

namespace PdfRecut {

class PRDocument;

/** Class that represents a sub-document, i.e. a range of pages,
 * usually homogeneous in certain way, from a parent PRDocument.
 */
class PRSubDocument : public QObject
{
    Q_OBJECT

public:
    /** Default constructor: initialize pointers and filename.
     * \param parent Parent PRDocument.
     * \param firstPageIndex First page of the sub-document.
     * \param lastPageIndex Last page of the sub-document.
     */
    PRSubDocument( PRDocument* parent,
                   size_t firstPageIndex,
                   size_t lastPageIndex );


public:
    /** Reimplement QObject parent function.
     * \return Pointer to the parent PRDocument.
     */
    PRDocument* parent() const;


private:
    // No copy constructor and operator= allowed.
    Q_DISABLE_COPY(PRSubDocument)

private:
    /// Index of the first page of the sub-document.
    size_t  m_firstPageIndex;
    /// Index of the last page of the sub-document.
    size_t  m_lastPageIndex;
    
    
};

}

#endif // PRSUBDOCUMENT_H
