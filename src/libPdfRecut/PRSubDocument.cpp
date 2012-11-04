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

#include "QsLog/QsLog.h"

#include "podofo/podofo.h"

using namespace PoDoFo;

namespace PdfRecut {

PRSubDocument::PRSubDocument( PRDocument* parent, size_t firstPageIndex, size_t lastPageIndex ) :
    QObject( parent ), m_firstPageIndex( firstPageIndex ), m_lastPageIndex( lastPageIndex )
{
    PdfRect cbox = PRDocument::PageCropBox( parent->podofoDocument()->GetPage( firstPageIndex ) );

    // Log information.
    QLOG_INFO() << QString( "<PRSubDocument> Create sub-document with range [%1,%2] and size (%3,%4)" )
                   .arg( m_firstPageIndex ).arg( m_lastPageIndex )
                   .arg( cbox.GetWidth() ).arg( cbox.GetHeight() )
                   .toAscii().constData();
}

void PRSubDocument::clearContent()
{
}

PRDocument* PRSubDocument::parent() const
{
    return dynamic_cast<PRDocument*>( this->QObject::parent() );
}

}
