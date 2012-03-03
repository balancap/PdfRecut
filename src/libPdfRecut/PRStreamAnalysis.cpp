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
    //Stream tokenizer and associated variables.
    PdfStreamTokenizer tokenizer( m_page );

    EPdfContentsType eType;
    PdfGraphicOperator gOperator;
    std::string strVariant;

    // Graphics and variables vectors.
    std::vector<PdfGraphicsState> vecGStates;
    std::vector<std::string> vecVariables;

    // Current path.
    PdfPath currentPath;

    // Temporary variables.
    std::string tmpString;
    PdfMatrix tmpMat;
    PdfRect tmpRect;

    // Initialize graphic state.
    vecGStates.push_back( PdfGraphicsState() );
    tmpRect = m_page->GetMediaBox();
    vecGStates.back().clippingPath.appendRectangle( PdfVector( tmpRect.GetLeft(), tmpRect.GetBottom() ),
                                                    PdfVector( tmpRect.GetWidth(), tmpRect.GetHeight() ) );

    // Analyse page stream / Also known as the big dirty loop !
    while( tokenizer.ReadNext( eType, gOperator, strVariant ) )
    {
        if ( eType == ePdfContentsType_Variant )
        {
            // Copy variant in the variables vector.
            vecVariables.push_back( strVariant );
        }
        else if( eType == ePdfContentsType_Keyword )
        {
            // Distinction between operator categories.
            if( gOperator.cat == ePdfGCategory_GeneralGState )
            {
                // Commands in this category: w, J, j, M, d, ri, i, gs.

                if( gOperator.code ==ePdfGOperator_w ) {
                    // Get line width.
                    this->readValue( vecVariables.back(), vecGStates.back().lineWidth );
                }
                else if( gOperator.code ==ePdfGOperator_J ) {
                    // Get line cap.
                    this->readValue( vecVariables.back(), vecGStates.back().lineCap );
                }
                else if( gOperator.code ==ePdfGOperator_j ) {
                    // Get line join.
                    this->readValue( vecVariables.back(), vecGStates.back().lineJoin );
                }
                else if( gOperator.code ==ePdfGOperator_gs ) {
                    // Get parameters from an ExtGState dictionary.
                    tmpString = vecVariables.back().substr( 1 );
                    vecGStates.back().importExtGState( m_page, tmpString );
                }
                // Call category function.
                this->fGeneralGState( gOperator, vecVariables, vecGStates );
            }
            else if( gOperator.cat == ePdfGCategory_SpecialGState )
            {
                // Commands in this category: q, Q, cm.

                if( gOperator.code == ePdfGOperator_q ) {
                    // Push on the graphics state stack.
                    vecGStates.push_back( vecGStates.back() );
                }
                else if( gOperator.code == ePdfGOperator_Q ) {
                    // Pop on the graphics state stack.
                    vecGStates.pop_back();
                }
                else if( gOperator.code == ePdfGOperator_cm ) {
                    // Get transformation matrix and compute new one.
                    size_t nbvars = vecVariables.size();
                    tmpMat.init();

                    this->readValue( vecVariables[nbvars-1], tmpMat(2,1) );
                    this->readValue( vecVariables[nbvars-2], tmpMat(2,0) );
                    this->readValue( vecVariables[nbvars-3], tmpMat(1,1) );
                    this->readValue( vecVariables[nbvars-4], tmpMat(1,0) );
                    this->readValue( vecVariables[nbvars-5], tmpMat(0,1) );
                    this->readValue( vecVariables[nbvars-6], tmpMat(0,0) );

                    vecGStates.back().transMat = tmpMat * vecGStates.back().transMat;
                }
                // Call category function.
                this->fSpecialGState( gOperator, vecVariables, vecGStates );
            }
            else if( gOperator.cat == ePdfGCategory_TextObjects )
            {
                // Commands in this category: BT, ET.

                // Initialize text transform matrices when op = "BT".
                if( gOperator.code == ePdfGOperator_BT ) {
                    vecGStates.back().textState.initMatrices();
                }
                // Call category function.
                this->fTextObjects( gOperator, vecVariables, vecGStates );
            }
            else if( gOperator.cat == ePdfGCategory_TextState )
            {
                // Commands in this category: Tc, Tw, Tz, TL, Tf, Tr, Ts.

                if( gOperator.code == ePdfGOperator_Tc ) {
                    // Read char space.
                    this->readValue( vecVariables.back(), vecGStates.back().textState.charSpace );
                }
                else if( gOperator.code == ePdfGOperator_Tw ) {
                    // Read word space.
                    this->readValue( vecVariables.back(), vecGStates.back().textState.wordSpace );
                }
                else if( gOperator.code == ePdfGOperator_Tz ) {
                    // Read horizontal scale.
                    this->readValue( vecVariables.back(), vecGStates.back().textState.hScale );
                }
                else if( gOperator.code == ePdfGOperator_TL ) {
                    // Read leading.
                    this->readValue( vecVariables.back(), vecGStates.back().textState.leading );
                }
                else if( gOperator.code == ePdfGOperator_Tf ) {
                    // Read font size and font name
                    size_t nbvars = vecVariables.size();
                    this->readValue( vecVariables[nbvars-1], vecGStates.back().textState.fontSize );

                    // Remove leading '/' and get font reference.
                    tmpString = vecVariables[nbvars-2];
                    vecGStates.back().textState.fontName = tmpString.substr( 1 );
                    vecGStates.back().importFontReference( m_page );
                }
                else if( gOperator.code == ePdfGOperator_Tr ) {
                    // Read render.
                    this->readValue( vecVariables.back(), vecGStates.back().textState.render );
                }
                else if( gOperator.code == ePdfGOperator_Ts ) {
                    // Read rise.
                    this->readValue( vecVariables.back(), vecGStates.back().textState.rise );
                }
                // Call category function.
                this->fTextState( gOperator, vecVariables, vecGStates );
            }
            else if( gOperator.cat == ePdfGCategory_TextPositioning )
            {
                // Commands in this category: Td, TD, Tm, T*.

                if( gOperator.code == ePdfGOperator_Td ) {
                    // Read and compute text transformation matrix.
                    tmpMat.init();
                    size_t nbvars = vecVariables.size();
                    this->readValue( vecVariables[nbvars-1], tmpMat(2,1) );
                    this->readValue( vecVariables[nbvars-2], tmpMat(2,0) );

                    vecGStates.back().textState.lineTransMat = tmpMat * vecGStates.back().textState.lineTransMat;
                    vecGStates.back().textState.transMat = vecGStates.back().textState.lineTransMat;
                }
                else if( gOperator.code == ePdfGOperator_TD ) {
                    // Read and compute text transformation matrix.
                    tmpMat.init();
                    size_t nbvars = vecVariables.size();
                    this->readValue( vecVariables[nbvars-1], tmpMat(2,1) );
                    this->readValue( vecVariables[nbvars-2], tmpMat(2,0) );

                    vecGStates.back().textState.lineTransMat = tmpMat * vecGStates.back().textState.lineTransMat;
                    vecGStates.back().textState.transMat = vecGStates.back().textState.lineTransMat;

                    // New leading value.
                    vecGStates.back().textState.leading = -tmpMat(2,1);
                }
                else if( gOperator.code == ePdfGOperator_Tm ) {
                    // Get transformation matrix new one.
                    size_t nbvars = vecVariables.size();

                    this->readValue( vecVariables[nbvars-1], tmpMat(2,1) );
                    this->readValue( vecVariables[nbvars-2], tmpMat(2,0) );
                    this->readValue( vecVariables[nbvars-3], tmpMat(1,1) );
                    this->readValue( vecVariables[nbvars-4], tmpMat(1,0) );
                    this->readValue( vecVariables[nbvars-5], tmpMat(0,1) );
                    this->readValue( vecVariables[nbvars-6], tmpMat(0,0) );

                    vecGStates.back().textState.transMat = tmpMat;
                    vecGStates.back().textState.lineTransMat = tmpMat;
                }
                else if( gOperator.code == ePdfGOperator_Tstar ) {
                    // "T*" equivalent to "0 -Tl Td".
                    tmpMat.init();
                    tmpMat(2,1) = -vecGStates.back().textState.leading;

                    vecGStates.back().textState.lineTransMat = tmpMat * vecGStates.back().textState.lineTransMat;
                    vecGStates.back().textState.transMat = vecGStates.back().textState.lineTransMat;
                }
                // Call category function.
                this->fTextPositioning( gOperator, vecVariables, vecGStates );
            }
            else if( gOperator.cat == ePdfGCategory_TextShowing )
            {
                // Commands in this category: Tj, TJ, ', "

                //Modify text graphics state parameters when necessary.
                if( gOperator.code == ePdfGOperator_Quote ) {
                    // Corresponds to T*, Tj.
                    tmpMat.init();
                    tmpMat(2,1) = -vecGStates.back().textState.leading;

                    vecGStates.back().textState.lineTransMat = tmpMat * vecGStates.back().textState.lineTransMat;
                    vecGStates.back().textState.transMat = vecGStates.back().textState.lineTransMat;
                }
                else if( gOperator.code == ePdfGOperator_DoubleQuote ) {
                    // Corresponds to Tw, Tc, Tj.
                    size_t nbvars = vecVariables.size();
                    tmpMat.init();

                    this->readValue( vecVariables[nbvars-1], vecGStates.back().textState.charSpace );
                    this->readValue( vecVariables[nbvars-2], vecGStates.back().textState.wordSpace );
                }
                // Call category function.
                this->fTextShowing( gOperator, vecVariables, vecGStates );
            }
            else if( gOperator.cat == ePdfGCategory_Color )
            {
                // Commands in this category: CS, cs, SC, SCN, sc, scn, G, g, RG, rg, K, k.
                // Not so much done in this category. Should maybe implement a bit more !

                // Call category function.
                this->fColor( gOperator, vecVariables, vecGStates );
            }
            else if( gOperator.cat == ePdfGCategory_PathConstruction )
            {
                // Commands in this category: m, l, c, v, y, h, re.

                if( gOperator.code == ePdfGOperator_m ) {
                    // Begin a new subpath.
                    size_t nbvars = vecVariables.size();
                    PdfVector point;

                    this->readValue( vecVariables[nbvars-1], point(1) );
                    this->readValue( vecVariables[nbvars-2], point(0) );

                    currentPath.beginSubpath( point );
                }
                else if( gOperator.code == ePdfGOperator_l ) {
                    // Append straight line.
                    size_t nbvars = vecVariables.size();
                    PdfVector point;

                    this->readValue( vecVariables[nbvars-1], point(1) );
                    this->readValue( vecVariables[nbvars-2], point(0) );

                    currentPath.appendLine( point );
                }
                else if( gOperator.code == ePdfGOperator_c ) {
                    // Append Bézier curve (c).
                    size_t nbvars = vecVariables.size();
                    PdfVector point1, point2, point3;

                    this->readValue( vecVariables[nbvars-1], point3(1) );
                    this->readValue( vecVariables[nbvars-2], point3(0) );
                    this->readValue( vecVariables[nbvars-3], point2(1) );
                    this->readValue( vecVariables[nbvars-4], point2(0) );
                    this->readValue( vecVariables[nbvars-5], point1(1) );
                    this->readValue( vecVariables[nbvars-6], point1(0) );

                    currentPath.appendBezierC( point1, point2, point3 );
                }
                else if( gOperator.code == ePdfGOperator_v ) {
                    // Append Bézier curve (c).
                    size_t nbvars = vecVariables.size();
                    PdfVector point2, point3;

                    this->readValue( vecVariables[nbvars-1], point3(1) );
                    this->readValue( vecVariables[nbvars-2], point3(0) );
                    this->readValue( vecVariables[nbvars-3], point2(1) );
                    this->readValue( vecVariables[nbvars-4], point2(0) );

                    currentPath.appendBezierV( point2, point3 );
                }
                else if( gOperator.code == ePdfGOperator_y ) {
                    // Append Bézier curve (c).
                    size_t nbvars = vecVariables.size();
                    PdfVector point1, point3;

                    this->readValue( vecVariables[nbvars-1], point3(1) );
                    this->readValue( vecVariables[nbvars-2], point3(0) );
                    this->readValue( vecVariables[nbvars-3], point1(1) );
                    this->readValue( vecVariables[nbvars-4], point1(0) );

                    currentPath.appendBezierY( point1, point3 );
                }
                else if( gOperator.code == ePdfGOperator_h ) {
                    // Close the current subpath by appending a straight line.
                    currentPath.closeSubpath();
                }
                else if( gOperator.code == ePdfGOperator_re ) {
                    // Append a rectangle to the current path as a complete subpath.
                    size_t nbvars = vecVariables.size();
                    PdfVector llpoint, size;

                    this->readValue( vecVariables[nbvars-1], size(1) );
                    this->readValue( vecVariables[nbvars-2], size(0) );
                    this->readValue( vecVariables[nbvars-3], llpoint(1) );
                    this->readValue( vecVariables[nbvars-4], llpoint(0) );

                    currentPath.appendRectangle( llpoint, size );
                }
                // Call category function.
                this->fPathConstruction( gOperator, vecVariables, vecGStates, currentPath );
            }
            else if( gOperator.cat == ePdfGCategory_PathPainting )
            {
                // Commands in this category: S, s, f, F, f*, B, B*, b, b*, n.

                // Call category function.
                this->fPathPainting( gOperator, vecVariables, vecGStates, currentPath );

                // Clear the current path.
                currentPath.init();
            }
            else if( gOperator.cat == ePdfGCategory_ClippingPath )
            {
                // Commands in this category: W, W*.

                // Append the current path to the clipping path.
                vecGStates.back().clippingPath.appendPath( currentPath );

                // Set the clipping path operator of the current path.
                currentPath.setClippingPathOp( gOperator.name );

                // Call category function.
                this->fClippingPath( gOperator, vecVariables, vecGStates, currentPath );
            }
            else if( gOperator.cat == ePdfGCategory_Type3Fonts )
            {
                // Commands in this category: d0, d1.

                // Call category function.
                this->fType3Fonts( gOperator, vecVariables, vecGStates );
            }
            else if( gOperator.cat == ePdfGCategory_ShadingPatterns )
            {
                // Commands in this category: sh.

                // Call category function.
                this->fShadingPatterns( gOperator, vecVariables, vecGStates );
            }
            else if( gOperator.cat == ePdfGCategory_InlineImages )
            {
                // Commands in this category: BI, ID, EI.

                // Call category function.
                this->fInlineImages( gOperator, vecVariables, vecGStates );
            }
            else if( gOperator.cat == ePdfGCategory_XObjects )
            {
                // Commands in this category: Do.

                // Call category function.
                this->fXObjects( gOperator, vecVariables, vecGStates );
            }
            else if( gOperator.cat == ePdfGCategory_Compatibility )
            {
                // Commands in this category: BX, EX.

                if( gOperator.code == ePdfGOperator_BX ) {
                    // Activate compatibility mode.
                    vecGStates.back().compatibilityMode = true;
                }
                else if( gOperator.code == ePdfGOperator_EX ) {
                    // Deactivate compatibility mode.
                    vecGStates.back().compatibilityMode = false;
                }

                // Call category function.
                this->fCompatibility( gOperator, vecVariables, vecGStates );
            }
            else if( gOperator.cat == ePdfGCategory_Unknown )
            {
                if( !vecGStates.back().compatibilityMode ) {
                    PODOFO_RAISE_ERROR_INFO( ePdfError_InvalidContentStream,
                                             "Invalid token in a stream" );
                }
            }
            // Clear variables vector.
            vecVariables.clear();
        }
        else if ( eType == ePdfContentsType_ImageData )
        {
            // Copy inline image data in the variables vector.
            vecVariables.push_back( strVariant );
        }
    }
}

}
