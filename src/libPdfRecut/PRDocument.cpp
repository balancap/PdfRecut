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
#include "PRException.h"

#include "PdfeFontType0.h"
#include "PdfeFontTrueType.h"
#include "PdfeFontType1.h"
#include "PdfeFontType3.h"

#include <QtCore>
#include <podofo/podofo.h>

#include <ft2build.h>
#include FT_FREETYPE_H

using namespace PoDoFo;
using namespace PoDoFoExtended;

namespace PdfRecut {

PRDocument::PRDocument( QObject* parent, const QString& filename ) :
    QObject( parent )
{
    m_filename = filename;
    m_podofoDocument = NULL;

    // Initialize freetype library.
    if( FT_Init_FreeType( &m_ftLibrary ) ) {
        PODOFO_RAISE_ERROR( ePdfError_FreeType );
    }
}
PRDocument::~PRDocument()
{
    this->freeFontCache();
    delete m_podofoDocument;
}

void PRDocument::loadDocument()
{
    QString methodTitle = tr( "Load PDF Document." );

    // Load PoDoFo document.
    emit methodProgress( methodTitle, 0.0 );
    try {
        this->loadPoDoFoDocument();
    }
    catch( const PRException& error ) {
        emit methodError( "PoDoFo document load error.",
                          error.getDescription() );
        return;
    }
    emit methodProgress( methodTitle, 1.0 );
}
PoDoFo::PdfMemDocument* PRDocument::loadPoDoFoDocument()
{
    // Not null pointer: free current document before loading.
    if( m_podofoDocument ) {
        this->freePoDoFoDocument();
    }

    // Get mutex and then load file.
    QMutexLocker locker( &m_podofoMutex );

    try
    {
        m_podofoDocument = new PoDoFo::PdfMemDocument();
        m_podofoDocument->Load( m_filename.toLocal8Bit().data() );
    }
    catch( const PoDoFo::PdfError& error )
    {
        delete m_podofoDocument;
        m_podofoDocument = NULL;

        // Throw exception...
        throw PRException( error );
    }
    return m_podofoDocument;
}
void PRDocument::writePoDoFoDocument( const QString& filename )
{
    // No document loaded.
    if( !m_podofoDocument ) {
        throw PRException( ePdfDocE_PoDoFo,
              tr( "Can not write PoDoFo document: no file loaded." ) );
        return;
    }

    // Fix filename if corresponds to m_filename.
    QString fileOut = filename;
    QFileInfo infoOut( fileOut );
    if( infoOut == QFileInfo( m_filename ) ) {
        fileOut = infoOut.canonicalPath() + "/"
                + infoOut.baseName() + "_eInk.pdf";
    }

    // Get mutex and then write file.
    QMutexLocker locker( &m_podofoMutex );
    try
    {
        //m_podofoDocument->SetWriteMode( PoDoFo::ePdfWriteMode_Compact );
        m_podofoDocument->SetWriteMode( PoDoFo::ePdfWriteMode_Clean );
        m_podofoDocument->Write( fileOut.toLocal8Bit().data() );
    }
    catch( const PoDoFo::PdfError& error )
    {
        // Throw exception...
        throw PRException( error );
    }
}
void PRDocument::freePoDoFoDocument()
{
    // Get mutex and then free.
    QMutexLocker locker( &m_podofoMutex );

    this->freeFontCache();
    delete m_podofoDocument;
    m_podofoDocument = NULL;
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
void PRDocument::freeFontCache()
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

void PRDocument::setFilename( const QString& filename )
{
    // Free PoDoFo document.
    this->freePoDoFoDocument();
    m_filename = filename;
}

}
