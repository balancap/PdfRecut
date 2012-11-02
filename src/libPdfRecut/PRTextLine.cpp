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

    // Cache data.
    m_bbox = PdfRect( 0, 0, 0, 0 );
    m_bboxNoLTSpaces = PdfRect( 0, 0, 0, 0 );
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
void PRTextLine::setSubgroup( size_t idx, const PRTextGroupWords::Subgroup& subgroup )
{
    // Set if not empty...
    PRTextGroupWords* pGroup = subgroup.group();
    if( !pGroup ) {
        return;
    }

    // Set subgroup and add reference of the line to the group.
    m_subgroupsWords.at( idx ) = subgroup;
    pGroup->addTextLine( this );

    // Line has changed...
    m_modified = true;
}
long PRTextLine::hasGroupWords( PRTextGroupWords* pGroup ) const
{
    // Look at subgroups...
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        if( m_subgroupsWords[i].group() == pGroup ) {
            return i;
        }
    }
    return -1;
}
void PRTextLine::clearEmptySubgroups()
{
    std::vector<PRTextGroupWords::Subgroup>::iterator it;
    for( it = m_subgroupsWords.begin() ; it != m_subgroupsWords.end() ; ) {
        // Empty subgroup.
        if( it->isEmpty() ) {
            // Remove the subgroup and the reference to the line.
            it->group()->rmTextLine( this );
            it = m_subgroupsWords.erase( it );
        }
        else {
            ++it;
        }
    }
    // Line has changed...
    m_modified = true;
}

long PRTextLine::minGroupIndex() const
{
    long minGroupIdx = std::numeric_limits<long>::max();
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        minGroupIdx = std::min( minGroupIdx, m_subgroupsWords[i].group()->groupIndex() );
    }
    return minGroupIdx;
}
long PRTextLine::maxGroupIndex() const
{
    long maxGroupIdx = std::numeric_limits<long>::min();
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        maxGroupIdx = std::max( maxGroupIdx, m_subgroupsWords[i].group()->groupIndex() );
    }
    return maxGroupIdx;
}

PdfeORect PRTextLine::bbox( PRTextLineCoordinates::Enum lineCoords, bool leadTrailSpaces )
{
    // Compute the bounding box if necessary.
    if( m_modified ) {
        this->computeCacheData();
    }

    // Classic or without leading and trailing spaces bbox.
    PdfeORect bbox( m_bbox );
    if( !leadTrailSpaces ) {
        bbox = m_bboxNoLTSpaces;
    }
    // Bottom coordinates: TODO.
//    if( !useBottomCoord ) {
//        PdfeVector lbPoint = bbox.leftBottom();

//        // Update height and bottom.
//        bbox.setHeight( bbox.height()+lbPoint(1) );
//        lbPoint(1) = 0.0;
//        bbox.setLeftBottom( lbPoint );
//    }

    // Transformation if necessary
    if( lineCoords == PRTextLineCoordinates::Page ) {
        bbox = m_transMatrix.map( bbox );
    }
    return bbox;
}

double PRTextLine::width( bool leadTrailSpaces )
{
    // Compute the bbox if necessary.
    if( m_modified ) {
        this->computeCacheData();
    }
    if( leadTrailSpaces ) {
        return m_bbox.GetWidth();
    }
    else {
        return m_bboxNoLTSpaces.GetWidth();
    }
}
size_t PRTextLine::length( bool countSpaces )
{
    size_t length = 0;
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        length += m_subgroupsWords[i].length( countSpaces );
    }
    return length;
}
PdfeMatrix PRTextLine::transMatrix( PRTextLineCoordinates::Enum startCoord,
                                    PRTextLineCoordinates::Enum endCoord )
{
    // Compute transformation matrix and bbox if necessary.
    if( m_modified ) {
        this->computeCacheData();
    }

    // Get the transformation matrix
    if( startCoord == PRTextLineCoordinates::Line &&
        endCoord == PRTextLineCoordinates::Page ) {
        return m_transMatrix;
    }
    else if( startCoord == PRTextLineCoordinates::Page &&
             endCoord == PRTextLineCoordinates::Line ) {
        return m_transMatrix.inverse();
    }
    // Identity in default case.
    return PdfeMatrix();
}
double PRTextLine::meanFontSize()
{
    // Compute transformation matrix and bbox if necessary.
    if( m_modified ) {
        this->computeCacheData();
    }
    return m_meanFontSize;
}

