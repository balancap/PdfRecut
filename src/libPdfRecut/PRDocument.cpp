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

void PRDocument::loadPages()
{
    this->clearPages();
    // Load pages from PoDoFo document.
    size_t nbPages = m_podofoDocument->GetPageCount();
    m_pPages.resize( nbPages );
    for( size_t i = 0 ; i < nbPages ; ++i ) {
        m_pPages[i] = new PRPage( m_podofoDocument->GetPage( i ), false );
        this->attachPage( i );
    }

}
void PRDocument::clearPages()
{
    for( size_t i = 0 ; i < m_pPages.size() ; ++i ) {
        delete m_pPages[i];
    }
    m_pPages.clear();
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
        page->attach( this, m_podofoDocument->GetPage( index ) );
        // Connect signals: TODO.
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
                + infoOut.baseName()
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
