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
    m_pDocument( NULL ), m_pPage( NULL ), m_ownPageContents( false ),
    m_pageIndex( 0 ),
    m_pContentsStream( NULL )
{
    this->init( mediaBox );
}
PRPage::PRPage( PdfPage* page, bool loadPageContents ) :
    QObject( NULL ),
    m_pDocument( NULL ), m_pPage( NULL ), m_ownPageContents( false ),
    m_pageIndex( 0 ),
    m_pContentsStream( NULL )
{
    this->init( page, loadPageContents );
}
PRPage::PRPage( const PRPage& rhs ) :
    QObject( NULL ),
    m_pDocument( NULL ), m_pPage( NULL ), m_ownPageContents( false ),
    m_pageIndex( 0 ),
    m_pContentsStream( NULL ),
    m_mediaBox( rhs.m_mediaBox ),
    m_cropBox( rhs.m_cropBox ), m_bleedBox( rhs.m_bleedBox ),
    m_trimBox( rhs.m_trimBox ), m_artBox( rhs.m_artBox )
{
    m_pContentsStream = new PdfeContentsStream( rhs.contents() );
}
PRPage& PRPage::operator=( const PRPage& rhs )
{
    this->detach();
    m_pageIndex = 0;
    this->contents( false ) = rhs.contents();
    m_mediaBox = rhs.m_mediaBox;
    m_cropBox = rhs.m_cropBox;
    m_bleedBox = rhs.m_bleedBox;
    m_trimBox = rhs.m_trimBox;
    m_artBox = rhs.m_artBox;
    return *this;
}
PRPage::~PRPage()
{
    delete m_pContentsStream;
}

void PRPage::init( const PdfRect& mediaBox )
{
    this->detach();
    m_pageIndex = 0;
    this->contents( false ).init();
    m_mediaBox = mediaBox;
    m_cropBox = m_bleedBox
            = m_trimBox
            = m_artBox = PdfRect();
}
void PRPage::init( PdfPage* page, bool loadPageContents )
{
    this->detach();
    m_pageIndex = 0;
    this->contents( false ).init();
    if( loadPageContents ) {
        m_pContentsStream->load( page, true, true );
    }
    this->setMediaBox( page->GetMediaBox() );
    this->setCropBox( page->GetCropBox() );
    this->setBleedBox( page->GetBleedBox() );
    this->setTrimBox( page->GetTrimBox() );
    this->setArtBox( page->GetArtBox() );
}
void PRPage::clear()
{
    this->init( PdfRect() );
}

void PRPage::loadContents() const
{
    if( m_pDocument && m_pPage ) {
        this->contents( false ).load( m_pPage, true, true );
        // Loaded/modified signal.
        emit contentsLoaded();
    }
}
void PRPage::clearContents() const
{
    if( m_pContentsStream ) {
        delete m_pContentsStream;
        m_pContentsStream = NULL;
        // Cleared signal.
        emit contentsCleared();
    }
}
bool PRPage::isContentsLoaded() const
{
    return ( m_pContentsStream != NULL ) && ( !m_pContentsStream->isEmpty() );
}

PdfObject* PRPage::cleanPoDoFoContents()
{
    if( m_pPage && m_pDocument ) {
        PdfObject* pContentsObj = m_pPage->GetContents();
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
            return pStreamObj;
        }
        else {
            // Simply clear the stream.
            PdfStream* pstream = pContentsObj->GetStream();
            pstream->BeginAppend( true );
            pstream->EndAppend();
            return pContentsObj;
        }
    }
    return NULL;
}

void PRPage::push( bool incContents )
{
    if( m_pDocument && m_pPage ) {
        if( incContents ) {
            // Clean page contents, if not done before.
            if( !m_ownPageContents ) {
                this->cleanPoDoFoContents();
                m_ownPageContents = true;
            }
            // Set page contents and resources.
            this->contents( false ).save( m_pPage );
        }
        // Set page boxes.
        PdfVariant pagebox;
        m_mediaBox.ToVariant( pagebox );
        m_pPage->GetObject()->GetDictionary().AddKey( "MediaBox", pagebox );
        if( m_cropBox.GetWidth() && m_cropBox.GetHeight() ) {
            m_cropBox.ToVariant( pagebox );
            m_pPage->GetObject()->GetDictionary().AddKey( "CropBox", pagebox );
        }
        if( m_bleedBox.GetWidth() && m_bleedBox.GetHeight() ) {
            m_bleedBox.ToVariant( pagebox );
            m_pPage->GetObject()->GetDictionary().AddKey( "BleedBox", pagebox );
        }
        if( m_trimBox.GetWidth() && m_trimBox.GetHeight() ) {
            m_trimBox.ToVariant( pagebox );
            m_pPage->GetObject()->GetDictionary().AddKey( "TrimBox", pagebox );
        }
        if( m_artBox.GetWidth() && m_artBox.GetHeight() ) {
            m_artBox.ToVariant( pagebox );
            m_pPage->GetObject()->GetDictionary().AddKey( "ArtBox", pagebox );
        }
        // Loaded/modified signal.
        emit contentsLoaded();
    }
}
void PRPage::pull()
{
    if( m_pDocument && m_pPage ) {
        // Retrieve page contents and data.
        this->contents( false ).load( m_pPage, true, true );

        this->setMediaBox( m_pPage->GetMediaBox() );
        this->setCropBox( m_pPage->GetCropBox() );
        this->setBleedBox( m_pPage->GetBleedBox() );
        this->setTrimBox( m_pPage->GetTrimBox() );
        this->setArtBox( m_pPage->GetArtBox() );
        // Loaded/modified signal.
        emit contentsLoaded();
    }
}
void PRPage::attach( PRDocument* document, PdfPage* page )
{
    this->detach();
    m_pDocument = document;
    m_pPage = page;
    m_ownPageContents = false;
    this->setParent( document );
}

void PRPage::detach()
{
    m_pDocument = NULL;
    m_pPage = NULL;
    m_ownPageContents = false;
    this->setParent( NULL );
    // Disconnect signals/slots.
    this->disconnect();
}

const PdfeContentsStream& PRPage::contents() const
{
    return this->contents( true );
}
void PRPage::setContents( const PdfeContentsStream& contents )
{
    // Set contents and push modifications.
    this->contents( false ) = contents;
    this->push( true );
}
PdfeContentsStream& PRPage::contents( bool loadPageContents ) const
{
    // Create contents stream if does not exist.
    if( !m_pContentsStream ) {
        m_pContentsStream = new PdfeContentsStream();
    }
    // If is empty and attached to a page, try to load.
    if( loadPageContents && m_pContentsStream->isEmpty() ) {
        this->loadContents();
    }
    return *m_pContentsStream;
}

}