bool PRTextLine::isEmpty() const
{
    return m_subgroupsWords.empty();
}
bool PRTextLine::isSpace() const
{
    // Check subgroups that make the line.
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        if( !m_subgroupsWords[i].isSpace() ) {
            return false;
        }
    }
    return true;
}

std::vector<PRTextLine::Block*> PRTextLine::horizontalBlocks( double hDistance ) const
{
    // Initialize the vector of blocks with line classic words (no space...).
    std::vector<Block*> hBlocks;

    // Add words as basic blocks.
    PRTextGroupWords::Subgroup subGrpWord;
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        const PRTextGroupWords::Subgroup& subgroup = m_subgroupsWords[i];

        // Words in the subgroup.
        for( size_t j = 0 ; j < subgroup.group()->nbWords() ; ++j ) {
            subGrpWord.init( *subgroup.group(), false );
            const PRTextWord* pWord = subgroup.word( j );

            // Add block corresponding to the word.
            if( pWord && pWord->type() == PRTextWordType::Classic ) {
                subGrpWord.setInside( j, true );
                hBlocks.push_back( new Block( this, subGrpWord, false, true ) );
            }
        }
    }
    // Empty vector... Must be a space line!
    if( hBlocks.empty() ) {
        return hBlocks;
    }

    // Horizontal sort of blocks.
    std::sort( hBlocks.begin(), hBlocks.end(), Block::horizontalCompPtr );

    // Merge blocks.
    std::vector<Block*>::iterator lIt, rIt;
    PdfRect lBBox, rBBox;
    for( rIt = hBlocks.begin()+1 ; rIt != hBlocks.end() ; ) {
        // Get left and right bounding boxes.
        lIt = rIt - 1;
        lBBox = (*lIt)->bbox();
        rBBox = (*rIt)->bbox();

        // Merge if horizontal overlap.
        if( rBBox.GetLeft() - ( lBBox.GetLeft()+lBBox.GetWidth() ) <= hDistance ) {
            (*lIt)->merge( *(*rIt) );
            rIt = hBlocks.erase( rIt );
        }
        else {
            ++rIt;
        }
    }
    return hBlocks;
}

std::list<PRTextLine::Block> PRTextLine::horizontalBlocksList(double hDistance) const
{
    // Initialize the list of blocks with line classic words (no space...).
    std::list<Block> hBlocksList;
    PRTextGroupWords::Subgroup subGrpWord;
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        const PRTextGroupWords::Subgroup& subgroup = m_subgroupsWords[i];

        // Words in the subgroup.
        for( size_t j = 0 ; j < subgroup.group()->nbWords() ; ++j ) {
            subGrpWord.init( *subgroup.group(), false );
            const PRTextWord* pWord = subgroup.word( j );

            // Add block corresponding to the word.
            if( pWord && pWord->type() == PRTextWordType::Classic ) {
                subGrpWord.setInside( j, true );

                hBlocksList.resize( hBlocksList.size()+1 );
                hBlocksList.back().init( this, subGrpWord, false, true );
                //hBlocksList.push_back( Block( this, subGrpWord, false, true )  );
            }
        }
    }
    // Empty list... Must be a space line!
    if( hBlocksList.empty() ) {
        return hBlocksList;
    }

    // Horizontal sort of blocks.
    hBlocksList.sort( Block::horizontalComp );

    // Merge blocks.
    std::list<Block>::iterator lIt, rIt;
    PdfRect lBBox, rBBox;
    for( rIt = ++hBlocksList.begin() ; rIt != hBlocksList.end() ; ) {
        // Get left and right bounding boxes.
        lIt = rIt;
        --lIt;

        lBBox = lIt->bbox();
        rBBox = rIt->bbox();

        // Merge if horizontal overlap.
        if( rBBox.GetLeft() - ( lBBox.GetLeft()+lBBox.GetWidth() ) <= hDistance ) {
            lIt->merge( *rIt );
            rIt = hBlocksList.erase( rIt );
        }
        else {
            ++rIt;
        }
    }
    return hBlocksList;
}

