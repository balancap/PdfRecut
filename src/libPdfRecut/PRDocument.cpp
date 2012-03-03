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

#include <QtCore>
#include <poppler/qt4/poppler-qt4.h>
#include <podofo/podofo.h>

namespace PdfRecut {

PRDocument::PRDocument( QObject* parent, const QString& filename ) :
    QObject( parent )
{
    m_filename = filename;
    m_podofoDocument = NULL;
    m_popplerDocument = NULL;
}
PRDocument::~PRDocument()
{
    delete m_podofoDocument;
    delete m_popplerDocument;
}

void PRDocument::loadDocuments( bool loadPoDoFo, bool loadPoppler )
{
    QString methodTitle = tr( "Load Pdf Document." );

    // Load PoDoFo document.
    emit methodProgress( methodTitle, 0.0 );
    if( loadPoDoFo )
    {
        try {
            this->loadPoDoFoDocument();
        }
        catch( const PRException& error ) {
            emit methodError( "PoDoFo document load error.",
                              error.getDescription() );
            return;
        }
    }

    if( loadPoDoFo && loadPoppler ) {
        emit methodProgress( methodTitle, 0.5 );
    }

    // Load Poppler document.
    if( loadPoppler )
    {
        try {
            this->loadPopplerDocument();
        }
        catch( const PRException& error ) {
            emit methodError( "Poppler document load error.",
                              error.getDescription() );
            return;
        }
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
Poppler::Document* PRDocument::loadPopplerDocument()
{
    // Not null pointer: free current document before loading.
    if( m_popplerDocument ) {
        this->freePopplerDocument();
    }

    // Get mutex and then load file.
    QMutexLocker locker( &m_popplerMutex );

    m_popplerDocument = Poppler::Document::load( m_filename );
    if( !m_popplerDocument || m_popplerDocument->isLocked() )
    {
        delete m_popplerDocument;
        m_popplerDocument = NULL;

        // Throw exception...
        throw PRException( ePdfDocE_Poppler,
              tr( "Poppler library can not load file" ) );
    }
    return m_popplerDocument;
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

    delete m_podofoDocument;
    m_podofoDocument = NULL;
}
void PRDocument::freePopplerDocument()
{
    // Get mutex and then free.
    QMutexLocker locker( &m_popplerMutex );

    delete m_popplerDocument;
    m_popplerDocument = NULL;
}

void PRDocument::setFilename( const QString& filename )
{
    // Free PoDoFo and Poppler documents.
    this->freePoDoFoDocument();
    this->freePopplerDocument();

    m_filename = filename;
}

}
