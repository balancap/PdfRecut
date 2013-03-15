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

#include "PdfeCanvasAnalysis.h"
#include "PdfeStreamTokenizer.h"

#include "podofo/podofo.h"

using namespace PoDoFo;

namespace PoDoFoExtended {

PdfeCanvasAnalysis::PdfeCanvasAnalysis()
{
    // Set locale to english for istringstream.
    PdfLocaleImbue( m_iStrStream );
}
PdfeCanvasAnalysis::PdfeCanvasAnalysis( const PdfeCanvasAnalysis& rhs )
{
    // Well, nothing to do!
}
PdfeCanvasAnalysis& PdfeCanvasAnalysis::operator =(const PdfeCanvasAnalysis &rhs)
{
    // Still nothing to do!
    return *this;
}

PdfeCanvasAnalysis::~PdfeCanvasAnalysis()
{
}

void PdfeCanvasAnalysis::analyseContents( PoDoFo::PdfCanvas* canvas,
                                          const PdfeGraphicsState& initialGState,
                                          const PdfeResources& initialResources )
{
    //Stream tokenizer and associated variables.
    PdfeStreamTokenizer tokenizer( canvas );

    // Stream state.
    PdfeStreamStateOld streamState;
    // Current path.
    PdfePath currentPath;

    // Temporary variables.
    std::string tmpString;
    PdfeMatrix tmpMat;
    PdfRect tmpRect;

    EPdfContentsType eType;
    std::string strVariant;

    // Initialize graphics state and resources.
    streamState.gStates.push_back( initialGState );
    tmpRect = canvas->GetPageSize();
    PdfePath clippingPath = streamState.gStates.back().clippingPath();
    clippingPath.appendRectangle( PdfeVector( tmpRect.GetLeft(), tmpRect.GetBottom() ),
                                  PdfeVector( tmpRect.GetWidth(), tmpRect.GetHeight() ) );
    streamState.gStates.back().setClippingPath( clippingPath );

    streamState.canvas = canvas;
    streamState.resources = initialResources;
    streamState.resources.append( PdfeResources( canvas->GetResources() ) );

    // Analyse page stream / Also known as the big dirty loop !
    while( tokenizer.ReadNext( eType, streamState.gOperator, strVariant ) )
    {
        // References to have simpler notations...
        PdfeGraphicOperator& gOperator = streamState.gOperator;
        std::vector<std::string>& gOperands = streamState.gOperands;
        PdfeGraphicsState& gState = streamState.gStates.back();
        PdfeResources& resources = streamState.resources;

        if ( eType == ePdfContentsType_Variant )
        {
            // Copy variant in the variables vector.
            gOperands.push_back( strVariant );
        }
        else if( eType == ePdfContentsType_Keyword )
        {
            //this->fUnknown( streamState );

            // Distinction between operator categories.
            if( gOperator.category() == PdfeGCategory::GeneralGState )
            {
                // Commands in this category: w, J, j, M, d, ri, i, gs.

                if( gOperator.type() ==PdfeGOperator::w ) {
                    // Get line width.
                    double val;
                    this->readValue( gOperands.back(), val );
                    gState.setLineWidth( val );
                }
                else if( gOperator.type() ==PdfeGOperator::J ) {
                    // Get line cap.
                    int val;
                    this->readValue( gOperands.back(), val );
                    gState.setLineCap( val );
                }
                else if( gOperator.type() ==PdfeGOperator::j ) {
                    // Get line join.
                    int val;
                    this->readValue( gOperands.back(), val );
                    gState.setLineJoin( val );
                }
                else if( gOperator.type() ==PdfeGOperator::gs ) {
                    // Get parameters from an ExtGState dictionary.
                    tmpString = gOperands.back().substr( 1 );
                    gState.loadExtGState( tmpString, resources );
                }
                // Call category function.
                this->fGeneralGState( streamState );
            }
            else if( gOperator.category() == PdfeGCategory::SpecialGState )
            {
                // Commands in this category: q, Q, cm.

                if( gOperator.type() == PdfeGOperator::q ) {
                    // Push on the graphics state stack.
                    streamState.gStates.push_back( gState );
                }
                else if( gOperator.type() == PdfeGOperator::Q ) {
                    // Pop on the graphics state stack.
                    streamState.gStates.pop_back();
                }
                else if( gOperator.type() == PdfeGOperator::cm ) {
                    // Get transformation matrix and compute new one.
                    size_t nbvars = gOperands.size();
                    tmpMat.init();

                    this->readValue( gOperands[nbvars-1], tmpMat(2,1) );
                    this->readValue( gOperands[nbvars-2], tmpMat(2,0) );
                    this->readValue( gOperands[nbvars-3], tmpMat(1,1) );
                    this->readValue( gOperands[nbvars-4], tmpMat(1,0) );
                    this->readValue( gOperands[nbvars-5], tmpMat(0,1) );
                    this->readValue( gOperands[nbvars-6], tmpMat(0,0) );

                    gState.setTransMat( tmpMat * gState.transMat() );
                }
                // Call category function.
                this->fSpecialGState( streamState );
            }
            else if( gOperator.category() == PdfeGCategory::TextObjects )
            {
                // Commands in this category: BT, ET.

                // Initialize text transform matrices when op = "BT".
                if( gOperator.type() == PdfeGOperator::BT ) {
                    gState.textState().initMatrices();
                }
                // Call category function.
                this->fTextObjects( streamState );
            }
            else if( gOperator.category() == PdfeGCategory::TextState )
            {
                // Commands in this category: Tc, Tw, Tz, TL, Tf, Tr, Ts.
                double tmpValue;

                if( gOperator.type() == PdfeGOperator::Tc ) {
                    // Read char space.
                    this->readValue( gOperands.back(), tmpValue );
                    gState.textState().setCharSpace( tmpValue );
                }
                else if( gOperator.type() == PdfeGOperator::Tw ) {
                    // Read word space.
                    this->readValue( gOperands.back(), tmpValue );
                    gState.textState().setWordSpace( tmpValue );
                }
                else if( gOperator.type() == PdfeGOperator::Tz ) {
                    // Read horizontal scale.
                    this->readValue( gOperands.back(), tmpValue );
                    gState.textState().setHScale( tmpValue );
                }
                else if( gOperator.type() == PdfeGOperator::TL ) {
                    // Read leading.
                    this->readValue( gOperands.back(), tmpValue );
                    gState.textState().setLeading( tmpValue );
                }
                else if( gOperator.type() == PdfeGOperator::Tf ) {
                    // Read font size and font name
                    size_t nbvars = gOperands.size();
                    this->readValue( gOperands[nbvars-1], tmpValue );
                    gState.textState().setFontSize( tmpValue );

                    // Remove leading '/' and get font reference.
                    tmpString = gOperands[nbvars-2];
                    gState.textState().setFont( tmpString.substr( 1 ), resources );
                }
                else if( gOperator.type() == PdfeGOperator::Tr ) {
                    // Read render.
                    int render;
                    this->readValue( gOperands.back(), render );
                    gState.textState().setRender( render );
                }
                else if( gOperator.type() == PdfeGOperator::Ts ) {
                    // Read rise.
                    this->readValue( gOperands.back(), tmpValue );
                    gState.textState().setRise( tmpValue );
                }
                // Call category function.
                this->fTextState( streamState );
            }
            else if( gOperator.category() == PdfeGCategory::TextPositioning )
            {
                // Commands in this category: Td, TD, Tm, T*.

                if( gOperator.type() == PdfeGOperator::Td ) {
                    // Read and compute text transformation matrix.
                    tmpMat.init();
                    size_t nbvars = gOperands.size();
                    this->readValue( gOperands[nbvars-1], tmpMat(2,1) );
                    this->readValue( gOperands[nbvars-2], tmpMat(2,0) );

                    gState.textState().setLineTransMat( tmpMat * gState.textState().lineTransMat() );
                    gState.textState().setTransMat( gState.textState().lineTransMat() );
                }
                else if( gOperator.type() == PdfeGOperator::TD ) {
                    // Read and compute text transformation matrix.
                    tmpMat.init();
                    size_t nbvars = gOperands.size();
                    this->readValue( gOperands[nbvars-1], tmpMat(2,1) );
                    this->readValue( gOperands[nbvars-2], tmpMat(2,0) );

                    gState.textState().setLineTransMat( tmpMat * gState.textState().lineTransMat() );
                    gState.textState().setTransMat( gState.textState().lineTransMat() );

                    // New leading value.
                    gState.textState().setLeading( -tmpMat(2,1) );
                }
                else if( gOperator.type() == PdfeGOperator::Tm ) {
                    // Get transformation matrix new one.
                    size_t nbvars = gOperands.size();

                    this->readValue( gOperands[nbvars-1], tmpMat(2,1) );
                    this->readValue( gOperands[nbvars-2], tmpMat(2,0) );
                    this->readValue( gOperands[nbvars-3], tmpMat(1,1) );
                    this->readValue( gOperands[nbvars-4], tmpMat(1,0) );
                    this->readValue( gOperands[nbvars-5], tmpMat(0,1) );
                    this->readValue( gOperands[nbvars-6], tmpMat(0,0) );

                    gState.textState().setTransMat( tmpMat );
                    gState.textState().setLineTransMat( tmpMat );
                }
                else if( gOperator.type() == PdfeGOperator::Tstar ) {
                    // "T*" equivalent to "0 -Tl Td".
                    tmpMat.init();
                    tmpMat(2,1) = -gState.textState().leading();

                    gState.textState().setLineTransMat( tmpMat * gState.textState().lineTransMat() );
                    gState.textState().setTransMat( gState.textState().lineTransMat() );
                }
                // Call category function.
                this->fTextPositioning( streamState );
            }
            else if( gOperator.category() == PdfeGCategory::TextShowing )
            {
                // Commands in this category: Tj, TJ, ', "
                double tmpValue;

                //Modify text graphics state parameters when necessary.
                if( gOperator.type() == PdfeGOperator::Quote ) {
                    // Corresponds to T*, Tj.
                    tmpMat.init();
                    tmpMat(2,1) = -gState.textState().leading();

                    gState.textState().setLineTransMat( tmpMat * gState.textState().lineTransMat() );
                    gState.textState().setTransMat( gState.textState().lineTransMat() );
                }
                else if( gOperator.type() == PdfeGOperator::DoubleQuote ) {
                    // Corresponds to Tw, Tc, Tj.
                    size_t nbvars = gOperands.size();
                    tmpMat.init();

                    this->readValue( gOperands[nbvars-1], tmpValue );
                    gState.textState().setCharSpace( tmpValue );
                    this->readValue( gOperands[nbvars-2], tmpValue );
                    gState.textState().setWordSpace( tmpValue );
                }
                // Call category function: return text displacement vector.
                PdfeVector textDispl = this->fTextShowing( streamState );

                // Update text transformation matrix using the vector.
                tmpMat.init();
                tmpMat(2,0) = textDispl(0);
                tmpMat(2,1) = textDispl(1);
                gState.textState().setTransMat( tmpMat * gState.textState().transMat() );
            }
            else if( gOperator.category() == PdfeGCategory::Color )
            {
                // Commands in this category: CS, cs, SC, SCN, sc, scn, G, g, RG, rg, K, k.
                // Not so much done in this category. Should maybe implement a bit more !

                // Call category function.
                this->fColor( streamState );
            }
            else if( gOperator.category() == PdfeGCategory::PathConstruction )
            {
                // Commands in this category: m, l, c, v, y, h, re.

                if( gOperator.type() == PdfeGOperator::m ) {
                    // Begin a new subpath.
                    size_t nbvars = gOperands.size();
                    PdfeVector point;

                    this->readValue( gOperands[nbvars-1], point(1) );
                    this->readValue( gOperands[nbvars-2], point(0) );

                    currentPath.beginSubpath( point );
                }
                else if( gOperator.type() == PdfeGOperator::l ) {
                    // Append straight line.
                    size_t nbvars = gOperands.size();
                    PdfeVector point;

                    this->readValue( gOperands[nbvars-1], point(1) );
                    this->readValue( gOperands[nbvars-2], point(0) );

                    currentPath.appendLine( point );
                }
                else if( gOperator.type() == PdfeGOperator::c ) {
                    // Append Bézier curve (c).
                    size_t nbvars = gOperands.size();
                    PdfeVector point1, point2, point3;

                    this->readValue( gOperands[nbvars-1], point3(1) );
                    this->readValue( gOperands[nbvars-2], point3(0) );
                    this->readValue( gOperands[nbvars-3], point2(1) );
                    this->readValue( gOperands[nbvars-4], point2(0) );
                    this->readValue( gOperands[nbvars-5], point1(1) );
                    this->readValue( gOperands[nbvars-6], point1(0) );

                    currentPath.appendBezierC( point1, point2, point3 );
                }
                else if( gOperator.type() == PdfeGOperator::v ) {
                    // Append Bézier curve (c).
                    size_t nbvars = gOperands.size();
                    PdfeVector point2, point3;

                    this->readValue( gOperands[nbvars-1], point3(1) );
                    this->readValue( gOperands[nbvars-2], point3(0) );
                    this->readValue( gOperands[nbvars-3], point2(1) );
                    this->readValue( gOperands[nbvars-4], point2(0) );

                    currentPath.appendBezierV( point2, point3 );
                }
                else if( gOperator.type() == PdfeGOperator::y ) {
                    // Append Bézier curve (c).
                    size_t nbvars = gOperands.size();
                    PdfeVector point1, point3;

                    this->readValue( gOperands[nbvars-1], point3(1) );
                    this->readValue( gOperands[nbvars-2], point3(0) );
                    this->readValue( gOperands[nbvars-3], point1(1) );
                    this->readValue( gOperands[nbvars-4], point1(0) );

                    currentPath.appendBezierY( point1, point3 );
                }
                else if( gOperator.type() == PdfeGOperator::h ) {
                    // Close the current subpath by appending a straight line.
                    currentPath.closeSubpath();
                }
                else if( gOperator.type() == PdfeGOperator::re ) {
                    // Append a rectangle to the current path as a complete subpath.
                    size_t nbvars = gOperands.size();
                    PdfeVector llpoint, size;

                    this->readValue( gOperands[nbvars-1], size(1) );
                    this->readValue( gOperands[nbvars-2], size(0) );
                    this->readValue( gOperands[nbvars-3], llpoint(1) );
                    this->readValue( gOperands[nbvars-4], llpoint(0) );

                    currentPath.appendRectangle( llpoint, size );
                }
                // Call category function.
                this->fPathConstruction( streamState, currentPath );
            }
            else if( gOperator.category() == PdfeGCategory::PathPainting )
            {
                // Commands in this category: S, s, f, F, f*, B, B*, b, b*, n.

                // Call category function.
                this->fPathPainting( streamState, currentPath );

                // Clear the current path.
                currentPath.init();
            }
            else if( gOperator.category() == PdfeGCategory::ClippingPath )
            {
                // Commands in this category: W, W*.

                // Append the current path to the clipping path.
                PdfePath clippingPath( gState.clippingPath() );
                clippingPath.appendPath( currentPath );
                gState.setClippingPath( clippingPath );

                // Set the clipping path operator of the current path.
                currentPath.setClippingPathOp( gOperator.str() );

                // Call category function.
                this->fClippingPath( streamState, currentPath );
            }
            else if( gOperator.category() == PdfeGCategory::Type3Fonts )
            {
                // Commands in this category: d0, d1.

                // Call category function.
                this->fType3Fonts( streamState );
            }
            else if( gOperator.category() == PdfeGCategory::ShadingPatterns )
            {
                // Commands in this category: sh.

                // Call category function.
                this->fShadingPatterns( streamState );
            }
            else if( gOperator.category() == PdfeGCategory::InlineImages )
            {
                // Commands in this category: BI, ID, EI.

                // Call category function.
                this->fInlineImages( streamState );
            }
            else if( gOperator.category() == PdfeGCategory::XObjects )
            {
                // Commands in this category: Do.

                // Get XObject and subtype.
                std::string xObjName = gOperands.back().substr( 1 );
                PdfObject* xObjPtr = streamState.resources.getIndirectKey( PdfeResourcesType::XObject, xObjName );
                std::string xObjSubtype = xObjPtr->GetIndirectKey( "Subtype" )->GetName().GetName();

                // Form object.
                if( !xObjSubtype.compare( "Form" ) )
                {
                    // PdfXObject corresponding.
                    PdfXObject xObject( xObjPtr );

                    // Push on the graphics state stack.
                    streamState.gStates.push_back( streamState.gStates.back() );

                    // Get transformation matrix of the form.
                    PdfeMatrix formMat;
                    if( xObjPtr->GetDictionary().HasKey( "Matrix" ) ) {
                        PdfArray& mat = xObjPtr->GetIndirectKey( "Matrix" )->GetArray();

                        formMat(0,0) = mat[0].GetReal();
                        formMat(0,1) = mat[1].GetReal();
                        formMat(1,0) = mat[2].GetReal();
                        formMat(1,1) = mat[3].GetReal();
                        formMat(2,0) = mat[4].GetReal();
                        formMat(2,1) = mat[5].GetReal();
                    }
                    streamState.gStates.back().setTransMat( formMat * streamState.gStates.back().transMat() );

                    // Get form's BBox and append it to clipping path.
                    PdfArray& bbox = xObjPtr->GetIndirectKey( "BBox" )->GetArray();

                    PdfePath pathBBox;
                    pathBBox.beginSubpath( PdfeVector( bbox[0].GetReal(), bbox[1].GetReal() ) );
                    pathBBox.appendLine( PdfeVector( bbox[2].GetReal(), bbox[1].GetReal() ) );
                    pathBBox.appendLine( PdfeVector( bbox[2].GetReal(), bbox[3].GetReal() ) );
                    pathBBox.appendLine( PdfeVector( bbox[0].GetReal(), bbox[3].GetReal() ) );
                    pathBBox.closeSubpath();

                    PdfePath clippingPath( streamState.gStates.back().clippingPath() );
                    clippingPath.appendPath( pathBBox );
                    streamState.gStates.back().setClippingPath( clippingPath );

                    // Analayse Form.
                    this->fFormBegin( streamState, &xObject );
                    this->analyseContents( &xObject,
                                         streamState.gStates.back(),
                                         streamState.resources );
                    this->fFormEnd( streamState, &xObject );

                    // Pop on the graphics state stack.
                    streamState.gStates.pop_back();
                }

                // Call category function.
                this->fXObjects( streamState );
            }
            else if( gOperator.category() == PdfeGCategory::Compatibility )
            {
                // Commands in this category: BX, EX.

                if( gOperator.type() == PdfeGOperator::BX ) {
                    // Activate compatibility mode.
                    gState.setCompatibilityMode( true );
                }
                else if( gOperator.type() == PdfeGOperator::EX ) {
                    // Deactivate compatibility mode.
                    gState.setCompatibilityMode( false );
                }

                // Call category function.
                this->fCompatibility( streamState );
            }
            else if( gOperator.category() == PdfeGCategory::Unknown )
            {
                // Call category function.
                this->fUnknown( streamState );

//                if( !gState.compatibilityMode ) {
//                    PODOFO_RAISE_ERROR_INFO( ePdfError_InvalidContentStream,
//                                             "Invalid token in a stream" );
//                }
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
