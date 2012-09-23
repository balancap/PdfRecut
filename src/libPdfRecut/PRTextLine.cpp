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
    m_subgroupsWords.clear();

    m_modified = false;
    m_bbox = PdfRect( 0, 0, 0, 0 );
    m_transMatrix.fill( 0.0 );
}

void PRTextLine::addGroupWords( PRTextGroupWords* pGroupWords )
{
    if( pGroupWords ) {
        // Add the complete subgroup to the line.
        this->addSubgroupWords( PRTextGroupWords::Subgroup( *pGroupWords ) );
    }
}
void PRTextLine::addSubgroupWords( const PRTextGroupWords::Subgroup& subgroup )
{
    // Add if not empty...
    PRTextGroupWords* pGroup = subgroup.group();
    if( !pGroup ) {
        return;
    }
    // Look if the group already belongs to the line.
    long idx = this->hasGroupWords( pGroup );
    if( idx != -1 ) {
        // Union of the subgroups...
        m_subgroupsWords[idx] = PRTextGroupWords::Subgroup::reunion( m_subgroupsWords[idx],
                                                                     subgroup );
    }
    else {
        m_subgroupsWords.push_back( subgroup );
    }
    // Add the line to the group.
    pGroup->addTextLine( this );

    // Line has changed...
    m_modified = true;
}
void PRTextLine::rmGroupWords( PRTextGroupWords* pGroupWords )
{
    // Find the corresponding subgroup and erase it.
    long idx = this->hasGroupWords( pGroupWords );
    if( idx != -1 ) {
        m_subgroupsWords.erase( m_subgroupsWords.begin()+idx );

        // Remove the line from the group.
        pGroupWords->rmTextLine( this );

        // Line has changed...
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
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        minGroupIdx = std::min( minGroupIdx, m_subgroupsWords[i].group()->groupIndex() );
    }
    return minGroupIdx;
}
long PRTextLine::maxGroupIndex()
{
    long maxGroupIdx = std::numeric_limits<long>::min();
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        maxGroupIdx = std::max( maxGroupIdx, m_subgroupsWords[i].group()->groupIndex() );
    }
    return maxGroupIdx;
}

PdfeORect PRTextLine::bbox( bool pageCoords, bool leadTrailSpaces, bool useBottomCoord )
{
    // Compute the bounding box if necessary.
    if( m_modified ) {
        this->analyse();
    }
    PdfeORect bbox( m_bbox );

    if( !useBottomCoord ) {
        PdfeVector lbPoint = bbox.leftBottom();

        // Update height and bottom.
        bbox.setHeight( bbox.height()+lbPoint(1) );
        lbPoint(1) = 0.0;
        bbox.setLeftBottom( lbPoint );
    }
    if( pageCoords ) {
        bbox = m_transMatrix.map( bbox );
    }
    return bbox;
}

double PRTextLine::width( bool leadTrailSpaces )
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
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        length += m_subgroupsWords[i].length( countSpaces );
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
long PRTextLine::hasGroupWords( PRTextGroupWords* pGroup ) const
{
    // Look at subgroups...
    for( long i = 0 ; i < static_cast<long>( m_subgroupsWords.size() ) ; ++i ) {
        if( m_subgroupsWords[i].group() == pGroup ) {
            return i;
        }
    }
    return -1;
}

std::vector<PRTextLine::Block> PRTextLine::horizontalBlocks( double hDistance ) const
{
    // Initial the vector of blocks with line's subgroups.
    std::vector<Block> hBlocks( m_subgroupsWords.size() );
    for( size_t i = 0 ; i < hBlocks.size() ; ++i ) {
        hBlocks[i].init( this, i, true, true );
    }

    // Horizontal sort of blocks.
    std::sort( hBlocks.begin(), hBlocks.end(), Block::horizontalSort );

    // Merge blocks.
    std::vector<Block>::iterator lIt, rIt;
    PdfRect lBBox, rBBox;
    for( rIt = hBlocks.begin()+1 ; rIt != hBlocks.end() ; ) {
        // Get left and right bounding boxes.
        lIt = rIt - 1;
        lBBox = lIt->bbox();
        rBBox = rIt->bbox();

        // Merge if horizontal overlap.
        if( rBBox.GetLeft() - ( lBBox.GetLeft()+lBBox.GetWidth() ) <= hDistance ) {
            lIt->merge( *rIt );
            rIt = hBlocks.erase( rIt );
        }
        else {
            ++rIt;
        }
    }
    return hBlocks;
}