void PRTextLine::computeCacheData()
{
    // Remove empty subgroups.
    this->clearEmptySubgroups();

    // Compute bounding box and transformation matrix.
    this->computeBBoxes();

    // Reset modified parameter.
    m_modified = false;
}
void PRTextLine::computeBBoxes() const
{
    // Not element inside.
    if( m_subgroupsWords.empty() ) {
        m_modified = false;
        m_bbox = PdfRect( 0, 0, 0, 0 );
        m_bboxNoLTSpaces = PdfRect( 0, 0, 0, 0 );
        m_transMatrix = PdfeMatrix();
    }

    // We assume that words have a zero angle between them.
    // That should usually be the case...

    // Use the rotation component of the first group for the transformation matrix.
    PdfeORect unitRect;
    unitRect = m_subgroupsWords[0].group()->getGlobalTransMatrix().map( unitRect );
    PdfeVector unitVect = unitRect.direction();

    PdfeMatrix transMat;
    transMat(0,0) =  unitVect(0);    transMat(0,1) = unitVect(1);
    transMat(1,0) = -unitVect(1);    transMat(1,1) = unitVect(0);
    PdfeMatrix invTransMat = transMat.inverse();

    // Left, Bottom, Right and Top coordinates of the bounding box.
    double left, bottom, right, top;
    left = bottom = std::numeric_limits<double>::max();
    right = top = -std::numeric_limits<double>::max();

    // Left, Bottom, Right and Top coordinates of the No LT bounding box.
    double leftNoLT, bottomNoLT, rightNoLT, topNoLT;
    leftNoLT = bottomNoLT = std::numeric_limits<double>::max();
    rightNoLT = topNoLT = -std::numeric_limits<double>::max();

    // Mean coordinate of the base line.
    double meanBaseCoord = 0.0;
    double totalWidth = 0.0;

    // Mean font size.
    m_meanFontSize = 0.0;

    // Big loop on subgroups of words.
    PdfeORect subGpBBox;
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        // Update line bbox coordinates.
        subGpBBox = m_subgroupsWords[i].bbox( PRTextWordCoordinates::Page, true, true );
        subGpBBox = invTransMat.map( subGpBBox );

        left = std::min( left, subGpBBox.leftBottomX() );
        bottom = std::min( bottom,  subGpBBox.leftBottomY() );
        right = std::max( right, subGpBBox.rightTopX() );
        top = std::max( top, subGpBBox.leftTopY() );

        // Update no LT line bbox coordinates, if the width is positive.
        subGpBBox = m_subgroupsWords[i].bbox( PRTextWordCoordinates::Page, false, true );
        subGpBBox = invTransMat.map( subGpBBox );

        if( subGpBBox.width() > 0 && subGpBBox.height() > 0 ) {
            leftNoLT = std::min( leftNoLT, subGpBBox.leftBottomX() );
            bottomNoLT = std::min( bottomNoLT, subGpBBox.leftBottomY() );
            rightNoLT = std::max( rightNoLT, subGpBBox.rightTopX() );
            topNoLT = std::max( topNoLT, subGpBBox.leftTopY() );
        }

        // Update mean base coordinate and font size.
        subGpBBox = m_subgroupsWords[i].bbox( PRTextWordCoordinates::Page, true, false );
        subGpBBox = invTransMat.map( subGpBBox );
        double width = subGpBBox.width();

        totalWidth += width;
        meanBaseCoord += subGpBBox.leftBottomY() * width;
        m_meanFontSize += m_subgroupsWords[i].group()->fontSize() * width;
    }
    meanBaseCoord = meanBaseCoord / totalWidth;
    m_meanFontSize = m_meanFontSize / totalWidth;

    // Transformation matrix used for the line. Set such the (0,0) corresponds to (left, meanBaseCoord),
    // and the rotation component is transMat.
    PdfeMatrix rescaleMat;
    rescaleMat(2,0) = left;
    rescaleMat(2,1) = meanBaseCoord;
    m_transMatrix = rescaleMat * transMat;

    // Set bounding boxes.
    if( right >= left && top >= bottom ) {
        m_bbox.SetLeft( 0.0 );
        m_bbox.SetBottom( bottom - meanBaseCoord );
        m_bbox.SetWidth( right - left );
        m_bbox.SetHeight( top - bottom );
    }
    else {
        m_bbox = PdfRect( 0,0,0,0 );
    }
    if( rightNoLT >= leftNoLT && topNoLT >= bottomNoLT ) {
        m_bboxNoLTSpaces.SetLeft( leftNoLT - left  );
        m_bboxNoLTSpaces.SetBottom( bottomNoLT - meanBaseCoord );
        m_bboxNoLTSpaces.SetWidth( rightNoLT - leftNoLT );
        m_bboxNoLTSpaces.SetHeight( topNoLT - bottomNoLT );
    }
    else {
        m_bboxNoLTSpaces = PdfRect( 0,0,0,0 );
    }
}

