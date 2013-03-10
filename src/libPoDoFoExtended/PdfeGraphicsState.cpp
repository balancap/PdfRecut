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
PdfeGraphicsState::PdfeGraphicsState()
{
    this->init();
}

void PdfeGraphicsState::init()
{
    transMat.init();

    lineWidth = 1.0;
    lineCap = ePdfLineCapStyle_Butt;
    lineJoin = ePdfLineJoinStyle_Miter;

    compatibilityMode = false;
}

bool PdfeGraphicsState::importExtGState( const PdfeResources& resources, const std::string& gsName )
{
    // Obtain the expected graphics state.
    PdfObject* eGState = resources.getIndirectKey( PdfeResourcesType::ExtGState, gsName );
    if( !eGState ) {
        return false;
    }

    // Line width.
    PdfObject* param;
    param = eGState->GetIndirectKey( "LW" );
    if( param ) {
        lineWidth = param->GetReal();
    }
    // Line cap style.
    param = eGState->GetIndirectKey( "LC" );
    if( param ) {
        lineCap = param->GetNumber();
    }
    // Line join style.
    param = eGState->GetIndirectKey( "LJ" );
    if( param ) {
        lineJoin = param->GetNumber();
    }
    // Font.
    param = eGState->GetIndirectKey( "Font" );
    if( param ) {
        PdfArray& array = param->GetArray();
        textState.setFont( "", array[0].GetReference() );
        textState.setFontSize( array[1].GetReal() );
    }

    // TODO: complete with other state parameters!
    return true;
}
bool PdfeGraphicsState::importFontReference( const PdfeResources& resources )
{
    // Obtain the expected font reference.
    const PdfObject* font = resources.getKey( PdfeResourcesType::Font, textState.fontName() );
    if( font && font->IsReference() ) {
        textState.setFont( textState.fontName(), font->GetReference() );
        return true;
    }
    return false;
}

}
