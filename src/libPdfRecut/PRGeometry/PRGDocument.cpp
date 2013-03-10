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

#include "PRGDocument.h"

#include "PRDocument.h"
#include "PRGSubDocument.h"
#include "PRGPage.h"

#include "QsLog/QsLog.h"

#include <QtCore>
#include <podofo/podofo.h>

#include <algorithm>
#include <limits>

using namespace PoDoFo;
using namespace PoDoFoExtended;

namespace PdfRecut {

//************************************************************//
//                         PRGDocument                         //
//************************************************************//
PRGDocument::PRGDocument( PRDocument* parent, double subDocumentTol ) :
    QObject( parent ),
    m_cachePagesSize( 10 )
{
    // Create sub-documents.
    this->createSubDocuments( subDocumentTol );
}
PRGDocument::~PRGDocument()
{
    this->clear();
}

void PRGDocument::analyse( const PRGDocument::GParameters& params )
{
    // Time spent analysing the document.
    QTime timeTask;
    timeTask.start();

    QLOG_INFO() << QString( "<PRGDocument> Begin analysis of the geometry of the PDF document." )
                   .toAscii().constData();

    // Analyse sub-documents content.
    for( size_t i = 0 ; i < m_subDocuments.size() ; ++i ) {
        m_subDocuments[i]->analyse( params );
    }
    QLOG_INFO() << QString( "<PRGDocument> End analysis of the geometry of the PDF document (%1 seconds)." )
                   .arg( timeTask.elapsed() / 1000. )
                   .toAscii().constData();
}


void PRGDocument::cacheAddPage( PRGPage* page )
{
    // Is the page already in the cache?
    std::list<PRGPage*>::iterator it;
    it = std::find( m_cachePages.begin(), m_cachePages.end(), page );
    if( it == m_cachePages.end() ) {
        // Push to front and remove back if necessary.
        m_cachePages.push_front( page );
        if( m_cachePages.size() > m_cachePagesSize ) {
            this->cacheRmPage( m_cachePages.back() );
        }
        QLOG_INFO() << QString( "<PRGDocument> Cache page data (index: %1)." )
                       .arg( page->pageIndex() )
                       .toAscii().constData();
    }
    else {
        // Move the page to the front.
        m_cachePages.erase( it );
        m_cachePages.push_front( page );
    }
}
void PRGDocument::cacheRmPage( PRGPage* page )
{
    // Remove the page from the cache and clear its data.
    std::list<PRGPage*>::iterator it;
    it = std::find( m_cachePages.begin(), m_cachePages.end(), page );
    if( it != m_cachePages.end() ) {
        (*it)->clearData();
        m_cachePages.erase( it );
//        QLOG_INFO() << QString( "<PRGDocument> Uncache page data (index: %1)." )
//                       .arg( page->pageIndex() )
//                       .toAscii().constData();
    }
}

void PRGDocument::createSubDocuments( double tolerance )
{
    // PoDoFo document.
    PdfMemDocument* pDocument = this->parent()->podofoDocument();

    long idx( 0 );
    long idxFirst;
    PdfRect cbox;
    PdfRect cboxst;

    while( idx < pDocument->GetPageCount() ) {
        // Initialize the subdocument with the current page.
        idxFirst = idx;
        cboxst = PRGPage::PageCropBox( pDocument->GetPage( idxFirst ) );
        bool belong = true;
        //++idx;

        while( idx < pDocument->GetPageCount() ) {
            // Check the size of the page (width and height).
            cbox = PRGPage::PageCropBox( pDocument->GetPage( idx ) );
            belong = ( fabs( cboxst.GetWidth() - cbox.GetWidth() ) <= cboxst.GetWidth() * tolerance ) &&
                    ( fabs( cboxst.GetHeight() - cbox.GetHeight() ) <= cboxst.GetHeight() * tolerance );
            if( !belong ) {
                break;
            }
            ++idx;
        }
        // Create the SubDocument and add it to the vector.
        m_subDocuments.push_back( new PRGSubDocument( this, idxFirst, idx-1 ) );
    }
}
void PRGDocument::clear()
{
    // Delete PRSubDocuments objects.
    for( size_t i = 0 ; i < m_subDocuments.size() ; ++i ) {
        delete m_subDocuments[i];
        m_subDocuments[i] = NULL;
    }
    m_subDocuments.clear();
}

PRDocument* PRGDocument::parent() const
{
    // TODO: dynamic_cast or static_cast?
    return static_cast<PRDocument*>( this->QObject::parent() );
}
size_t PRGDocument::nbPages() const
{
    return this->parent()->podofoDocument()->GetPageCount();
}
PRGPage* PRGDocument::page( size_t idx )
{
    // Find the sub-document if belongs to.
    PRGSubDocument* subDocument = NULL;
    for( size_t i = 0 ; i < m_subDocuments.size() && !subDocument ; ++i ) {
        if( idx >= m_subDocuments[i]->firstPageIndex() && idx <= m_subDocuments[i]->lastPageIndex() ) {
            subDocument = m_subDocuments[i];
        }
    }
    if( subDocument ) {
        return subDocument->page( idx );
    }
    return NULL;
}
const PRGPage* PRGDocument::page( size_t idx ) const
{
    // Use the non-const implementation!
    return const_cast<PRGPage*>( const_cast<PRGDocument*>( this )->page( idx ) );
}


//************************************************************//
//                   PRGDocument::GParameters                 //
//************************************************************//
PRGDocument::GParameters::GParameters()
{
    this->init();
}
void PRGDocument::GParameters::init()
{
    // Page range.
    firstPageIndex = 0;
    lastPageIndex = std::numeric_limits<size_t>::max();
    // Detect lines.
    textLineDetection = true;
}




}
