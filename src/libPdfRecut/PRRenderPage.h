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

#include "PRStreamAnalysis.h"
#include "PdfFontMetricsCache.h"

#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QBrush>

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
            if( fillBrush ) {
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
        void setPenBrush( QPainter* painter )
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

    /** Pen and brush used to draw and fill text objects.
     */
    PRPenBrush textPB;

    /** Pen and brush used to draw and fill text spaces.
     */
    PRPenBrush textSpacePB;

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
    void initPenBrush();
};

/** Class used to obtain a basic render a Pdf page.
 */
class PRRenderPage : public PRStreamAnalysis
{
public:
    /** Default constructor.
     * \param pageIn Input page to draw.
     */
    PRRenderPage( PoDoFo::PdfPage* pageIn,
                  PdfFontMetricsCache* fontMetricsCache );

    /** Destructor: release image resources.
     */
    ~PRRenderPage();

    /** Render the page, using given parameters.
     * \param parameters Structure containing resolution and painting parameters.
     */
    void renderPage( const PRRenderParameters& parameters );

    /** Save page into a file.
     * \param filename File where to save the page.
     */
    void saveToFile( const QString& filename );


    void fGeneralGState( const PdfStreamState& streamState );

    void fSpecialGState( const PdfStreamState& streamState );

    void fPathConstruction( const PdfStreamState& streamState,
                            const PdfPath& currentPath );

    void fPathPainting( const PdfStreamState& streamState,
                        const PdfPath& currentPath );

    void fClippingPath( const PdfStreamState& streamState,
                        const PdfPath& currentPath );

    void fTextObjects( const PdfStreamState& streamState );

    void fTextState( const PdfStreamState& streamState );

    void fTextPositioning( const PdfStreamState& streamState );

    void fTextShowing( const PdfStreamState& streamState );

    void fType3Fonts( const PdfStreamState& streamState );

    void fColor( const PdfStreamState& streamState );

    void fShadingPatterns( const PdfStreamState& streamState );

    void fInlineImages( const PdfStreamState& streamState );

    void fXObjects( const PdfStreamState& streamState );

    void fMarkedContents( const PdfStreamState& streamState );

    void fCompatibility( const PdfStreamState& streamState );


private:

    /** Structure representing the length of a word.
     */
    struct WordWidth
    {
        /** Width...
         */
        double width;

        /** Is it an empty space ?
         */
        bool isSpace;

        WordWidth( double _width, bool _isSpace ) :
            width(_width), isSpace(_isSpace) { }
    };

    /** Compute a string width given a PdfFontMetrics object.
     * Enhance and correct the version given in PoDoFo.
     */
    static void getStringWidth( std::vector<WordWidth>& wordWidths,
                                const PoDoFo::PdfString& str,
                                PoDoFo::PdfFontMetrics* fontMetrics,
                                double wordSpace );

    static int imgNbs;
    void testPdfImage( PoDoFo::PdfObject* xobj );


private:
    /** Document object.
     */
    PoDoFo::PdfMemDocument* m_document;

    /** Font metrics cache.
     */
    PdfFontMetricsCache* m_fontMetricsCache;

    /** QImage used to draw the page.
     */
    QImage* m_pageImage;

    /** QPainter used to draw the page.
     */
    QPainter* m_pagePainter;

    /** Render parameters.
     */
    PRRenderParameters m_renderParameters;

    /** PdfRect corresponding to the rectangle of the page drawn.
     * Usually page crop box.
     */
    PoDoFo::PdfRect m_pageRect;

    /** Transform from page space to image space.
     * In particular, invert y-axis coordinate.
     */
    PdfMatrix m_pageImgTrans;

    /** Text transform.
     */
    PdfMatrix m_textMatrix;
};

}

#endif // PRRENDERPAGE_H
