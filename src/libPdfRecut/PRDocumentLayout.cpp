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

#include "PRDocumentLayout.h"
#include "PRException.h"
#include "PRStreamLayoutZone.h"

#include <podofo/podofo.h>
#include <QtCore>

//#include <iostream>
#include <ostream>
#include <stack>
#include <algorithm>

#include <climits>
#include <cmath>

using namespace PoDoFo;
using namespace PoDoFoExtended;

namespace PdfRecut {

std::string PRDocumentLayout::zoneSuffixe( "_ZS" );

//**********************************************************//
//                      Public methods                      //
//**********************************************************//
PRDocumentLayout::PRDocumentLayout( QObject* parent ) :
    QObject( parent ), m_abortOperation( false ), m_pageLayouts()
{
}

void PRDocumentLayout::init()
{
    m_pageLayouts.clear();
    m_abortOperation = false;
}

void PRDocumentLayout::addPages( int nbPages )
{
    if( nbPages > 0 )
    {
        // Write Locker semaphore needed.
        PdfeSemaphoreWriteLocker writeLocker( &m_semaphore );

        // Add pages and set index.
        size_t oldSize = m_pageLayouts.size();
        m_pageLayouts.resize( oldSize + nbPages );
        for( size_t i = oldSize ; i < m_pageLayouts.size() ; i++ )
            m_pageLayouts[i].indexOut = i;
    }
}

void PRDocumentLayout::addPageZone( int pageIndexOut, const PRPageZone& zoneIn )
{
    // Resize the vector of pages (if necessary).
    this->addPages( pageIndexOut+1 - m_pageLayouts.size() );

    // Write Locker semaphore needed.
    PdfeSemaphoreWriteLocker writeLocker( &m_semaphore );

    // Add zone to page.
    m_pageLayouts[pageIndexOut].zonesIn.push_back(zoneIn);
    m_pageLayouts[pageIndexOut].zonesIn.back().parent = & (m_pageLayouts[pageIndexOut] );
}

void PRDocumentLayout::setPageBoxes( int pageIndexOut,
                                      const PoDoFo::PdfRect& mediaBox,
                                      const PoDoFo::PdfRect& cropBox )
{
    // Resize the vector of pages (if necessary).
    this->addPages( pageIndexOut+1 - m_pageLayouts.size() );

    // Write Locker semaphore needed.
    PdfeSemaphoreWriteLocker writeLocker( &m_semaphore );

    // Set media box.
    m_pageLayouts[pageIndexOut].mediaBox = mediaBox;

    // Set crop box (and check it is smaller than mediabox...)
    if( cropBox.GetWidth() > mediaBox.GetWidth() || cropBox.GetHeight() > mediaBox.GetHeight() ) {
        m_pageLayouts[pageIndexOut].cropBox = mediaBox;
    }
    else {
        m_pageLayouts[pageIndexOut].cropBox = cropBox;
    }
}

PoDoFo::PdfRect PRDocumentLayout::getPageMediaBox( int pageIndexOut ) const
{
    if( pageIndexOut >= int(m_pageLayouts.size()) )
        PODOFO_RAISE_ERROR( ePdfError_ValueOutOfRange );
    return m_pageLayouts[pageIndexOut].mediaBox;
}

PoDoFo::PdfRect PRDocumentLayout::getPageCropBox( int pageIndexOut ) const
{
    if( pageIndexOut >= int(m_pageLayouts.size()) )
        PODOFO_RAISE_ERROR( ePdfError_ValueOutOfRange );
    return m_pageLayouts[pageIndexOut].cropBox;
}

void PRDocumentLayout::setLayoutParameters( const PRLayoutParameters& params )
{
    // Write Locker semaphore needed.
    PdfeSemaphoreWriteLocker writeLocker( &m_semaphore );

    m_parameters = params;
}

PRLayoutParameters PRDocumentLayout::getLayoutParameters() const
{
    // Read Locker semaphore needed.
    PdfeSemaphoreReadLocker readLocker( const_cast<PdfeClassSemaphore*> (&m_semaphore) );

    return m_parameters;
}

//************************************************************//
//                        Public slots                        //
//************************************************************//
void PRDocumentLayout::writeLayoutToPdf( PRDocument* documentHandle,
                                         const QString& filename )
{
    // Read Locker semaphore needed.
    PdfeSemaphoreReadLocker readLocker( &m_semaphore );
    try
    {
        // Load PoDoFo document if necessary and obtain mutex on it.
//        if( !documentHandle->isDocumentLoaded() ) {
//            documentHandle->loadPoDoFoDocument();
//        }
        QMutexLocker pdfLocker( documentHandle->podofoMutex() );

        // Transform PoDoFo document.
        this->transformDocument( documentHandle );
        pdfLocker.unlock();

        // Write it to a pdf.
        QString writeTitle = tr( "Write Pdf document with new layout." );
        emit methodProgress( writeTitle, 0.0 );
        documentHandle->save( filename );
        emit methodProgress( writeTitle, 0.5 );

        // Restore original document.
        //documentHandle->loadPoDoFoDocument();
        emit methodProgress( writeTitle, 1.0 );

        // Export done.
        emit methodProgress( "", 2.0 );
    }
    catch( const PRException& error )
    {
        // Restore original document.
        //documentHandle->loadPoDoFoDocument();

        emit methodError( tr("Pdf document export error."),
                          error.description() );
    }
}

void PRDocumentLayout::writeOverlayInToPdf( PRDocument* documentHandle,
                                             const QString& filename )
{
    // Read Locker semaphore needed.
    PdfeSemaphoreReadLocker readLocker( &m_semaphore );
    try
    {
        // Load PoDoFo document if necessary and obtain mutex on it.
//        if( !documentHandle->isDocumentLoaded() ) {
//            documentHandle->loadPoDoFoDocument();
//        }
        QMutexLocker pdfLocker( documentHandle->podofoMutex() );

        // Write overlay.
        this->printLayoutIn( documentHandle );
        pdfLocker.unlock();

        // Write it to a pdf.
        QString writeTitle = tr( "Write Pdf document with input layout overlay." );
        emit methodProgress( writeTitle, 0.0 );
        documentHandle->save( filename );
        emit methodProgress( writeTitle, 0.5 );

        // Restore original document.
//        documentHandle->loadPoDoFoDocument();
        emit methodProgress( writeTitle, 1.0 );

        // Export done.
        emit methodProgress( "", 2.0 );
    }
    catch( const PRException& error )
    {
        // Restore original document.
//        documentHandle->loadPoDoFoDocument();

        emit methodError( tr("Pdf document overlay export error."),
                          error.description() );
    }
}

void PRDocumentLayout::writeOverlayOutToPdf( PRDocument* documentHandle,
                                              const QString& filename )
{
    // Read Locker semaphore needed.
    PdfeSemaphoreReadLocker readLocker( &m_semaphore );
    try
    {
        // Load PoDoFo document if necessary and obtain mutex on it.
//        if( !documentHandle->isDocumentLoaded() ) {
//            documentHandle->loadPoDoFoDocument();
//        }
        QMutexLocker pdfLocker( documentHandle->podofoMutex() );

        // Write overlay.
        this->printLayoutOut( documentHandle );
        pdfLocker.unlock();

        // Write it to a pdf.
        QString writeTitle = tr( "Write Pdf document with output layout overlay." );
        emit methodProgress( writeTitle, 0.0 );
        documentHandle->save( filename );
        emit methodProgress( writeTitle, 0.5 );

        // Restore original document.
//        documentHandle->loadPoDoFoDocument();
        emit methodProgress( writeTitle, 1.0 );

        // Export done.
        emit methodProgress( "", 2.0 );
    }
    catch( const PRException& error )
    {
        // Restore original document.
//        documentHandle->loadPoDoFoDocument();

        emit methodError( tr("Pdf document overlay export error."),
                          error.description() );
    }
}

//*************************************************************//
//                      Protected methods                      //
//*************************************************************//
void PRDocumentLayout::transformDocument( PRDocument* documentHandle ) const
{
    // Get PoDoFo document.
    PdfMemDocument* document = documentHandle->podofoDocument();
    QString methodTitle = tr( "Reorganize Pdf document." );

    // Construct vector and map which reference zones for each page in the input document.
    std::vector< std::vector<const PRPageZone*> > vecPageInZones( document->GetPageCount() );
    std::map<PdfReference, std::vector<const PRPageZone*> > mapPageInZones;
    for(size_t idx = 0 ; idx < m_pageLayouts.size() ; idx++)
    {
        for(size_t i = 0 ; i < m_pageLayouts[idx].zonesIn.size() ; i++)
        {
            PdfPage* pageIn = document->GetPage( m_pageLayouts[idx].zonesIn[i].indexIn );
            mapPageInZones[ pageIn->GetObject()->Reference() ].push_back( &m_pageLayouts[idx].zonesIn[i] );
            vecPageInZones[ m_pageLayouts[idx].zonesIn[i].indexIn ].push_back( &m_pageLayouts[idx].zonesIn[i] );
        }
    }
    int origPagesNb = document->GetPageCount();

    // Form objects.
    std::vector<PoDoFo::PdfObject*> formObjects;

    // Local variables.
    int indexIn;
    PdfPage* pageIn;
    PdfPage* pageOut;
    PdfObject* streamObj;
    PdfVariant pagebox;

    // Pages loop construction.
    emit methodProgress( methodTitle, 0.0 );
    for(size_t idx = 0 ; idx < m_pageLayouts.size() ; idx++)
    {
        // Create an empty page in the document.
        pageOut = document->CreatePage( m_pageLayouts[idx].mediaBox );

        // Resources associated to the page.
        PdfeResources resourcesOut;
        resourcesOut.pushBack( pageOut->GetResources() );

        // Set cropbox.
        m_pageLayouts[idx].cropBox.ToVariant( pagebox );
        pageOut->GetObject()->GetDictionary().AddKey( "CropBox", pagebox );

        // Set page rotation.
        pageOut->GetObject()->GetDictionary().AddKey( "Rotate",
                                                      pdf_int64(m_parameters.pagesRotation) );

        // Add contents array to the page.
        pageOut->GetObject()->GetDictionary().AddKey( "Contents", PdfArray() );
        PdfArray& streamsArray = pageOut->GetObject()->GetDictionary().GetKey( "Contents" )->GetArray();

        // Zones loop analysis.
        for(size_t i = 0 ; i < m_pageLayouts[idx].zonesIn.size() ; i++)
        {
            // Abort operation.
            if( m_abortOperation ) {
                throw PRException( PRExceptionCode::Abort, methodTitle );
            }

            // Create a stream which corresponds to the current zone.
            streamObj = document->GetObjects().CreateObject();
            streamObj->GetStream();

            PdfReference streamRef( streamObj->Reference().ObjectNumber(),
                                    streamObj->Reference().GenerationNumber() );
            streamsArray.push_back( streamRef );

            // Get input page used in this zone.
            indexIn = m_pageLayouts[idx].zonesIn[i].indexIn;
            pageIn = document->GetPage( indexIn );

            try
            {
                // Copy ressources in the output page.
                //this->copyPageRessources( pageOut, pageIn, i );

                // Compute prefix string.
                std::ostringstream suffixe;
                suffixe << zoneSuffixe << i;
                std::string suffixeStr = suffixe.str();

                // Obtain stream which corresponds to zone.
                PRStreamLayoutZone streamLayout( pageIn,
                                                 streamObj->GetStream(),
                                                 &resourcesOut,
                                                 m_pageLayouts[idx].zonesIn[i],
                                                 m_parameters,
                                                 suffixeStr );
                streamLayout.generateStream();

                // Get form objects.
                std::vector<PoDoFo::PdfObject*> formObjectsPage = streamLayout.getFormObjects();
                formObjects.insert( formObjects.end(), formObjectsPage.begin(), formObjectsPage.end() );
            }
            catch( const PdfError& error )
            {
                qWarning().nospace() << "Pdf Error (" << error.what()
                           << "): can not copy stream corresponding to zone "
                           << (i+1) << " in page " << (idx+1) << ".";

                // Remove stream from array
                streamsArray.erase( streamsArray.end()-1 );
            }
        }
        emit methodProgress( methodTitle, double(idx+1) / double(m_pageLayouts.size()) );
    }
    emit methodProgress( methodTitle, 1.0 );

    // Copy document outlines.
    this->copyOutlines( document, mapPageInZones, origPagesNb );

    // Copy pages annotations.
    this->copyAnnotations( document, mapPageInZones, origPagesNb );

    // Copy page labels.
    this->copyPageLabels( document, vecPageInZones );

    // Delete old pages and attached contents & resources.
    this->deletePagesAndContents( document, 0, origPagesNb );

    // Delete unused forms.
    this->deleteFormObjects( document, formObjects );

    // Clear pages tree cache.
    document->GetPagesTree()->ClearCache();

    // If wanted, add zones overlay.
    if( m_parameters.overlayLayoutZones ) {
        this->printLayoutOut( documentHandle );
    }
}

void PRDocumentLayout::printLayoutIn( PRDocument* documentHandle ) const
{
    // Get PoDoFo document.
    PdfMemDocument* document = documentHandle->podofoDocument();
    QString methodTitle = tr( "Print out layout overlay." );

    // Watermark parameters.
    PdfColor fillColor( 0.0, 0.0, 1.0 );
    PdfColor strokeColor( 0.0, 0.0, 0.8 );
    double fillOpacity = 0.15;
    double strokeWidth = 1.0;
    double strokeOpacity = 1.0;
    double fontSize = 14.0;
    double deltaText = 4.0;

    // Document graphic states and font.
    PdfExtGState rectGstate( document );
    rectGstate.SetFillOpacity( fillOpacity );
    PdfExtGState textGstate( document );
    textGstate.SetFillOpacity( strokeOpacity );

    PdfFont* pFont;
    pFont = document->CreateFont( "Verdana" );
    pFont->SetFontSize( fontSize );

    PdfPage* pPage;
    PdfPainter painter;
    std::ostringstream streamIdx;

    // Print page zones...
    emit methodProgress( methodTitle, 0.0 );
    for(size_t idx = 0 ; idx < m_pageLayouts.size() ; idx++)
    {
        streamIdx.str("");
        streamIdx << (idx+1);

        for(size_t i = 0 ; i < m_pageLayouts[idx].zonesIn.size() ; i++)
        {
            // Abort operation.
            if( m_abortOperation ) {
                throw PRException( PRExceptionCode::Abort, methodTitle );
            }

            const PRPageZone& zone = m_pageLayouts[idx].zonesIn[i];
            pPage = document->GetPage( zone.indexIn );

            // Set painter
            painter.SetPage( pPage );
            painter.Save();

            painter.SetExtGState( &rectGstate );
            painter.SetStrokingColor( strokeColor );
            painter.SetColor( fillColor );
            painter.SetStrokeWidth( strokeWidth );

            // Draw rectangle and text
            painter.DrawRect( zone.zoneIn );
            painter.FillRect( zone.zoneIn );

            painter.SetFont( pFont );
            painter.SetExtGState( &textGstate );
            painter.DrawText( zone.zoneIn.GetLeft()+deltaText,
                              zone.zoneIn.GetBottom()+deltaText,
                              streamIdx.str().c_str() );
            painter.Restore();
            painter.FinishPage();
        }
        emit methodProgress( methodTitle, double(idx+1)/double(m_pageLayouts.size()) );
    }
    emit methodProgress( methodTitle, 1.0 );
}

void PRDocumentLayout::printLayoutOut( PRDocument* documentHandle ) const
{
    // Get PoDoFo document.
    PdfMemDocument* document = documentHandle->podofoDocument();
    QString methodTitle = tr( "Print in layout overlay." );

    // Watermark parameters
    PdfColor fillColor( 0.0, 0.0, 1.0 );
    PdfColor strokeColor( 0.0, 0.0, 0.8 );
    double fillOpacity = 0.15;
    double strokeWidth = 1.0;
    double strokeOpacity = 1.0;
    double fontSize = 14.0;
    double deltaText = 4.0;

    // Document graphic states and font
    PdfExtGState rectGstate( document );
    rectGstate.SetFillOpacity( fillOpacity );
    PdfExtGState textGstate( document );
    textGstate.SetFillOpacity( strokeOpacity );

    PdfFont* pFont = NULL;
    PdfPage* pPage;
    PdfVariant pagebox;
    PdfPainter painter;
    std::ostringstream streamIdx;

    // Print pages of the output document
    emit methodProgress( methodTitle, 0.0 );
    for(size_t idx = 0 ; idx < m_pageLayouts.size() ; idx++)
    {
        // Create page if necessary
        if( document->GetPageCount() <= int(idx) )
            pPage = document->CreatePage( m_pageLayouts[idx].mediaBox );
        else
            pPage = document->GetPage( idx );

        // Reset mediabox and cropbox
        m_pageLayouts[idx].mediaBox.ToVariant( pagebox );
        pPage->GetResources()->GetDictionary().AddKey( "MediaBox", pagebox );
        m_pageLayouts[idx].cropBox.ToVariant( pagebox );
        pPage->GetResources()->GetDictionary().AddKey( "CropBox", pagebox );

        // Set painter
        painter.SetPage( pPage );
        painter.Save();

        painter.SetStrokingColor( strokeColor );
        painter.SetColor( fillColor );
        painter.SetStrokeWidth( strokeWidth );

        pFont = document->CreateFont( "Verdana" );
        pFont->SetFontSize( fontSize );
        painter.SetFont( pFont );

        // Print different zones in the page
        for(size_t i = 0 ; i < m_pageLayouts[idx].zonesIn.size() ; i++)
        {
            // Abort operation.
            if( m_abortOperation ) {
                throw PRException( PRExceptionCode::Abort, methodTitle );
            }

            const PRPageZone& zone = m_pageLayouts[idx].zonesIn[i];
            streamIdx.str("");
            streamIdx << (zone.indexIn + 1);

            // Draw rectangle and text
            painter.SetExtGState( &rectGstate );
            painter.DrawRect( zone.leftZoneOut, zone.bottomZoneOut,
                              zone.zoneIn.GetWidth(), zone.zoneIn.GetHeight() );
            painter.FillRect( zone.leftZoneOut, zone.bottomZoneOut,
                              zone.zoneIn.GetWidth(), zone.zoneIn.GetHeight() );

            painter.SetExtGState( &textGstate );
            painter.DrawText( zone.leftZoneOut+deltaText,
                              zone.bottomZoneOut+deltaText,
                              streamIdx.str().c_str() );
        }
        painter.Restore();
        painter.FinishPage();
        emit methodProgress( methodTitle, double(idx+1)/double(m_pageLayouts.size()) );
    }
    emit methodProgress( methodTitle, 1.0 );
}

void PRDocumentLayout::copyPageRessources( PoDoFo::PdfPage* pageOut,
                                            PoDoFo::PdfPage* pageIn,
                                            int zoneIdx ) const
{
    // Pdf resources from pages.
    PdfDictionary& resDictIn = pageIn->GetResources()->GetDictionary();
    PdfDictionary& resDictOut = pageOut->GetResources()->GetDictionary();

    // Key Prefix.
    std::ostringstream suffixe;
    suffixe << zoneSuffixe << zoneIdx;
//    std::string suffixeStr = suffixe.str();
    std::string suffixeStr = "_ZS0F0";

    // List of resources to copy.
    std::string resName;
    std::vector< std::string > resources;
    resources.push_back( "Font" );
    resources.push_back( "ExtGState" );
    resources.push_back( "ColorSpace" );
    resources.push_back( "Pattern" );
    resources.push_back( "Shading" );
    resources.push_back( "XObject" );
    resources.push_back( "Properties" );

    for(size_t i = 0 ; i < resources.size() ; i++)
    {
        resName = resources[i];

        // Copy if exists in input page.
        if( resDictIn.HasKey( resName ) )
        {
            PdfDictionary& resIn = pageIn->GetResources()->GetIndirectKey( resName )->GetDictionary();

            // Create dictionary in output page, if necessary.
            if( !resDictOut.HasKey( resName ) ) {
                resDictOut.AddKey( resName, PdfDictionary() );
            }
            PdfDictionary& resOut = resDictOut.GetKey( resName )->GetDictionary();

            // Map from the input dictionary.
            TKeyMap& mapIn = resIn.GetKeys();
            TCIKeyMap it;

            for( it = mapIn.begin() ; it != mapIn.end() ; ++it ) {
                resOut.AddKey( (*it).first.GetName() + suffixeStr, (*it).second );
            }
        }
    }
}

void PRDocumentLayout::deletePagesAndContents( PoDoFo::PdfMemDocument* document,
                                               int firstPage, int nbPages ) const
{
    QString methodTitle = tr( "Delete old pages and content." );

    // Local variables.
    PdfPage* page;
    PdfObject* contents;
    PdfObject* objtemp;

    // Vector of objects to be deleted.
    std::vector< PdfObject* > vecObjects;
    std::vector< PdfObject* >::iterator it;

    for( int i = 0 ; i < nbPages ; i++ )
    {
        page = document->GetPage( firstPage );

        // Delete contents.
        contents = page->GetContents();

        if( contents->GetDataType() == ePdfDataType_Dictionary ) {
            // Simply remove object.
            it = std::find( vecObjects.begin(), vecObjects.end(), contents );
            if( it == vecObjects.end() )
                vecObjects.push_back( contents );
        }
        else if( contents->GetDataType() == ePdfDataType_Array ) {
            // Delete every reference in the array.
            PdfArray& arrayC = contents->GetArray();
            for( size_t i = 0 ; i < arrayC.size() ; i++)
            {
                objtemp = document->GetObjects().GetObject( arrayC[i].GetReference() );
                it = std::find( vecObjects.begin(), vecObjects.end(), objtemp );
                if( it == vecObjects.end() )
                    vecObjects.push_back( objtemp );
            }
        }

        // Delete page itself.
        objtemp = page->GetObject();
        document->GetPagesTree()->DeletePage( firstPage );
        document->GetObjects().RemoveObject( objtemp->Reference() );
        delete objtemp;
    }

    // Finally delete objects listed.
    emit methodProgress( methodTitle, 0.0 );
    for( size_t i = 0 ; i < vecObjects.size() ; i++ )
    {
        // Abort operation.
        if( m_abortOperation ) {
            throw PRException( PRExceptionCode::Abort, methodTitle );
        }

        document->GetObjects().RemoveObject( vecObjects[i]->Reference() );
        delete vecObjects[i];

        emit methodProgress( methodTitle, double(i+1)/double(vecObjects.size()) );
    }
    emit methodProgress( methodTitle, 1.0 );
}
void PRDocumentLayout::deleteFormObjects( PoDoFo::PdfMemDocument* document,
                                          const std::vector<PoDoFo::PdfObject*>& formObjects ) const
{
    // Keep unique values.
    std::vector<PdfObject*> vecObjects;
    std::vector<PdfObject*>::const_iterator it, it2;
    for( it = formObjects.begin() ; it != formObjects.end() ; ++it )
    {
        it2 = std::find( it+1, formObjects.end(), *it );
        if( it2 == formObjects.end() ) {
            vecObjects.push_back( *it );
        }
    }

    // Finally delete objects listed.
    for( size_t i = 0 ; i < vecObjects.size() ; i++ )
    {
        document->GetObjects().RemoveObject( vecObjects[i]->Reference() );
        delete vecObjects[i];
    }
}

void PRDocumentLayout::copyOutlines( PoDoFo::PdfMemDocument* document,
                                      std::map<PdfReference, std::vector<const PRPageZone*> > &mapPageInZones,
                                      int origPagesNb ) const
{
    // Recursively modify outlines.
    PdfOutlines* docOutlines = document->GetOutlines( false );
    if( docOutlines )
    {
        PdfOutlineItem* item = docOutlines->First();
        if( item )
            this->copyOutlineItem( document, item, mapPageInZones, origPagesNb );
    }
}

void PRDocumentLayout::copyOutlineItem( PoDoFo::PdfMemDocument* document,
                                    PoDoFo::PdfOutlineItem* item,
                                    std::map<PdfReference, std::vector<const PRPageZone*> > &mapPageInZones,
                                    int origPagesNb ) const
{
    // Abort operation.
    if( m_abortOperation ) {
        throw PRException( PRExceptionCode::Abort, "Abort outlines copy." );
    }

    // Get destination and action of the outline item.
    PdfDestination* dest = item->GetDestination( document );
    PdfAction* action = item->GetAction();

    try
    {
        // Apply modification to destination if possible, or action else if.
        if( dest )
            this->copyDestination( document, dest->GetObject(), mapPageInZones, origPagesNb );
        else if( action )
            this->copyAction( document, action->GetObject(), mapPageInZones, origPagesNb );
    }
    catch( const PdfError& error )
    {
        std::string itemTitle = item->GetTitle().GetStringUtf8();
        qWarning().nospace() << "Pdf Error (" << error.what()
                   << ") during the modification of outline item \""
                   << itemTitle.c_str() << "\".";
    }

    // Apply to first and next.
    if( item->First() )
        this->copyOutlineItem( document, item->First(), mapPageInZones, origPagesNb );
    if( item->Next() )
        this->copyOutlineItem( document, item->Next(), mapPageInZones, origPagesNb );
}

void PRDocumentLayout::copyAction( PoDoFo::PdfMemDocument* document,
                                    PoDoFo::PdfObject* action,
                                    std::map<PdfReference, std::vector<const PRPageZone*> > &mapPageInZones,
                                    int origPagesNb ) const
{
    // Get action type
    PdfObject* actionS = action->GetIndirectKey( "S" );
    if( !actionS ) {
        PODOFO_RAISE_ERROR( ePdfError_NoObject );
    }
    std::string actionType = actionS->GetName().GetName();

    // GoTo action: change destination.
    if( !actionType.compare( "GoTo" ) )
    {
        PdfObject* dest = action->GetIndirectKey( "D" );
        this->copyDestination( document, dest, mapPageInZones, origPagesNb );
    }

    // Next action.
    PdfObject* nextAction = action->GetIndirectKey( "Next" );
    if( nextAction )
    {
        // Type: dictionary (single action) or array of actions.
        if( nextAction->GetDataType() == ePdfDataType_Dictionary ) {
            this->copyAction( document, nextAction, mapPageInZones, origPagesNb );
        }
        else if( nextAction->GetDataType() == ePdfDataType_Array ) {
            PdfArray& actionArray = nextAction->GetArray();
            for(size_t i = 0 ; i < actionArray.size() ; i++)
                this->copyAction( document, &actionArray[i], mapPageInZones, origPagesNb );
        }
    }
}

void PRDocumentLayout::copyDestination( PoDoFo::PdfMemDocument* document,
                                    PoDoFo::PdfObject* destination,
                                    std::map<PdfReference, std::vector<const PRPageZone*> > &mapPageInZones,
                                    int origPagesNb ) const
{
    // Get destination array.
    PdfArray* destArray = NULL;
    if ( destination->GetDataType() == ePdfDataType_Array )
    {
        destArray = &destination->GetArray();
    }
    else if( destination->GetDataType() == ePdfDataType_String )
    {
        PdfNamesTree* pNames = document->GetNamesTree( ePdfDontCreateObject );
        if( !pNames )
            PODOFO_RAISE_ERROR( ePdfError_NoObject );

        std::string destName = destination->GetString().GetStringUtf8();
        PdfObject* pValue = pNames->GetValue( "Dests", destName );
        if( !pValue )
            PODOFO_RAISE_ERROR( ePdfError_InvalidName );

        if( pValue->IsArray() )
            destArray = &pValue->GetArray();
        else if( pValue->IsDictionary() )
            destArray = &pValue->GetDictionary().GetKey( "D" )->GetArray();

        if( !destArray )
            PODOFO_RAISE_ERROR( ePdfError_NoObject );
    }

    // Destination parameters.
    PdfReference destPage = (*destArray)[0].GetReference();
    std::string destType = (*destArray)[1].GetName().GetName();

    // Tmp variables.
    std::vector<const PRPageZone*>& pageInZones = mapPageInZones[ destPage ];
    int idxZone = 0;
    double deltaTop = 10;

    // No zones corresponding to page: can happen if destination already updated
    // to a new page.
    if( !pageInZones.size() )
        return;

    // Modifications depend on the destination type.
    if( !destType.compare( "XYZ" ) || !destType.compare( "FitH" ) ||
        !destType.compare( "FitBH" ) || !destType.compare( "FitR" ) )
    {
        // Index of the top value, depends on destination type
        int indexTop = 2;
        if( !destType.compare( "XYZ" ) )
            indexTop = 3;
        else if( !destType.compare( "FitR" ) )
            indexTop = 5;

        // Change top value if not null and choose zone in the page (0 by default)
        if( !(*destArray)[indexTop].IsNull() )
        {
            double top = (*destArray)[indexTop].GetReal();

            // Search best index (0 by default)
            for( size_t i = 0 ; i < pageInZones.size() ; i++ )
            {
                if( top - deltaTop >= pageInZones[i]->zoneIn.GetBottom() &&
                    top - deltaTop <= (pageInZones[i]->zoneIn.GetBottom()+
                                       pageInZones[i]->zoneIn.GetHeight()) )
                    idxZone = i;
            }

            // Set top coordinate
            (*destArray)[indexTop] = top - pageInZones[idxZone]->zoneIn.GetBottom()
                                         + pageInZones[idxZone]->bottomZoneOut;
        }

        // Set output page
        PdfPage* pageOut = document->GetPage( pageInZones[idxZone]->parent->indexOut+origPagesNb );
        (*destArray)[0] = pageOut->GetObject()->Reference();

        // In the case of FitR, also update other coordinates
        if( !destType.compare( "FitR" ) )
        {
            // Left
            if( !(*destArray)[2].IsNull() )
                (*destArray)[2] = (*destArray)[2].GetReal()
                                - pageInZones[idxZone]->zoneIn.GetLeft()
                                + pageInZones[idxZone]->leftZoneOut;
            // Bottom
            if( !(*destArray)[3].IsNull() )
                (*destArray)[3] = (*destArray)[3].GetReal()
                                - pageInZones[idxZone]->zoneIn.GetBottom()
                                + pageInZones[idxZone]->bottomZoneOut;
            // Right
            if( !(*destArray)[4].IsNull() )
                (*destArray)[4] = (*destArray)[4].GetReal()
                                - pageInZones[idxZone]->zoneIn.GetLeft()
                                + pageInZones[idxZone]->leftZoneOut;
        }

        // In the case of XYZ, also update left coordinate
        if( !destType.compare( "XYZ" ) )
        {
            // Left
            if( !(*destArray)[2].IsNull() )
                (*destArray)[2] = (*destArray)[2].GetReal()
                                - pageInZones[idxZone]->zoneIn.GetLeft()
                                + pageInZones[idxZone]->leftZoneOut;
        }
    }
    else if( !destType.compare( "Fit" ) || !destType.compare( "FitV" ) ||
             !destType.compare( "FitB" ) || !destType.compare( "FitBV" ) )
    {
        // Can not guess much -> set destination to first page zone
        idxZone = 0;
        PdfPage* pageOut = document->GetPage( pageInZones[idxZone]->parent->indexOut+origPagesNb );
        (*destArray)[idxZone] = pageOut->GetObject()->Reference();

        // In the case of FitV or FitBV, update left coordinate
        if( !destType.compare( "FitV" ) || !destType.compare( "FitBV" ) )
        {
            // Left
            if( !(*destArray)[2].IsNull() )
                (*destArray)[2] = (*destArray)[2].GetReal()
                                - pageInZones[idxZone]->zoneIn.GetLeft()
                                + pageInZones[idxZone]->leftZoneOut;
        }
    }
}

void PRDocumentLayout::copyAnnotations( PdfMemDocument* document,
                                    std::map<PdfReference, std::vector<const PRPageZone*> > &mapPageInZones,
                                    int origPagesNb ) const
{
    QString methodTitle = tr( "Copy page annotations." );

    // Copy annotations in every page
    emit methodProgress( methodTitle, 0.0 );
    for( int i = 0 ; i < origPagesNb ; i++ )
    {
        this->copyPageAnnotations( document, i, mapPageInZones, origPagesNb );
        emit methodProgress( methodTitle, double(i+1)/double(origPagesNb) );
    }
    emit methodProgress( methodTitle, 1.0 );
}

void PRDocumentLayout::copyPageAnnotations( PdfMemDocument* document,
                                    int idxPageIn,
                                    std::map<PdfReference, std::vector<const PRPageZone*> > &mapPageInZones,
                                    int origPagesNb ) const
{
    // Page pointer and page zones
    PdfPage* pageIn = document->GetPage( idxPageIn );
    std::vector<const PRPageZone*>& pageInZones = mapPageInZones[ pageIn->GetObject()->Reference() ];
    std::vector<PdfeVector> vecAnnPoints;
    int idxZone;

    for( int i = 0 ; i < pageIn->GetNumAnnots() ; i++ )
    {
        // Abort operation.
        if( m_abortOperation ) {
            throw PRException( PRExceptionCode::Abort, "Abort annotations copy." );
        }

        PdfAnnotation* pageAnn = pageIn->GetAnnotation( i );

        // Points corresponding to annotation rectangle
        PdfRect annRect = pageAnn->GetRect();
        vecAnnPoints.push_back( PdfeVector( annRect.GetLeft(),
                                           annRect.GetBottom() ) );
        vecAnnPoints.push_back( PdfeVector( annRect.GetLeft()+annRect.GetWidth(),
                                           annRect.GetBottom() ) );
        vecAnnPoints.push_back( PdfeVector( annRect.GetLeft(),
                                           annRect.GetBottom()+annRect.GetHeight() ) );
        vecAnnPoints.push_back( PdfeVector( annRect.GetLeft()+annRect.GetWidth(),
                                           annRect.GetBottom()+annRect.GetHeight() ) );

        // Find zone where the annotation rect is
        bool insideZone = false;
        for( idxZone = 0 ; idxZone < int(pageInZones.size()) ; idxZone++ )
        {
            insideZone = this->intersectZone( vecAnnPoints, pageInZones[idxZone]->zoneIn, true );
            if( insideZone )
                break;
        }

        // If zone found, modify annotation and add to page out
        if( insideZone )
        {
            annRect.SetLeft( annRect.GetLeft()
                             - pageInZones[idxZone]->zoneIn.GetLeft()
                             + pageInZones[idxZone]->leftZoneOut );
            annRect.SetBottom( annRect.GetBottom()
                               - pageInZones[idxZone]->zoneIn.GetBottom()
                               + pageInZones[idxZone]->bottomZoneOut );
            PdfVariant annRectV;
            annRect.ToVariant( annRectV );

            pageAnn->GetObject()->GetDictionary().AddKey( PdfName::KeyRect,
                                                          annRectV );

            // Add annotation to page out
            PdfPage* pageOut = document->GetPage( pageInZones[idxZone]->parent->indexOut
                                                  + origPagesNb );

            if ( !pageOut->GetObject()->GetDictionary().HasKey( "Annots" ) ) {
                // Create the annotations array
                pageOut->GetObject()->GetDictionary().AddKey( "Annots", PdfArray() );
            }
            PdfObject* pageAnnots = pageOut->GetObject()->GetIndirectKey( "Annots" );
            pageAnnots->GetArray().push_back( pageAnn->GetObject()->Reference() );

            try
            {
                // Modify annotation destination and action
                PdfObject* action = pageAnn->GetObject()->GetIndirectKey( "A" );
                if( action )
                    this->copyAction( document, action, mapPageInZones, origPagesNb );
                PdfObject* dest = pageAnn->GetObject()->GetIndirectKey( "Dest" );
                if( dest )
                    this->copyDestination( document, dest, mapPageInZones, origPagesNb );
            }
            catch( const PdfError& error )
            {
                qWarning().nospace() << "Pdf Error (" << error.what()
                                     << ") during the copy of annotation " << (i+1)
                                     << " from page " << (idxPageIn+1) << ".";

                // Remove reference.
                pageAnnots->GetArray().erase( pageAnnots->GetArray().end()-1 );
            }
        }
        vecAnnPoints.clear();
    }
}

void PRDocumentLayout::copyPageLabels( PdfMemDocument* document,
                                        std::vector< std::vector<const PRPageZone*> >& vecPageInZones ) const
{
    // Get PageLabels in document catalog
    PdfObject* pageLabels = document->GetCatalog()->GetIndirectKey( "PageLabels" );

    try
    {
        // Modify recursively the page labels tree
        if( pageLabels ) {
            this->copyPageLabelsNode( document, pageLabels, vecPageInZones );
        }
    }
    catch( const PdfError& error )
    {
        qWarning().nospace() << "Pdf Error (" << error.what()
                           << ") during the modification of page labels.";
    }
}

std::vector<int> PRDocumentLayout::copyPageLabelsNode( PoDoFo::PdfMemDocument* document,
                                            PdfObject* node,
                                            std::vector< std::vector<const PRPageZone*> >& vecPageInZones ) const
{
    // Abort operation.
    if( m_abortOperation ) {
        throw PRException( PRExceptionCode::Abort, "Abort labels copy." );
    }

    PdfObject* nodeKids = node->GetIndirectKey( "Kids" );
    PdfObject* nodeNums = node->GetIndirectKey( "Nums" );
    PdfObject* nodeLimits = node->GetIndirectKey( "Limits" );

    int minKey = INT_MAX;
    int maxKey = INT_MIN;
    std::vector<int> tmpVec;

    // Root or intermediate nodes
    if( nodeKids )
    {
        // Modify children nodes
        for( size_t i = 0 ; i < nodeKids->GetArray().size() ; i++ )
        {
            tmpVec = this->copyPageLabelsNode( document, &(nodeKids->GetArray()[i]), vecPageInZones );
            minKey = std::min( tmpVec[0], minKey );
            maxKey = std::max( tmpVec[1], maxKey );
        }
    }
    // Leaf node
    if( nodeNums )
    {
        // Modify node tree values
        for( size_t i = 0 ; i < nodeNums->GetArray().size() ; i++ )
        {
            // Get page (=key) and first zone in this page
            int indexIn = nodeNums->GetArray()[i].GetNumber();
            int indexOut = INT_MAX;
            size_t indexZone;

            // Find the first page with zones inside
            while( !vecPageInZones[ indexIn ].size() &&
                   size_t(indexIn) < vecPageInZones.size() )
                indexIn++;

            for( indexZone = 0 ; indexZone < vecPageInZones[indexIn].size() ; indexZone++ ) {
                indexOut = std::min( indexOut, vecPageInZones[indexIn][indexZone]->parent->indexOut );
            }

            minKey = std::min( indexOut, minKey );
            maxKey = std::max( indexOut, maxKey );

            // Set key
            nodeNums->GetArray()[i] = pdf_int64( indexOut );
            i++;

            // Key value (i+1)
        }
    }

    // Intermediate or leaf nodes
    if( nodeLimits )
    {
        // Set min and max in limits array
        nodeLimits->GetArray()[0] = pdf_int64( minKey );
        nodeLimits->GetArray()[1] = pdf_int64( maxKey );
    }
    tmpVec.clear();
    tmpVec.push_back( minKey );
    tmpVec.push_back( maxKey );

    return tmpVec;
}

bool PRDocumentLayout::intersectZone( const std::vector<PdfeVector>& points,
                                       const PoDoFo::PdfRect& zone,
                                       bool strictInclusion )
{
    bool leftTop(false), left(false), leftBottom(false);
    bool centerTop(false), center(false), centerBottom(false);
    bool rightTop(false), right(false), rightBottom(false);
    bool strictCenter(true);

    for( size_t i = 0 ; i < points.size() ; i++ )
    {
        // Only check center with strict inclusion
        if( strictInclusion )
        {
            strictCenter = strictCenter &&
                           points[i](0) >= zone.GetLeft() &&
                           points[i](0) <= zone.GetLeft()+zone.GetWidth() &&
                           points[i](1) >= zone.GetBottom() &&
                           points[i](1) <= zone.GetBottom()+zone.GetHeight();
        }
        else
        {
            if( points[i](0) <= zone.GetLeft() &&
                    points[i](1) <= zone.GetBottom() )
                leftBottom = true;
            else if( points[i](0) <= zone.GetLeft() &&
                     points[i](1) <= zone.GetBottom()+zone.GetHeight() &&
                     points[i](1) >= zone.GetBottom() )
                left = true;
            else if( points[i](0) <= zone.GetLeft() &&
                     points[i](1) >= zone.GetBottom()+zone.GetHeight() )
                leftTop = true;
            else if( points[i](0) >= zone.GetLeft() &&
                     points[i](0) <= zone.GetLeft()+zone.GetWidth() &&
                     points[i](1) <= zone.GetBottom() )
                centerBottom = true;
            else if( points[i](0) >= zone.GetLeft() &&
                     points[i](0) <= zone.GetLeft()+zone.GetWidth() &&
                     points[i](1) >= zone.GetBottom()+zone.GetHeight() )
                centerTop = true;
            else if( points[i](0) >= zone.GetLeft()+zone.GetWidth() &&
                     points[i](1) <= zone.GetBottom() )
                rightBottom = true;
            else if( points[i](0) >= zone.GetLeft()+zone.GetWidth() &&
                     points[i](1) <= zone.GetBottom()+zone.GetHeight() &&
                     points[i](1) >= zone.GetBottom() )
                right = true;
            else if( points[i](0) >= zone.GetLeft()+zone.GetWidth() &&
                     points[i](1) >= zone.GetBottom()+zone.GetHeight() )
                rightTop = true;
            else
                center = true;
        }
    }
    if(strictInclusion)
        return strictCenter;

    // Try to estimate if the path intersects the zone.
    bool intersect = center ||
            ( centerTop && ( leftBottom || centerBottom || rightBottom ) ) ||
            ( centerBottom && ( leftTop || centerTop || rightTop ) ) ||
            ( left && ( rightBottom || right || rightTop ) ) ||
            ( right && ( leftBottom || left || leftTop ) ) ||
            ( leftBottom && rightTop ) ||
            ( leftTop && rightBottom );

    return intersect;
}
bool PRDocumentLayout::intersectZone( const PoDoFo::PdfRect& path,
                                       const PoDoFo::PdfRect& zone,
                                       bool strictInclusion )
{
    if( strictInclusion )
    {
        strictInclusion = ( path.GetLeft() >= zone.GetLeft() ) &&
                ( path.GetBottom() >= zone.GetBottom() ) &&
                ( path.GetLeft()+path.GetWidth() <= zone.GetLeft()+zone.GetWidth() ) &&
                ( path.GetBottom()+path.GetHeight() <= zone.GetBottom()+zone.GetHeight() );
        return strictInclusion;
    }
    else
    {
        bool left(false), right(false), bottom(false), top(false), center(false);

        // Evaluate the intersection with the different part of the zone.
        center = ( path.GetLeft() >= zone.GetLeft() ) &&
                ( path.GetBottom() >= zone.GetBottom() ) &&
                ( path.GetLeft()+path.GetWidth() <= zone.GetLeft()+zone.GetWidth() ) &&
                ( path.GetBottom()+path.GetHeight() <= zone.GetBottom()+zone.GetHeight() );

        left = !( path.GetBottom()+path.GetHeight() <= zone.GetBottom() ||
                  path.GetBottom() >= zone.GetBottom()+zone.GetHeight() ) &&
                path.GetLeft() <= zone.GetLeft();

        right = !( path.GetBottom()+path.GetHeight() <= zone.GetBottom() ||
                   path.GetBottom() >= zone.GetBottom()+zone.GetHeight() ) &&
                path.GetLeft()+path.GetWidth() >= zone.GetLeft()+zone.GetWidth();

        bottom = !( path.GetLeft()+path.GetWidth() <= zone.GetLeft() ||
                  path.GetLeft() >= zone.GetWidth()+zone.GetLeft() ) &&
                path.GetBottom() <= zone.GetBottom();

        top = !( path.GetLeft()+path.GetWidth() <= zone.GetLeft() ||
                 path.GetLeft() >= zone.GetWidth()+zone.GetLeft() ) &&
                path.GetBottom()+path.GetHeight() <= zone.GetBottom()+zone.GetHeight();

        bool intersect = center ||
                ( left && top ) ||
                ( left && bottom ) ||
                ( right && top ) ||
                ( right && bottom ) ||
                ( top && bottom ) ||
                ( left && right );
        return intersect;
    }
}

void PRDocumentLayout::reduceToZone( PdfeVector& point,
                                      const PoDoFo::PdfRect& zone )
{
    point(0) = std::min( std::max( point(0), zone.GetLeft() ),
                             zone.GetLeft() + zone.GetWidth() );
    point(1) = std::min( std::max( point(1), zone.GetBottom() ),
                             zone.GetBottom() + zone.GetHeight() );
}
void PRDocumentLayout::reduceToZone( PoDoFo::PdfRect& path,
                                      const PoDoFo::PdfRect& zone )
{
    path.SetLeft( std::min( std::max( path.GetLeft(), zone.GetLeft() ),
                            zone.GetLeft() + zone.GetWidth() ) );

    path.SetBottom( std::min( std::max( path.GetBottom(), zone.GetBottom() ),
                              zone.GetBottom() + zone.GetHeight() ) );

    path.SetWidth( std::min( path.GetWidth(), zone.GetWidth()-
                                              path.GetLeft()+zone.GetLeft() ) );

    path.SetHeight( std::min( path.GetHeight(), zone.GetHeight()-
                                                path.GetBottom()+zone.GetBottom() ) );
}

}
