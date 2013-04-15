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
#include "PdfeGraphicsState.h"

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
    streamState.resources = stream.resources();

    // Analyse contents stream's nodes.
    streamState.pNode = stream.firstNode();
    while( streamState.pNode ) {
        // References to have simpler notations...
        PdfeContentsStream::Node* pnode = streamState.pNode;
        PdfeGraphicsState& gstate = streamState.gstates.back();
        PdfeResources& resources = streamState.resources;

        // Update graphics state.
        gstate.update( pnode, currentPath, resources );

        // Specific treatment for each kind of node.
        if( pnode->category() == PdfeGCategory::GeneralGState ) {
            // Commands in this category: w, J, j, M, d, ri, i, gs.
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
            // Call category function.
            this->fSpecialGState( streamState );
        }
        else if( pnode->category() == PdfeGCategory::TextObjects ) {
            // Commands in this category: BT, ET.
            this->fTextObjects( streamState );
        }
        else if( pnode->category() == PdfeGCategory::TextState ) {
            // Commands in this category: Tc, Tw, Tz, TL, Tf, Tr, Ts.
            this->fTextState( streamState );
        }
        else if( pnode->category() == PdfeGCategory::TextPositioning ) {
            // Commands in this category: Td, TD, Tm, T*.
            this->fTextPositioning( streamState );
        }
        else if( pnode->category() == PdfeGCategory::TextShowing ) {
            // Commands in this category: Tj, TJ, ', "

            // Call category function: return text displacement vector.
            PdfeVector textDispl = this->fTextShowing( streamState );
            // Update text transformation matrix using the vector.
            PdfeMatrix transMat;
            transMat(2,0) = textDispl(0);
            transMat(2,1) = textDispl(1);
            gstate.textState().setTransMat( transMat * gstate.textState().transMat() );
        }
        else if( pnode->category() == PdfeGCategory::Color ) {
            // Commands in this category: CS, cs, SC, SCN, sc, scn, G, g, RG, rg, K, k.
            this->fColor( streamState );
        }
        else if( pnode->category() == PdfeGCategory::PathConstruction ) {
            // Commands in this category: m, l, c, v, y, h, re.

            // Load current path.
            pnode = currentPath.load( pnode );
            // Call category function.
            this->fPathConstruction( streamState, currentPath );
        }
        else if( pnode->category() == PdfeGCategory::PathPainting ) {
            // Commands in this category: S, s, f, F, f*, B, B*, b, b*, n.

            // Set path painting operator.
            currentPath.setPaintingOp( pnode->goperator() );
            // Call category function.
            this->fPathPainting( streamState, currentPath );

            // Update graphics state clipping rectangle.
            gstate.clippingRect().append( currentPath );
            // Clear the current path.
            currentPath.init();
        }
        else if( pnode->category() == PdfeGCategory::ClippingPath ) {
            // Commands in this category: W, W*.

            // Set the clipping path operator of the current path.
            currentPath.setClippingPathOp( pnode->goperator() );
            // Call category function.
            this->fClippingPath( streamState, currentPath );
        }
        else if( pnode->category() == PdfeGCategory::Type3Fonts ) {
            // Commands in this category: d0, d1.
            this->fType3Fonts( streamState );
        }
        else if( pnode->category() == PdfeGCategory::ShadingPatterns ) {
            // Commands in this category: sh.
            this->fShadingPatterns( streamState );
        }
        else if( pnode->category() == PdfeGCategory::InlineImages ) {
            // Commands in this category: BI, ID, EI.
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
                    //streamState.resources.push_back( xobject.GetResources() );
                    streamState.resources.append( PdfeResources( xobject.GetResources() ) );

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
        streamState.pNode = pnode;
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
