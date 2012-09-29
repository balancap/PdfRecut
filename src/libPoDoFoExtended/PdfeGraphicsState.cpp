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

#include <podofo/podofo.h>

#include "PdfeGraphicsState.h"
#include "PdfePath.h"

using namespace PoDoFo;

namespace PdfRecut {

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

bool PdfeGraphicsState::importExtGState( const PdfResources& resources, const std::string& gsName )
{
    // Obtain the expected graphics state.
    PdfObject* eGState = resources.getIndirectKey( PdfResourcesType::ExtGState, gsName );
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
bool PdfeGraphicsState::importFontReference( const PdfResources& resources )
{
    // Obtain the expected font reference.
    PdfObject* font = resources.getKey( PdfResourcesType::Font, textState.fontName );

    if( font && font->IsReference() ) {
        textState.fontRef = font->GetReference();
        return true;
    }
    return false;
}

}
