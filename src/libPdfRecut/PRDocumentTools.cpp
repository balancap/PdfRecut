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

#include "PRDocumentTools.h"

#include <podofo/podofo.h>

using namespace PoDoFo;

namespace PdfRecut {

void PRDocumentTools::addGraphicStateStack( PRDocument* documentHandle )
{
    // Get PoDoFo document and mutex on it.
    PdfMemDocument* document = documentHandle->getPoDoFoDocument();
    QMutexLocker locker( documentHandle->getPoDoFoMutex() );

    // Create two PdfObjects which correspond to streams with graphic operators q and Q.
    PdfObject* qPdfObj = document->GetObjects().CreateObject();
    PdfObject* QPdfObj = document->GetObjects().CreateObject();

    qPdfObj->GetStream()->BeginAppend();
    qPdfObj->GetStream()->Append( "q" );
    qPdfObj->GetStream()->EndAppend();

    QPdfObj->GetStream()->BeginAppend();
    QPdfObj->GetStream()->Append( "Q" );
    QPdfObj->GetStream()->EndAppend();

    // References to these objects
    PdfReference qRef( qPdfObj->Reference().ObjectNumber(), qPdfObj->Reference().GenerationNumber() );
    PdfReference QRef( QPdfObj->Reference().ObjectNumber(), QPdfObj->Reference().GenerationNumber() );

    PdfPage* page;
    PdfObject* contents;

    // Modify stream content in every page
    for(int idx = 0 ; idx < document->GetPageCount() ; idx++)
    {
        page = document->GetPage( idx );
        contents = page->GetContents();

        // Dictionary: replace the reference by an array
        if( contents->GetDataType() == ePdfDataType_Dictionary )
        {
            PdfReference streamRef( contents->Reference().ObjectNumber(),
                                    contents->Reference().GenerationNumber() );

            PdfArray arrayContents;
            arrayContents.push_back( qRef );
            arrayContents.push_back( streamRef );
            arrayContents.push_back( QRef );

            page->GetObject()->GetDictionary().AddKey( "Contents", arrayContents );
        }
        else if( contents->GetDataType() == ePdfDataType_Array ) // Already an array
        {
            PdfArray& arrayCont = contents->GetArray();
            arrayCont.insert( arrayCont.begin(), qRef );
            arrayCont.insert( arrayCont.end(), QRef );
        }
    }
    // Clear page tree cache
    document->GetPagesTree()->ClearCache();
}

void PRDocumentTools::uncompressStreams( PRDocument* documentHandle )
{
    // Get PoDoFo document and mutex on it.
    PdfMemDocument* document = documentHandle->getPoDoFoDocument();
    QMutexLocker locker( documentHandle->getPoDoFoMutex() );

    // Iterator on document objects
    TIVecObjects it = document->GetObjects().begin();

    while( it != document->GetObjects().end() )
    {
        if( (*it)->HasStream() )
        {
            PdfMemStream* pStream = dynamic_cast<PdfMemStream*>( (*it)->GetStream() );
            pStream->Uncompress();
        }
        ++it;
    }
}

}
