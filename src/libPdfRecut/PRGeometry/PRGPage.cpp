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

#include "PRGPage.h"

#include "PRDocument.h"
#include "PRGSubDocument.h"

#include "PdfeTypes.h"

#include <podofo/podofo.h>

using namespace PoDoFo;
using namespace PoDoFoExtended;

namespace PdfRecut {

PRGPage::PRGPage( PRGSubDocument* parent, size_t pageIndex ) :
    QObject( parent ),
    m_pageIndex( pageIndex ),
    m_page( parent->parent()->parent()->podofoDocument()->GetPage( pageIndex ) ),
    m_text( NULL )
{
}
PRGPage::~PRGPage()
{
    this->clear();
}
PRGSubDocument* PRGPage::parent() const
{
    return static_cast<PRGSubDocument*>( this->QObject::parent() );
}

void PRGPage::analyse( const PRGDocument::GParameters& params )
{
    // Clear existing content.
    this->clear();
}
void PRGPage::clear()
{
    // Delete content objects.
    delete m_text;
    m_text = NULL;
}

PdfRect PRGPage::PageMediaBox( PdfPage* pPage )
{
    PdfRect mediaBox( pPage->GetMediaBox() );
    return mediaBox;
}

PdfRect PRGPage::PageCropBox( PdfPage* pPage )
{
    PdfRect mediaBox( PRGPage::PageMediaBox( pPage ) );
    PdfRect cropBox( pPage->GetCropBox() );

    // Intersection between the two.
    cropBox = PdfeORect::intersection( mediaBox, cropBox );
    return cropBox;
}

}
