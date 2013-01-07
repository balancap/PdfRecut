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
#include "PRGDocument.h"
#include "PRGSubDocument.h"
#include "PRGTextPage.h"

#include "PdfeTypes.h"

#include <QsLog/QsLog.h>
#include <podofo/podofo.h>

using namespace PoDoFo;
using namespace PoDoFoExtended;

namespace PdfRecut {

PRGPage::PRGPage( PRGSubDocument* parent, size_t pageIndex ) :
    QObject( parent ),
    m_pageIndex( pageIndex ),
    m_page( parent->parent()->parent()->podofoDocument()->GetPage( pageIndex ) ),
    m_pContentsStream( NULL ),
    m_textPage( NULL ),
    m_pathPage( NULL ),
    m_imagePage( NULL )
{
    // Create content objects.
    m_textPage = new PRGTextPage( this );
//    m_pathPage;
//    m_imagePage;

    // Connect signals.
    QObject::connect( this, SIGNAL(dataLoaded(PRGPage*)),
                      this->gdocument(), SLOT(cacheAddPage(PRGPage*)));

}
PRGPage::~PRGPage()
{
    // Remove page from document cache and clear contents.
    this->gdocument()->cacheRmPage( this );
    this->clearContents();

    // Delete content objects.
    delete m_textPage;
//    delete m_pathPage;
//    delete m_imagePage;
    m_textPage = NULL;
//    m_pathPage = NULL;loadData
//    m_imagePage = NULL;
}

void PRGPage::loadContents() const
{
    // Log information.
    QLOG_INFO() << QString( "<PRGPage> Load page contents stream (index: %1)." )
                   .arg( m_pageIndex ).toAscii().constData();
    // Load contents stream and send signal
    if( !m_pContentsStream ) {
        m_pContentsStream = new PdfeContentsStream();
    }
    m_pContentsStream->load( this->podofoPage(), true );
    emit contentsLoaded( this );
}
void PRGPage::clearContents()
{
    delete m_pContentsStream;
}
void PRGPage::loadData()
{
    // Load text, paths and images contents.
    m_textPage->loadData();
    // Send signal.
    emit dataLoaded( this );
}
void PRGPage::clearData()
{
    // Clear page content.
    if( m_textPage ) {
        m_textPage->clearData();
    }
}

void PRGPage::analyse( const PRGDocument::GParameters& params )
{
    // Load page data.
    this->loadData();
    // Analyse text content.
    if( params.textLineDetection ) {
        m_textPage->detectLines();
    }
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

PRGSubDocument* PRGPage::parent() const
{
    return static_cast<PRGSubDocument*>( this->QObject::parent() );
}
PRGDocument* PRGPage::gdocument() const
{
    return this->parent()->parent();
}

}
