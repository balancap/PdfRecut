/***************************************************************************
 *   Copyright (C) 2011 by Paul Balança                                    *
 *   paul.balanca@gmail.com                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "PRDocument.h"
#include "PRException.h"

#include "PdfeFontType0.h"
#include "PdfeFontTrueType.h"
#include "PdfeFontType1.h"
#include "PdfeFontType3.h"

#include <QtCore>
#include <podofo/podofo.h>

using namespace PoDoFo;
using namespace PoDoFoExtended;

namespace PdfRecut {

PRDocument::PRDocument( QObject* parent, const QString& filename ) :
    QObject( parent )
{
    m_filename = filename;
    m_podofoDocument = NULL;
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
        pFont = new PdfeFontType0( fontObj );;
    }
    else if( fontSubType == PdfName("Type1") ) {
        pFont = new PdfeFontType1( fontObj );
    }
    else if( fontSubType == PdfName("TrueType") ) {
        pFont = new PdfeFontTrueType( fontObj );
    }
    else if( fontSubType == PdfName("Type3") ) {
        pFont = new PdfeFontType3( fontObj );
    }
    return pFont;
}

void PRDocument::setFilename( const QString& filename )
{
    // Free PoDoFo document.
    this->freePoDoFoDocument();
    m_filename = filename;
}


}
