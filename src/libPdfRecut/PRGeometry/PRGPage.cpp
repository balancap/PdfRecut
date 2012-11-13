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
#include "PRGTextPage.h"

#include "PdfeTypes.h"

#include <podofo/podofo.h>

using namespace PoDoFo;
using namespace PoDoFoExtended;

namespace PdfRecut {

PRGPage::PRGPage( PRGSubDocument* parent, size_t pageIndex ) :
    QObject( parent ),
    m_pageIndex( pageIndex ),
    m_page( parent->parent()->parent()->podofoDocument()->GetPage( pageIndex ) ),
    m_textPage( NULL ),
    m_pathPage( NULL ),
    m_imagePage( NULL )
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

    // Create text, path and image page objects.
    m_textPage = new PRGTextPage( this );
//    m_pathPage;
//    m_imagePage;

    // Analyse text content.
    m_textPage->detectGroupsWords();
    if( params.textLineDetection ) {
        m_textPage->detectLines();
    }


//    delete m_textPage;
//    m_textPage = NULL;

}
void PRGPage::clear()
{
    // Delete content objects.
    delete m_textPage;
//    delete m_pathPage;
//    delete m_imagePage;
    m_textPage = NULL;
//    m_pathPage = NULL;
//    m_imagePage = NULL;
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
