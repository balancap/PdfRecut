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

#ifndef PDFEGRAPHICSSTATE_H
#define PDFEGRAPHICSSTATE_H

#include "podofo/base/PdfDefines.h"
#include "podofo/base/PdfReference.h"

#include "PdfePath.h"
#include "PdfeResources.h"

namespace PoDoFo {
    class PdfPage;
}

namespace PoDoFoExtended {

/** Structure describing a text state in a Pdf stream.
 */
struct PdfeTextState
{
    /// Text transformation matrix.
    PdfeMatrix transMat;
    /// Line transformation matrix.
    PdfeMatrix lineTransMat;

    /// Font name (as used in Font resources).
    std::string fontName;
    /// Reference to the Pdf font object.
    PoDoFo::PdfReference fontRef;

    /// Font size.
    double fontSize;
    /// Character space.
    double charSpace;
    /// Word space.
    double wordSpace;
    /// Horizontal scale.
    double hScale;
    /// Leading.
    double leading;
    /// Render.
    int render;
    /// Rise.
    double rise;

    /** Default constructor: initialize values as described in Pdf reference.
     */
    PdfeTextState();

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
struct PdfeGraphicsState
{
    /// Transformation matrix (set with cm).
    PdfeMatrix transMat;

    /// Clipping path.
    PdfePath clippingPath;

    /// Pdf Text graphics state.
    PdfeTextState textState;

    /// Line width.
    double lineWidth;
    /// Line cap style.
    int lineCap;
    /// Line join style.
    int lineJoin;
    /// Compatibility mode.
    bool compatibilityMode;

    /** Default constructor.
     */
    PdfeGraphicsState();

    /** Function which initializes members to default Pdf values.
     */
    void init();

    /** Import parameters from ExtGState.
     * \param resources Resources where to find to the GState.
     * \param gsName Name of the graphics state to import.
     * \return True if the extGState was found and loaded. False else.
     */
    bool importExtGState( const PdfeResources& resources, const std::string& gsName );

    /** Import the Pdf reference corresponding to the font object (with name fontName).
     * \param resources Resources where to find the font reference.
     */
    bool importFontReference( const PdfeResources& resources );
};

}

#endif // PDFEGRAPHICSSTATE_H
