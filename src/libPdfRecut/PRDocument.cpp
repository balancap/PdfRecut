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

#include "PRDocument.h"
#include "PRPage.h"

#include "PRException.h"
#include "PRGeometry/PRGDocument.h"
#include "PRGeometry/PRGSubDocument.h"

#include "PdfeFontType0.h"
#include "PdfeFontTrueType.h"
#include "PdfeFontType1.h"
#include "PdfeFontType3.h"
#include "PdfeUtils.h"

#include <QtCore>
#include <podofo/podofo.h>

#include <ft2build.h>
#include FT_FREETYPE_H

using namespace PoDoFo;
using namespace PoDoFoExtended;

namespace PdfRecut {

//************************************************************//
//                         PRDocument                         //
//************************************************************//
PRDocument::PRDocument( QObject* parent ) :
    QObject( parent )
{
    m_filename = QString();
    m_podofoDocument = NULL;
    // Initialize freetype library.
    if( FT_Init_FreeType( &m_ftLibrary ) ) {
        throw PRException( PRExceptionCode::FreeType,
                           tr( "Can not initialize FreeType library." ),
                           true );
    }
    // Default cache size.
    m_pagesCacheSize = std::numeric_limits<size_t>::max();
}
PRDocument::~PRDocument()
{
    this->clear();
}

void PRDocument::load( const QString& filename )
{
    // Clear document.
    this->clear();
    // Load PoDoFo document.
    this->loadPoDoFoDocument( filename );
    // Load pages.
    this->loadPages();

    // Old ?
//    QString methodTitle = tr( "Load PDF Document." );
//    emit methodProgress( methodTitle, 0.0 );
//    emit methodProgress( methodTitle, 1.0 );
}
void PRDocument::save( const QString& filename )
{
    // Write down PoDoFo document.
    QString suffix( "_PdfRecut" );
    this->writePoDoFoDocument( filename, suffix );
}
void PRDocument::clear()
{
    // Cleat font cache.
    this->clearFontCache();
    // Clear pages.
    this->clearPages();
    // Free PoDoFo document.
    this->freePoDoFoDocument();
}

PRPage* PRDocument::insertPage( size_t index, PRPage* pPage )
{
    index = std::min( std::max( index, size_t(0) ), m_pPages.size() );
    // Create temp page and insert it in the PoDoFo pages tree.
    PdfPage* tmpPage = new PdfPage( pPage->mediaBox(), m_podofoDocument );
    m_podofoDocument->GetPagesTree()->InsertPage( int(index)-1, tmpPage );
    delete tmpPage;

    // Insert PRPage object and push modifications.
    m_pPages.insert( m_pPages.begin()+index, pPage );
    this->attachPage( index );
    m_pPages[ index ]->pushModifications( true, true );
    // Update pages indexes.
    this->setPagesIndex();
    return m_pPages[ index ];
}
PRPage* PRDocument::insertPage( size_t index )
{
    return this->insertPage( index, new PRPage() );
}
void PRDocument::deletePage( size_t index )
{
    index = std::min( std::max( index, size_t(0) ), m_pPages.size()-1 );
    // Delete PoDoFo::PdfPage.
    m_podofoDocument->GetPagesTree()->DeletePage( index );
    // TODO: move page objects to trash before deleting...

    // Delete PRPage.
    delete m_pPages[ index ];
    m_pPages.erase( m_pPages.begin() + index );
    this->setPagesIndex();
}

void PRDocument::loadPages()
{
    this->clearPages();
    // Load pages from PoDoFo document.
    size_t nbPages = m_podofoDocument->GetPageCount();
    m_pPages.resize( nbPages );
    for( size_t i = 0 ; i < nbPages ; ++i ) {
        m_pPages[i] = new PRPage();
        m_pPages[i]->load( m_podofoDocument->GetPage( i ), false, true );
        this->attachPage( i );
    }
}
void PRDocument::clearPages()
{
    for( size_t i = 0 ; i < m_pPages.size() ; ++i ) {
        delete m_pPages[i];
    }
    m_pPages.clear();
    // Clear cache.
    m_pagesCacheList.clear();
}
void PRDocument::setPagesIndex()
{
    for( size_t i = 0 ; i < m_pPages.size() ; ++i ) {
        m_pPages[i]->setPageIndex( i );
    }
}

void PRDocument::attachPage( size_t index )
{
    if( index < m_pPages.size() && m_pPages[index] ) {
        PRPage* page = m_pPages[index];
        page->setPageIndex( index );
        page->setParent( this );
        // Connect signals:
        QObject::connect( page, SIGNAL(contentsCached(size_t)),
                          this, SLOT(cachePageContents(size_t)) );
        QObject::connect( page, SIGNAL(modified(size_t,bool,bool)),
                          this, SLOT(cachePageContents(size_t)) );
        QObject::connect( page, SIGNAL(contentsUncached(size_t)),
                          this, SLOT(uncachePageContents(size_t)) );
    }
}

void PRDocument::setPagesCacheSize( size_t cacheSize )
{
    size_t minCacheSize = 10;
    m_pagesCacheSize = std::max( cacheSize, size_t( minCacheSize ) );
    this->cleanCachePages();
}
void PRDocument::cachePageContents( size_t pageIndex )
{
    // Find index in the cache list.
    std::list<size_t>::iterator it;
    it = std::find( m_pagesCacheList.begin(), m_pagesCacheList.end(), pageIndex );
    if( it == m_pagesCacheList.end() ) {
        m_pagesCacheList.push_back( pageIndex );
        m_pPages[ pageIndex ]->cacheContents();
        // Log information.
        QLOG_INFO() << QString( "<PRDocument> Cache page contents stream (index: %1)." )
                       .arg( pageIndex ).toAscii().constData();
    }
    else {
        // Already cached: push to the back of the list.
        m_pagesCacheList.erase( it );
        m_pagesCacheList.push_back( pageIndex );
    }
    // Clean cache.
    this->cleanCachePages();
}
void PRDocument::uncachePageContents( size_t pageIndex )
{
    // Find index in the cache list.
    std::list<size_t>::iterator it;
    it = std::find( m_pagesCacheList.begin(), m_pagesCacheList.end(), pageIndex );
    if( it != m_pagesCacheList.end() ) {
        // Remove index and uncache contents.
        m_pagesCacheList.erase( it );
        m_pPages[ pageIndex ]->uncacheContents();
        // Log information.
        QLOG_INFO() << QString( "<PRDocument> Uncache page contents stream (index: %1)." )
                       .arg( pageIndex ).toAscii().constData();
    }
}
void PRDocument::cleanCachePages()
{
    while( m_pagesCacheList.size() > m_pagesCacheSize ) {
        this->uncachePageContents( m_pagesCacheList.front() );
    }
}


PoDoFo::PdfMemDocument* PRDocument::loadPoDoFoDocument( const QString& filename )
{
    // Not null pointer: free current document before loading.
    if( m_podofoDocument ) {
        this->freePoDoFoDocument();
    }
    // Get mutex and then load file.
    QMutexLocker locker( &m_podofoMutex );
    try
    {
        m_filename = filename;
        // Log information.
        QLOG_INFO() << QString( "<PRDocument> Begin loading PoDoFo document \"%1\"." )
                       .arg( QFileInfo( m_filename ).fileName() )
                       .toAscii().constData();

        m_podofoDocument = new PoDoFo::PdfMemDocument();
        m_podofoDocument->Load( m_filename.toLocal8Bit().data() );
        // Log information.
        QLOG_INFO() << QString( "<PRDocument> End loading PoDoFo document \"%1\" (%2 pages)." )
                       .arg( QFileInfo( m_filename ).fileName() )
                       .arg( m_podofoDocument->GetPageCount() )
                       .toAscii().constData();
    }
    catch( const PoDoFo::PdfError& error )
    {
        // Reset members.
        delete m_podofoDocument;
        m_podofoDocument = NULL;
        m_filename.clear();
        // Throw exception...
        PRException errPR( error );
        errPR.log( QsLogging::ErrorLevel );
        throw errPR;
    }
    return m_podofoDocument;
}
void PRDocument::writePoDoFoDocument( const QString& filename, const QString& suffix )
{
    // No document loaded.
    if( !m_podofoDocument ) {
        throw PRException( PRExceptionCode::PoDoFo,
                           tr( "Can not write down PoDoFo document: no file loaded." ),
                           true );
        return;
    }
    // Fix filename if corresponds to m_filename.
    QString fileOut = filename;
    QFileInfo infoOut( fileOut );
    if( infoOut == QFileInfo( m_filename ) ) {
        fileOut = infoOut.canonicalPath() + "/"
                + infoOut.completeBaseName()
                + QString("%1.pdf").arg( suffix );
    }
    // Get mutex and then write file.
    QMutexLocker locker( &m_podofoMutex );
    try
    {
        // Log information.
        QLOG_INFO() << QString( "<PRDocument> Begin writing PoDoFo document \"%1\"." )
                       .arg( QFileInfo( fileOut ).fileName() )
                       .toAscii().constData();

        m_podofoDocument->SetWriteMode( PoDoFo::ePdfWriteMode_Compact );
        //m_podofoDocument->SetWriteMode( PoDoFo::ePdfWriteMode_Clean );
        m_podofoDocument->Write( fileOut.toLocal8Bit().data() );
        // Log information.
        QLOG_INFO() << QString( "<PRDocument> End writing PoDoFo document \"%1\"." )
                       .arg( QFileInfo( fileOut ).fileName() )
                       .toAscii().constData();
    }
    catch( const PoDoFo::PdfError& error )
    {
        // Throw exception...
        PRException errPR( error );
        errPR.log( QsLogging::ErrorLevel );
        throw errPR;
    }
}
void PRDocument::freePoDoFoDocument()
{
    // Get mutex and then free.
    QMutexLocker locker( &m_podofoMutex );
    delete m_podofoDocument;
    m_podofoDocument = NULL;
    m_filename = QString();
}

PoDoFoExtended::PdfeFont* PRDocument::fontCache( const PoDoFo::PdfReference& fontRef )
{
    // Find the reference in the cache.
    std::map< PdfReference, PdfeFont* >::iterator it;
    it = m_fontCache.find( fontRef );
    // If not found, add to cache.
    if( it == m_fontCache.end() ) {
        return this->addFontToCache( fontRef );
    }
    else {
        return it->second;
    }
}
void PRDocument::clearFontCache()
{
    // Free PDF font metrics object.
    std::map< PdfReference, PdfeFont* >::iterator it;
    for( it = m_fontCache.begin() ; it != m_fontCache.end() ; ++it ) {
        delete it->second;
        it->second = NULL;
    }
}
PoDoFoExtended::PdfeFont* PRDocument::addFontToCache( const PoDoFo::PdfReference& fontRef )
{
    // Get PoDoFo font object.
    PdfObject* fontObj = m_podofoDocument->GetObjects().GetObject( fontRef );
    PdfeFont* pFont = NULL;

    // Check it is a font object and get font subtype.
    if( fontObj->GetDictionary().GetKey( PdfName::KeyType )->GetName() != PdfName("Font") ) {
        PODOFO_RAISE_ERROR( ePdfError_InvalidDataType );
    }
    const PdfName& fontSubType = fontObj->GetDictionary().GetKey( PdfName::KeySubtype )->GetName();
    if( fontSubType == PdfName("Type0") ) {
        pFont = new PdfeFontType0( fontObj, m_ftLibrary );
    }
    else if( fontSubType == PdfName("Type1") || fontSubType == PdfName("MMType1") ) {
        pFont = new PdfeFontType1( fontObj, m_ftLibrary );
    }
    else if( fontSubType == PdfName("TrueType") ) {
        pFont = new PdfeFontTrueType( fontObj, m_ftLibrary );
    }
    else if( fontSubType == PdfName("Type3") ) {
        pFont = new PdfeFontType3( fontObj, m_ftLibrary );
    }
    // Insert font in the cache.
    m_fontCache.insert( std::make_pair( fontRef, pFont ) );
    return pFont;
}

}
