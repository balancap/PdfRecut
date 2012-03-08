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

#include "PRStreamAnalysis.h"
#include "PdfStreamTokenizer.h"

#include "podofo/podofo.h"

using namespace PoDoFo;

namespace PdfRecut {

PRStreamAnalysis::PRStreamAnalysis( PoDoFo::PdfPage* page )
{
    // Set page analysed.
    m_page = page;

    // Set locale to english for istringstream.
    PdfLocaleImbue( m_iStrStream );
}

void PRStreamAnalysis::analyse()
{
    // Analyse main page.
    this->analyseCanvas( m_page,
                         PdfGraphicsState(),
                         PdfResources() );
}

void PRStreamAnalysis::analyseCanvas( PoDoFo::PdfCanvas* canvas,
                                      const PdfGraphicsState& initialGState,
                                      const PdfResources& initialResources )
{
    //Stream tokenizer and associated variables.
    PdfStreamTokenizer tokenizer( canvas );

    // Stream state.
    PdfStreamState streamState;
    // Current path.
    PdfPath currentPath;

    // Temporary variables.
    std::string tmpString;
    PdfMatrix tmpMat;
    PdfRect tmpRect;

    EPdfContentsType eType;
    std::string strVariant;

    // Initialize graphics state and resources.
    streamState.gStates.push_back( initialGState );
    tmpRect = canvas->GetPageSize();
    streamState.gStates.back().clippingPath.appendRectangle( PdfVector( tmpRect.GetLeft(), tmpRect.GetBottom() ),
                                                             PdfVector( tmpRect.GetWidth(), tmpRect.GetHeight() ) );

    streamState.canvas = canvas;
    streamState.resources = initialResources;
    streamState.resources.pushBack( canvas->GetResources() );

    // Analyse page stream / Also known as the big dirty loop !
    while( tokenizer.ReadNext( eType, streamState.gOperator, strVariant ) )
    {
        // References to have simpler notations...
        PdfGraphicOperator& gOperator = streamState.gOperator;
        std::vector<std::string>& gOperands = streamState.gOperands;
        PdfGraphicsState& gState = streamState.gStates.back();

        if ( eType == ePdfContentsType_Variant )
        {
            // Copy variant in the variables vector.
            gOperands.push_back( strVariant );
        }
        else if( eType == ePdfContentsType_Keyword )
        {
            // Distinction between operator categories.
            if( gOperator.cat == ePdfGCategory_GeneralGState )
            {
                // Commands in this category: w, J, j, M, d, ri, i, gs.

                if( gOperator.code ==ePdfGOperator_w ) {
                    // Get line width.
                    this->readValue( gOperands.back(), gState.lineWidth );
                }
                else if( gOperator.code ==ePdfGOperator_J ) {
                    // Get line cap.
                    this->readValue( gOperands.back(), gState.lineCap );
                }
                else if( gOperator.code ==ePdfGOperator_j ) {
                    // Get line join.
                    this->readValue( gOperands.back(), gState.lineJoin );
                }
                else if( gOperator.code ==ePdfGOperator_gs ) {
                    // Get parameters from an ExtGState dictionary.
                    tmpString = gOperands.back().substr( 1 );
                    gState.importExtGState( m_page, tmpString );
                }
                // Call category function.
                this->fGeneralGState( streamState );
            }
            else if( gOperator.cat == ePdfGCategory_SpecialGState )
            {
                // Commands in this category: q, Q, cm.

                if( gOperator.code == ePdfGOperator_q ) {
                    // Push on the graphics state stack.
                    streamState.gStates.push_back( gState );
                }
                else if( gOperator.code == ePdfGOperator_Q ) {
                    // Pop on the graphics state stack.
                    streamState.gStates.pop_back();
                }
                else if( gOperator.code == ePdfGOperator_cm ) {
                    // Get transformation matrix and compute new one.
                    size_t nbvars = gOperands.size();
                    tmpMat.init();

                    this->readValue( gOperands[nbvars-1], tmpMat(2,1) );
                    this->readValue( gOperands[nbvars-2], tmpMat(2,0) );
                    this->readValue( gOperands[nbvars-3], tmpMat(1,1) );
                    this->readValue( gOperands[nbvars-4], tmpMat(1,0) );
                    this->readValue( gOperands[nbvars-5], tmpMat(0,1) );
                    this->readValue( gOperands[nbvars-6], tmpMat(0,0) );

                    gState.transMat = tmpMat * gState.transMat;
                }
                // Call category function.
                this->fSpecialGState( streamState );
            }
            else if( gOperator.cat == ePdfGCategory_TextObjects )
            {
                // Commands in this category: BT, ET.

                // Initialize text transform matrices when op = "BT".
                if( gOperator.code == ePdfGOperator_BT ) {
                    gState.textState.initMatrices();
                }
                // Call category function.
                this->fTextObjects( streamState );
            }
            else if( gOperator.cat == ePdfGCategory_TextState )
            {
                // Commands in this category: Tc, Tw, Tz, TL, Tf, Tr, Ts.

                if( gOperator.code == ePdfGOperator_Tc ) {
                    // Read char space.
                    this->readValue( gOperands.back(), gState.textState.charSpace );
                }
                else if( gOperator.code == ePdfGOperator_Tw ) {
                    // Read word space.
                    this->readValue( gOperands.back(), gState.textState.wordSpace );
                }
                else if( gOperator.code == ePdfGOperator_Tz ) {
                    // Read horizontal scale.
                    this->readValue( gOperands.back(), gState.textState.hScale );
                }
                else if( gOperator.code == ePdfGOperator_TL ) {
                    // Read leading.
                    this->readValue( gOperands.back(), gState.textState.leading );
                }
                else if( gOperator.code == ePdfGOperator_Tf ) {
                    // Read font size and font name
                    size_t nbvars = gOperands.size();
                    this->readValue( gOperands[nbvars-1], gState.textState.fontSize );

                    // Remove leading '/' and get font reference.
                    tmpString = gOperands[nbvars-2];
                    gState.textState.fontName = tmpString.substr( 1 );
                    gState.importFontReference( m_page );
                }
                else if( gOperator.code == ePdfGOperator_Tr ) {
                    // Read render.
                    this->readValue( gOperands.back(), gState.textState.render );
                }
                else if( gOperator.code == ePdfGOperator_Ts ) {
                    // Read rise.
                    this->readValue( gOperands.back(), gState.textState.rise );
                }
                // Call category function.
                this->fTextState( streamState );
            }
            else if( gOperator.cat == ePdfGCategory_TextPositioning )
            {
                // Commands in this category: Td, TD, Tm, T*.

                if( gOperator.code == ePdfGOperator_Td ) {
                    // Read and compute text transformation matrix.
                    tmpMat.init();
                    size_t nbvars = gOperands.size();
                    this->readValue( gOperands[nbvars-1], tmpMat(2,1) );
                    this->readValue( gOperands[nbvars-2], tmpMat(2,0) );

                    gState.textState.lineTransMat = tmpMat * gState.textState.lineTransMat;
                    gState.textState.transMat = gState.textState.lineTransMat;
                }
                else if( gOperator.code == ePdfGOperator_TD ) {
                    // Read and compute text transformation matrix.
                    tmpMat.init();
                    size_t nbvars = gOperands.size();
                    this->readValue( gOperands[nbvars-1], tmpMat(2,1) );
                    this->readValue( gOperands[nbvars-2], tmpMat(2,0) );

                    gState.textState.lineTransMat = tmpMat * gState.textState.lineTransMat;
                    gState.textState.transMat = gState.textState.lineTransMat;

                    // New leading value.
                    gState.textState.leading = -tmpMat(2,1);
                }
                else if( gOperator.code == ePdfGOperator_Tm ) {
                    // Get transformation matrix new one.
                    size_t nbvars = gOperands.size();

                    this->readValue( gOperands[nbvars-1], tmpMat(2,1) );
                    this->readValue( gOperands[nbvars-2], tmpMat(2,0) );
                    this->readValue( gOperands[nbvars-3], tmpMat(1,1) );
                    this->readValue( gOperands[nbvars-4], tmpMat(1,0) );
                    this->readValue( gOperands[nbvars-5], tmpMat(0,1) );
                    this->readValue( gOperands[nbvars-6], tmpMat(0,0) );

                    gState.textState.transMat = tmpMat;
                    gState.textState.lineTransMat = tmpMat;
                }
                else if( gOperator.code == ePdfGOperator_Tstar ) {
                    // "T*" equivalent to "0 -Tl Td".
                    tmpMat.init();
                    tmpMat(2,1) = -gState.textState.leading;

                    gState.textState.lineTransMat = tmpMat * gState.textState.lineTransMat;
                    gState.textState.transMat = gState.textState.lineTransMat;
                }
                // Call category function.
                this->fTextPositioning( streamState );
            }
            else if( gOperator.cat == ePdfGCategory_TextShowing )
            {
                // Commands in this category: Tj, TJ, ', "

                //Modify text graphics state parameters when necessary.
                if( gOperator.code == ePdfGOperator_Quote ) {
                    // Corresponds to T*, Tj.
                    tmpMat.init();
                    tmpMat(2,1) = -gState.textState.leading;

                    gState.textState.lineTransMat = tmpMat * gState.textState.lineTransMat;
                    gState.textState.transMat = gState.textState.lineTransMat;
                }
                else if( gOperator.code == ePdfGOperator_DoubleQuote ) {
                    // Corresponds to Tw, Tc, Tj.
                    size_t nbvars = gOperands.size();
                    tmpMat.init();

                    this->readValue( gOperands[nbvars-1], gState.textState.charSpace );
                    this->readValue( gOperands[nbvars-2], gState.textState.wordSpace );
                }
                // Call category function.
                this->fTextShowing( streamState );
            }
            else if( gOperator.cat == ePdfGCategory_Color )
            {
                // Commands in this category: CS, cs, SC, SCN, sc, scn, G, g, RG, rg, K, k.
                // Not so much done in this category. Should maybe implement a bit more !

                // Call category function.
                this->fColor( streamState );
            }
            else if( gOperator.cat == ePdfGCategory_PathConstruction )
            {
                // Commands in this category: m, l, c, v, y, h, re.

                if( gOperator.code == ePdfGOperator_m ) {
                    // Begin a new subpath.
                    size_t nbvars = gOperands.size();
                    PdfVector point;

                    this->readValue( gOperands[nbvars-1], point(1) );
                    this->readValue( gOperands[nbvars-2], point(0) );

                    currentPath.beginSubpath( point );
                }
                else if( gOperator.code == ePdfGOperator_l ) {
                    // Append straight line.
                    size_t nbvars = gOperands.size();
                    PdfVector point;

                    this->readValue( gOperands[nbvars-1], point(1) );
                    this->readValue( gOperands[nbvars-2], point(0) );

                    currentPath.appendLine( point );
                }
                else if( gOperator.code == ePdfGOperator_c ) {
                    // Append Bézier curve (c).
                    size_t nbvars = gOperands.size();
                    PdfVector point1, point2, point3;

                    this->readValue( gOperands[nbvars-1], point3(1) );
                    this->readValue( gOperands[nbvars-2], point3(0) );
                    this->readValue( gOperands[nbvars-3], point2(1) );
                    this->readValue( gOperands[nbvars-4], point2(0) );
                    this->readValue( gOperands[nbvars-5], point1(1) );
                    this->readValue( gOperands[nbvars-6], point1(0) );

                    currentPath.appendBezierC( point1, point2, point3 );
                }
                else if( gOperator.code == ePdfGOperator_v ) {
                    // Append Bézier curve (c).
                    size_t nbvars = gOperands.size();
                    PdfVector point2, point3;

                    this->readValue( gOperands[nbvars-1], point3(1) );
                    this->readValue( gOperands[nbvars-2], point3(0) );
                    this->readValue( gOperands[nbvars-3], point2(1) );
                    this->readValue( gOperands[nbvars-4], point2(0) );

                    currentPath.appendBezierV( point2, point3 );
                }
                else if( gOperator.code == ePdfGOperator_y ) {
                    // Append Bézier curve (c).
                    size_t nbvars = gOperands.size();
                    PdfVector point1, point3;

                    this->readValue( gOperands[nbvars-1], point3(1) );
                    this->readValue( gOperands[nbvars-2], point3(0) );
                    this->readValue( gOperands[nbvars-3], point1(1) );
                    this->readValue( gOperands[nbvars-4], point1(0) );

                    currentPath.appendBezierY( point1, point3 );
                }
                else if( gOperator.code == ePdfGOperator_h ) {
                    // Close the current subpath by appending a straight line.
                    currentPath.closeSubpath();
                }
                else if( gOperator.code == ePdfGOperator_re ) {
                    // Append a rectangle to the current path as a complete subpath.
                    size_t nbvars = gOperands.size();
                    PdfVector llpoint, size;

                    this->readValue( gOperands[nbvars-1], size(1) );
                    this->readValue( gOperands[nbvars-2], size(0) );
                    this->readValue( gOperands[nbvars-3], llpoint(1) );
                    this->readValue( gOperands[nbvars-4], llpoint(0) );

                    currentPath.appendRectangle( llpoint, size );
                }
                // Call category function.
                this->fPathConstruction( streamState, currentPath );
            }
            else if( gOperator.cat == ePdfGCategory_PathPainting )
            {
                // Commands in this category: S, s, f, F, f*, B, B*, b, b*, n.

                // Call category function.
                this->fPathPainting( streamState, currentPath );

                // Clear the current path.
                currentPath.init();
            }
            else if( gOperator.cat == ePdfGCategory_ClippingPath )
            {
                // Commands in this category: W, W*.

                // Append the current path to the clipping path.
                gState.clippingPath.appendPath( currentPath );

                // Set the clipping path operator of the current path.
                currentPath.setClippingPathOp( gOperator.name );

                // Call category function.
                this->fClippingPath( streamState, currentPath );
            }
            else if( gOperator.cat == ePdfGCategory_Type3Fonts )
            {
                // Commands in this category: d0, d1.

                // Call category function.
                this->fType3Fonts( streamState );
            }
            else if( gOperator.cat == ePdfGCategory_ShadingPatterns )
            {
                // Commands in this category: sh.

                // Call category function.
                this->fShadingPatterns( streamState );
            }
            else if( gOperator.cat == ePdfGCategory_InlineImages )
            {
                // Commands in this category: BI, ID, EI.

                // Call category function.
                this->fInlineImages( streamState );
            }
            else if( gOperator.cat == ePdfGCategory_XObjects )
            {
                // Commands in this category: Do.

                // Call category function.
                this->fXObjects( streamState );
            }
            else if( gOperator.cat == ePdfGCategory_Compatibility )
            {
                // Commands in this category: BX, EX.

                if( gOperator.code == ePdfGOperator_BX ) {
                    // Activate compatibility mode.
                    gState.compatibilityMode = true;
                }
                else if( gOperator.code == ePdfGOperator_EX ) {
                    // Deactivate compatibility mode.
                    gState.compatibilityMode = false;
                }

                // Call category function.
                this->fCompatibility( streamState );
            }
            else if( gOperator.cat == ePdfGCategory_Unknown )
            {
                if( !gState.compatibilityMode ) {
                    PODOFO_RAISE_ERROR_INFO( ePdfError_InvalidContentStream,
                                             "Invalid token in a stream" );
                }
            }
            // Clear variables vector.
            gOperands.clear();
        }
        else if ( eType == ePdfContentsType_ImageData )
        {
            // Copy inline image data in the variables vector.
            gOperands.push_back( strVariant );
        }
    }
}

}
