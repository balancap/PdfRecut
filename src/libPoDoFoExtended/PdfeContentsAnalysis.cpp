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

#include "PdfeContentsAnalysis.h"

#include <podofo/podofo.h>

using namespace PoDoFo;

namespace PoDoFoExtended {

PdfeContentsAnalysis::PdfeContentsAnalysis()
{
}
PdfeContentsAnalysis::PdfeContentsAnalysis( const PdfeContentsAnalysis& )
{
}
PdfeContentsAnalysis& PdfeContentsAnalysis::operator=( const PdfeContentsAnalysis& )
{
    return *this;
}
PdfeContentsAnalysis::~PdfeContentsAnalysis()
{
}

void PdfeContentsAnalysis::analyseContents( const PdfeContentsStream& stream )
{
    // Stream state and current path.
    PdfeStreamState streamState;
    PdfePath currentPath;
    // Resources stack.
    std::vector<PdfeResources> resourcesStack;

    // Initialize graphics state and resources.
    streamState.pStream = const_cast<PdfeContentsStream*>( &stream );
    streamState.gstates.push_back( stream.initialGState() );
    streamState.resources = stream.initialResources();

    // Analyse contents stream's nodes.
    streamState.pNode = stream.firstNode();
    while( streamState.pNode ) {
        // References to have simpler notations...
        PdfeContentsStream::Node* pnode = streamState.pNode;
        PdfeGraphicsState& gstate = streamState.gstates.back();
        PdfeResources& resources = streamState.resources;

        // Type of node...
        if( pnode->category() == PdfeGCategory::GeneralGState ) {
            // Commands in this category: w, J, j, M, d, ri, i, gs.

            if( pnode->type() ==PdfeGOperator::w ) {
                // Get line width.
                gstate.lineWidth = pnode->operand<double>( 0 );
            }
            else if( pnode->type() ==PdfeGOperator::J ) {
                // Get line cap.
                gstate.lineCap = pnode->operand<int>( 0 );
            }
            else if( pnode->type() ==PdfeGOperator::j ) {
                // Get line join.
                gstate.lineJoin = pnode->operand<int>( 0 );
            }
            else if( pnode->type() ==PdfeGOperator::gs ) {
                // Get parameters from an ExtGState dictionary.
                std::string str = pnode->operands().back().substr( 1 );
                gstate.importExtGState( resources, str );
            }
            // Call category function.
            this->fGeneralGState( streamState );
        }
        else if( pnode->category() == PdfeGCategory::SpecialGState ) {
            // Commands in this category: q, Q, cm.

            if( pnode->type() == PdfeGOperator::q ) {
                // Push on the graphics state stack.
                streamState.gstates.push_back( gstate );
            }
            else if( pnode->type() == PdfeGOperator::Q ) {
                // Pop on the graphics state stack.
                streamState.gstates.pop_back();
            }
            else if( pnode->type() == PdfeGOperator::cm ) {
                // Get transformation matrix and compute new one.
                PdfeMatrix transMat;
                transMat(0,0) = pnode->operand<double>( 0 );
                transMat(0,1) = pnode->operand<double>( 1 );
                transMat(1,0) = pnode->operand<double>( 2 );
                transMat(1,1) = pnode->operand<double>( 3 );
                transMat(2,0) = pnode->operand<double>( 4 );
                transMat(2,1) = pnode->operand<double>( 5 );

                gstate.transMat = transMat * gstate.transMat;
            }
            // Call category function.
            this->fSpecialGState( streamState );
        }
        else if( pnode->category() == PdfeGCategory::TextObjects ) {
            // Commands in this category: BT, ET.

            // Initialize text transform matrices when op = "BT".
            if( pnode->type() == PdfeGOperator::BT ) {
                gstate.textState.initMatrices();
            }
            // Call category function.
            this->fTextObjects( streamState );
        }
        else if( pnode->category() == PdfeGCategory::TextState ) {
            // Commands in this category: Tc, Tw, Tz, TL, Tf, Tr, Ts.

            if( pnode->type() == PdfeGOperator::Tc ) {
                // Read char space.
                gstate.textState.charSpace = pnode->operand<double>( 0 );
            }
            else if( pnode->type() == PdfeGOperator::Tw ) {
                // Read word space.
                gstate.textState.wordSpace = pnode->operand<double>( 0 );
            }
            else if( pnode->type() == PdfeGOperator::Tz ) {
                // Read horizontal scale.
                gstate.textState.hScale = pnode->operand<double>( 0 );
            }
            else if( pnode->type() == PdfeGOperator::TL ) {
                // Read leading.
                gstate.textState.leading = pnode->operand<double>( 0 );
            }
            else if( pnode->type() == PdfeGOperator::Tf ) {
                // Read font name and font size.
                gstate.textState.fontName = pnode->operands().at( 0 ).substr( 1 );
                gstate.importFontReference( resources );
                gstate.textState.fontSize = pnode->operand<double>( 1 );
            }
            else if( pnode->type() == PdfeGOperator::Tr ) {
                // Read render.
                gstate.textState.render = pnode->operand<int>( 0 );
            }
            else if( pnode->type() == PdfeGOperator::Ts ) {
                // Read rise.
                gstate.textState.rise = pnode->operand<double>( 0 );
            }
            // Call category function.
            this->fTextState( streamState );
        }
        else if( pnode->category() == PdfeGCategory::TextPositioning ) {
            // Commands in this category: Td, TD, Tm, T*.

            if( pnode->type() == PdfeGOperator::Td ) {
                // Read and compute text transformation matrix.
                PdfeMatrix transMat;
                transMat(2,0) = pnode->operand<double>( 0 );
                transMat(2,1) = pnode->operand<double>( 1 );

                gstate.textState.lineTransMat = transMat * gstate.textState.lineTransMat;
                gstate.textState.transMat = gstate.textState.lineTransMat;
            }
            else if( pnode->type() == PdfeGOperator::TD ) {
                // Read and compute text transformation matrix.
                PdfeMatrix transMat;
                transMat(2,0) = pnode->operand<double>( 0 );
                transMat(2,1) = pnode->operand<double>( 1 );

                gstate.textState.lineTransMat = transMat * gstate.textState.lineTransMat;
                gstate.textState.transMat = gstate.textState.lineTransMat;
                // New leading value.
                gstate.textState.leading = -transMat(2,1);
            }
            else if( pnode->type() == PdfeGOperator::Tm ) {
                // Get transformation matrix.
                PdfeMatrix transMat;
                transMat(0,0) = pnode->operand<double>( 0 );
                transMat(0,1) = pnode->operand<double>( 1 );
                transMat(1,0) = pnode->operand<double>( 2 );
                transMat(1,1) = pnode->operand<double>( 3 );
                transMat(2,0) = pnode->operand<double>( 4 );
                transMat(2,1) = pnode->operand<double>( 5 );

                gstate.textState.transMat = transMat;
                gstate.textState.lineTransMat = transMat;
            }
            else if( pnode->type() == PdfeGOperator::Tstar ) {
                // "T*" equivalent to "0 -Tl Td".
                PdfeMatrix transMat;
                transMat(2,1) = -gstate.textState.leading;

                gstate.textState.lineTransMat = transMat * gstate.textState.lineTransMat;
                gstate.textState.transMat = gstate.textState.lineTransMat;
            }
            // Call category function.
            this->fTextPositioning( streamState );
        }
        else if( pnode->category() == PdfeGCategory::TextShowing ) {
            // Commands in this category: Tj, TJ, ', "

            //Modify text graphics state parameters when necessary.
            if( pnode->type() == PdfeGOperator::Quote ) {
                // Corresponds to T*, Tj.
                PdfeMatrix transMat;
                transMat(2,1) = -gstate.textState.leading;

                gstate.textState.lineTransMat = transMat * gstate.textState.lineTransMat;
                gstate.textState.transMat = gstate.textState.lineTransMat;
            }
            else if( pnode->type() == PdfeGOperator::DoubleQuote ) {
                // Corresponds to Tw, Tc, Tj.
                gstate.textState.wordSpace = pnode->operand<double>( 0 );
                gstate.textState.charSpace = pnode->operand<double>( 1 );
            }
            // Call category function: return text displacement vector.
            PdfeVector textDispl = this->fTextShowing( streamState );

            // Update text transformation matrix using the vector.
            PdfeMatrix transMat;
            transMat(2,0) = textDispl(0);
            transMat(2,1) = textDispl(1);
            gstate.textState.transMat = transMat * gstate.textState.transMat;
        }
        else if( pnode->category() == PdfeGCategory::Color ) {
            // Commands in this category: CS, cs, SC, SCN, sc, scn, G, g, RG, rg, K, k.
            // Not so much done in this category. Should maybe implement a bit more. TODO !

            // Call category function.
            this->fColor( streamState );
        }
        else if( pnode->category() == PdfeGCategory::PathConstruction ) {
            // Commands in this category: m, l, c, v, y, h, re.

            if( pnode->type() == PdfeGOperator::m ) {
                // Begin a new subpath.
                PdfeVector point( pnode->operand<double>( 0 ),
                                  pnode->operand<double>( 1 ) );
                currentPath.beginSubpath( point );
            }
            else if( pnode->type() == PdfeGOperator::l ) {
                // Append straight line.
                PdfeVector point( pnode->operand<double>( 0 ),
                                  pnode->operand<double>( 1 ) );
                currentPath.appendLine( point );
            }
            else if( pnode->type() == PdfeGOperator::c ) {
                // Append Bézier curve (c).
                PdfeVector point1( pnode->operand<double>( 0 ),
                                   pnode->operand<double>( 1 ) );
                PdfeVector point2( pnode->operand<double>( 2 ),
                                   pnode->operand<double>( 3 ) );
                PdfeVector point3( pnode->operand<double>( 4 ),
                                   pnode->operand<double>( 5 ) );
                currentPath.appendBezierC( point1, point2, point3 );
            }
            else if( pnode->type() == PdfeGOperator::v ) {
                // Append Bézier curve (c).
                PdfeVector point2( pnode->operand<double>( 0 ),
                                   pnode->operand<double>( 1 ) );
                PdfeVector point3( pnode->operand<double>( 2 ),
                                   pnode->operand<double>( 3 ) );
                currentPath.appendBezierV( point2, point3 );
            }
            else if( pnode->type() == PdfeGOperator::y ) {
                // Append Bézier curve (c).
                PdfeVector point1( pnode->operand<double>( 0 ),
                                   pnode->operand<double>( 1 ) );
                PdfeVector point3( pnode->operand<double>( 2 ),
                                   pnode->operand<double>( 3 ) );
                currentPath.appendBezierY( point1, point3 );
            }
            else if( pnode->type() == PdfeGOperator::h ) {
                // Close the current subpath by appending a straight line.
                currentPath.closeSubpath();
            }
            else if( pnode->type() == PdfeGOperator::re ) {
                // Append a rectangle to the current path as a complete subpath.
                PdfeVector lbPoint( pnode->operand<double>( 0 ),
                                    pnode->operand<double>( 1 ) );
                PdfeVector size( pnode->operand<double>( 2 ),
                                 pnode->operand<double>( 3 ) );
                currentPath.appendRectangle( lbPoint, size );
            }
            // Call category function.
            this->fPathConstruction( streamState, currentPath );
        }
        else if( pnode->category() == PdfeGCategory::PathPainting ) {
            // Commands in this category: S, s, f, F, f*, B, B*, b, b*, n.

            // Call category function.
            this->fPathPainting( streamState, currentPath );
            // Clear the current path.
            currentPath.init();
        }
        else if( pnode->category() == PdfeGCategory::ClippingPath ) {
            // Commands in this category: W, W*.

            // Append the current path to the clipping path.
            gstate.clippingPath.appendPath( currentPath );
            // Set the clipping path operator of the current path.
            currentPath.setClippingPathOp( pnode->goperator().str() );

            // Call category function.
            this->fClippingPath( streamState, currentPath );
        }
        else if( pnode->category() == PdfeGCategory::Type3Fonts ) {
            // Commands in this category: d0, d1.

            // Call category function.
            this->fType3Fonts( streamState );
        }
        else if( pnode->category() == PdfeGCategory::ShadingPatterns ) {
            // Commands in this category: sh.

            // Call category function.
            this->fShadingPatterns( streamState );
        }
        else if( pnode->category() == PdfeGCategory::InlineImages ) {
            // Commands in this category: BI, ID, EI.

            // Call category function.
            this->fInlineImages( streamState );
        }
        else if( pnode->category() == PdfeGCategory::XObjects ) {
            // Commands in this category: Do.

            // Does it correspond to a form XObject which is Loaded?
            if( pnode->xobjectType() == PdfeXObjectType::Form && pnode->isFormXObjectLoaded() ) {
                // Opening node.
                if( pnode->isOpeningNode() ) {
                    // Save back resources and add form resources.
                    PdfXObject xobject( pnode->xobject() );
                    resourcesStack.push_back( streamState.resources );
                    streamState.resources.push_back( xobject.GetResources() );

                    this->fFormBegin( streamState, &xobject );
                }
                // Closing node.
                else if( pnode->isClosingNode() ) {
                    // Restore resources.
                    PdfXObject xobject( pnode->xobject() );
                    streamState.resources = resourcesStack.back();
                    resourcesStack.pop_back();

                    this->fFormEnd( streamState, &xobject );
                }
//                // Get form's BBox and append it to clipping path.
//                PdfArray& bbox = xObjPtr->GetIndirectKey( "BBox" )->GetArray();
//                PdfePath pathBBox;
//                pathBBox.beginSubpath( PdfeVector( bbox[0].GetReal(), bbox[1].GetReal() ) );
//                pathBBox.appendLine( PdfeVector( bbox[2].GetReal(), bbox[1].GetReal() ) );
//                pathBBox.appendLine( PdfeVector( bbox[2].GetReal(), bbox[3].GetReal() ) );
//                pathBBox.appendLine( PdfeVector( bbox[0].GetReal(), bbox[3].GetReal() ) );
//                pathBBox.closeSubpath();
            }
            // Call category function.
            this->fXObjects( streamState );
        }
        else if( pnode->category() == PdfeGCategory::Compatibility ) {
            // Commands in this category: BX, EX.

            if( pnode->type() == PdfeGOperator::BX ) {
                // Activate compatibility mode.
                gstate.compatibilityMode = true;
            }
            else if( pnode->type() == PdfeGOperator::EX ) {
                // Deactivate compatibility mode.
                gstate.compatibilityMode = false;
            }
            // Call category function.
            this->fCompatibility( streamState );
        }
        else if( pnode->category() == PdfeGCategory::Unknown ) {
            // Call category function.
            this->fUnknown( streamState );

//                if( !gstate.compatibilityMode ) {
//                    PODOFO_RAISE_ERROR_INFO( ePdfError_InvalidContentStream,
//                                             "Invalid token in a stream" );
//                }
        }

        // Next node in the stream...
        streamState.pNode = streamState.pNode->next();
    }
}

// Default implementations... Usually empty.
void PdfeContentsAnalysis::fGeneralGState( const PdfeStreamState& ) { }
void PdfeContentsAnalysis::fSpecialGState( const PdfeStreamState& ) { }
void PdfeContentsAnalysis::fPathConstruction( const PdfeStreamState&,
                                const PdfePath& ) { }
void PdfeContentsAnalysis::fPathPainting( const PdfeStreamState&,
                            const PdfePath& ) { }
void PdfeContentsAnalysis::fClippingPath( const PdfeStreamState&,
                            const PdfePath& ) { }
void PdfeContentsAnalysis::fTextObjects( const PdfeStreamState& ) { }
void PdfeContentsAnalysis::fTextState( const PdfeStreamState& ) { }
void PdfeContentsAnalysis::fTextPositioning( const PdfeStreamState& ) { }
PdfeVector PdfeContentsAnalysis::fTextShowing( const PdfeStreamState& ) {
    return PdfeVector();
}
void PdfeContentsAnalysis::fType3Fonts( const PdfeStreamState& ) { }
void PdfeContentsAnalysis::fColor( const PdfeStreamState& ) { }
void PdfeContentsAnalysis::fShadingPatterns( const PdfeStreamState& ) { }
void PdfeContentsAnalysis::fInlineImages( const PdfeStreamState& ) { }
void PdfeContentsAnalysis::fXObjects( const PdfeStreamState& ) { }
void PdfeContentsAnalysis::fMarkedContents( const PdfeStreamState& ) { }
void PdfeContentsAnalysis::fCompatibility( const PdfeStreamState& ) { }
void PdfeContentsAnalysis::fUnknown( const PdfeStreamState& ) { }
void PdfeContentsAnalysis::fFormBegin( const PdfeStreamState&,
                         PoDoFo::PdfXObject* ) { }
void PdfeContentsAnalysis::fFormEnd( const PdfeStreamState&,
                       PoDoFo::PdfXObject* ) { }

}
