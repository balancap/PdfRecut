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

#ifndef PDFGRAPHICSSTATE_H
#define PDFGRAPHICSSTATE_H

#include "podofo/base/PdfDefines.h"
#include "podofo/base/PdfReference.h"
#include "PdfPath.h"

namespace PoDoFo {
    class PdfPage;
}

namespace PdfRecut {

/** Structure describing a text state in a Pdf stream.
 */
struct PdfTextState
{
    /** Text transformation matrix.
     */
    PdfMatrix transMat;

    /** Line transformation matrix.
     */
    PdfMatrix lineTransMat;

    /** Font name (as used in Font resources).
     */
    std::string fontName;

    /** Reference to the Pdf font object.
     */
    PoDoFo::PdfReference fontRef;

    /** Font size.
     */
    double fontSize;

    /** Character space.
     */
    double charSpace;

    /** Word space.
     */
    double wordSpace;

    /** Horizontal scale.
     */
    double hScale;

    /** Leading.
     */
    double leading;

    /** Render.
     */
    int render;

    /** Rise.
     */
    double rise;

    /** Default constructor: initialize values as described in Pdf reference.
     */
    PdfTextState();

    /** Function which initializes members to default Pdf values.
     */
    void init();

    /** Initialize only transformation matrices (used with BT/ET operators).
     */
    void initMatrices();
};

/** Structure describing a graphics state in a Pdf stream.
 * This structure does not reproduce all properties described the Pdf reference.
 */
struct PdfGraphicsState
{
    /** Transformation matrix (set with cm).
     */
    PdfMatrix transMat;

    /** Clipping path rectangle.
     */
    PdfPath clippingPath;

    /** Pdf Text graphics state.
     */
    PdfTextState textState;

    /** Line width.
     */
    double lineWidth;

    /** Line cap style.
     */
    int lineCap;

    /** Line join style.
     */
    int lineJoin;

    /** Compatibility mode.
     */
    bool compatibilityMode;

    /** Default constructor.
     */
    PdfGraphicsState();

    /** Function which initializes members to default Pdf values.
     */
    void init();

    /** Import parameters from ExtGState.
     * \param page Page where to find to ExtGState resources.
     * \param gsName Name of the graphics state to import.
     * \return True if the extGState was found and loaded. False else.
     */
    bool importExtGState( PoDoFo::PdfPage* page, const std::string& gsName );

    /** Import the Pdf reference corresponding to the font object (with name fontName)
     * in page resources.
     * \param page Page where to obtain the reference.
     */
    bool importFontReference( PoDoFo::PdfPage* page );
};

}

#endif // PDFGRAPHICSSTATE_H
