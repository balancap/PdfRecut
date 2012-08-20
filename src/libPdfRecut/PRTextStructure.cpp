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
using namespace PoDoFoExtended;

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
                                      PoDoFoExtended::PdfeFont* pFont )
{
    // To CID string.
    PdfeCIDString cidstr = pFont->toCIDString( str );

    // Text parameters.
    //double hScale = m_textState.hScale;
    double charSpace = m_textState.charSpace;
    double wordSpace = m_textState.wordSpace;
    double fontSize = m_textState.fontSize;

    // Set font parameters (not necessary in theory).
    pFont->setCharSpace( 0.0 );
    pFont->setFontSize( 1.0 );
    pFont->setHScale( 100. );

    double charWidth = 1.0;

    // Read characters from the string.
    size_t i = 0;
    while( i < cidstr.length() )
    {
        // Read space word.
        if( pFont->isSpace( cidstr[i] ) )
        {
            Word textWord( 0, 0.0, charHeight, 1 );

            // Read spaces (can be multiple...)
            while( i < cidstr.length() && pFont->isSpace( cidstr[i] ) ) {
                charWidth = pFont->width( cidstr[i], false );
                textWord.width += ( charWidth + charSpace / fontSize );

                if( pFont->isSpace( cidstr[i] ) == PdfeFontSpace::Code32 ) {
                    textWord.width += wordSpace / fontSize;
                }

                textWord.length++;
                i++;
            }
            m_words.push_back( textWord );
        }
        else // Read classic word.
        {
            Word textWord( 0, 0.0, charHeight, 0 );

            // Read word characters.
            while( i < cidstr.length() && !pFont->isSpace( cidstr[i] ) ) {
                charWidth = pFont->width( cidstr[i], false );
                textWord.width += ( charWidth + charSpace / fontSize );

                textWord.length++;
                i++;
            }
            m_words.push_back( textWord );
        }
    }
}

void PRTextGroupWords::readPdfVariant( const PdfVariant& variant,
                                       PoDoFoExtended::PdfeFont* pFont )
{
    // Get font bounding box.
    PdfArray bbox = pFont->fontBBox();
    m_boundingBox[0] = bbox[0].GetReal() / 1000.;
    m_boundingBox[1] = bbox[1].GetReal() / 1000.;
    m_boundingBox[2] = bbox[2].GetReal() / 1000.;
    m_boundingBox[3] = bbox[3].GetReal() / 1000.;

    // double charHeight = 1.0;
    double charHeight = m_boundingBox[3];

    // Variant is a string.
    if( variant.IsString() || variant.IsHexString() )
    {
        this->readPdfString( variant.GetString(), charHeight, pFont );
    }
    // Variant is an array (c.f. operator TJ).
    else if( variant.IsArray() )
    {
        const PdfArray& array = variant.GetArray();
        for( size_t i = 0 ; i < array.size() ; i++ ) {
            if( array[i].IsString() || array[i].IsHexString() ) {
                // Read string.
                this->readPdfString( array[i].GetString(), charHeight, pFont );
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

double PRTextGroupWords::getWidth() const
{
    double width = 0;
    for( size_t i = 0 ; i < m_words.size() ; ++i ) {
        width += m_words[i].width;
    }
    return width;
}
double PRTextGroupWords::getHeight() const
{
    double height = 0;
    for( size_t i = 0 ; i < m_words.size() ; ++i ) {
        height = std::max( m_words[i].height, height );
    }
    return height;
}

PdfeMatrix PRTextGroupWords::getGlobalTransMatrix() const
{
    // Compute text rendering matrix.
    PdfeMatrix tmpMat, textMat;
    tmpMat(0,0) = m_textState.fontSize * ( m_textState.hScale / 100. );
    tmpMat(1,1) = m_textState.fontSize * m_words.back().height;
    tmpMat(2,1) = m_textState.rise;
    textMat = tmpMat * m_textState.transMat * m_transMatrix;

    return textMat;
}
PdfeORect PRTextGroupWords::getOrientedRect() const
{
    PdfeORect groupORect( 0.0, 1.0 );

    // Compute width and height.
    double width = 0;
    for( size_t i = 0 ; i < m_words.size() ; ++i ) {
        width += m_words[i].width;
        //groupORect.setWidth( groupORect.getWidth() + );
        //groupORect.setHeight( std::max( groupORect.getHeight(), m_words[i].height ) );
    }
    groupORect.setWidth( width );

    // Apply global transform.
    PdfeMatrix textMat = this->getGlobalTransMatrix();
    groupORect = textMat.map( groupORect );

    return groupORect;
}


//**********************************************************//
//                        PRTextLine                        //
//**********************************************************//
PRTextLine::PRTextLine()
{
    this->init();
}
void PRTextLine::init()
{
    m_pageIndex = -1;
    m_lineIndex = -1;
    m_groupsWords.clear();
}
void PRTextLine::addGroupWords( const PRTextGroupWords& groupWords )
{
    m_groupsWords.push_back( groupWords );
}

//**********************************************************//
//                     PRTextStructure                      //
//**********************************************************//
PRTextStructure::PRTextStructure()
{
}

}
