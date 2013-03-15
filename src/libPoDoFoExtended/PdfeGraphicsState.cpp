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

#include <podofo/podofo.h>

#include "PdfeGraphicsState.h"

#include "PdfePath.h"
#include "PdfeResources.h"

using namespace PoDoFo;

namespace PoDoFoExtended {

//**********************************************************//
//                       PdfeTextState                      //
//**********************************************************//
PdfeTextState::PdfeTextState()
{
    this->init();
}
void PdfeTextState::init()
{
    m_render = 0;
    m_fontSize = m_charSpace = m_wordSpace = m_leading = m_rise = 0.;
    m_hScale = 100.;
    m_fontName = "";
    m_fontRef = PdfReference();

    m_transMat.init();
    m_lineTransMat.init();
}
void PdfeTextState::initMatrices()
{
    m_transMat.init();
    m_lineTransMat.init();
}

void PdfeTextState::setFont( const std::string& name, const PdfReference& ref )
{
    m_fontName = name;
    m_fontRef = ref;
}
bool PdfeTextState::setFont( const std::string& name, const PdfeResources& resources )
{
    // Obtain the expected font reference.
    const PdfObject* pFontRef = resources.getKey( PdfeResourcesType::Font, name );
    if( pFontRef && pFontRef->IsReference() ) {
        m_fontName = name;
        m_fontRef = pFontRef->GetReference();
        return true;
    }
    // Something wrong happened...
    m_fontName = name;
    m_fontRef = PdfReference();
    return false;
}

//**********************************************************//
//                     PdfeGraphicsState                    //
//**********************************************************//
PdfeGraphicsState::PdfeGraphicsState() :
    m_pTextState( NULL )
{
    this->init();
}
PdfeGraphicsState::PdfeGraphicsState( const PdfeGraphicsState& rhs ):
    m_transMat( rhs.m_transMat ),
    m_clippingPath( rhs.m_clippingPath ),
    m_lineWidth( rhs.m_lineWidth ),
    m_lineCap( rhs.m_lineCap ),
    m_lineJoin( rhs.m_lineJoin ),
    m_miterLimit( rhs.m_miterLimit ),
    m_compatibilityMode( rhs.m_compatibilityMode )
{
    if( rhs.m_pTextState ) {
        m_pTextState = new PdfeTextState( *(rhs.m_pTextState) );
    }
    else {
        m_pTextState = NULL;
    }
}
PdfeGraphicsState &PdfeGraphicsState::operator=( const PdfeGraphicsState& rhs )
{
    if( &rhs != this ) {
        // Copy members...
        m_transMat = rhs.m_transMat;
        m_clippingPath = rhs.m_clippingPath;
        m_lineWidth = rhs.m_lineWidth;
        m_lineCap = rhs.m_lineCap;
        m_lineJoin = rhs.m_lineJoin;
        m_miterLimit = rhs.m_miterLimit;
        m_compatibilityMode = rhs.m_compatibilityMode;

        this->clearTextState();
        if( rhs.m_pTextState ) {
            m_pTextState = new PdfeTextState( *(rhs.m_pTextState) );
        }
    }
    return *this;
}
void PdfeGraphicsState::init()
{
    // Default values stated in the PDF reference.
    this->clearTextState();

    m_transMat.init();
    m_clippingPath.init();

    m_lineWidth = 1.0;
    m_lineCap = ePdfLineCapStyle_Butt;
    m_lineJoin = ePdfLineJoinStyle_Miter;
    m_miterLimit = 10.0;

    m_compatibilityMode = false;
}
PdfeGraphicsState::~PdfeGraphicsState()
{
    this->clearTextState();
}

void PdfeGraphicsState::update( const PdfeContentsStream::Node* pnode,
                                const PdfePath& currentPath,
                                const PdfeResources& resources )
{
    // Nodes which affect the graphics state...
    if( pnode->category() == PdfeGCategory::GeneralGState ) {
        // Commands in this category: w, J, j, M, d, ri, i, gs.
        if( pnode->type() ==PdfeGOperator::w ) {
            // Get line width.
            m_lineWidth = pnode->operand<double>( 0 );
        }
        else if( pnode->type() ==PdfeGOperator::J ) {
            // Get line cap.
            m_lineCap = pnode->operand<int>( 0 );
        }
        else if( pnode->type() ==PdfeGOperator::j ) {
            // Get line join.
            m_lineJoin = pnode->operand<int>( 0 );
        }
        else if( pnode->type() ==PdfeGOperator::M ) {
            // Get miter limit.
            m_miterLimit = pnode->operand<double>( 0 );
        }
        else if( pnode->type() ==PdfeGOperator::gs ) {
            // Get parameters from an ExtGState dictionary.
            std::string extGStateName = pnode->operands().back().substr( 1 );
            this->loadExtGState( extGStateName, resources );
        }
    }
    else if( pnode->category() == PdfeGCategory::SpecialGState ) {
        if( pnode->type() == PdfeGOperator::cm ) {
            // Get transformation matrix and compute the new one.
            PdfeMatrix transMat;
            transMat(0,0) = pnode->operand<double>( 0 );
            transMat(0,1) = pnode->operand<double>( 1 );
            transMat(1,0) = pnode->operand<double>( 2 );
            transMat(1,1) = pnode->operand<double>( 3 );
            transMat(2,0) = pnode->operand<double>( 4 );
            transMat(2,1) = pnode->operand<double>( 5 );

            m_transMat = transMat * m_transMat;
        }
    }
    else if( pnode->category() == PdfeGCategory::TextObjects ) {
        // Commands in this category: BT, ET.
        // Initialize text transform matrices when op = "BT".
        if( pnode->type() == PdfeGOperator::BT ) {
            this->textState().initMatrices();
        }
    }
    else if( pnode->category() == PdfeGCategory::TextState ) {
        // Commands in this category: Tc, Tw, Tz, TL, Tf, Tr, Ts.
        if( pnode->type() == PdfeGOperator::Tc ) {
            // Read char space.
            this->textState().setCharSpace( pnode->operand<double>( 0 ) );
        }
        else if( pnode->type() == PdfeGOperator::Tw ) {
            // Read word space.
            this->textState().setWordSpace( pnode->operand<double>( 0 ) );
        }
        else if( pnode->type() == PdfeGOperator::Tz ) {
            // Read horizontal scale.
            this->textState().setHScale( pnode->operand<double>( 0 ) );
        }
        else if( pnode->type() == PdfeGOperator::TL ) {
            // Read leading.
            this->textState().setLeading( pnode->operand<double>( 0 ) );
        }
        else if( pnode->type() == PdfeGOperator::Tf ) {
            // Read font name and font size.
            std::string fontName = pnode->operands().at( 0 ).substr( 1 );
            this->textState().setFont( fontName, resources );
            this->textState().setFontSize( pnode->operand<double>( 1 ) );
        }
        else if( pnode->type() == PdfeGOperator::Tr ) {
            // Read render.
            this->textState().setRender( pnode->operand<int>( 0 ) );
        }
        else if( pnode->type() == PdfeGOperator::Ts ) {
            // Read rise.
            this->textState().setRise( pnode->operand<double>( 0 ) );
        }
    }
    else if( pnode->category() == PdfeGCategory::TextPositioning ) {
        // Commands in this category: Td, TD, Tm, T*.
        if( pnode->type() == PdfeGOperator::Td ) {
            // Read and compute text transformation matrix.
            PdfeMatrix transMat;
            transMat(2,0) = pnode->operand<double>( 0 );
            transMat(2,1) = pnode->operand<double>( 1 );

            this->textState().setLineTransMat( transMat * this->textState().lineTransMat() );
            this->textState().setTransMat( this->textState().lineTransMat() );
        }
        else if( pnode->type() == PdfeGOperator::TD ) {
            // Read and compute text transformation matrix.
            PdfeMatrix transMat;
            transMat(2,0) = pnode->operand<double>( 0 );
            transMat(2,1) = pnode->operand<double>( 1 );

            this->textState().setLineTransMat( transMat * this->textState().lineTransMat() );
            this->textState().setTransMat( this->textState().lineTransMat() );
            // New leading value.
            this->textState().setLeading( -transMat(2,1) );
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

            this->textState().setTransMat( transMat );
            this->textState().setLineTransMat( transMat );
        }
        else if( pnode->type() == PdfeGOperator::Tstar ) {
            // "T*" equivalent to "0 -Tl Td".
            PdfeMatrix transMat;
            transMat(2,1) = -this->textState().leading();

            this->textState().setLineTransMat( transMat * this->textState().lineTransMat() );
            this->textState().setTransMat( this->textState().lineTransMat() );
        }
    }
    else if( pnode->category() == PdfeGCategory::TextShowing ) {
        // Commands in this category: Tj, TJ, ', "

        //Modify text graphics state parameters when necessary.
        if( pnode->type() == PdfeGOperator::Quote ) {
            // Corresponds to T*, Tj.
            PdfeMatrix transMat;
            transMat(2,1) = -this->textState().leading();

            this->textState().setLineTransMat( transMat * this->textState().lineTransMat() );
            this->textState().setTransMat( this->textState().lineTransMat() );
        }
        else if( pnode->type() == PdfeGOperator::DoubleQuote ) {
            // Corresponds to Tw, Tc, Tj.
            this->textState().setWordSpace( pnode->operand<double>( 0 ) );
            this->textState().setCharSpace( pnode->operand<double>( 1 ) );
        }
    }
    else if( pnode->category() == PdfeGCategory::Color ) {
        // Commands in this category: CS, cs, SC, SCN, sc, scn, G, g, RG, rg, K, k.
        // TODO: implement color standard.
    }
    else if( pnode->category() == PdfeGCategory::ClippingPath ) {
        // Commands in this category: W, W*.
        // Append the current path to the clipping path.
        m_clippingPath.appendPath( currentPath );
        // Set the clipping path operator of the current path.
        //currentPath.setClippingPathOp( pnode->goperator().str() );
    }
    else if( pnode->category() == PdfeGCategory::Compatibility ) {
        // Commands in this category: BX, EX.
        if( pnode->type() == PdfeGOperator::BX ) {
            // Activate compatibility mode.
            m_compatibilityMode = true;
        }
        else if( pnode->type() == PdfeGOperator::EX ) {
            // Deactivate compatibility mode.
            m_compatibilityMode = false;
        }
    }
}
bool PdfeGraphicsState::loadExtGState( const std::string& gstateName,
                                       const PdfeResources& resources )
{
    // Obtain the expected graphics state object.
    PdfObject* pExtGStateObj = resources.getIndirectKey( PdfeResourcesType::ExtGState,
                                                         gstateName );
    if( !pExtGStateObj ) {
        return false;
    }
    // Line width.
    PdfObject* param;
    param = pExtGStateObj->GetIndirectKey( "LW" );
    if( param ) {
        m_lineWidth = param->GetReal();
    }
    // Line cap style.
    param = pExtGStateObj->GetIndirectKey( "LC" );
    if( param ) {
        m_lineCap = param->GetNumber();
    }
    // Line join style.
    param = pExtGStateObj->GetIndirectKey( "LJ" );
    if( param ) {
        m_lineJoin = param->GetNumber();
    }
    // Miter limit.
    param = pExtGStateObj->GetIndirectKey( "ML" );
    if( param ) {
        m_miterLimit = param->GetReal();
    }
    // Font.
    param = pExtGStateObj->GetIndirectKey( "Font" );
    if( param ) {
        PdfArray& array = param->GetArray();
        this->textState();
        m_pTextState->setFont( "", array[0].GetReference() );
        m_pTextState->setFontSize( array[1].GetReal() );
    }

    // TODO: complete with other state parameters!
    return true;
}

PdfeTextState& PdfeGraphicsState::textState()
{
    if( !m_pTextState ) {
        m_pTextState = new PdfeTextState();
    }
    return *m_pTextState;
}
const PdfeTextState& PdfeGraphicsState::textState() const
{
    if( !m_pTextState ) {
        m_pTextState = new PdfeTextState();
    }
    return *m_pTextState;
}
void PdfeGraphicsState::clearTextState()
{
    delete m_pTextState;
    m_pTextState = NULL;
}

}
