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

#include <QsLog/QsLog.h>
#include <QsLog/QsLogDest.h>

#include "PRDocument.h"
#include "PRPage.h"
#include "PRDocumentLayout.h"
#include "PRDocumentTools.h"
#include "PRRenderPage.h"

#include "PRGeometry/PRGDocument.h"
#include "PRGeometry/PRGSubDocument.h"
#include "PRGeometry/PRGPage.h"
#include "PRGeometry/PRGTextPage.h"

#include "PdfeFontType1.h"

#include <podofo/podofo.h>

#include <QtCore>
#include <QApplication>

#include <iostream>
#include <string>

using namespace PdfRecut;
using namespace PoDoFo;
using namespace PoDoFoExtended;
using namespace std;

void computeBBoxStats( const PRDocument& document, PdfeFont14Standard::Enum stdFontType )
{
    // Create standard font object.
    PdfeFontType1 font( stdFontType, document.ftLibrary() );
    PdfeFont::Statistics stats = font.statistics();

    std::cout << "Stats (" << font.standard14FontName( stdFontType ) << ") : "
              << stats.meanAdvance << " / "
              << stats.meanBBox.GetLeft() << " : "
              << stats.meanBBox.GetBottom() << " : "
              << stats.meanBBox.GetWidth() << " : "
              << stats.meanBBox.GetHeight() << " : "
              << std::endl;

//    QLOG_INFO() << QString( "Stats( %1 ) :" ).arg( font.standard14FontName( stdFontType ).c_str(), 20 )
//                   .toAscii().constData();

}

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
void proceedFile( QString fileName )
{
    // Document objects.
    PRDocument document( 0 );
    PRDocumentLayout docLayout;

    QTime timeTask;
    timeTask.start();

    // File info
    QFileInfo infoFile( fileName );
    cout << "Proceeding file: " << infoFile.fileName().toLocal8Bit().data() << endl;

    // Load PDF file
    document.load( fileName );
    document.setPagesCacheSize( 100 );


    PdfeContentsStream contents;
    for( size_t i = 0 ; i < document.nbPages()-1 ; ++i ) {
        PRPage* page = document.page( i );

        contents = page->contents();
        page->setContents( contents );

        PRPage* page2 = document.insertPage( i+1 );
        page2->operator=( *page );

        document.deletePage( i+2 );

        // Save stream to file.
        contents = page->contents();
        QString filename = QString("./stream/stream%1.txt").arg( i, 3, 10, QLatin1Char('0') );
        contents.exportToFile( filename );

        ++i;

    }
//    PRPage* page = document.page( 0 );
//    page->loadContents();
//    contents = page->contents();
//    page->setContents( contents );

    // Save into...
    QFileInfo fileInfo( fileName );
    fileName = fileInfo.absolutePath()
            + "/recut/"
            + infoFile.fileName();

    document.save( fileName );
    document.clear();

    /*
    // Analyse document geometry.
    PRGDocument::GParameters gparams;
    gparams.firstPageIndex = 0;
    gparams.lastPageIndex = 50;

    PRGDocument* pGDocument = new PRGDocument( &document, 0.05 );
    pGDocument->loadPagesContents();

    pGDocument->analyse( gparams );

    // Render page and save.
    PRRenderPage::Parameters renderParams;
//    renderParams.textPB.init();
    renderParams.textSpacePB.init();
    renderParams.textPDFTranslationPB.init();
//    renderParams.pathPB.init();
//    renderParams.formPB.init();

    renderParams.resolution = 1.0;
//    renderParams.clippingPath.addRect( 50, 50, 300, 400 );

    QString filename;
    for( size_t i = 0 ; i < std::min( gparams.lastPageIndex+1, pGDocument->nbPages() ) ; i++ ) {
        // Text and render page objects.
        PRRenderPage renderPage( &document, i, NULL );
        PRGTextPage* textPage = pGDocument->page( i )->text();

        // Render some elements.
        renderPage.initRendering( renderParams.resolution );
        renderPage.renderElements( renderParams );
//        textPage->renderGroupsWords( renderPage, false );
//        textPage->renderLines( renderPage, false, false );

        // Save image to file.
        filename = QString("./img/page%1.png").arg( i, 3, 10, QLatin1Char('0') );
        renderPage.image().save( filename );

        // Save stream to file.
        filename = QString("./stream/stream%1.txt").arg( i, 3, 10, QLatin1Char('0') );
        pGDocument->page( i )->contents().exportToFile( filename );
    }
    */

    cout << " >>> Time elapsed: " << timeTask.elapsed() << " ms." << endl << endl;

//    std::cout << "Press RETURN to finish..." << std::endl;
//    std::cin.get();

//    delete pGDocument;

    // Compute some mean values bbox.
//    computeBBoxStats( document, PdfeFont14Standard::Helvetica );
//    computeBBoxStats( document, PdfeFont14Standard::TimesRoman );
//    computeBBoxStats( document, PdfeFont14Standard::Courier );

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

    logger.addDestination( fileDestination.get() );
    logger.addDestination( debugDestination.get() );

    // Set Standard 14 fonts path.
    PoDoFoExtended::PdfeFont::Standard14FontsDir.setPath( "./standard14fonts" );

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
