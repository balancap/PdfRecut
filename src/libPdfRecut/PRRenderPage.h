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

#ifndef PRRENDERPAGE_H
#define PRRENDERPAGE_H

#include <QtGui/QPainter>

#include "PdfeCanvasAnalysis.h"

#include "PRDocument.h"
#include "PRTextWords.h"

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
        /// Pointer to a draw pen object (belongs to the structure). If NULL, object is not drawned.
        QPen* drawPen;
        /// Pointer to a fill brush object (belongs to the structure). If NULL, object is not filled.
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
        bool isPainting() const
        {
            return ( drawPen || fillBrush );
        }
        /** Set pen and brush to a painter.
         */
        void applyToPainter( QPainter* painter ) const
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

    /// Resolution used to render a page (default = 1.0).
    double resolution;
    /// Initial clipping path used to render the page.
    QPainterPath clippingPath;

    /// Pen and brush used to draw and fill text objects.
    PRPenBrush textPB;
    /// Pen and brush used to draw and fill text spaces.
    PRPenBrush textSpacePB;
    /// Pen and brush used to draw and fill PDF text translation characters.
    PRPenBrush textPDFTranslationPB;

    /// Pen and brush used to draw and fill path objects.
    PRPenBrush pathPB;
    /// Pen and brush used to draw and fill clipping path objects.
    PRPenBrush clippingPathPB;

    /// Pen and brush used to draw and fill inline images.
    PRPenBrush inlineImagePB;
    /// Pen and brush used to draw and fill images (xobjects).
    PRPenBrush imagePB;
    /// Pen and brush used to draw and fill forms (xobjects).
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
class PRRenderPage : public PoDoFoExtended::PdfeCanvasAnalysis
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

    /** Initialize the rendering of the page, with a given resolution.
     * \param resolution Resolution to use for the rendering.
     */
    void initRendering( double resolution );
    /** Clear rendering components (image and painter).
     */
    void clearRendering();

    /** Render the page, using given rendering parameters.
     * \param parameters Structure containing resolution and painting parameters.
     */
    void render( const PRRenderParameters& parameters );

    /** Get rendered image of the page.
     * \return Image.
     */
    QImage image();

protected:
    virtual void fGeneralGState( const PoDoFoExtended::PdfeStreamState& streamState );

    virtual void fSpecialGState( const PoDoFoExtended::PdfeStreamState& streamState );

    virtual void fPathConstruction( const PoDoFoExtended::PdfeStreamState& streamState,
                                    const PoDoFoExtended::PdfePath& currentPath );

    virtual void fPathPainting( const PoDoFoExtended::PdfeStreamState& streamState,
                                const PoDoFoExtended::PdfePath& currentPath );

    virtual void fClippingPath( const PoDoFoExtended::PdfeStreamState& streamState,
                                const PoDoFoExtended::PdfePath& currentPath );

    virtual void fTextObjects( const PoDoFoExtended::PdfeStreamState& streamState );

    virtual void fTextState( const PoDoFoExtended::PdfeStreamState& streamState );

    virtual void fTextPositioning( const PoDoFoExtended::PdfeStreamState& streamState );

    virtual PdfeVector fTextShowing( const PoDoFoExtended::PdfeStreamState& streamState );

    virtual void fType3Fonts( const PoDoFoExtended::PdfeStreamState& streamState );

    virtual void fColor( const PoDoFoExtended::PdfeStreamState& streamState );

    virtual void fShadingPatterns( const PoDoFoExtended::PdfeStreamState& streamState );

    virtual void fInlineImages( const PoDoFoExtended::PdfeStreamState& streamState );

    virtual void fXObjects( const PoDoFoExtended::PdfeStreamState& streamState );

    virtual void fMarkedContents( const PoDoFoExtended::PdfeStreamState& streamState );

    virtual void fCompatibility( const PoDoFoExtended::PdfeStreamState& streamState );

    virtual void fUnknown( const PoDoFoExtended::PdfeStreamState& streamState ) {}

    virtual void fFormBegin( const PoDoFoExtended::PdfeStreamState& streamState,
                             PoDoFo::PdfXObject* form ) {}

    virtual void fFormEnd( const PoDoFoExtended::PdfeStreamState& streamState,
                           PoDoFo::PdfXObject* form ) {}


public:
    // Specific drawing functions.
    /** Draw a PdfeORect on the page.
     * \param orect Oriented rectangle to draw.
     * \param penBrush Pen/Brush to use for rendering.
     */
    void drawPdfeORect( const PdfeORect& orect,
                        const PRRenderParameters::PRPenBrush& penBrush );

    /** Draw a group of words on the page.
     * \param groupWords Group of words to draw.
     * \param parameters Rendering parameters to use.
     */
    void textDrawGroup( const PRTextGroupWords& groupWords,
                        const PRRenderParameters& parameters );
    /** Draw a subgroup of words on the pdf image painter.
     * \param subgroup Subgroup of words to draw.
     * \param parameters Rendering parameters to use.
     */
    void textDrawSubgroup( const PRTextGroupWords::Subgroup& subgroup,
                           const PRRenderParameters& parameters );
    /** Draw main subgroups of a PRTextGroupWords.
     * \param subgroup Group of words to consider.
     * \param parameters Rendering parameters to use.
     */
    void textDrawMainSubgroups( const PRTextGroupWords& groupWords,
                                const PRRenderParameters& parameters );

    /** Render glyphs from a group of words on the page.
     * \param groupWords Group of words to render.
     */
    void textRenderGroup( const PRTextGroupWords& groupWords );

protected:
    /** Test function on images.
     */
    static int imgNbs;
    void testPdfImage( PoDoFo::PdfObject* xobj );

protected:
    /// Pointer to PRDocument object.
    PRDocument*  m_document;
    /// Page pointer.
    PoDoFo::PdfPage*  m_page;
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
};

//**********************************************************//
//                      Inline methods                      //
//**********************************************************//

}

#endif // PRRENDERPAGE_H
