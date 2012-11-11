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

#include "PRRenderPage.h"
#include "PRDocument.h"

#include "PdfeFont.h"

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
    PdfeCanvasAnalysis(),
    m_document( document ),
    m_page( document->podofoDocument()->GetPage( pageIndex ) ),
    m_pageIndex( pageIndex ),
    m_pageImage( NULL ),
    m_pagePainter( NULL )
{
}
PRRenderPage::~PRRenderPage()
{
    delete m_pagePainter;
    delete m_pageImage;
}

void PRRenderPage::initRendering( double resolution )
{
    // Clear.
    this->clearRendering();

    // Get crop and media boxes and set painted box.
    PdfRect mediaBox = m_page->GetMediaBox();
    PdfRect cropBox = m_page->GetCropBox();
    if( cropBox.GetWidth() > mediaBox.GetWidth() || cropBox.GetHeight() > mediaBox.GetHeight() ) {
        cropBox = mediaBox;
    }
    m_pageRect = cropBox;

    // Create associated image.
    m_pageImage = new QImage( int( m_pageRect.GetWidth() * resolution ),
                              int( m_pageRect.GetHeight() * resolution ),
                              QImage::Format_RGB32 );
    m_pageImage->fill( QColor( Qt::white ).rgb() );

    // QPainter used to draw the page.
    m_pagePainter = new QPainter( m_pageImage );
    m_pagePainter->setRenderHint( QPainter::Antialiasing, false );

    // Transform from page space to image space.
    m_pageImgTrans.init();
    m_pageImgTrans(0,0) = resolution;
    m_pageImgTrans(1,1) = -resolution;
    m_pageImgTrans(2,0) = -m_pageRect.GetLeft() * resolution;
    m_pageImgTrans(2,1) = ( m_pageRect.GetBottom() + m_pageRect.GetHeight() ) * resolution;
}
void PRRenderPage::clearRendering()
{
    delete m_pagePainter;
    m_pagePainter = NULL;
    delete m_pageImage;
    m_pageImage = NULL;
}

void PRRenderPage::render( const PRRenderParameters& parameters )
{
    // Initialize image and painter.
    this->initRendering( parameters.resolution );

    // Render parameters.
    m_renderParameters = parameters;

    // Initial clipping path.
    m_clippingPathStack.push_back( QPainterPath() );
    if( !parameters.clippingPath.isEmpty() ) {
        // Save and set clipping path.
        m_clippingPathStack.back() = parameters.clippingPath;

        m_pagePainter->setTransform( m_pageImgTrans.toQTransform() );
        m_pagePainter->setClipPath( m_clippingPathStack.back(), Qt::ReplaceClip );
    }

    // Perform the analysis and draw.
    this->analyseContents( m_page, PdfeGraphicsState(), PdfeResources() );
}
QImage PRRenderPage::image()
{
    if( m_pageImage ) {
        return *m_pageImage;
    }
    else {
        return QImage();
    }
}

