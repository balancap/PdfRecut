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

#include "PRTextLine.h"

#include <podofo/podofo.h>
#include <limits>

using namespace PoDoFo;
using namespace PoDoFoExtended;

namespace PdfRecut {

//**********************************************************//
//                         PRTextLine                       //
//**********************************************************//
PRTextLine::PRTextLine()
{
    this->init();
}
PRTextLine::~PRTextLine()
{
}

void PRTextLine::init()
{
    m_pageIndex = -1;
    m_lineIndex = -1;
    m_pGroupsWords.clear();

    m_modified = false;
    m_bbox = PdfRect( 0, 0, 0, 0 );
    m_transMatrix.fill( 0.0 );
}

void PRTextLine::addGroupWords( PRTextGroupWords* pGroupWords )
{
    if( pGroupWords ) {
        // Set group textline (assume there is only one !).
        pGroupWords->setTextLine( this );
        m_pGroupsWords.push_back( pGroupWords );

        // Bounding box has changed...
        m_modified = true;
    }
}
void PRTextLine::analyse()
{
    // Compute bounding box and transformation matrix.
    this->computeBBox();

    // Reset modified parameter.
    m_modified = false;
}


long PRTextLine::minGroupIndex()
{
    long minGroupIdx = std::numeric_limits<long>::max();
    for( size_t i = 0 ; i < m_pGroupsWords.size() ; ++i ) {
        minGroupIdx = std::min( minGroupIdx, m_pGroupsWords[i]->groupIndex() );
    }
    return minGroupIdx;
}
long PRTextLine::maxGroupIndex()
{
    long maxGroupIdx = std::numeric_limits<long>::min();
    for( size_t i = 0 ; i < m_pGroupsWords.size() ; ++i ) {
        maxGroupIdx = std::max( maxGroupIdx, m_pGroupsWords[i]->groupIndex() );
    }
    return maxGroupIdx;
}

PdfeORect PRTextLine::bbox( bool pageCoords, bool useBottomCoord )
{
    // Compute the bbox if necessary.
    if( m_modified ) {
        this->analyse();
    }
    PdfeORect bbox( m_bbox );

    if( !useBottomCoord ) {
        PdfeVector lbPoint = bbox.leftBottom();

        bbox.setHeight( bbox.height()+lbPoint(1) );
        lbPoint(1) = 0.0;
        bbox.setLeftBottom( lbPoint );
    }
    if( pageCoords ) {
        bbox = m_transMatrix.map( bbox );
    }
    return bbox;
}
double PRTextLine::width()
{
    // Compute the bbox if necessary.
    if( m_modified ) {
        this->analyse();
    }

    return m_bbox.GetWidth();
}
size_t PRTextLine::length( bool countSpaces )
{
    size_t length = 0;
    for( size_t i = 0 ; i < m_pGroupsWords.size() ; ++i ) {
        length += m_pGroupsWords[i]->length( countSpaces );
    }
    return length;
}

PdfeMatrix PRTextLine::transMatrix()
{
    // Compute transformation matrix and bbox if necessary.
    if( m_modified ) {
        this->analyse();
    }
    return m_transMatrix;
}

void PRTextLine::computeBBox()
{
    // We basically assume that words have a zero angle between them.
    // That should usually be the case...

    // Not element inside.
    if( m_pGroupsWords.empty() ) {
        m_modified = false;
        m_bbox = PdfRect( 0, 0, 0, 0 );
        m_transMatrix.fill( 0.0 );
    }

    // Used the inverse transformation matrix of the first group of words.
    PdfeMatrix transMat = m_pGroupsWords[0]->getGlobalTransMatrix();
    PdfeMatrix invTransMat = transMat.inverse();

    // Left, Bottom, Right and Top coordinates of the bounding box.
    double left, bottom, right, top;
    left = bottom = std::numeric_limits<double>::max();
    right = top = std::numeric_limits<double>::min();

    // Mean coordinate of the base line.
    double meanYCoord = 0.0;
    double meanHeight = 0.0;
    double widthTotal = 0.0;
    double width;

    // Big loop on groups of words.
    PdfeORect subGpBBox;
    PdfeVector leftBottom, rightTop;
    for( size_t i = 0 ; i < m_pGroupsWords.size() ; ++i ) {
        // Second loop on subgroups.
        for( size_t j = 0 ; j < m_pGroupsWords[i]->nbSubGroups() ; ++j ) {
            // Update line bbox coordinates.
            subGpBBox = m_pGroupsWords[i]->bbox( true, j, true );
            subGpBBox = invTransMat.map( subGpBBox );

            leftBottom = subGpBBox.leftBottom();
            rightTop = subGpBBox.rightTop();

            left = std::min( left, leftBottom( 0 ) );
            bottom = std::min( bottom, leftBottom( 1 ) );
            right = std::max( right, rightTop( 0 ) );
            top = std::max( top, rightTop( 1 ) );

            // Update mean Y coordinate.
            subGpBBox = m_pGroupsWords[i]->bbox( true, j, false );
            subGpBBox = invTransMat.map( subGpBBox );

            leftBottom = subGpBBox.leftBottom();
            width = subGpBBox.width();
            widthTotal += width;
            meanYCoord += leftBottom( 1 ) * width;

            // Update mean height.
            subGpBBox = PdfRect( 0, 0, 1, 1 );
            subGpBBox = invTransMat.map( transMat.map( subGpBBox ) );
            meanHeight += subGpBBox.height() * width;
        }
    }
    // Final mean Y coordinate.
    meanYCoord = meanYCoord / widthTotal;
    double scaleCoef = meanHeight / widthTotal ;

    // Transformation matrix used for the line. Set such the (0,0) corresponds to (left, meanYCoord),
    // and the scaling coefficient corresponds to the mean height.
    PdfeMatrix rescaleMat;
    rescaleMat(0,0) = rescaleMat(1,1) = scaleCoef;
    rescaleMat(2,0) = left;
    rescaleMat(2,1) = meanYCoord;
    m_transMatrix = rescaleMat * m_pGroupsWords[0]->getGlobalTransMatrix();

    // Set bounding box.
    m_bbox.SetLeft( 0.0 );
    m_bbox.SetBottom( (bottom - meanYCoord) / scaleCoef );
    m_bbox.SetWidth( (right - left) / scaleCoef );
    m_bbox.SetHeight( (top - bottom) / scaleCoef );
}

bool PRTextLine::sortLines( PRTextLine* pLine1, PRTextLine* pLine2 )
{
    // Compare minimum group index found in each line.
    return ( pLine1->minGroupIndex() < pLine2->minGroupIndex() );
}

//**********************************************************//
//                     PRTextLine::Block                    //
//**********************************************************//
PRTextLine::Block::Block( PRTextGroupWords* pGroupWords, PdfeMatrix* pLineTransMat )
{
    // Initialize members.
    m_pLineTransMat = pLineTransMat;
    m_pGroupsWords.push_back( pGroupWords );

    // Compute bounding box.
    PdfeORect bbox = pGroupWords->bbox( true, -1, true );
    bbox = pLineTransMat->inverse().map( bbox );

    m_bbox.SetLeft( bbox.leftBottomX() );
    m_bbox.SetBottom( bbox.leftBottomY() );
    m_bbox.SetWidth( bbox.width() );
    m_bbox.SetHeight( bbox.height() );
}
void PRTextLine::Block::merge( const PRTextLine::Block& block2nd )
{
    // Add groups.
    m_pGroupsWords.reserve( m_pGroupsWords.size()+block2nd.m_pGroupsWords.size() );
    for( size_t i = 0 ; i < block2nd.m_pGroupsWords.size() ; ++i ) {
        m_pGroupsWords.push_back( block2nd.m_pGroupsWords[i] );
    }

    // Compute new bounding box.
    double left = std::min( this->m_bbox.GetLeft(), block2nd.m_bbox.GetLeft() );
    double bottom = std::min( this->m_bbox.GetBottom(), block2nd.m_bbox.GetBottom() );
    double right = std::max( this->m_bbox.GetLeft() + this->m_bbox.GetWidth(),
                             block2nd.m_bbox.GetLeft() + block2nd.m_bbox.GetWidth() );
    double top = std::max( this->m_bbox.GetBottom() + this->m_bbox.GetHeight(),
                           block2nd.m_bbox.GetBottom() + block2nd.m_bbox.GetHeight() );

    m_bbox.SetLeft( left );
    m_bbox.SetBottom( bottom );
    m_bbox.SetWidth( right-left );
    m_bbox.SetHeight( top-bottom );
}

PdfRect PRTextLine::Block::bbox()
{
    return m_bbox;
}

}
