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

#include "PRTextStructure.h"

#include <podofo/podofo.h>

using namespace PoDoFo;

namespace PdfRecut {

//**********************************************************//
//                      PdfGroupWords                       //
//**********************************************************//
PRTextGroupWords::PRTextGroupWords()
{
    this->init();
}
void PRTextGroupWords::init()
{
    // Initialize members to default values.
    m_pageIndex = -1;
    m_groupIndex = -1;
    m_words.clear();
    m_transMatrix.init();
    m_textState.init();
}

void PRTextGroupWords::readPdfString( const PoDoFo::PdfString& str,
                                      double charHeight,
                                      PoDoFo::PdfFontMetrics* fontMetrics )
{
    // Unicode string. Don't know how to handle it, therefore ignore !
    if( str.IsUnicode() ) {
        return;
    }

    // String data.
    const char* strData = str.GetString();
    size_t strLength = str.GetLength();

    // Text parameters.
    double hScale = fontMetrics->GetFontScale() / 100.;
    double wordSpace = m_textState.wordSpace;

    // Font metrics parameters.
    if( fontMetrics ) {
        // Strange implementation of char space in CharWidth functions from metrics classes: must multiply by 100.
        fontMetrics->SetFontCharSpace( m_textState.charSpace / m_textState.fontSize * 100 );
        fontMetrics->SetFontSize( 1.0 );
        fontMetrics->SetFontScale( 100. );
    }
    double charWidth = 1.0;

    // Read characters from the string.
    size_t i = 0;
    while( i < strLength )
    {
        // Read space word.
        if( strData[i] == ' ' )
        {
            Word textWord( 0, 0, charHeight, 1 );

            // Read spaces (can be multiple...)
            while( i < strLength && strData[i] == ' ' ) {
                if( fontMetrics ) {
                    charWidth = fontMetrics->CharWidth( strData[i] );
                }
                textWord.length++;
                textWord.width += ( charWidth + wordSpace * hScale );
                ++i;
            }
            m_words.push_back( textWord );
        }
        else // Read classic word.
        {
            Word textWord( 0, 0.0, charHeight, 0 );

            // Read word characters.
            while( i < strLength && strData[i] != ' ' ) {
                if( fontMetrics ) {
                    charWidth = fontMetrics->CharWidth( strData[i] );
                }
                textWord.length++;
                textWord.width += charWidth;
                ++i;
            }
            m_words.push_back( textWord );
        }
    }
}

void PRTextGroupWords::readPdfVariant( const PoDoFo::PdfVariant& variant,
                                       PoDoFo::PdfFontMetrics* fontMetrics )
{
    // Get bounding box.
    PdfArray boundingBox;
    if( fontMetrics ) {
        fontMetrics->GetBoundingBox( boundingBox );
    }
    else {
        // Default values.
        boundingBox.push_back( 0.0 );
        boundingBox.push_back( 0.0 );
        boundingBox.push_back( 1000.0 );
        boundingBox.push_back( 1000.0 );
    }
    double charHeight = boundingBox[3].GetReal() / 1000.;

    // Variant is a string.
    if( variant.IsString() || variant.IsHexString() )
    {
        this->readPdfString( variant.GetString(), charHeight, fontMetrics );
    }
    // Variant is an array (c.f. operator TJ).
    else if( variant.IsArray() )
    {
        const PdfArray& array = variant.GetArray();
        for( size_t i = 0 ; i < array.size() ; i++ ) {
            if( array[i].IsString() || array[i].IsHexString() ) {
                // Read string.
                this->readPdfString( array[i].GetString(), charHeight, fontMetrics );
            }
            else if( array[i].IsNumber() ) {
                // Read pdf space.
                m_words.push_back( Word( 1, -array[i].GetNumber() / 1000.0, charHeight, 2 ) );
            }
            else if( array[i].IsReal() ) {
                // Read pdf space.
                m_words.push_back( Word( 1, -array[i].GetReal() / 1000.0, charHeight, 2 ) );
            }
        }
    }
}

void PRTextGroupWords::appendWord( const Word& word )
{
    // Append the word to the vector.
    m_words.push_back( word );
}

//**********************************************************//
//                     PRTextStructure                      //
//**********************************************************//
PRTextStructure::PRTextStructure()
{
}

}
