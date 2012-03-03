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

#include "PdfGraphicsState.h"
#include "PdfPath.h"

using namespace PoDoFo;

namespace PdfRecut {

//**********************************************************//
//                      Pdf Text State                      //
//**********************************************************//
PdfTextState::PdfTextState()
{
    this->init();
}

void PdfTextState::init()
{
    render = 0;
    fontSize = charSpace = wordSpace = leading = rise = 0;
    hScale = 100;
    fontName = "";

    transMat.init();
    lineTransMat.init();
}
void PdfTextState::initMatrices()
{
    transMat.init();
    lineTransMat.init();
}

//**********************************************************//
//                    Pdf Graphics State                    //
//**********************************************************//
PdfGraphicsState::PdfGraphicsState()
{
    this->init();
}

void PdfGraphicsState::init()
{
    transMat.init();

    lineWidth = 1.0;
    lineCap = ePdfLineCapStyle_Butt;
    lineJoin = ePdfLineJoinStyle_Miter;

    compatibilityMode = false;
}

bool PdfGraphicsState::importExtGState( PoDoFo::PdfPage* page, const std::string& gsName )
{
    // Pdf resources of the page and ExtGState dictionary.
    PdfObject* pageRes = page->GetResources();
    PdfObject* extGStates = pageRes->GetIndirectKey( "ExtGState" );
    PdfObject* param;

    // Obtain the wanted graphics state.
    PdfObject* eGState = extGStates->GetIndirectKey( gsName );
    if( !eGState ) {
        return false;
    }

    // Line width.
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
bool PdfGraphicsState::importFontReference( PoDoFo::PdfPage* page )
{
    // Pdf resources of the page and Font dictionary.
    PdfObject* pageRes = page->GetResources();
    PdfObject* fontRes = pageRes->GetIndirectKey( "Font" );

    // Obtain the expected font reference.
    PdfObject* font = fontRes->GetDictionary().GetKey( textState.fontName );
    //PdfObject* font = fontRes->GetIndirectKey(  );
    if( font && font->IsReference() ) {
        textState.fontRef = font->GetReference();
        return true;
    }
    return false;
}


}