void PRTextLine::computeBBox()
{
    // We basically assume that words have a zero angle between them.
    // That should usually be the case...

    // Not element inside.
    if( m_subgroupsWords.empty() ) {
        m_modified = false;
        m_bbox = PdfRect( 0, 0, 0, 0 );
        m_transMatrix.fill( 0.0 );
    }

    // Used the inverse transformation matrix of the first group of words.
    PdfeMatrix transMat = m_subgroupsWords[0].group()->getGlobalTransMatrix();
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

    // Big loop on subgroups of words.
    PdfeORect subGpBBox;
    PdfeVector leftBottom, rightTop;
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        // Update line bbox coordinates.
        subGpBBox = m_subgroupsWords[i].bbox( true, true, true );
        subGpBBox = invTransMat.map( subGpBBox );

        leftBottom = subGpBBox.leftBottom();
        rightTop = subGpBBox.rightTop();

        left = std::min( left, leftBottom(0) );
        bottom = std::min( bottom, leftBottom(1) );
        right = std::max( right, rightTop(0) );
        top = std::max( top, rightTop(1) );

        // Update mean Y coordinate.
        subGpBBox = m_subgroupsWords[i].bbox( true, true, false );
        subGpBBox = invTransMat.map( subGpBBox );

        leftBottom = subGpBBox.leftBottom();
        width = subGpBBox.width();
        widthTotal += width;
        meanYCoord += leftBottom(1) * width;

        // Update mean height.
        subGpBBox = PdfRect( 0, 0, 1, 1 );
        subGpBBox = invTransMat.map( transMat.map( subGpBBox ) );
        meanHeight += subGpBBox.height() * width;
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
    m_transMatrix = rescaleMat * transMat;

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
PRTextLine::Block::Block()
{
    // Empty block.
    m_pLine = NULL;
    m_subgroupsInside.clear();
    m_bbox = PdfRect( 0, 0, 0, 0 );
}

PRTextLine::Block::Block( const PRTextLine* pLine, size_t idxSubgroup, bool leadTrailSpaces, bool useBottomCoord )
{
    // Initialize block.
    this->init( pLine, idxSubgroup, leadTrailSpaces, useBottomCoord );
}
void PRTextLine::Block::init( const PRTextLine* pLine, size_t idxSubgroup, bool leadTrailSpaces, bool useBottomCoord )
{
    // Initialize members.
    m_pLine = const_cast<PRTextLine*>( pLine );
    m_subgroupsInside.resize( pLine->nbSubgroups(), false );
    m_subgroupsInside.at( idxSubgroup ) = true;

    // Compute bounding box.
    PdfeORect bbox = this->subgroup( idxSubgroup )->bbox( true, leadTrailSpaces, useBottomCoord );
    bbox = m_pLine->transMatrix().inverse().map( bbox );

    m_bbox.SetLeft( bbox.leftBottomX() );
    m_bbox.SetBottom( bbox.leftBottomY() );
    m_bbox.SetWidth( bbox.width() );
    m_bbox.SetHeight( bbox.height() );
}

void PRTextLine::Block::merge( const PRTextLine::Block& block2nd )
{
    // Check they have a common parent line.
    if( m_pLine != block2nd.m_pLine ) {
        return;
    }
    // Add subgroups of the second block.
    for( size_t i = 0 ; i < m_subgroupsInside.size() ; ++i ) {
        m_subgroupsInside[i] = m_subgroupsInside[i] || block2nd.m_subgroupsInside[i];
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

PdfRect PRTextLine::Block::bbox() const
{
    return m_bbox;
}
bool PRTextLine::Block::horizontalSort( const PRTextLine::Block& block1, const PRTextLine::Block& block2 )
{
    return ( block1.m_bbox.GetLeft() < block2.m_bbox.GetLeft() );
}

}
