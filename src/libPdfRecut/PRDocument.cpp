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
#include "PRSubDocument.h"
#include "PRException.h"

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

    // Old ?
//    QString methodTitle = tr( "Load PDF Document." );
//    emit methodProgress( methodTitle, 0.0 );
//    emit methodProgress( methodTitle, 1.0 );
}
void PRDocument::save( const QString& filename )
{
    // Write down PoDoFo document.
    QString suffix( "_recut" );
    this->writePoDoFoDocument( filename, suffix );
}
void PRDocument::clear()
{
    // Free PoDoFo document.
    this->freePoDoFoDocument();
    // Cleat font cache.
    this->clearFontCache();
    // Clear contents.
    this->clearContent();
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

    // If not find, add to cache.
    if( it == m_fontCache.end() ) {
        return this->addFontToCache( fontRef );
    }
    else {
        return it->second;
    }
}
void PRDocument::clearFontCache()
{
    // Free Pdf font metrics object.
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
    else if( fontSubType == PdfName("Type1") ) {
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

void PRDocument::analyseContent( const ContentParameters& params )
{
    // First clear content.
    this->clearContent();

    // Detect sub-documents and analyse their content.
    this->detectSubDocuments( params.subDocumentTolerance );
    for( size_t i = 0 ; i < m_subDocuments.size() ; ++i ) {
        m_subDocuments[i]->analyseContent( params );
    }
}
void PRDocument::clearContent()
{
    // Delete PRSubDocuments objects.
    for( size_t i = 0 ; i < m_subDocuments.size() ; ++i ) {
        m_subDocuments[i]->clearContent();
        delete m_subDocuments[i];
        m_subDocuments[i] = NULL;
    }
    m_subDocuments.clear();
}
void PRDocument::detectSubDocuments( double tolerance )
{
    long idx( 0 );
    long idxFirst;
    PdfRect cbox;
    PdfRect cboxst;

    while( idx < m_podofoDocument->GetPageCount() ) {
        // Initialize the subdocument with the current page.
        idxFirst = idx;
        cboxst = PageCropBox( m_podofoDocument->GetPage( idxFirst ) );
        bool belong = true;
        //++idx;

        while( idx < m_podofoDocument->GetPageCount() ) {
            // Check the size of the page (width and height).
            cbox = PageCropBox( m_podofoDocument->GetPage( idx ) );
            belong = ( fabs( cboxst.GetWidth() - cbox.GetWidth() ) <= cboxst.GetWidth() * tolerance ) &&
                    ( fabs( cboxst.GetHeight() - cbox.GetHeight() ) <= cboxst.GetHeight() * tolerance );
            if( !belong ) {
                break;
            }
            ++idx;
            // QLOG_INFO() << QString( "Crop box: %1" ).arg( PdfRectToString( cbox ).c_str() ).toLocal8Bit().constData();
        }
        // Create the SubDocument and add it to the vector.
        m_subDocuments.push_back( new PRSubDocument( this, idxFirst, idx-1 ) );
    }
}

PdfRect PRDocument::PageMediaBox( PdfPage* pPage )
{
    PdfRect mediaBox( pPage->GetMediaBox() );
    return mediaBox;
}

PdfRect PRDocument::PageCropBox( PdfPage* pPage )
{
    PdfRect mediaBox( PRDocument::PageMediaBox( pPage ) );
    PdfRect cropBox( pPage->GetCropBox() );

    // Intersection between the two.
    cropBox = PdfeORect::intersection( mediaBox, cropBox );
    return cropBox;
}

size_t PRDocument::nbPages() const
{
    return m_podofoDocument->GetPageCount();
}
PRPage* PRDocument::page( size_t idx )
{
    // Find the sub-document if belongs to.
    PRSubDocument* subDocument = NULL;
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
const PRPage* PRDocument::page( size_t idx ) const
{
    // Use the non-const implementation!
    return const_cast<PRPage*>( const_cast<PRDocument*>( this )->page( idx ) );
}


//************************************************************//
//                PRDocument::ContentParameters               //
//************************************************************//
PRDocument::ContentParameters::ContentParameters()
{
    this->init();
}
void PRDocument::ContentParameters::init()
{
    // 2 percent tolerance on the size.
    subDocumentTolerance = 0.05;

    // Detect lines.
    textLineDetection = true;
}




}
