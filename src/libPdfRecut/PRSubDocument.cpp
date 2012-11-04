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
#include "PRPage.h"

#include "QsLog/QsLog.h"

#include "podofo/podofo.h"

using namespace PoDoFo;

namespace PdfRecut {

PRSubDocument::PRSubDocument( PRDocument* parent, size_t firstPageIndex, size_t lastPageIndex ) :
    QObject( parent ), m_firstPageIndex( firstPageIndex ), m_lastPageIndex( lastPageIndex )
{    
    // Log information.
    PdfRect cbox = PRDocument::PageCropBox( parent->podofoDocument()->GetPage( firstPageIndex ) );
    QLOG_INFO() << QString( "<PRSubDocument> Create sub-document with range [%1,%2] and size (%3,%4)" )
                   .arg( m_firstPageIndex ).arg( m_lastPageIndex )
                   .arg( cbox.GetWidth() ).arg( cbox.GetHeight() )
                   .toAscii().constData();
}
PRSubDocument::~PRSubDocument()
{
    this->clearContent();
}

void PRSubDocument::analyseContent( const PRDocument::ContentParameters& params )
{
    this->clearContent();

    // Create pages corresponding to the sub-document.
    m_pages.resize( m_lastPageIndex-m_firstPageIndex+1, NULL );
    for( size_t i = 0 ; i < m_pages.size() ; ++i ) {
        m_pages[i] = new PRPage( this, m_firstPageIndex + i );
    }

    // Analyse content of pages.
    for( size_t i = 0 ; i < m_pages.size() ; ++i ) {
        m_pages[i]->analyseContent( params );
    }


}
void PRSubDocument::clearContent()
{
    // Delete PRPage objects.
    for( size_t i = 0 ; i < m_pages.size() ; ++i ) {
        m_pages[i]->clearContent();
        delete m_pages[i];
        m_pages[i] = NULL;
    }
    m_pages.clear();
}

PRDocument* PRSubDocument::parent() const
{
    // TODO: dynamic_cast or static_cast?
    return static_cast<PRDocument*>( this->QObject::parent() );
}

PRPage* PRSubDocument::page( size_t idx )
{
    if( idx < m_firstPageIndex || idx > m_lastPageIndex || !m_pages.size() ) {
        return NULL;
    }
    return m_pages.at( idx - m_firstPageIndex );
}
const PRPage* PRSubDocument::page( size_t idx ) const
{
    // Use the non-const implementation!
    return const_cast<PRPage*>( const_cast<PRSubDocument*>( this )->page( idx ) );
}

}
