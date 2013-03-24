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

#include "PRPage.h"
#include "PRDocument.h"

#include <podofo/podofo.h>

using namespace PoDoFo;
using namespace PoDoFoExtended;

namespace PdfRecut {

PRPage::PRPage( const PdfRect& mediaBox ) :
    QObject( NULL ),
    m_pageIndex( 0 ),
    m_pContentsStream( NULL ),
    m_ownPageContentsObj( false )
{
    this->initAttributes( mediaBox );
}
PRPage::PRPage( const PRPage& rhs ) :
    QObject( NULL ),
    m_pageIndex( 0 ),
    m_pContentsStream( NULL ),
    m_ownPageContentsObj( false )
{
    this->copyContents( rhs );
    this->copyAttributes( rhs );
}
PRPage& PRPage::operator=( const PRPage& rhs )
{
    if( this == &rhs ) {
        return *this;
    }
    this->copyContents( rhs );
    this->copyAttributes( rhs );
    // Modified signal !!!
    this->pushModifications( true, true );
    return *this;
}
PRPage::~PRPage()
{
    this->uncacheContents();
    delete m_pContentsStream;
}

void PRPage::init( const PdfRect& mediaBox )
{
    // Initialize contents and attributes.
    delete m_pContentsStream;
    m_pContentsStream = NULL;
    this->initAttributes( mediaBox );
    this->pushModifications( true, true );
}
void PRPage::clear()
{
    this->init( PdfRect() );
}

void PRPage::initAttributes( const PdfRect& mediaBox )
{
    // Initialize page boxes.
    m_mediaBox = mediaBox;
    m_cropBox = m_bleedBox
            = m_trimBox
            = m_artBox = PdfRect();
}
void PRPage::copyAttributes( const PRPage& rhs )
{
    // Page boxes.
    m_mediaBox = rhs.m_mediaBox;
    m_cropBox = rhs.m_cropBox;
    m_bleedBox = rhs.m_bleedBox;
    m_trimBox = rhs.m_trimBox;
    m_artBox = rhs.m_artBox;
}
void PRPage::copyContents( const PRPage& rhs )
{
    if( rhs.m_pContentsStream ) {
        this->pContents()->operator=( rhs.contents() );
    }
    else {
        delete m_pContentsStream;
        m_pContentsStream = NULL;
    }
}

void PRPage::load( const PdfPage* page, bool incContents, bool incAttributes )
{
    if( page && incAttributes ) {
        // Load page boxes.
        this->setMediaBox( page->GetMediaBox() );
        this->setCropBox( page->GetCropBox() );
        this->setBleedBox( page->GetBleedBox() );
        this->setTrimBox( page->GetTrimBox() );
        this->setArtBox( page->GetArtBox() );
    }
    if( page && incContents ) {
        // TODO: fix const_cast...
        this->pContents()->load( const_cast<PdfPage*>( page ), true, true );
    }
    else if( !incContents ) {
        // Set contents to empty if not loaded.
        delete m_pContentsStream;
        m_pContentsStream = NULL;
    }
    emit loaded( m_pageIndex, incContents, incAttributes );
}
void PRPage::save( PdfPage* page, bool incContents, bool incAttributes )
{
    if( page && incAttributes ) {
        // Set page boxes.
        PdfVariant pagebox;
        m_mediaBox.ToVariant( pagebox );
        page->GetObject()->GetDictionary().AddKey( "MediaBox", pagebox );
        if( m_cropBox.GetWidth() && m_cropBox.GetHeight() ) {
            m_cropBox.ToVariant( pagebox );
            page->GetObject()->GetDictionary().AddKey( "CropBox", pagebox );
        }
        if( m_bleedBox.GetWidth() && m_bleedBox.GetHeight() ) {
            m_bleedBox.ToVariant( pagebox );
            page->GetObject()->GetDictionary().AddKey( "BleedBox", pagebox );
        }
        if( m_trimBox.GetWidth() && m_trimBox.GetHeight() ) {
            m_trimBox.ToVariant( pagebox );
            page->GetObject()->GetDictionary().AddKey( "TrimBox", pagebox );
        }
        if( m_artBox.GetWidth() && m_artBox.GetHeight() ) {
            m_artBox.ToVariant( pagebox );
            page->GetObject()->GetDictionary().AddKey( "ArtBox", pagebox );
        }
    }
    if( page && incContents ) {
        this->cleanPoDoFoPageStreams( page );
        // Set page contents and resources.
        this->pContents()->save( page );
    }
}

void PRPage::cacheContents() const
{
    // Convention: m_pContentsStream set to NULL when uncached.
    if( !m_pContentsStream ) {
        PdfPage* page = this->podofoPage();
        if( page ) {
            this->pContents()->load( page, true, true );
            emit contentsCached( m_pageIndex );
        }
    }
}
void PRPage::uncacheContents() const
{
    // Convention: m_pContentsStream set to NULL when uncached.
    if( m_pContentsStream && this->podofoPage() ) {
        delete m_pContentsStream;
        m_pContentsStream = NULL;
        emit contentsUncached( m_pageIndex );
    }
}
bool PRPage::isContentsCached() const
{
    return m_pContentsStream;
}

void PRPage::pushModifications( bool incContents, bool incAttributes )
{
    PoDoFo::PdfPage* page = this->podofoPage();
    if( page ) {
        this->save( page, incContents, incAttributes );
    }
    emit modified( m_pageIndex, incContents, incAttributes );
}
void PRPage::cleanPoDoFoPageStreams( PdfPage* page )
{
    if( page && !m_ownPageContentsObj ) {
        PdfObject* pContentsObj = page->GetContents();
        if( pContentsObj->IsArray() ) {
            PdfArray& contentsArray = pContentsObj->GetArray();
            for( size_t i = 0 ; i < contentsArray.size() ; ++i ) {
                // TODO: PRDocument trash. Do not clean them since they can be used elsewhere.
            }
            contentsArray.Clear();
            // Create new stream.
            PdfObject* pStreamObj = pContentsObj->GetOwner()->CreateObject();
            pStreamObj->GetStream();
            PdfReference streamRef( pStreamObj->Reference().ObjectNumber(),
                                    pStreamObj->Reference().GenerationNumber() );
            contentsArray.push_back( streamRef );
        }
        else {
            // Simply clear the stream.
            PdfStream* pstream = pContentsObj->GetStream();
            pstream->BeginAppend( true );
            pstream->EndAppend();
        }
        m_ownPageContentsObj = true;
    }
}

PRDocument* PRPage::document() const
{
    return static_cast<PRDocument*>( this->QObject::parent() );
}
PRDocument* PRPage::parent() const
{
    return static_cast<PRDocument*>( this->QObject::parent() );
}
PdfPage* PRPage::podofoPage() const
{
    PRDocument* pdocument = this->document();
    if( pdocument && pdocument->podofoDocument() ) {
        return pdocument->podofoDocument()->GetPage( m_pageIndex );
    }
    return NULL;
}
size_t PRPage::pageIndex() const
{
    return m_pageIndex;
}

const PdfeContentsStream& PRPage::contents() const
{
    if( !this->isContentsCached() ) {
        this->cacheContents();
    }
    return *this->pContents();
}

void PRPage::setParent( PRDocument* document )
{
    this->QObject::setParent( document );
    m_ownPageContentsObj = false;
}
void PRPage::setContents( const PdfeContentsStream& contents )
{
    // Set contents and signal modifications.
    this->pContents()->operator=( contents );
    this->pushModifications( true, false );
}

PdfeContentsStream* PRPage::pContents() const
{
    if( !m_pContentsStream ) {
        m_pContentsStream = new PdfeContentsStream();
    }
    return m_pContentsStream;
}

void PRPage::setMediaBox( const PoDoFo::PdfRect& rhs )
{
    m_mediaBox = rhs;
    this->pushModifications( false, true );
}
void PRPage::setCropBox( const PoDoFo::PdfRect& rhs )
{
    m_cropBox = PdfeRect::intersection( m_mediaBox, rhs );
    this->pushModifications( false, true );
}
void PRPage::setBleedBox( const PoDoFo::PdfRect& rhs )
{
    m_bleedBox = PdfeRect::intersection( m_mediaBox, rhs );
    this->pushModifications( false, true );
}
void PRPage::setTrimBox( const PoDoFo::PdfRect& rhs )
{
    m_trimBox = PdfeRect::intersection( m_mediaBox, rhs );
    this->pushModifications( false, true );
}
void PRPage::setArtBox( const PoDoFo::PdfRect& rhs )
{
    m_artBox = PdfeRect::intersection( m_mediaBox, rhs );
    this->pushModifications( false, true );
}

}
