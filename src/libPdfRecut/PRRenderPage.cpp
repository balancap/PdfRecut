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


#include "PRRenderPage.h"

#include <podofo/podofo.h>
#include <QtCore>
#include <QtGui>

#include <fstream>

using namespace PoDoFo;
using namespace PoDoFoExtended;

namespace PdfRecut {

int PRRenderPage::imgNbs = 0;

//**********************************************************//
//                    PRRenderParameters                    //
//**********************************************************//
PRRenderParameters::PRRenderParameters()
{
    this->resolution = 1.0;
    this->initToDefault();
}
void PRRenderParameters::initToDefault()
{
    // Clipping path set to empty.
    clippingPath = QPainterPath();

    // Path (normal & clipping) pens.
    pathPB.drawPen = new QPen( Qt::magenta );
    clippingPathPB.drawPen = new QPen( Qt::darkMagenta );

    // Text filling gradient.
    QLinearGradient textGradient( 0.0, 0.0, 0.0, 1.0 );
    textGradient.setColorAt( 0.0, Qt::blue );
    textGradient.setColorAt( 1.0, Qt::white );
    textPB.fillBrush = new QBrush( textGradient );

    // Space filling color.
    textSpacePB.fillBrush = new QBrush( Qt::lightGray );
    textPDFTranslationPB.fillBrush = new QBrush( Qt::lightGray );

    // Inline image pen color.
    inlineImagePB.drawPen = new QPen( Qt::darkCyan );
    inlineImagePB.fillBrush = new QBrush( Qt::cyan );

    // Image pen color.
    imagePB.drawPen = new QPen( Qt::darkRed );
    imagePB.fillBrush = new QBrush( Qt::red );

    // Form pen color.
    formPB.drawPen = new QPen( Qt::green );
    //formPB.fillBrush = new QBrush( Qt::green );
}
void PRRenderParameters::initToEmpty()
{
    clippingPath = QPainterPath();
    resolution = 1.0;

    textPB = PRRenderParameters::PRPenBrush();
    textSpacePB = PRRenderParameters::PRPenBrush();
    textPDFTranslationPB = PRRenderParameters::PRPenBrush();
    pathPB = PRRenderParameters::PRPenBrush();
    clippingPathPB = PRRenderParameters::PRPenBrush();
    inlineImagePB = PRRenderParameters::PRPenBrush();
    imagePB = PRRenderParameters::PRPenBrush();
    formPB = PRRenderParameters::PRPenBrush();
    textPB = PRRenderParameters::PRPenBrush();
    textSpacePB = PRRenderParameters::PRPenBrush();
}

//**********************************************************//
//                       PRRenderPage                       //
//**********************************************************//
PRRenderPage::PRRenderPage( PRDocument* document,
                            long pageIndex ) :
    PRStreamAnalysis( document->getPoDoFoDocument()->GetPage( pageIndex ) ),
    m_document( document ),
    m_pageIndex( pageIndex )
{
    m_pageImage = NULL;
    m_pagePainter = NULL;
}
PRRenderPage::~PRRenderPage()
{
    delete m_pagePainter;
    delete m_pageImage;
}

void PRRenderPage::renderPage( const PRRenderParameters & parameters )
{
    // Render parameters.
    m_renderParameters = parameters;

    // Get crop and media boxes and set painted box.
    PdfRect mediaBox = m_page->GetMediaBox();
    PdfRect cropBox = m_page->GetCropBox();
    if( cropBox.GetWidth() > mediaBox.GetWidth() || cropBox.GetHeight() > mediaBox.GetHeight() ) {
        cropBox = mediaBox;
    }
    m_pageRect = cropBox;

    // Create associated image.
    delete m_pageImage;
    m_pageImage = new QImage( int( m_pageRect.GetWidth() * parameters.resolution ),
                              int( m_pageRect.GetHeight() * parameters.resolution ),
                              QImage::Format_RGB32 );
    m_pageImage->fill( QColor( Qt::white ).rgb() );

    // QPainter used to draw the page.
    delete m_pagePainter;
    m_pagePainter = new QPainter( m_pageImage );
    m_pagePainter->setRenderHint( QPainter::Antialiasing, false );

    // Transform from page space to image space.
    m_pageImgTrans.init();
    m_pageImgTrans(0,0) = parameters.resolution;
    m_pageImgTrans(1,1) = -parameters.resolution;
    m_pageImgTrans(2,0) = -m_pageRect.GetLeft() * parameters.resolution;
    m_pageImgTrans(2,1) = (m_pageRect.GetBottom() + m_pageRect.GetHeight()) * parameters.resolution;

    // Initial clipping path.
    m_clippingPathStack.push_back( QPainterPath() );
    if( !parameters.clippingPath.isEmpty() ) {
        // Save and set clipping path.
        m_clippingPathStack.back() = parameters.clippingPath;

        m_pagePainter->setTransform( m_pageImgTrans.toQTransform() );
        m_pagePainter->setClipPath( m_clippingPathStack.back(), Qt::ReplaceClip );
    }
    m_nbTextGroups = 0;

    // Perform the analysis and draw.
    this->analyse();

    //m_pagePainter->end();
}

QImage *PRRenderPage::getRenderImage()
{
    return m_pageImage;
}
void PRRenderPage::clearPageImage()
{
    delete m_pageImage;
    delete m_pagePainter;
}

void PRRenderPage::saveToFile( const QString& filename )
{
    m_pageImage->save( filename );
}

void PRRenderPage::fGeneralGState( const PdfStreamState& streamState ) { }

void PRRenderPage::fSpecialGState( const PdfStreamState& streamState )
{
    const PdfGraphicOperator& gOperator = streamState.gOperator;
    if( gOperator.code == ePdfGOperator_q ) {
        // Push on the clipping paths stm_documentack.
        m_clippingPathStack.push_back( m_clippingPathStack.back() );
    }
    else if( gOperator.code == ePdfGOperator_Q ) {
        // Pop on the graphics state stack.
        m_clippingPathStack.pop_back();

        // Restore previous clipping path.
        if( m_clippingPathStack.back().isEmpty() ) {
            m_pagePainter->setClipPath( m_clippingPathStack.back(), Qt::NoClip );
        }
        else {
            m_pagePainter->setTransform( m_pageImgTrans.toQTransform() );
            m_pagePainter->setClipPath( m_clippingPathStack.back(), Qt::ReplaceClip );
        }
    }
}
void PRRenderPage::fPathConstruction( const PdfStreamState& streamState,
                                      const PdfPath& currentPath ) { }

void PRRenderPage::fPathPainting( const PdfStreamState& streamState,
                                  const PdfPath& currentPath )
{
    // Simpler references.
    const PdfGraphicOperator& gOperator = streamState.gOperator;
    const PdfGraphicsState& gState = streamState.gStates.back();

    // No painting require.
    if( !m_renderParameters.pathPB.isPainting() ) {
        return;
    }

    // Qt painter path to create from Pdf path.
    bool closeSubpaths = gOperator.isClosePainting();
    bool evenOddRule = gOperator.isEvenOddRule();
    QPainterPath qCurrentPath = currentPath.toQPainterPath( closeSubpaths, evenOddRule );

    // Compute path rendering matrix.
    PdfeMatrix pathMat;
    pathMat = gState.transMat * m_pageImgTrans;
    m_pagePainter->setTransform( pathMat.toQTransform() );

    // Draw path.
    if( currentPath.getClippingPathOp().length() ) {
        m_renderParameters.clippingPathPB.applyToPainter( m_pagePainter );
    }
    else {
        m_renderParameters.pathPB.applyToPainter( m_pagePainter );
    }
    m_pagePainter->drawPath( qCurrentPath );
}

void PRRenderPage::fClippingPath( const PdfStreamState& streamState,
                                  const PdfPath& currentPath )
{
    // Simpler references.
    const PdfGraphicOperator& gOperator = streamState.gOperator;
    const PdfGraphicsState& gState = streamState.gStates.back();

    // Get Qt painter path which represents the clipping path.
    bool evenOddRule = gOperator.isEvenOddRule();
    QPainterPath qClippingPath = currentPath.toQPainterPath( true, evenOddRule );

    // Apply transformation, to have a common coordinates system.
    QTransform qTrans = gState.transMat.toQTransform();
    qClippingPath = qTrans.map( qClippingPath );

    // Intersect the clipping path with the current one.
    if( m_clippingPathStack.back().isEmpty() ) {
        m_clippingPathStack.back() = qClippingPath;
    }
    else {
        m_clippingPathStack.back() = m_clippingPathStack.back().intersected( qClippingPath );
    }

    // Finally, set clipping path.
    m_pagePainter->setTransform( m_pageImgTrans.toQTransform() );
    m_pagePainter->setClipPath( m_clippingPathStack.back(), Qt::ReplaceClip );
}

void PRRenderPage::fTextObjects( const PdfStreamState& streamState ) { }

void PRRenderPage::fTextState( const PdfStreamState& streamState ) { }

void PRRenderPage::fTextPositioning( const PdfStreamState& streamState )
{
    // Update text transformation matrix.
    this->textUpdateTransMatrix( streamState );
}

void PRRenderPage::fTextShowing( const PdfStreamState& streamState )
{
    // Update text transformation matrix.
    this->textUpdateTransMatrix( streamState );

    // Read the group of words.
    PRTextGroupWords groupWords = this->textReadGroupWords( streamState );

    // Draw the group of words.
    this->textDrawGroupWords( groupWords );
}

void PRRenderPage::fType3Fonts( const PdfStreamState& streamState ) { }

void PRRenderPage::fColor( const PdfStreamState& streamState ) { }

void PRRenderPage::fShadingPatterns( const PdfStreamState& streamState ) { }

void PRRenderPage::fInlineImages( const PdfStreamState& streamState )
{
    // Simpler references.
    const PdfGraphicOperator& gOperator = streamState.gOperator;
    const PdfGraphicsState& gState = streamState.gStates.back();

    if( gOperator.code == ePdfGOperator_EI )
    {
        // Compute path rendering matrix.
        PdfeMatrix pathMat;
        pathMat = gState.transMat * m_pageImgTrans;
        m_pagePainter->setTransform( pathMat.toQTransform() );

        // Draw the inline image: corresponds to a rectangle (0,0,1,1).
        m_renderParameters.inlineImagePB.applyToPainter( m_pagePainter );
        m_pagePainter->drawRect(  QRectF( 0.0, 0.0, 1.0, 1.0 ) );
    }
}

void PRRenderPage::fXObjects( const PdfStreamState& streamState )
{
    // Simpler references.
    //const PdfGraphicOperator& gOperator = streamState.gOperator;
    const std::vector<std::string>& gOperands = streamState.gOperands;
    const PdfGraphicsState& gState = streamState.gStates.back();

    // Name of the XObject and dictionary entry.
    std::string xobjName = gOperands.back().substr( 1 );
    PdfObject* xobjPtr = streamState.resources.getIndirectKey( ePdfResourcesType_XObject, xobjName );
    std::string xobjSubtype = xobjPtr->GetIndirectKey( "Subtype" )->GetName().GetName();
    PdfeMatrix pathMat;

    // Distinction between different type of XObjects
    if( !xobjSubtype.compare( "Image" ) )
    {
        //testPdfImage( xobjPtr );

        // Compute path rendering matrix.
        pathMat = gState.transMat * m_pageImgTrans;
        m_pagePainter->setTransform( pathMat.toQTransform() );

        // Draw the image: corresponds to a rectangle (0,0,1,1).
        m_renderParameters.imagePB.applyToPainter( m_pagePainter );
        m_pagePainter->drawRect(  QRectF( 0.0, 0.0, 1.0, 1.0 ) );
    }
    else if( !xobjSubtype.compare( "Form" ) )
    {
        // Get transformation matrix in form's dictionary
        PdfeMatrix formMat;
        if( xobjPtr->GetDictionary().HasKey( "Matrix" ) ) {
            PdfArray& mat = xobjPtr->GetIndirectKey( "Matrix" )->GetArray();

            formMat(0,0) = mat[0].GetReal();
            formMat(0,1) = mat[1].GetReal();
            formMat(1,0) = mat[2].GetReal();
            formMat(1,1) = mat[3].GetReal();
            formMat(2,0) = mat[4].GetReal();
            formMat(2,1) = mat[5].GetReal();
        }
        // Position of form's BBox.
        PdfArray& bbox = xobjPtr->GetIndirectKey( "BBox" )->GetArray();

        // Draw form according to its properties.
        pathMat = formMat * gState.transMat * m_pageImgTrans;
        m_pagePainter->setTransform( pathMat.toQTransform() );

        // Draw the image: corresponds to a rectangle (0,0,1,1).
        m_renderParameters.formPB.applyToPainter( m_pagePainter );
        m_pagePainter->drawRect(  QRectF( bbox[0].GetReal(),
                                          bbox[1].GetReal(),
                                          bbox[2].GetReal()-bbox[0].GetReal(),
                                          bbox[3].GetReal()-bbox[1].GetReal() ) );
    }
}

void PRRenderPage::fMarkedContents( const PdfStreamState& streamState ) { }

void PRRenderPage::fCompatibility( const PdfStreamState& streamState ) { }


PRTextGroupWords PRRenderPage::textReadGroupWords( const PdfStreamState& streamState )
{
    // Simpler references.
    const std::vector<std::string>& gOperands = streamState.gOperands;
    const PdfGraphicsState& gState = streamState.gStates.back();

    PdfTextState textState = streamState.gStates.back().textState;
    textState.transMat = m_textMatrix;

    // Get variant from string.
    PdfVariant variant;
    PdfTokenizer tokenizer( gOperands.back().c_str(), gOperands.back().length() );
    tokenizer.GetNextVariant( variant, NULL );

    // Get font metrics.
    PdfeFont* pFont = m_document->fontCache( gState.textState.fontRef );

    // Read group of words.
    PRTextGroupWords groupWords;
    groupWords.readPdfVariant( variant,
                               streamState.gStates.back().transMat,
                               textState,
                               pFont );
    groupWords.setGroupIndex( m_nbTextGroups );

    // Increment the number of group of words.
    ++m_nbTextGroups;

    // Update text transform matrix.
    PdfeMatrix tmpMat;
    tmpMat(2,0) = groupWords.width() * textState.fontSize * ( textState.hScale / 100. );
    m_textMatrix = tmpMat * m_textMatrix;

    return groupWords;
}

void PRRenderPage::textDrawGroupWords( const PRTextGroupWords& groupWords )
{
    // Nothing to draw...
    if( !groupWords.nbWords() ) {
        return;
    }

    // Compute text rendering matrix.
    PdfeMatrix textMat;
    textMat = groupWords.getGlobalTransMatrix() * m_pageImgTrans;
    m_pagePainter->setTransform( textMat.toQTransform() );

    // Paint words.
    double widthStr = 0;
    for( size_t i = 0 ; i < groupWords.nbWords() ; i++ )
    {
        const PRTextWord& word = groupWords.word( i );

        // Set pen & brush
        if( word.type() == PRTextWordType::Classic ) {
            m_renderParameters.textPB.applyToPainter( m_pagePainter );
        }
        else if( word.type() == PRTextWordType::Space ) {
            m_renderParameters.textSpacePB.applyToPainter( m_pagePainter );
        }
        else if( word.type() == PRTextWordType::PDFTranslation ||
                 word.type() == PRTextWordType::PDFTranslationCS ) {
            m_renderParameters.textPDFTranslationPB.applyToPainter( m_pagePainter );
        }
        // Paint word, if the width is positive !
        PdfRect bbox = word.bbox( false );
        if( bbox.GetWidth() >= 0) {
            m_pagePainter->drawRect( QRectF( widthStr, bbox.GetBottom(), bbox.GetWidth(), bbox.GetHeight() ) );
        }
        widthStr += word.width( true );
    }
}

void PRRenderPage::textUpdateTransMatrix( const PdfStreamState& streamState )
{
    // Text positioning operator or showing operator (quote or double quote).
    if( streamState.gOperator.cat == ePdfGCategory_TextPositioning ||
        ( streamState.gOperator.cat == ePdfGCategory_TextShowing &&
          ( streamState.gOperator.code == ePdfGOperator_DoubleQuote || streamState.gOperator.code == ePdfGOperator_Quote ) ) )
    {
        // Reset text transform matrix.
        m_textMatrix = streamState.gStates.back().textState.transMat;
    }
}

//**********************************************************//
//                      Image testing                       //
//**********************************************************//
void PRRenderPage::testPdfImage( PoDoFo::PdfObject* xobj )
{
    // Get image properties.
    std::string xobjSubtype = xobj->GetIndirectKey( "Subtype" )->GetName().GetName();
    long width = xobj->GetIndirectKey( "Width" )->GetNumber();
    long height = xobj->GetIndirectKey( "Height" )->GetNumber();

    std::string filter;
    if( xobj->GetIndirectKey( "Filter" )->IsName() )
        filter = xobj->GetIndirectKey( "Filter" )->GetName().GetName();
    else
        xobj->GetIndirectKey( "Filter" )->ToString( filter );

    std::cout << xobjSubtype << " : "
              << width << "x" << height << " // "
              << filter;

    if( filter == "DCTDecode" || filter == "JBIG2Decode" )
    {
        imgNbs++;
        PdfMemStream* imgStream = dynamic_cast<PdfMemStream*> ( xobj->GetStream() );

        QImage imgTest;
        bool success = imgTest.loadFromData( (const uchar*) imgStream->Get(), imgStream->GetLength(), "JPG");

        std::cout << " // " << success << "(" << imgNbs << ")";

        if(success) {
            QString filename = QString("./img/imgTest%1.jpg").arg( imgNbs, 3, 10, QLatin1Char('0') );
            imgTest.save( filename );
        }
    }
    if( filter == "CCITTFaxDecode" )
    {
        PdfMemStream* imgStream = dynamic_cast<PdfMemStream*> ( xobj->GetStream() );

        QImage imgTest;
        bool success = imgTest.loadFromData( (const uchar*) imgStream->Get(), imgStream->GetLength(), "PPM");
        std::cout << " // " << success << "(" << imgNbs << ")";

        std::string dparams;
        xobj->GetIndirectKey( "DecodeParms" )->ToString( dparams );
        std::cout << " // " << imgStream->GetLength();

        std::ofstream outData( "./img.raw", std::ios_base::trunc );
        outData.write( imgStream->Get(), imgStream->GetLength() );
        outData.close();
    }
    std::cout << std::endl;
}


}

