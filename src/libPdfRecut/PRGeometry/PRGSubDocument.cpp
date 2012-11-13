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

#include "PRGSubDocument.h"

#include "PRDocument.h"
#include "PRGDocument.h"
#include "PRGPage.h"

#include "QsLog/QsLog.h"

#include "podofo/podofo.h"

using namespace PoDoFo;

namespace PdfRecut {

PRGSubDocument::PRGSubDocument( PRGDocument* parent, size_t firstPageIndex, size_t lastPageIndex ) :
    QObject( parent ),
    m_firstPageIndex( firstPageIndex ),
    m_lastPageIndex( lastPageIndex )
{    
    // Log information.
    PdfRect cbox = PRGPage::PageCropBox( parent->parent()->podofoDocument()->GetPage( firstPageIndex ) );
    QLOG_INFO() << QString( "<PRGSubDocument> Create sub-document with range [%1,%2] and size (%3,%4)" )
                   .arg( m_firstPageIndex ).arg( m_lastPageIndex )
                   .arg( cbox.GetWidth() ).arg( cbox.GetHeight() )
                   .toAscii().constData();
}
PRGSubDocument::~PRGSubDocument()
{
    this->clear();
}
PRGDocument* PRGSubDocument::parent() const
{
    return static_cast<PRGDocument*>( this->QObject::parent() );
}

void PRGSubDocument::analyse( const PRGDocument::GParameters& params )
{
    this->clear();

    // Create pages corresponding to the sub-document.
    m_pages.resize( m_lastPageIndex-m_firstPageIndex+1, NULL );
    for( size_t i = 0 ; i < m_pages.size() ; ++i ) {
        m_pages[i] = new PRGPage( this, m_firstPageIndex + i );
    }
    // Compute mean bounding box.
    this->computeMeanCropBox();

    // Log analysis.
    QLOG_INFO() << QString( "<PRGSubDocument> Begin analysis of sub-document with range [%1,%2] and mean size (%3,%4)" )
                   .arg( m_firstPageIndex ).arg( m_lastPageIndex )
                   .arg( m_meanCropBox.GetWidth() ).arg( m_meanCropBox.GetHeight() )
                   .toAscii().constData();
    // Analyse content of pages.
    for( size_t i = 0 ; i < m_pages.size() ; ++i ) {
        m_pages[i]->analyse( params );
    }

    // Log analysis.
    QLOG_INFO() << QString( "<PRGSubDocument> End analysis of sub-document with range [%1,%2] and mean size (%3,%4)" )
                   .arg( m_firstPageIndex ).arg( m_lastPageIndex )
                   .arg( m_meanCropBox.GetWidth() ).arg( m_meanCropBox.GetHeight() )
                   .toAscii().constData();
}
void PRGSubDocument::clear()
{
    // Delete PRGPage objects.
    for( size_t i = 0 ; i < m_pages.size() ; ++i ) {
        m_pages[i]->clear();
        delete m_pages[i];
        m_pages[i] = NULL;
    }
    m_pages.clear();
}

void PRGSubDocument::computeMeanCropBox()
{
    double width(0), height(0);
    for( size_t i = 0 ; i < this->nbPages() ; ++i ) {
        PdfRect cbox = PRGPage::PageCropBox( m_pages[i]->podofoPage() );
        width += cbox.GetWidth();
        height += cbox.GetHeight();
    }
    m_meanCropBox = PdfRect( 0.0, 0.0, width / this->nbPages(), height / this->nbPages() );
}

PRGPage* PRGSubDocument::page( size_t idx )
{
    if( idx < m_firstPageIndex || idx > m_lastPageIndex || !m_pages.size() ) {
        return NULL;
    }
    return m_pages.at( idx - m_firstPageIndex );
}
const PRGPage* PRGSubDocument::page( size_t idx ) const
{
    // Use the non-const implementation!
    return const_cast<PRGPage*>( const_cast<PRGSubDocument*>( this )->page( idx ) );
}

}
