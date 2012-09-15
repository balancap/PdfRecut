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

    m_bboxHasChanged = false;
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
        m_bboxHasChanged = true;
    }
}

bool PRTextLine::sortLines( PRTextLine* pLine1, PRTextLine* pLine2 )
{
    // Compare minimum group index found in each line.
    return ( pLine1->minGroupIndex() < pLine2->minGroupIndex() );
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

void PRTextLine::computeBBox()
{
    // We basically assume that words have a zero angle between them.
    // That should usually be the case...

    // Not element inside.
    if( m_pGroupsWords.empty() ) {
        m_bboxHasChanged = false;
        m_bbox = PdfRect( 0, 0, 0, 0 );
        m_transMatrix.fill( 0.0 );
    }

    // Used the inverse transformation matrix of the first group of words.
    PdfeMatrix invTransMat = m_pGroupsWords[0]->getGlobalTransMatrix().inverse();

    // Left, Bottom, Right and Top coordinates of the bounding box.
    double left, bottom, right, top;
    left = bottom = std::numeric_limits<double>::min();
    right = top = std::numeric_limits<double>::max();

    // Mean coordinate of the base line.
    double meanYCoord = 0;
    double widthSum = 0;

    // Big loop on groups of words.
    PdfeORect  sbBBox;
    PdfeVector leftBottom, rightTop;
    for( size_t i = 0 ; i < m_pGroupsWords.size() ; ++i ) {
        // Second loop on subgroups.
        for( size_t j = 0 ; j < m_pGroupsWords[i]->nbSubGroups() ; ++j ) {
            sbBBox = m_pGroupsWords[i]->bbox( true, j, true );
            sbBBox = invTransMat.map( sbBBox );

            // Update bbox coordinates.
            leftBottom = sbBBox.leftBottom();
            rightTop = sbBBox.rightTop();

            left = std::min( left, leftBottom( 0 ) );
            bottom = std::min( bottom, leftBottom( 1 ) );
            right = std::max( right, rightTop( 0 ) );
            top = std::max( top, rightTop( 1 ) );

            // Update mean Y coordinate.
            sbBBox = m_pGroupsWords[i]->bbox( true, j, false );
            sbBBox = invTransMat.map( sbBBox );

            leftBottom = sbBBox.leftBottom();
            widthSum += sbBBox.width();
            meanYCoord += leftBottom( 1 ) * sbBBox.width();
        }
    }
    // Final mean Y coordinate.
    meanYCoord = meanYCoord / widthSum;

    // Transformation matrix used for the line. Set such the (0,0) corresponds to (left, meanY).
    PdfeMatrix tmpMat;
    tmpMat(2,0) = left;
    tmpMat(2,1) = meanYCoord;
    m_transMatrix = tmpMat * m_pGroupsWords[0]->getGlobalTransMatrix();

    // Set bounding box.
    m_bbox.SetLeft( 0.0 );
    m_bbox.SetBottom( bottom-meanYCoord );
    m_bbox.SetWidth( right-left );
    m_bbox.SetHeight( top-bottom );

    m_bboxHasChanged = false;
}

}
