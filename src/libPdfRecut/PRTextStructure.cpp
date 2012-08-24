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
#include <limits>

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

    double cwidth = 1.0;
    PdfRect cbbox;


    // Read characters from the string.
    size_t i = 0;
    while( i < cidstr.length() )
    {
        // Read space word.
        if( pFont->isSpace( cidstr[i] ) )
        {
            double width = 0.0;
            double bottom = std::numeric_limits<double>::max();
            double top = std::numeric_limits<double>::min();
            Word textWord( 0, 0.0, 0.0, 1 );

            // Read spaces (can be multiple...)
            while( i < cidstr.length() && pFont->isSpace( cidstr[i] ) ) {
                //cwidth = pFont->width( cidstr[i], false );
                // Get bounding box of the character.
                cbbox = pFont->bbox( cidstr[i], false );

                // Width to add.
                width += ( cbbox.GetWidth() + charSpace / fontSize );
                if( pFont->isSpace( cidstr[i] ) == PdfeFontSpace::Code32 ) {
                    width += wordSpace / fontSize;
                }

                // New bottom and top values.
                bottom = std::min( bottom, cbbox.GetBottom() );
                top = std::max( top, cbbox.GetBottom() + cbbox.GetHeight() );

                textWord.length++;
                i++;
            }
            textWord.rect.SetWidth( width );
            textWord.rect.SetBottom( bottom );
            textWord.rect.SetHeight( top-bottom );

            m_words.push_back( textWord );
        }
        else // Read classic word.
        {
            double width = 0.0;
            double bottom = std::numeric_limits<double>::max();
            double top = std::numeric_limits<double>::min();
            Word textWord( 0, 0.0, 0.0, 0 );

            // Read word characters.
            while( i < cidstr.length() && !pFont->isSpace( cidstr[i] ) ) {
                //cwidth = pFont->width( cidstr[i], false );
                // Get bounding box of the character.
                cbbox = pFont->bbox( cidstr[i], false );

                // Update width, bottom and top.
                width += ( cbbox.GetWidth() + charSpace / fontSize );
                bottom = std::min( bottom, cbbox.GetBottom() );
                top = std::max( top, cbbox.GetBottom() + cbbox.GetHeight() );

                textWord.length++;
                i++;
            }
            textWord.rect.SetWidth( width );
            textWord.rect.SetBottom( bottom );
            textWord.rect.SetHeight( top-bottom );

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

double PRTextGroupWords::width() const
{
    double width = 0;
    for( size_t i = 0 ; i < m_words.size() ; ++i ) {
        width += m_words[i].rect.GetWidth();
    }
    return width;
}
double PRTextGroupWords::height() const
{
    double height = 0;
    for( size_t i = 0 ; i < m_words.size() ; ++i ) {
        height = std::max( m_words[i].rect.GetHeight(), height );
    }
    return height;
}

PdfeMatrix PRTextGroupWords::getGlobalTransMatrix() const
{
    // Compute text rendering matrix.
    PdfeMatrix tmpMat, textMat;
    tmpMat(0,0) = m_textState.fontSize * ( m_textState.hScale / 100. );
//    tmpMat(1,1) = m_textState.fontSize * m_words.back().height;
    tmpMat(1,1) = m_textState.fontSize;
    tmpMat(2,1) = m_textState.rise;
    textMat = tmpMat * m_textState.transMat * m_transMatrix;

    return textMat;
}
PdfeORect PRTextGroupWords::getOrientedRect() const
{
    PdfeORect groupORect( 0.0, 0.0 );
    if( !m_words.size() ) {
        return groupORect;
    }

    // Compute width and height.
    double width = 0;
    double bottom = std::numeric_limits<double>::max();
    double top = std::numeric_limits<double>::min();

    for( size_t i = 0 ; i < m_words.size() ; ++i ) {
        width += m_words[i].rect.GetWidth();

        bottom = std::min( bottom, m_words[i].rect.GetBottom() );
        top = std::max( top, m_words[i].rect.GetBottom() + m_words[i].rect.GetHeight() );
    }
    groupORect.setWidth( width );
    groupORect.setLeftBottom( PdfeVector( 0, bottom ) );
    groupORect.setHeight( top - bottom );

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
