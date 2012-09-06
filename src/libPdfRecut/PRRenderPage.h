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

#ifndef PRRENDERPAGE_H
#define PRRENDERPAGE_H

#include <QtGui/QPainter>

#include "PRDocument.h"
#include "PRStreamAnalysis.h"
#include "PRTextElements.h"

namespace PoDoFo {
    class PdfPage;
    class PdfRect;
    class PdfMemDocument;
    class PdfFontMetrics;
    class PdfString;
}

namespace PdfRecut {

/** Parameters used to render a Pdf page.
 */
struct PRRenderParameters
{
    /** Simple structure which contains pen and brush objects
     * used to respectively draw and fill.
     */
    struct PRPenBrush
    {
        /** Pointer to a draw pen object (belongs to the structure).
         * If NULL, object is not drawned.
         */
        QPen* drawPen;
        /** Pointer to a fill brush object (belongs to the structure).
         * If NULL, object is not filled.
         */
        QBrush* fillBrush;

        /** Basic constructor: set pointers to NULL.
         */
        PRPenBrush() :
            drawPen( NULL ), fillBrush( NULL ) {}

        /** Copy constructor.
         */
        PRPenBrush( const PRPenBrush& penBrush )
        {
            (*this) = penBrush;
        }
        /** Operator=.
         */
        PRPenBrush& operator=( const PRPenBrush& penBrush )
        {
            delete drawPen;
            delete fillBrush;
            if( penBrush.drawPen ) {
                drawPen = new QPen( *penBrush.drawPen );
            } else {
                drawPen = NULL;
            }
            if( penBrush.fillBrush ) {
                fillBrush = new QBrush( *penBrush.fillBrush );
            } else {
                fillBrush = NULL;
            }
            return *this;
        }
        /** Destructor: delete pen and brush.
         */
        ~PRPenBrush()
        {
            delete drawPen;
            delete fillBrush;
        }

        /** Is painting necessary ?
         */
        bool isPainting()
        {
            return (drawPen || fillBrush);
        }
        /** Set pen and brush to a painter.
         */
        void applyToPainter( QPainter* painter )
        {
            if( drawPen ) {
                painter->setPen( *drawPen );
            } else {
                painter->setPen( Qt::NoPen );
            }
            if( fillBrush ) {
                painter->setBrush( *fillBrush );
            } else {
                painter->setBrush( Qt::NoBrush );
            }
        }
    };

    /** Resolution used to render a page (default = 1.0).
     */
    double resolution;
    /** Initial clipping path used to render the page.
     */
    QPainterPath clippingPath;

    /** Pen and brush used to draw and fill text objects.
     */
    PRPenBrush textPB;
    /** Pen and brush used to draw and fill text spaces.
     */
    PRPenBrush textSpacePB;
    /** Pen and brush used to draw and fill PDF text spaces.
     */
    PRPenBrush textPDFSpacePB;

    /** Pen and brush used to draw and fill path objects.
     */
    PRPenBrush pathPB;
    /** Pen and brush used to draw and fill clipping path objects.
     */
    PRPenBrush clippingPathPB;

    /** Pen and brush used to draw and fill inline images.
     */
    PRPenBrush inlineImagePB;
    /** Pen and brush used to draw and fill images (xobjects).
     */
    PRPenBrush imagePB;
    /** Pen and brush used to draw and fill forms (xobjects).
     */
    PRPenBrush formPB;

public:
    /** Constructor, initialize members to default values.
     */
    PRRenderParameters();

    /** Initialize pen and brush to a default layout.
     */
    void initToDefault();

    /** Initialize to an empty layout.
     */
    void initToEmpty();
};

/** Class used to obtain a basic render a Pdf page.
 */
class PRRenderPage : public PRStreamAnalysis
{
public:
    /** Default constructor.
     * \param document Input document.
     * \param pageIndex Index of the page to render.
     */
    PRRenderPage( PRDocument* document,
                  long pageIndex );

    /** Destructor: release image resources.
     */
    virtual ~PRRenderPage();

    /** Render the page, using given parameters.
     * \param parameters Structure containing resolution and painting parameters.
     */
    void renderPage( const PRRenderParameters& parameters );

    /** Get rendering image.
     * \return Image pointer.
     */
    QImage* getRenderImage();

    /** Clear image and painter.
     */
    void clearPageImage();

    /** Save page image into a file.
     * \param filename File where to save the page.
     */
    void saveToFile( const QString& filename );

    virtual void fGeneralGState( const PdfStreamState& streamState );

    virtual void fSpecialGState( const PdfStreamState& streamState );

    virtual void fPathConstruction( const PdfStreamState& streamState,
                            const PdfPath& currentPath );

    virtual void fPathPainting( const PdfStreamState& streamState,
                        const PdfPath& currentPath );

    virtual void fClippingPath( const PdfStreamState& streamState,
                        const PdfPath& currentPath );

    virtual void fTextObjects( const PdfStreamState& streamState );

    virtual void fTextState( const PdfStreamState& streamState );

    virtual void fTextPositioning( const PdfStreamState& streamState );

    virtual void fTextShowing( const PdfStreamState& streamState );

    virtual void fType3Fonts( const PdfStreamState& streamState );

    virtual void fColor( const PdfStreamState& streamState );

    virtual void fShadingPatterns( const PdfStreamState& streamState );

    virtual void fInlineImages( const PdfStreamState& streamState );

