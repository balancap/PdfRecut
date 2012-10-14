/***************************************************************************
 *   Copyright (C) 2011 by Paul Balança                                    *
 *   paul.balanca@gmail.com                                                    *
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

#include <QsLog/QsLog.h>
#include <QsLog/QsLogDest.h>

#include "PRDocument.h"
#include "PRDocumentLayout.h"
#include "PRDocumentTools.h"
#include "PRRenderPage.h"
#include "PRTextPageStructure.h"

#include "PdfeFontType1.h"

#include <podofo/podofo.h>

#include <QtCore>
#include <QApplication>

#include <iostream>
#include <string>

using namespace PdfRecut;
using namespace PoDoFo;
using namespace std;

void readTokens( PdfDocument& doc, int pageIdx )
{
    PdfPage* page = doc.GetPage( pageIdx );
    PdfContentsTokenizer tokenizer( page );

    const char*  pszToken;
    EPdfTokenType 	peType;
    int i=0;

    while( tokenizer.GetNextToken(pszToken, &peType) )
    {
        cout << "Token " << i << " : " << pszToken << " : " << peType << endl;
        i++;
    }
}
void pageExtract( PdfDocument& doc, int page )
{
    for(int i = 0 ; i < page-1 ; i++)
        doc.GetPagesTree()->DeletePage(0);

    for(int i=0 ; doc.GetPageCount() > 1 ; i++)
        doc.GetPagesTree()->DeletePage(1);
}

void splitPagesLayout( PdfDocument& doc, PRDocumentLayout& docLayout )
{
    PdfPage* page;
    PdfRect pageBox, mediaBox, bleedBox, artBox;
    PRPageZone pageZone;
    double delta = 20;

    for( int i = 0 ; i < doc.GetPageCount() ; i++ )
    {
        page = doc.GetPage( i );

        pageBox = page->GetCropBox();
        mediaBox = page->GetMediaBox();
        bleedBox = page->GetBleedBox();
        artBox = page->GetArtBox();

        /*if(i < 20)
        {
            cout << i << " M: " << mediaBox.GetLeft() << " / " << mediaBox.GetBottom() << " / " << mediaBox.GetWidth() << " / " << mediaBox.GetHeight() << endl;
            cout << i << " P: " << pageBox.GetLeft() << " / " << pageBox.GetBottom() << " / " << pageBox.GetWidth() << " / " << pageBox.GetHeight() << endl;
            cout << i << " B: " << bleedBox.GetLeft() << " / " << bleedBox.GetBottom() << " / " << bleedBox.GetWidth() << " / " << bleedBox.GetHeight() << endl;
            cout << i << " A: " << artBox.GetLeft() << " / " << artBox.GetBottom() << " / " << artBox.GetWidth() << " / " << artBox.GetHeight() << endl;
        }*/

        if( pageBox.GetWidth() > mediaBox.GetWidth() || pageBox.GetHeight() > mediaBox.GetHeight() ) {
            pageBox = mediaBox;
        }

        // First zone in the page
        pageZone.indexIn = i;
        pageZone.leftZoneOut = pageBox.GetLeft() + delta;
        pageZone.bottomZoneOut = pageBox.GetBottom();
        pageZone.bottomZoneOut = pageBox.GetBottom() + pageBox.GetHeight()/4;
        pageZone.zoneIn = PdfRect(pageBox.GetLeft() + delta,
                                  pageBox.GetBottom() + pageBox.GetHeight()/2,
                                  pageBox.GetWidth() - 2*delta,
                                  pageBox.GetHeight()/2 - delta );

        docLayout.setPageBoxes(2*i, mediaBox, pageBox);
        docLayout.addPageZone(2*i, pageZone);

        // Second zone in the page
        pageZone.indexIn = i;
        pageZone.leftZoneOut = pageBox.GetLeft() + delta;
        pageZone.bottomZoneOut = pageBox.GetBottom() + pageBox.GetHeight()/4;
        //pageZone.bottomZoneOut = pageBox.GetBottom() + delta;
        pageZone.zoneIn = PdfRect(pageBox.GetLeft() + delta,
                                  pageBox.GetBottom() + delta,
                                  pageBox.GetWidth() - 2*delta,
                                  pageBox.GetHeight()/2 - delta );

        docLayout.setPageBoxes(2*i+1, mediaBox, pageBox);
        docLayout.addPageZone(2*i+1, pageZone);
    }
}