// Reimplement PdfeCanvasAnalysis interface.
void PRRenderPage::fGeneralGState( const PdfeStreamState& streamState ) { }
void PRRenderPage::fSpecialGState( const PdfeStreamState& streamState )
{
    const PdfeGraphicOperator& gOperator = streamState.gOperator;
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
void PRRenderPage::fPathConstruction( const PdfeStreamState& streamState,
                                      const PdfePath& currentPath ) { }

void PRRenderPage::fPathPainting( const PdfeStreamState& streamState,
                                  const PdfePath& currentPath )
{
    // Simpler references.
    const PdfeGraphicOperator& gOperator = streamState.gOperator;
    const PdfeGraphicsState& gState = streamState.gStates.back();

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
    if( currentPath.clippingPathOp().length() ) {
        m_renderParameters.clippingPathPB.applyToPainter( m_pagePainter );
    }
    else {
        m_renderParameters.pathPB.applyToPainter( m_pagePainter );
    }
    m_pagePainter->drawPath( qCurrentPath );
}

void PRRenderPage::fClippingPath( const PdfeStreamState& streamState,
                                  const PdfePath& currentPath )
{
    // Simpler references.
    const PdfeGraphicOperator& gOperator = streamState.gOperator;
    const PdfeGraphicsState& gState = streamState.gStates.back();

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

void PRRenderPage::fTextObjects( const PdfeStreamState& streamState ) { }
void PRRenderPage::fTextState( const PdfeStreamState& streamState ) { }
void PRRenderPage::fTextPositioning( const PdfeStreamState& streamState ) { }

PdfeVector PRRenderPage::fTextShowing( const PdfeStreamState& streamState )
{
    // Create the group of words.
    PRGTextGroupWords groupWords( m_document, streamState );

    // Draw the group of words.
    this->textDrawGroup( groupWords, m_renderParameters );

    // Return text displacement.
    return groupWords.displacement();
}

void PRRenderPage::fType3Fonts( const PdfeStreamState& streamState ) { }
void PRRenderPage::fColor( const PdfeStreamState& streamState ) { }
void PRRenderPage::fShadingPatterns( const PdfeStreamState& streamState ) { }

void PRRenderPage::fInlineImages( const PdfeStreamState& streamState )
{
    // Simpler references.
    const PdfeGraphicOperator& gOperator = streamState.gOperator;
    const PdfeGraphicsState& gState = streamState.gStates.back();

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

void PRRenderPage::fXObjects( const PdfeStreamState& streamState )
{
    // Simpler references.
    //const PdfeGraphicOperator& gOperator = streamState.gOperator;
    const std::vector<std::string>& gOperands = streamState.gOperands;
    const PdfeGraphicsState& gState = streamState.gStates.back();

    // Name of the XObject and dictionary entry.
    std::string xobjName = gOperands.back().substr( 1 );
    PdfObject* xobjPtr = streamState.resources.getIndirectKey( PdfeResourcesType::XObject, xobjName );
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
void PRRenderPage::fMarkedContents( const PdfeStreamState& streamState ) { }
void PRRenderPage::fCompatibility( const PdfeStreamState& streamState ) { }

//**********************************************************//
//                 Drawing member functions                 //
//**********************************************************//
void PRRenderPage::drawPdfeORect( const PdfeORect& orect, const PRRenderParameters::PRPenBrush& penBrush )
{
    // No image or painter...
    if( !m_pageImage || !m_pagePainter ) {
        return;
    }

    // Basic transformation matrix.
    m_pagePainter->setTransform( m_pageImgTrans.toQTransform() );

    // Create polygon corresponding to the oriented rectangle.
    QPolygonF polygon;
    polygon << orect.leftBottom().toQPoint();
    polygon << orect.rightBottom().toQPoint();
    polygon << orect.rightTop().toQPoint();
    polygon << orect.leftTop().toQPoint();

    // Draw polygon using the given Pen/Brush.
    penBrush.applyToPainter( m_pagePainter );
    m_pagePainter->drawPolygon( polygon );
}
void PRRenderPage::textDrawGroup( const PRGTextGroupWords& groupWords,
                                       const PRRenderParameters& parameters )
{
    // Render the complete subgroup.
    this->textDrawSubgroup( PRGTextGroupWords::Subgroup( groupWords, true ), parameters );
}
void PRRenderPage::textDrawSubgroup( const PRGTextGroupWords::Subgroup& subgroup,
                                     const PRRenderParameters& parameters )
{
    // Nothing to draw or can not draw...
    PRGTextGroupWords* pGroup = subgroup.group();
    if( !m_pageImage || !m_pagePainter ||
        !pGroup || !pGroup->nbWords() ) {
        return;
    }

    // Compute text rendering matrix.
    PdfeMatrix textMat;
    textMat = pGroup->getGlobalTransMatrix() * m_pageImgTrans;
    m_pagePainter->setTransform( textMat.toQTransform() );

    // Paint words.
    PdfeVector advance;
    for( size_t i = 0 ; i < pGroup->nbWords() ; ++i ) {
        const PRGTextWord& word = pGroup->word( i );
        // Set pen & brush
        if( word.type() == PRGTextWordType::Classic ) {
            parameters.textPB.applyToPainter( m_pagePainter );
        }
        else if( word.type() == PRGTextWordType::Space ) {
            parameters.textSpacePB.applyToPainter( m_pagePainter );
        }
        else if( word.type() == PRGTextWordType::PDFTranslation ||
                 word.type() == PRGTextWordType::PDFTranslationCS ) {
            parameters.textPDFTranslationPB.applyToPainter( m_pagePainter );
        }
        // Paint word, if it is inside the subgroup and the width is positive !
        PdfRect bbox = word.bbox( true );
        if( subgroup.inside( i ) && bbox.GetWidth() >= 0) {
            m_pagePainter->drawRect( QRectF( advance(0) + bbox.GetLeft(),
                                             advance(1) + bbox.GetBottom(),
                                             bbox.GetWidth(),
                                             bbox.GetHeight() ) );
        }
        advance += word.advance();
    }
}
void PRRenderPage::textDrawMainSubgroups( const PRGTextGroupWords& groupWords,
                                          const PRRenderParameters& parameters )
{
    // Nothing to draw...
    if( !m_pageImage || !m_pagePainter || !groupWords.nbWords() ) {
        return;
    }

    // Compute text rendering matrix.
    PdfeMatrix textMat;
    textMat = groupWords.getGlobalTransMatrix() * m_pageImgTrans;
    m_pagePainter->setTransform( textMat.toQTransform() );

    // Paint subgroups: only text is considered in these subgroups.
    for( size_t i = 0 ; i < groupWords.nbMSubgroups() ; i++ )
    {
        // Set pen & brush
        parameters.textPB.applyToPainter( m_pagePainter );

        // Paint word, if the width is positive !
        PdfeORect bbox = groupWords.mSubgroup(i).bbox( PRGTextWordCoordinates::Font, true, true );
        PdfeVector lb = bbox.leftBottom();
        m_pagePainter->drawRect( QRectF( lb(0) , lb(1), bbox.width(), bbox.height() ) );
    }
}
void PRRenderPage::textRenderGroup( const PRGTextGroupWords& groupWords )
{
    //m_pagePainter->setRenderHint( QPainter::Antialiasing, true );
    //m_pagePainter->setRenderHint( QPainter::SmoothPixmapTransform, true );

    // Font.
    PdfeFont* pFont = groupWords.font();

    // Transformation matrix.
    PdfeMatrix transMat, tmpMat;
    transMat = groupWords.getGlobalTransMatrix() * m_pageImgTrans;
    m_pagePainter->setTransform( transMat.toQTransform() );

    // Render glyphs of every word.
    PdfeVector advance;
    for( size_t i = 0 ; i < groupWords.nbWords() ; ++i ) {
        const PRGTextWord& word = groupWords.word( i );

        // Draw only the type correspond to a classic word.
        if( word.type() == PRGTextWordType::Classic ) {
            PdfeCIDString cidstr = word.cidString();

            for( size_t j = 0 ; j < cidstr.length() ; ++j ) {
                pdfe_gid gid = pFont->fromCIDToGID( cidstr[j] );

                // Render glyph.
                if( gid ) {
                    // TODO: set resolution and size.
                    PdfRect bbox = pFont->bbox( cidstr[j], false );
                    PdfeFont::GlyphImage glyphRender = pFont->ftGlyphRender( gid, 100, 100 );
                    glyphRender.image = glyphRender.image.mirrored( false, true );

                    // Draw it on the page.
                    m_pagePainter->drawImage( QRectF( bbox.GetLeft() + advance(0),
                                                      bbox.GetBottom() + advance(1),
                                                      bbox.GetWidth(),
                                                      bbox.GetHeight() ),
                                              glyphRender.image );
                }
                // Update advance vector.
                advance += pFont->advance( cidstr[j], false );
                advance(0) += word.charSpace();
                //advance(1) += word.charSpace();
            }
        }
        else {
            // Update advance vector
            advance += word.advance();
        }
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