    virtual void fXObjects( const PdfStreamState& streamState );

    virtual void fMarkedContents( const PdfStreamState& streamState );

    virtual void fCompatibility( const PdfStreamState& streamState );

    virtual void fFormBegin( const PdfStreamState& streamState,
                             PoDoFo::PdfXObject* form ) {}

    virtual void fFormEnd( const PdfStreamState& streamState,
                           PoDoFo::PdfXObject* form ) {}

    /** Convert Image coordinates to Page coordinates.
     * \param imgCoord Coordinates of a pixel in the image.
     * \return Coordinates in the page of the pixel center.
     */
    PdfeVector fromImagetoPageCoord( const QPoint& imgCoord );

    /** Convert Page coordinates to Image coordinates.
     * \param pageCoord Coordinates in the page of a point.
     * return Coordinates of the pixel which contains the point in the image.
     */
    QPoint fromPagetoImageCoord( const PdfeVector& pageCoord );

    /** Convert Image rectangle to page coordinates.
     * \param imgRect Rectangle in image coordinates.
     * \return Rectangle in page coordinates.
     */
    PoDoFo::PdfRect fromImagetoPageCoord( const QRect& imgRect );

    /** Convert Page rectangle to image coordinates.
     * \param pageRect Rectangle in page coordinates.
     * \return Rectangle in image coordinates.
     */
    QRect fromPagetoImageCoord( const PoDoFo::PdfRect& pageRect );

protected:

    /** Read a group of words from a pdf stream state.
     * \param streamState Stream state to consider.
     * \return Group of words read.
     */
    PRTextGroupWords textReadGroupWords( const PdfStreamState& streamState );

    /** Draw a group of words on the pdf image painter.
     * \param groupWords Words to draw.
     */
    void textDrawGroupWords( const PRTextGroupWords& groupWords );

    /** Update text transformation matrix. if necessary.
     * \param streamState Current tream state to consider.
     */
    void textUpdateTransMatrix( const PdfStreamState& streamState );

protected:
    /** Test function on images.
     */
    static int imgNbs;
    void testPdfImage( PoDoFo::PdfObject* xobj );

protected:
    /// Pointer to PRDocument object.
    PRDocument*  m_document;
    /// Page index.
    long  m_pageIndex;

    /// PdfRect corresponding to the rectangle of the page drawn (usually page crop box).
    PoDoFo::PdfRect  m_pageRect;

    /// QImage in which is drawn the page.
    QImage*  m_pageImage;
    /// QPainter used to draw the page.
    QPainter*  m_pagePainter;

    /// Rendering parameters.
    PRRenderParameters  m_renderParameters;

    /// Clipping paths stack. Used to store the clipping path in page coordinates.
    std::vector<QPainterPath>  m_clippingPathStack;

    /** Transform from page space to image space.
     * In particular, invert y-axis coordinate.
     */
    PdfeMatrix  m_pageImgTrans;
    /// Text transform.
    PdfeMatrix  m_textMatrix;

    /// Number of text groups read.
    long  m_nbTextGroups;
};

//**********************************************************//
//                      Inline methods                      //
//**********************************************************//
inline PdfeVector PRRenderPage::fromImagetoPageCoord( const QPoint& imgCoord )
{
    PdfeVector pageCoord;
    pageCoord(0) = ( imgCoord.x() + 0.5 ) / m_renderParameters.resolution + m_pageRect.GetLeft();
    pageCoord(1) = ( m_pageImage->height() - 0.5 - imgCoord.y() ) / m_renderParameters.resolution + m_pageRect.GetBottom();

    return pageCoord;
}
inline QPoint PRRenderPage::fromPagetoImageCoord( const PdfeVector& pageCoord )
{
    QPoint imgCoord;
    imgCoord.rx() = floor( ( pageCoord(0) - m_pageRect.GetLeft() ) * m_renderParameters.resolution );
    imgCoord.ry() = floor( ( m_pageRect.GetHeight() - pageCoord(1) + m_pageRect.GetLeft() ) * m_renderParameters.resolution );

    return imgCoord;
}
inline PoDoFo::PdfRect PRRenderPage::fromImagetoPageCoord( const QRect& imgRect )
{
    PoDoFo::PdfRect pageRect;
    pageRect.SetLeft( ( imgRect.x() + 0.5 ) / m_renderParameters.resolution + m_pageRect.GetLeft() );
    pageRect.SetBottom( ( m_pageImage->height() - 0.5 - imgRect.y() ) / m_renderParameters.resolution + m_pageRect.GetBottom() );
    pageRect.SetWidth( imgRect.width() / m_renderParameters.resolution );
    pageRect.SetHeight( imgRect.height() / m_renderParameters.resolution );

    return pageRect;
}
inline QRect PRRenderPage::fromPagetoImageCoord( const PoDoFo::PdfRect& pageRect )
{
    QRect imgRect;
    imgRect.setLeft( floor( ( pageRect.GetLeft() - m_pageRect.GetLeft() ) * m_renderParameters.resolution ) );
    imgRect.setTop( floor( ( m_pageRect.GetHeight() - pageRect.GetBottom() + m_pageRect.GetLeft() ) * m_renderParameters.resolution ) );
    imgRect.setWidth( pageRect.GetWidth() * m_renderParameters.resolution );
    imgRect.setHeight( pageRect.GetHeight() * m_renderParameters.resolution );

    return imgRect;
}

}

#endif // PRRENDERPAGE_H
