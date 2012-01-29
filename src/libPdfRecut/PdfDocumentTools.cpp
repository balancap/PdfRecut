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

#include "PdfDocumentTools.h"

#include <podofo/podofo.h>

using namespace PoDoFo;

namespace PdfeBooker {

void PdfDocumentTools::addGraphicStateStack( PdfDocumentHandle* documentHandle )
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

void PdfDocumentTools::uncompressStreams( PdfDocumentHandle* documentHandle )
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
