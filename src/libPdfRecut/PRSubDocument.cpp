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

#include "PRSubDocument.h"
#include "PRDocument.h"

namespace PdfRecut {

PRSubDocument::PRSubDocument( PRDocument* parent, size_t firstPageIndex, size_t lastPageIndex ) :
    QObject( parent ), m_firstPageIndex( firstPageIndex ), m_lastPageIndex( lastPageIndex )
{
}

PRDocument* PRSubDocument::parent() const
{
    return dynamic_cast<PRDocument*>( this->QObject::parent() );
}

}
