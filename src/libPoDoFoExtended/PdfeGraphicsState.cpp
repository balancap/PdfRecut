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
//                      Pdf Text State                      //
//**********************************************************//
PdfeTextState::PdfeTextState()
{
    this->init();
}

void PdfeTextState::init()
{
    render = 0;
    fontSize = charSpace = wordSpace = leading = rise = 0;
    hScale = 100;
    fontName = "";

    transMat.init();
    lineTransMat.init();
}
void PdfeTextState::initMatrices()
{
    transMat.init();
    lineTransMat.init();
}

//**********************************************************//
//                    Pdf Graphics State                    //
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
        textState.fontRef = array[0].GetReference();
        textState.fontName = "";
        textState.fontSize = array[1].GetReal();
    }
    return true;
}
bool PdfeGraphicsState::importFontReference( const PdfeResources& resources )
{
    // Obtain the expected font reference.
    PdfObject* font = resources.getKey( PdfeResourcesType::Font, textState.fontName );

    if( font && font->IsReference() ) {
        textState.fontRef = font->GetReference();
        return true;
    }
    return false;
}

}
