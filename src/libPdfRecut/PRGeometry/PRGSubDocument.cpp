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
#include "PRGTextPage.h"

#include "QsLog/QsLog.h"

#include "podofo/podofo.h"

using namespace PoDoFo;

namespace PdfRecut {

PRGSubDocument::PRGSubDocument( PRGDocument* parent, size_t firstPageIndex, size_t lastPageIndex ) :
    QObject( parent ),
    m_firstPageIndex( firstPageIndex ),
    m_lastPageIndex( lastPageIndex )
{
    // Create pages corresponding to the sub-document.
    m_pages.resize( m_lastPageIndex-m_firstPageIndex+1, NULL );
    for( size_t i = 0 ; i < m_pages.size() ; ++i ) {
        m_pages[i] = new PRGPage( this, m_firstPageIndex + i );
    }
    // Compute mean bounding box.
    this->computeMeanCropBox();

    // Log information.
    QLOG_INFO() << QString( "<PRGSubDocument> Create sub-document with range [%1,%2] and size (%3,%4)" )
                   .arg( m_firstPageIndex ).arg( m_lastPageIndex )
                   .arg( m_meanCropBox.GetWidth() ).arg( m_meanCropBox.GetHeight() )
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
    // Log analysis.
    QLOG_INFO() << QString( "<PRGSubDocument> Begin analysis of sub-document with range [%1,%2] and mean size (%3,%4)." )
                   .arg( m_firstPageIndex ).arg( m_lastPageIndex )
                   .arg( m_meanCropBox.GetWidth() ).arg( m_meanCropBox.GetHeight() )
                   .toAscii().constData();

    // Page range.
    size_t firstIndex = std::max( params.firstPageIndex, m_firstPageIndex );
    size_t lastIndex = std::min( params.lastPageIndex, m_lastPageIndex );

    if( firstIndex <= lastIndex ) {
        // Compute basic subdocument statistics.
        this->computeBasicStats( firstIndex, lastIndex );

        // Analyse page content.
        for( size_t i = firstIndex ; i <= lastIndex ; ++i ) {
            m_pages[ i - m_firstPageIndex ]->analyse( params );
        }
    }

    // Log analysis.
    QLOG_INFO() << QString( "<PRGSubDocument> End analysis of sub-document with range [%1,%2] and mean size (%3,%4)." )
                   .arg( m_firstPageIndex ).arg( m_lastPageIndex )
                   .arg( m_meanCropBox.GetWidth() ).arg( m_meanCropBox.GetHeight() )
                   .toAscii().constData();
}
void PRGSubDocument::clear()
{
    // Delete PRGPage objects.
    for( size_t i = 0 ; i < m_pages.size() ; ++i ) {
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
void PRGSubDocument::computeBasicStats( size_t firstIndex, size_t lastIndex )
{
    QLOG_INFO() << QString( "<PRGSubDocument> Begin text statistics on sub-document." )
                   .toAscii().constData();

    // Compute groups of words statistics.
    for( size_t i = firstIndex ; i <= lastIndex ; ++i ) {
        PRGPage* page = this->page( i );
        page->loadData();
        for( size_t j = 0 ; j < page->text()->nbGroupsWords() ; ++j ) {
            m_textStatistics.addGroupWords( *(page->text()->groupWords(j)) );
        }
    }
    QLOG_INFO() << QString( "<PRGSubDocument> End text statistics on sub-document." )
                   .toAscii().constData();
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
