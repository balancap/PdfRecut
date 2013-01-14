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
#include <QtGui/QPainterPath>
#include <QtCore/QRectF>

#include "PdfeCanvasAnalysis.h"

namespace PoDoFo {
class PdfPage;
class PdfRect;
class PdfMemDocument;
class PdfFontMetrics;
class PdfString;
}

namespace PdfRecut {

class PRDocument;

//************************************************************//
//                         PRPenBrush                         //
//************************************************************//
/** Simple class that describes pen and brush used to respectively
 * draw and fill when an object is rendered on a page.
 */
class PRPenBrush
{
public:
    /** Basic constructor: set pen and brush to NULL objects.
     */
    PRPenBrush() :
        m_pPen( NULL ), m_pBrush( NULL ) {}
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
        delete m_pPen;
        delete m_pBrush;
        if( penBrush.m_pPen ) {
            m_pPen = new QPen( *penBrush.m_pPen );
        } else {
            m_pPen = NULL;
        }
        if( penBrush.m_pBrush ) {
            m_pBrush = new QBrush( *penBrush.m_pBrush );
        } else {
            m_pBrush = NULL;
        }
        return *this;
    }
    /** Destructor: delete pen and brush.
     */
    ~PRPenBrush()
    {
        delete m_pPen;
        delete m_pBrush;
    }
    /** Initialize to an empty object.
     */
    void init() {
        delete m_pPen;      m_pPen = NULL;
        delete m_pBrush;    m_pBrush = NULL;
    }
    /** Is the Pen/Brush empty, i.e. both pen and brush are set to NULL.
     */
    bool isEmpty() const
    {
        return !( m_pPen || m_pBrush );
    }
    /** Apply pen and brush to a painter.
     */
    void applyToPainter( QPainter* painter ) const
    {
        if( m_pPen ) {
            painter->setPen( *m_pPen );
        } else {
            painter->setPen( Qt::NoPen );
        }
        if( m_pBrush ) {
            painter->setBrush( *m_pBrush );
        } else {
            painter->setBrush( Qt::NoBrush );
        }
    }
public:
    // Getters...
    QPen* pen() const       {   return m_pPen;      }
    QBrush* brush() const   {   return m_pBrush;    }

    // Setters... Pointers given belongs to the PRPenBrush object.
    void setPen( QPen* pen ) {
        delete m_pPen;
        m_pPen = pen;
    }
    void setBrush( QBrush* brush ) {
        delete m_pBrush;
        m_pBrush = brush;
    }

private:
    /// Pointer to a draw pen object. If NULL, pen is not used.
    QPen*  m_pPen;
    /// Pointer to a fill brush object. If NULL, bursh is not used.
    QBrush* m_pBrush;
};

//************************************************************//
//                        PRRenderPage                        //
//************************************************************//
/** Class used as interface to render elements of PDF Page.
 * Can perform a basic rendering of all elements inside a page.
 */
class PRRenderPage : public PoDoFoExtended::PdfeCanvasAnalysis
{
public:
    /** Default constructor.
     * \param document Parent document object.
     * \param pageIndex Index of the page which is rendered.
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
    /** Get rendered image of the page.
     * \return Image.
     */
    QImage image();

public:
    /** Parameters used to render elements inside a page.
     */
    struct Parameters
    {
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
        Parameters();
        /** Initialize pen and brush to a default layout.
         */
        void initToDefault();
        /** Initialize to an empty layout.
         */
        void initToEmpty();
    };

    // Basic rendering of a page.
    /** Render the page, using given rendering parameters.
     * \param parameters Structure containing resolution and painting parameters.
     */
    void renderElements( const PRRenderPage::Parameters& parameters );

protected:
    // PdfeCanvasAnalysis interface.
    virtual void fGeneralGState( const PoDoFoExtended::PdfeStreamStateOld& streamState );
    virtual void fSpecialGState( const PoDoFoExtended::PdfeStreamStateOld& streamState );
    virtual void fPathConstruction( const PoDoFoExtended::PdfeStreamStateOld& streamState,
                                    const PoDoFoExtended::PdfePath& currentPath );
    virtual void fPathPainting( const PoDoFoExtended::PdfeStreamStateOld& streamState,
                                const PoDoFoExtended::PdfePath& currentPath );
    virtual void fClippingPath( const PoDoFoExtended::PdfeStreamStateOld& streamState,
                                const PoDoFoExtended::PdfePath& currentPath );
    virtual void fTextObjects( const PoDoFoExtended::PdfeStreamStateOld& streamState );
    virtual void fTextState( const PoDoFoExtended::PdfeStreamStateOld& streamState );
    virtual void fTextPositioning( const PoDoFoExtended::PdfeStreamStateOld& streamState );
    virtual PdfeVector fTextShowing( const PoDoFoExtended::PdfeStreamStateOld& streamState );
    virtual void fType3Fonts( const PoDoFoExtended::PdfeStreamStateOld& streamState );
    virtual void fColor( const PoDoFoExtended::PdfeStreamStateOld& streamState );
    virtual void fShadingPatterns( const PoDoFoExtended::PdfeStreamStateOld& streamState );
    virtual void fInlineImages( const PoDoFoExtended::PdfeStreamStateOld& streamState );
    virtual void fXObjects( const PoDoFoExtended::PdfeStreamStateOld& streamState );
    virtual void fMarkedContents( const PoDoFoExtended::PdfeStreamStateOld& streamState );
    virtual void fCompatibility( const PoDoFoExtended::PdfeStreamStateOld& streamState );
    virtual void fUnknown( const PoDoFoExtended::PdfeStreamStateOld& streamState ) {}
    virtual void fFormBegin( const PoDoFoExtended::PdfeStreamStateOld& streamState,
                             PoDoFo::PdfXObject* form ) {}
    virtual void fFormEnd( const PoDoFoExtended::PdfeStreamStateOld& streamState,
                           PoDoFo::PdfXObject* form ) {}

public:
    // Basic drawing routines. Operate in page coordinate system.
    /** Draw a painter path on the page.
     * \param path QPainterPath to draw.
     * \param penBrush Pen/Brush to use for rendering.
     * \param transMat Transformation matrix (identity by default).
     */
    void drawPath( QPainterPath path,
                   const PRPenBrush& penBrush,
                   const PdfeMatrix& transMat = PdfeMatrix() );
    /** Draw a painter path on the page.
     * \param image Qimage to draw.
     * \param rect Target rectangle inside the page.
     * \param transMat Transformation matrix (identity by default).
     */
    void drawImage( QImage image,
                    QRectF rect,
                    const PdfeMatrix& transMat = PdfeMatrix() );
    /** Draw a PdfeORect on the page.
     * \param orect Oriented rectangle to draw.
     * \param penBrush Pen/Brush to use for rendering.
     */
    void drawPdfeORect( const PdfeORect& orect,
                        const PRPenBrush& penBrush,
                        const PdfeMatrix& transMat = PdfeMatrix() );

protected:
    /** Test function on images.
     */
    static int imgNbs;
    void testPdfImage( PoDoFo::PdfObject* xobj );

private:
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
    /** Transform from page space to image space.
     * In particular, invert y-axis coordinate.
     */
    PdfeMatrix  m_pageImgTrans;

    /// Clipping paths stack. Used to store the clipping path in page coordinates.
    std::vector<QPainterPath>  m_clippingPathStack;

    /// Rendering parameters.
    Parameters  m_renderParameters;
};

//**********************************************************//
//                      Inline methods                      //
//**********************************************************//

}

#endif // PRRENDERPAGE_H