void proceedFile( QString filePath )
{
    // Document objects.
    PRDocument document( 0, filePath );
    PRDocumentLayout docLayout;

    QTime timeTask;
    timeTask.start();

    // File info
    QFileInfo infoFile( filePath );
    cout << "Proceeding file: " << infoFile.fileName().toLocal8Bit().data() << endl;

    // Open file
    cout << " >>> Opening Pdf file..." << endl;
    document.loadPoDoFoDocument();
    //document.loadPopplerDocument();

    // Generate a PdfDocumentLayout
//    cout << " >>> Generating a Pdf Document Layout..." << endl;
//    splitPagesLayout( *document.getPoDoFoDocument(), docLayout );

//    // Layout parameters.
//    PRLayoutParameters params;
//    params.zoneClippingPath = false;
//    params.pathReduce = true;
//    params.pathStrictlyInside = false;
//    params.overlayLayoutZones = true;
//    docLayout.setLayoutParameters( params );

//    filePath = infoFile.canonicalPath() + "/xpdfOut_"
//             + infoFile.fileName();

//    // Transform document and export to Pdf file.
//    cout << " >>> Transform Pdf Document..." << endl;
//    docLayout.writeLayoutToPdf( &document, filePath );

    //PdfDocumentTools::uncompressStreams( &document );
    //docLayout.printLayoutOut( &document );
    //docLayout.printLayoutIn( &document );
    //document.writePoDoFoDocument( filePath );

    // Render page and save.
    PRRenderParameters renderParams;
    renderParams.resolution = 3.0;
//    renderParams.clippingPath.addRect( 50, 50, 300, 400 );

    QString filename;
    for( int i = 0 ; i < std::min(50,document.getPoDoFoDocument()->GetPageCount()) ; i++ ) {
        filename = QString("./img/page%1.png").arg( i, 3, 10, QLatin1Char('0') );

//        PRRenderPage renderPage( &document, i );
//        renderPage.renderPage( renderParams );
//        renderPage.saveToFile( filename );

        PRTextPageStructure textPage( &document, i );
        textPage.detectGroupsWords();
//        textPage.detectLines();

        textPage.renderTextGroupsWords();
//        textPage.renderTextLines();
        textPage.saveToFile( filename );
    }
    cout << " >>> Time elapsed: " << timeTask.elapsed() << " ms." << endl << endl;
}

void proceedDir( QString dirPath )
{
    QDir dirFiles (dirPath );
    dirFiles.setFilter( QDir::Files | QDir::NoSymLinks );
    dirFiles.setNameFilters( QStringList( "*.pdf" ) );

    QFileInfoList listFiles = dirFiles.entryInfoList();
    for( int i = 0 ; i < listFiles.size() ; i++ )
    {
        QFileInfo fileInfo = listFiles.at(i);
        if( !fileInfo.baseName().startsWith( "xpdf" ) ) {
            proceedFile( fileInfo.filePath() );
        }
    }
}

int main( int argc, char *argv[] )
{
    QApplication a(argc, argv);

    // Disable PoDoFo log and debug messages.
    PdfError::EnableLogging( false );
    PdfError::EnableDebug( false );

    // Init the logging mechanism
    // TODO: allow finer level selections, depending on the destination.
    // TODO: add more format options to QsLog.
    QsLogging::Logger& logger = QsLogging::Logger::instance();
    logger.setLoggingLevel( QsLogging::TraceLevel );

    const QString sLogPath( "./log.txt" );
    QsLogging::DestinationPtr fileDestination( QsLogging::DestinationFactory::MakeFileDestination(sLogPath) );
    QsLogging::DestinationPtr debugDestination( QsLogging::DestinationFactory::MakeDebugOutputDestination() );

    logger.addDestination( debugDestination.get() );
    logger.addDestination( fileDestination.get() );

    // Set Standard Type 1 fonts path.
    PoDoFoExtended::PdfeFontType1::Standard14FontsPath.setPath( "./standard14fonts" );

    if( argc != 2 ) {
        cout << "Input: file or directory to proceed..." << endl;
        return 0;
    }

    QString pathIn = QCoreApplication::arguments().at( 1 );
    if( QFileInfo( pathIn ).isDir() ) {
        proceedDir( pathIn );
    }
    else {
        proceedFile( pathIn );
    }

    /*
    // Graphic state stack
    PdfDocumentTools::addGraphicStateStack( &doc );

    // Uncompress document
    PdfDocumentTools::uncompressStreams( &doc );*/

    //return a.exec();
    return 0;
}