bool PRTextLine::compareGroupIndex( PRTextLine* pLine1, PRTextLine* pLine2 )
{
    // Compare minimum group index found in each line.
    return ( pLine1->minGroupIndex() < pLine2->minGroupIndex() );
}

//**********************************************************//
//                     PRTextLine::Block                    //
//**********************************************************//
PRTextLine::Block::Block()
{
    this->init();
}
PRTextLine::Block::Block( const PRTextLine* pLine,
                          const PRTextGroupWords::Subgroup& subgroup,
                          bool leadTrailSpaces, bool useBottomCoord )
{
    // Initialize block.
    this->init( pLine, subgroup, leadTrailSpaces, useBottomCoord );
}

void PRTextLine::Block::init()
{
    // Empty block.
    m_pLine = NULL;
    m_subgroupsInside.clear();
    m_subgroupsWords.clear();
    m_bbox = PdfRect( 0, 0, 0, 0 );
}
void PRTextLine::Block::init( const PRTextLine* pLine,
                              const PRTextGroupWords::Subgroup& subgroup,
                              bool leadTrailSpaces, bool useBottomCoord )
{
    // Check the group of words belongs to the line.
    if( !pLine  ) {
        this->init();
        return;
    }
    long idxSubgroup = pLine->hasGroupWords( subgroup.group() );
    if( idxSubgroup == -1 ) {
        this->init();
        return;
    }

    // Initialize members.
    m_pLine = const_cast<PRTextLine*>( pLine );
    m_subgroupsInside.resize( pLine->nbSubgroups(), false );
    m_subgroupsInside.at( idxSubgroup ) = true;

    m_subgroupsWords.resize( pLine->nbSubgroups() );
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        m_subgroupsWords[i].init( *pLine->subgroup( i ).group(), false );
    }
    m_subgroupsWords.at( idxSubgroup ) = PRTextGroupWords::Subgroup::intersection( subgroup,
                                                                                   pLine->subgroup( idxSubgroup ) );

    // Compute bounding box.
    PdfeORect bbox = subgroup.bbox( PRTextWordCoordinates::Page, leadTrailSpaces, useBottomCoord );
    bbox = m_pLine->transMatrix( PRTextLineCoordinates::Page,
                                 PRTextLineCoordinates::Line ).map( bbox );

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
    // Merge subgroups with the second block.
    for( size_t i = 0 ; i < m_subgroupsInside.size() ; ++i ) {
        m_subgroupsInside[i] = m_subgroupsInside[i] || block2nd.m_subgroupsInside[i];
        m_subgroupsWords[i] = PRTextGroupWords::Subgroup::reunion( m_subgroupsWords[i],
                                                                   block2nd.m_subgroupsWords[i] );
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
bool PRTextLine::Block::horizontalComp( const PRTextLine::Block& block1, const PRTextLine::Block& block2 )
{
    return ( block1.m_bbox.GetLeft() < block2.m_bbox.GetLeft() );
}
bool PRTextLine::Block::horizontalCompPtr( PRTextLine::Block* pBlock1, PRTextLine::Block* pBlock2 )
{
    return (  pBlock1->m_bbox.GetLeft() < pBlock2->m_bbox.GetLeft() );
}

}
