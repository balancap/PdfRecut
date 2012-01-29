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

#ifndef PDFRENDERPAGE_H
#define PDFRENDERPAGE_H

#include "PdfStreamAnalysis.h"
#include "PdfFontMetricsCache.h"
#include <QtGui/QImage>

namespace PoDoFo {
    class PdfPage;
    class PdfRect;
    class PdfMemDocument;
    class PdfFontMetrics;
    class PdfString;
}

namespace PdfeBooker {

/** Class used to obtain a basic render a Pdf page.
 */
class PdfRenderPage : public PdfStreamAnalysis
{
public:
    /** Default constructor.
     * \param pageIn Input page to draw.
     */
    PdfRenderPage( PoDoFo::PdfPage* pageIn,
                   PdfFontMetricsCache* fontMetricsCache );

    /** Render the page, using given parameters.
     */
    void renderPage( double resolution );

    /** Save page into a file.
     * \param filename File where to save the page.
     */
    void saveToFile( const QString& filename );

    void fGeneralGState( const PdfGraphicOperator& gOperator,
                         const std::vector<std::string>& vecVariables,
                         const std::vector<PdfGraphicsState>& vecGStates );

    void fSpecialGState( const PdfGraphicOperator& gOperator,
                         const std::vector<std::string>& vecVariables,
                         const std::vector<PdfGraphicsState>& vecGStates );

    void fPathConstruction( const PdfGraphicOperator& gOperator,
                            const std::vector<std::string>& vecVariables,
                            const std::vector<PdfGraphicsState>& vecGStates,
                            const PdfPath& currentPath );

    void fPathPainting( const PdfGraphicOperator& gOperator,
                        const std::vector<std::string>& vecVariables,
                        const std::vector<PdfGraphicsState>& vecGStates,
                        const PdfPath& currentPath );

    void fClippingPath( const PdfGraphicOperator& gOperator,
                        const std::vector<std::string>& vecVariables,
                        const std::vector<PdfGraphicsState>& vecGStates,
                        const PdfPath& currentPath );

    void fTextObjects( const PdfGraphicOperator& gOperator,
                       const std::vector<std::string>& vecVariables,
                       const std::vector<PdfGraphicsState>& vecGStates );

    void fTextState( const PdfGraphicOperator& gOperator,
                     const std::vector<std::string>& vecVariables,
                     const std::vector<PdfGraphicsState>& vecGStates );

    void fTextPositioning( const PdfGraphicOperator& gOperator,
                           const std::vector<std::string>& vecVariables,
                           const std::vector<PdfGraphicsState>& vecGStates );

    void fTextShowing( const PdfGraphicOperator& gOperator,
                       const std::vector<std::string>& vecVariables,
                       const std::vector<PdfGraphicsState>& vecGStates );

    void fType3Fonts( const PdfGraphicOperator& gOperator,
                      const std::vector<std::string>& vecVariables,
                      const std::vector<PdfGraphicsState>& vecGStates );

    void fColor( const PdfGraphicOperator& gOperator,
                 const std::vector<std::string>& vecVariables,
                 const std::vector<PdfGraphicsState>& vecGStates );

    void fShadingPatterns( const PdfGraphicOperator& gOperator,
                           const std::vector<std::string>& vecVariables,
                           const std::vector<PdfGraphicsState>& vecGStates );

    void fInlineImages( const PdfGraphicOperator& gOperator,
                        const std::vector<std::string>& vecVariables,
                        const std::vector<PdfGraphicsState>& vecGStates );

    void fXObjects( const PdfGraphicOperator& gOperator,
                    const std::vector<std::string>& vecVariables,
                    const std::vector<PdfGraphicsState>& vecGStates );

    void fMarkedContents( const PdfGraphicOperator& gOperator,
                          const std::vector<std::string>& vecVariables,
                          const std::vector<PdfGraphicsState>& vecGStates );

    void fCompatibility( const PdfGraphicOperator& gOperator,
                         const std::vector<std::string>& vecVariables,
                         const std::vector<PdfGraphicsState>& vecGStates );

public:
    /** Compute a string width given a PdfFontMetrics object.
     * Enhance and correct the version given in PoDoFo.
     */
    static double getStringWidth( const PoDoFo::PdfString& str,
                                  PoDoFo::PdfFontMetrics* fontMetrics,
                                  double wordSpace );

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

#endif // PDFRENDERPAGE_H
