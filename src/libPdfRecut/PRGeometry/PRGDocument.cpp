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

using namespace PoDoFo;
using namespace PoDoFoExtended;

namespace PdfRecut {

//************************************************************//
//                         PRGDocument                         //
//************************************************************//
PRGDocument::PRGDocument( PRDocument* parent ) :
    QObject( parent )
{
}
PRGDocument::~PRGDocument()
{
    this->clear();
}
PRDocument* PRGDocument::parent() const
{
    // TODO: dynamic_cast or static_cast?
    return static_cast<PRDocument*>( this->QObject::parent() );
}

void PRGDocument::analyse(const PRGDocument::GParameters &params)
{
    // Time spent analysing the document.
    QTime timeTask;
    timeTask.start();

    // First clear content.
    this->clear();
    QLOG_INFO() << QString( "<PRGDocument> Begin analysis of the geometry of the PDF document." )
                   .toAscii().constData();

    // Detect sub-documents and analyse their content.
    this->detectSubDocuments( params.subDocumentTolerance );
    for( size_t i = 0 ; i < m_subDocuments.size() ; ++i ) {
        m_subDocuments[i]->analyse( params );
    }
    QLOG_INFO() << QString( "<PRGDocument> End analysis of the geometry of the PDF document (%1 seconds)." )
                   .arg( timeTask.elapsed() / 1000. )
                   .toAscii().constData();
}
void PRGDocument::clear()
{
    // Delete PRSubDocuments objects.
    for( size_t i = 0 ; i < m_subDocuments.size() ; ++i ) {
        m_subDocuments[i]->clear();
        delete m_subDocuments[i];
        m_subDocuments[i] = NULL;
    }
    m_subDocuments.clear();
}
void PRGDocument::detectSubDocuments( double tolerance )
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
            // QLOG_INFO() << QString( "Crop box: %1" ).arg( PdfRectToString( cbox ).c_str() ).toLocal8Bit().constData();
        }
        // Create the SubDocument and add it to the vector.
        m_subDocuments.push_back( new PRGSubDocument( this, idxFirst, idx-1 ) );
    }
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
        if( i >= m_subDocuments[i]->firstPageIndex() && i <= m_subDocuments[i]->lastPageIndex() ) {
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
    // 2 percent tolerance on the size.
    subDocumentTolerance = 0.05;

    // Detect lines.
    textLineDetection = true;
}




}
