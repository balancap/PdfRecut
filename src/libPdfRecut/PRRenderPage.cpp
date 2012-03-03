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

using namespace PoDoFo;

namespace PdfRecut {

//**********************************************************//
//                    PRRenderParameters                    //
//**********************************************************//
PRRenderParameters::PRRenderParameters()
{
    this->resolution = 1.0;
    this->initPenBrush();
}
void PRRenderParameters::initPenBrush()
{
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

    // Inline image pen color.
    inlineImagePB.drawPen = new QPen( Qt::darkCyan );
    inlineImagePB.fillBrush = new QBrush( Qt::cyan );

    // Image pen color.
    imagePB.drawPen = new QPen( Qt::darkRed );
    imagePB.fillBrush = new QBrush( Qt::red );

    // Form pen color.
    formPB.drawPen = new QPen( Qt::darkGreen );
    formPB.fillBrush = new QBrush( Qt::green );
}

//**********************************************************//
//                       PRRenderPage                       //
//**********************************************************//
PRRenderPage::PRRenderPage( PoDoFo::PdfPage* pageIn,
                              PdfFontMetricsCache* fontMetricsCache ) :
    PRStreamAnalysis( pageIn ), m_fontMetricsCache( fontMetricsCache )
{
    m_pageImage = NULL;
    m_pagePainter = NULL;

    m_document = dynamic_cast<PdfMemDocument*> ( m_page->GetObject()->GetOwner()->GetParentDocument() );
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
    m_pageImgTrans(2,0) = -cropBox.GetLeft() * parameters.resolution;
    m_pageImgTrans(2,1) = (cropBox.GetBottom() + cropBox.GetHeight()) * parameters.resolution;

    // Perform the analysis and draw.
    this->analyse();

    m_pagePainter->end();
}

void PRRenderPage::saveToFile( const QString& filename )
{
    m_pageImage->save( filename );
}

void PRRenderPage::fGeneralGState( const PdfGraphicOperator& gOperator,
                                   const std::vector<std::string>& vecVariables,
                                   const std::vector<PdfGraphicsState>& vecGStates ) { }

void PRRenderPage::fSpecialGState( const PdfGraphicOperator& gOperator,
                                   const std::vector<std::string>& vecVariables,
                                   const std::vector<PdfGraphicsState>& vecGStates ) { }

void PRRenderPage::fPathConstruction( const PdfGraphicOperator& gOperator,
                                      const std::vector<std::string>& vecVariables,
                                      const std::vector<PdfGraphicsState>& vecGStates,
                                      const PdfPath& currentPath ) { }

void PRRenderPage::fPathPainting( const PdfGraphicOperator& gOperator,
                                  const std::vector<std::string>& vecVariables,
                                  const std::vector<PdfGraphicsState>& vecGStates,
                                  const PdfPath& currentPath )
{
    // No painting require.
    if( !m_renderParameters.pathPB.isPainting() ) {
        return;
    }

    // Subpaths from the current path.
    std::vector<PdfSubPath> subpaths = currentPath.getSubpaths();
    bool closeSubpaths = gOperator.isClosePainting();

    // Qt painter path to create from Pdf path.
    QPainterPath qCurrentPath;

    // Add every subpath to the qt painter path.
    for( size_t i = 0 ; i < subpaths.size() ; ++i )
    {
        // Points from the subpath.
        for( size_t j = 0 ; j < subpaths[i].points.size() ; ++j )
        {
            PdfVector& point = subpaths[i].points[j];
            std::string& opPoint = subpaths[i].opPoints[j];

            // Distinction between different painting operators.
            if( opPoint == "m" ) {
                qCurrentPath.moveTo( point(0), point(1) );
            }
            else if( opPoint == "l" ) {
                qCurrentPath.lineTo( point(0), point(1) );
            }
            else if( opPoint == "c" ) {
                QPointF c1Pt( subpaths[i].points[j](0), subpaths[i].points[j](1) );
                QPointF c2Pt( subpaths[i].points[j+1](0), subpaths[i].points[j+1](1) );
                QPointF endPt( subpaths[i].points[j+2](0), subpaths[i].points[j+2](1) );

                qCurrentPath.cubicTo( c1Pt, c2Pt, endPt );
                j+=2;
            }
            else if( opPoint == "v" ) {
                QPointF c1Pt( qCurrentPath.currentPosition() );
                QPointF c2Pt( subpaths[i].points[j](0), subpaths[i].points[j](1) );
                QPointF endPt( subpaths[i].points[j+1](0), subpaths[i].points[j+1](1) );

                qCurrentPath.cubicTo( c1Pt, c2Pt, endPt );
                j+=1;
            }
            else if( opPoint == "y" ) {
                QPointF c1Pt( subpaths[i].points[j](0), subpaths[i].points[j](1) );
                QPointF c2Pt( subpaths[i].points[j+1](0), subpaths[i].points[j+1](1) );
                QPointF endPt( c2Pt );

                qCurrentPath.cubicTo( c1Pt, c2Pt, endPt );
                j+=1;
            }
            else if( opPoint == "h" ) {
                qCurrentPath.closeSubpath();
            }
            else if( opPoint == "re" ) {
                // Rectangle:  "re" corresponds to "m l l l h"
                PdfVector& pointUR = subpaths[i].points[j+2];

                qCurrentPath.addRect( point(0), point(1), pointUR(0)-point(0), pointUR(1)-point(1) );
                j+=4;
            }
        }
        // Force the subpaths to be closed according, based on the painting operator.
        if( closeSubpaths ) {
            qCurrentPath.closeSubpath();
        }
    }
    // Compute path rendering matrix.
    PdfMatrix pathMat;
    pathMat = vecGStates.back().transMat * m_pageImgTrans;
    m_pagePainter->setTransform( pathMat.toQTransform() );

    // Draw path.
    if( currentPath.getClippingPathOp().length() ) {
        m_renderParameters.clippingPathPB.setPenBrush( m_pagePainter );
    }
    else {
        m_renderParameters.pathPB.setPenBrush( m_pagePainter );
    }
    m_pagePainter->drawPath( qCurrentPath );
}

void PRRenderPage::fClippingPath( const PdfGraphicOperator& gOperator,
                                   const std::vector<std::string>& vecVariables,
                                   const std::vector<PdfGraphicsState>& vecGStates,
                                   const PdfPath& currentPath ) { }

void PRRenderPage::fTextObjects( const PdfGraphicOperator& gOperator,
                                  const std::vector<std::string>& vecVariables,
                                  const std::vector<PdfGraphicsState>& vecGStates ) { }

void PRRenderPage::fTextState( const PdfGraphicOperator& gOperator,
                                const std::vector<std::string>& vecVariables,
                                const std::vector<PdfGraphicsState>& vecGStates ) { }

void PRRenderPage::fTextPositioning( const PdfGraphicOperator& gOperator,
                                      const std::vector<std::string>& vecVariables,
                                      const std::vector<PdfGraphicsState>& vecGStates )
{
    // Reset text transform matrix.
    m_textMatrix = vecGStates.back().textState.transMat;
}

void PRRenderPage::fTextShowing( const PdfGraphicOperator& gOperator,
                                  const std::vector<std::string>& vecVariables,
                                  const std::vector<PdfGraphicsState>& vecGStates )
{
    // Get variant from string.
    PdfVariant variant;
    PdfTokenizer tokenizer( vecVariables.back().c_str(), vecVariables.back().length() );
    tokenizer.GetNextVariant( variant, NULL );

    // Compute string width using font metrics.
    std::vector<WordWidth> wordWidths;
    double widthStr = 0;
    PdfArray boundingBox;
    PdfFontMetrics* fontMetrics = m_fontMetricsCache->getFontMetrics( vecGStates.back().textState.fontRef );
    if( fontMetrics )
    {
        // Font metrics parameters.
        fontMetrics->SetFontSize( 1.0 );
        fontMetrics->SetFontScale( 100. );
        //fontMetrics->SetFontSize( vecGStates.back().textState.fontSize );
        //fontMetrics->SetFontScale( vecGStates.back().textState.hScale );

        // Strange implementation of char space in CharWidth functions from metrics classes: must multiply by 100.
        fontMetrics->SetFontCharSpace( vecGStates.back().textState.charSpace / vecGStates.back().textState.fontSize * 100 );

        // Compute string width.
        if( variant.IsString() || variant.IsHexString() )
        {
            this->getStringWidth( wordWidths, variant.GetString(), fontMetrics,
                                             vecGStates.back().textState.wordSpace );
        }
        else if( variant.IsArray() )
        {
            PdfArray& array = variant.GetArray();
            for( size_t i = 0 ; i < array.size() ; i++ ) {
                if( array[i].IsString() || array[i].IsHexString() ) {
                    this->getStringWidth( wordWidths, array[i].GetString(), fontMetrics,
                                                      vecGStates.back().textState.wordSpace );
                }
                else if( array[i].IsNumber() ) {
                    wordWidths.push_back( WordWidth( -array[i].GetNumber() / 1000.0, true ) );
                }
                else if( array[i].IsReal() ) {
                    wordWidths.push_back( WordWidth( -array[i].GetReal() / 1000.0, true ) );
                }
            }
        }
        fontMetrics->GetBoundingBox( boundingBox );
    }
    else
    {
        // Default width: font size.
        wordWidths.push_back( WordWidth( 1.0, false ) );
        boundingBox.push_back( 0.0 );
        boundingBox.push_back( 0.0 );
        boundingBox.push_back( 1.0 );
        boundingBox.push_back( 1.0 );
    }
    double heightStr = boundingBox[3].GetReal() / 1000.;

    // std::cout << "CS: " << vecGStates.back().textState.charSpace << std::endl;
    // std::cout << "WS: " << vecGStates.back().textState.wordSpace << std::endl;
    // std::cout << "HS: " << vecGStates.back().textState.hScale << std::endl;
    // std::cout << "FS: " << vecGStates.back().textState.fontSize << std::endl;

    // Compute text rendering matrix.
    PdfMatrix tmpMat, textMat;
    tmpMat(0,0) = vecGStates.back().textState.fontSize * vecGStates.back().textState.hScale / 100;
    tmpMat(1,1) = vecGStates.back().textState.fontSize * heightStr;
    tmpMat(2,1) = vecGStates.back().textState.rise;
    textMat = tmpMat * m_textMatrix * vecGStates.back().transMat * m_pageImgTrans;
    m_pagePainter->setTransform( textMat.toQTransform() );

    // Paint words.
    for( size_t i = 0 ; i < wordWidths.size() ; i++ )
    {
        // Set pen & brush
        if( wordWidths[i].isSpace ) {
            m_renderParameters.textSpacePB.setPenBrush( m_pagePainter );
        } else {
            m_renderParameters.textPB.setPenBrush( m_pagePainter );
        }

        // Paint word.
        m_pagePainter->drawRect( QRectF( widthStr, 0.0, wordWidths[i].width, 1.0 ) );
        widthStr += wordWidths[i].width;
    }

    // Update text transform matrix.
    tmpMat.init();
    tmpMat(2,0) = widthStr * vecGStates.back().textState.fontSize * vecGStates.back().textState.hScale / 100;
    m_textMatrix = tmpMat * m_textMatrix;
}

void PRRenderPage::fType3Fonts( const PdfGraphicOperator& gOperator,
                                 const std::vector<std::string>& vecVariables,
                                 const std::vector<PdfGraphicsState>& vecGStates ) { }

void PRRenderPage::fColor( const PdfGraphicOperator& gOperator,
                            const std::vector<std::string>& vecVariables,
                            const std::vector<PdfGraphicsState>& vecGStates ) { }

void PRRenderPage::fShadingPatterns( const PdfGraphicOperator& gOperator,
                                      const std::vector<std::string>& vecVariables,
                                      const std::vector<PdfGraphicsState>& vecGStates ) { }

void PRRenderPage::fInlineImages( const PdfGraphicOperator& gOperator,
                                   const std::vector<std::string>& vecVariables,
                                   const std::vector<PdfGraphicsState>& vecGStates )
{
    if( gOperator.code == ePdfGOperator_EI )
    {
        // Compute path rendering matrix.
        PdfMatrix pathMat;
        pathMat = vecGStates.back().transMat * m_pageImgTrans;
        m_pagePainter->setTransform( pathMat.toQTransform() );

        // Draw the inline image: corresponds to a rectangle (0,0,1,1).
        m_renderParameters.inlineImagePB.setPenBrush( m_pagePainter );
        m_pagePainter->drawRect(  QRectF( 0.0, 0.0, 1.0, 1.0 ) );
    }
}

void PRRenderPage::fXObjects( const PdfGraphicOperator& gOperator,
                               const std::vector<std::string>& vecVariables,
                               const std::vector<PdfGraphicsState>& vecGStates )
{
    // Name of the XObject and dictionary entry.
    std::string xobjName = vecVariables.back().substr( 1 );
    PdfObject* xobjDict = m_page->GetResources()->GetIndirectKey( "XObject" );
    PdfObject* xobjPtr = xobjDict->GetIndirectKey( xobjName );
    std::string xobjSubtype = xobjPtr->GetIndirectKey( "Subtype" )->GetName().GetName();
    PdfMatrix pathMat;

    // Distinction between different type of XObjects
    if( !xobjSubtype.compare( "Image" ) )
    {
        // Compute path rendering matrix.
        pathMat = vecGStates.back().transMat * m_pageImgTrans;
        m_pagePainter->setTransform( pathMat.toQTransform() );

        // Draw the image: corresponds to a rectangle (0,0,1,1).
        m_renderParameters.imagePB.setPenBrush( m_pagePainter );
        m_pagePainter->drawRect(  QRectF( 0.0, 0.0, 1.0, 1.0 ) );
    }
    else if( !xobjSubtype.compare( "Form" ) )
    {
        // Get transformation matrix in form's dictionary
        PdfMatrix formMat;
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
        pathMat = formMat * vecGStates.back().transMat * m_pageImgTrans;
        m_pagePainter->setTransform( pathMat.toQTransform() );

        // Draw the image: corresponds to a rectangle (0,0,1,1).
        m_renderParameters.formPB.setPenBrush( m_pagePainter );
        m_pagePainter->drawRect(  QRectF( bbox[0].GetReal(),
                                          bbox[1].GetReal(),
                                          bbox[2].GetReal()-bbox[0].GetReal(),
                                          bbox[3].GetReal()-bbox[1].GetReal() ) );
    }
}

void PRRenderPage::fMarkedContents( const PdfGraphicOperator& gOperator,
                                     const std::vector<std::string>& vecVariables,
                                     const std::vector<PdfGraphicsState>& vecGStates ) { }

void PRRenderPage::fCompatibility( const PdfGraphicOperator& gOperator,
                                    const std::vector<std::string>& vecVariables,
                                    const std::vector<PdfGraphicsState>& vecGStates ) { }


void PRRenderPage::getStringWidth( std::vector<WordWidth>& wordWidths,
                                    const PoDoFo::PdfString& str,
                                    PoDoFo::PdfFontMetrics* fontMetrics,
                                    double wordSpace )
{
    // Unicode string. Euhhhh, don't care...
    if( str.IsUnicode() ) {
        return;
    }

    double hScale = fontMetrics->GetFontScale() / 100.;
    const char* strData = str.GetString();
    size_t strLength = str.GetLength();
    double charWidth;

    // Init first word.
    wordWidths.push_back( WordWidth( 0, false ) );

    // Compute width for each char.
    for( size_t i = 0 ; i < strLength ; ++i)
    {
        // Get char width.
        charWidth = fontMetrics->CharWidth( strData[i] );

        // If space: create a specific word corresponding to it.
        if( strData[i] == ' ' ) {
            wordWidths.push_back( WordWidth( 0, true ) );
            wordWidths.back().width = charWidth + wordSpace * hScale;
            wordWidths.push_back( WordWidth( 0, false ) );
        }
        else {
            // Add char length to current word.
            wordWidths.back().width += charWidth;
        }
    }

    // Remove empty entries.
    std::vector<WordWidth>::iterator it;
    for( it = wordWidths.begin() ; it != wordWidths.end() ; ) {
        if( it->width == 0 ) {
            it = wordWidths.erase( it );
        }
        else {
            ++it;
        }
    }
}

}

