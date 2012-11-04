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

#include "PRPage.h"
#include "PRSubDocument.h"

#include <podofo/podofo.h>

namespace PdfRecut {

PRPage::PRPage( PRSubDocument* parent, size_t pageIndex ) :
    QObject( parent ),
    m_pageIndex( pageIndex ),
    m_page( parent->parent()->podofoDocument()->GetPage( pageIndex ) )
{
}
PRPage::~PRPage()
{
}

void PRPage::analyseContent( const PRDocument::ContentParameters& params )
{
}
void PRPage::clearContent()
{
}

PRSubDocument* PRPage::parent() const
{
    return static_cast<PRSubDocument*>( this->QObject::parent() );
}


}
