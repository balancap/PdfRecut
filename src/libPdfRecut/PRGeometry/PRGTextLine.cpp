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

#include "PRGTextLine.h"

#include "PRGSubDocument.h"
#include "PRGPage.h"
#include "PRGTextPage.h"
#include "PRGTextStatistics.h"

#include "PRUtils.h"

#include <podofo/podofo.h>
#include <limits>
#include <algorithm>

using namespace PoDoFo;
using namespace PoDoFoExtended;

namespace PdfRecut {

//**********************************************************//
//                        PRGTextLine                       //
//**********************************************************//
PRGTextLine::PRGTextLine( PRGTextPage* textPage )
{
    this->init();
    m_textPage = textPage;
}
PRGTextLine::~PRGTextLine()
{
}

void PRGTextLine::init()
{
    m_textPage = NULL;
    m_lineIndex = -1;
    m_subgroupsWords.clear();
    m_resetCachedData = false;
    m_data.init();
}

void PRGTextLine::addGroupWords( PRGTextGroupWords* pGroupWords )
{
    if( pGroupWords ) {
        // Add the complete subgroup to the line.
        this->addSubgroupWords( PRGTextGroupWords::Subgroup( *pGroupWords ) );
    }
}
void PRGTextLine::addSubgroupWords( const PRGTextGroupWords::Subgroup& subgroup )
{
    // Add if not empty...
    PRGTextGroupWords* pGroup = subgroup.group();
    if( !pGroup ) {
        return;
    }
    // Look if the group already belongs to the line.
    long idx = this->hasGroupWords( pGroup );
    if( idx != -1 ) {
        // Union of the subgroups...
        m_subgroupsWords[idx] = PRGTextGroupWords::Subgroup::reunion( m_subgroupsWords[idx],
                                                                      subgroup );
    }
    else {
        m_subgroupsWords.push_back( subgroup );
    }
    // Add the line to the group.
    pGroup->addTextLine( this );
    // Line has changed...
    this->modified();
}
void PRGTextLine::rmGroupWords( PRGTextGroupWords* pGroupWords )
{
    // Find the corresponding subgroup and erase it.
    long idx = this->hasGroupWords( pGroupWords );
    if( idx != -1 ) {
        m_subgroupsWords.erase( m_subgroupsWords.begin()+idx );
        // Remove the line from the group.
        pGroupWords->rmTextLine( this );
        // Line has changed...
        this->modified();
    }
}
void PRGTextLine::setSubgroup( size_t idx, const PRGTextGroupWords::Subgroup& subgroup )
{
    // Set if not empty...
    PRGTextGroupWords* pGroup = subgroup.group();
    if( !pGroup ) {
        return;
    }
    // Set subgroup and add reference of the line to the group.
    m_subgroupsWords.at( idx ) = subgroup;
    pGroup->addTextLine( this );
    // Line has changed...
    this->modified();
}
long PRGTextLine::hasGroupWords( PRGTextGroupWords* pGroup ) const
{
    // Look at subgroups...
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        if( m_subgroupsWords[i].group() == pGroup ) {
            return i;
        }
    }
    return -1;
}
long PRGTextLine::hasLCL() const
{
    return this->data()->idxLargeCapitalLetter;
}

long PRGTextLine::minGroupIndex() const
{
    long minGroupIdx = std::numeric_limits<long>::max();
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        minGroupIdx = std::min( minGroupIdx, m_subgroupsWords[i].group()->groupIndex() );
    }
    return minGroupIdx;
}
long PRGTextLine::maxGroupIndex() const
{
    long maxGroupIdx = std::numeric_limits<long>::min();
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        maxGroupIdx = std::max( maxGroupIdx, m_subgroupsWords[i].group()->groupIndex() );
    }
    return maxGroupIdx;
}

PdfeORect PRGTextLine::bbox( PRGTextLineCoordinates::Enum endCoords, bool incLTSpaces, bool incLCL ) const
{
    Data* pData = this->data();

    // Choose the correct bounding box!
    PdfRect bbox;
    if( incLTSpaces ) {
        bbox = pData->bbox;
    }
    else {
        bbox = pData->bboxNoLTSpaces;
    }
    if( incLCL && pData->idxLargeCapitalLetter >= 0 ) {
        bbox = PdfeORect::reunion( pData->bboxLCL, bbox );
    }
    PdfeORect obbox( bbox );

    // Bottom coordinates: TODO.
    //    if( !useBottomCoord ) {
    //        PdfeVector lbPoint = bbox.leftBottom();

    //        // Update height and bottom.
    //        bbox.setHeight( bbox.height()+lbPoint(1) );
    //        lbPoint(1) = 0.0;
    //        bbox.setLeftBottom( lbPoint );
    //    }

    // Transformation if necessary
    if( endCoords != PRGTextLineCoordinates::Line ) {
        obbox = this->transMatrix( PRGTextLineCoordinates::Line,
                                   endCoords).map( obbox );
    }
    return obbox;
}
PdfeORect PRGTextLine::bboxLCL( PRGTextLineCoordinates::Enum endCoords ) const
{
    Data* pData = this->data();
    PdfeORect obbox = pData->bboxLCL;

    // Transformation if necessary
    if( endCoords != PRGTextLineCoordinates::Line ) {
        obbox = this->transMatrix( PRGTextLineCoordinates::Line,
                                   endCoords).map( obbox );
    }
    return obbox;
}

double PRGTextLine::width( PRGTextLineCoordinates::Enum endCoords, bool leadTrailSpaces ) const
{
    PdfeORect obbox = this->bbox( endCoords, leadTrailSpaces, true );
    return obbox.width();
}
double PRGTextLine::widthCumulative( PRGTextLineCoordinates::Enum endCoords, bool incSpaces ) const
{
    Data* pData = this->data();
    double width;
    if( incSpaces ) {
        width = pData->widthCumul;
    }
    else {
        width = pData->widthCumulNoSpaces;
    }
    // Coordinates system expected?
    if( endCoords != PRGTextLineCoordinates::Line ) {
        PdfeORect orect( width, 1.0 );
        orect = this->transMatrix( PRGTextLineCoordinates::Line, endCoords ).map( orect );
        return orect.width();
    }
    return width;
}
size_t PRGTextLine::length( bool countSpaces ) const
{
    size_t length = 0;
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        length += m_subgroupsWords[i].length( countSpaces );
    }
    return length;
}
QString PRGTextLine::toUnicode( bool incSpaces, bool smartSpaces ) const
{
    QString ustr;
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        ustr.append( m_subgroupsWords[i].toUnicode( incSpaces, smartSpaces ) );
    }
    return ustr;
}

PdfeMatrix PRGTextLine::transMatrix( PRGTextLineCoordinates::Enum startCoord,
                                     PRGTextLineCoordinates::Enum endCoord ) const
{
    Data* pData = this->data();
    // Compute the transformation matrix
    if( startCoord == PRGTextLineCoordinates::Line &&
            endCoord == PRGTextLineCoordinates::Page ) {
        return pData->transMatPage;
    }
    else if( startCoord == PRGTextLineCoordinates::Line &&
             endCoord == PRGTextLineCoordinates::LineDocRescaled ) {
        return pData->transMatDocRescale;
    }
    else if( startCoord == PRGTextLineCoordinates::Page &&
             endCoord == PRGTextLineCoordinates::Line ) {
        return pData->transMatPage.inverse();
    }
    else if( startCoord == PRGTextLineCoordinates::Page &&
             endCoord == PRGTextLineCoordinates::LineDocRescaled ) {
        return (pData->transMatPage.inverse() * pData->transMatDocRescale);
    }
    else if( startCoord == PRGTextLineCoordinates::LineDocRescaled &&
             endCoord == PRGTextLineCoordinates::Line ) {
        return pData->transMatDocRescale.inverse();
    }
    else if( startCoord == PRGTextLineCoordinates::LineDocRescaled &&
             endCoord == PRGTextLineCoordinates::Page ) {
        return (pData->transMatDocRescale.inverse() * pData->transMatPage);
    }
    // Identity in default case.
    return PdfeMatrix();
}
double PRGTextLine::meanFontSize() const
{
    return this->data()->meanFontSize;
}

bool PRGTextLine::isEmpty() const
{
    return m_subgroupsWords.empty();
}
bool PRGTextLine::isSpace() const
{
    // Check subgroups that make the line.
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        if( !m_subgroupsWords[i].isSpace() ) {
            return false;
        }
    }
    return true;
}

std::vector<PRGTextLine::Block*> PRGTextLine::horizontalBlocks( double hDistance ) const
{
    // Initialize the vector of blocks with line classic words (no space...).
    std::vector<Block*> hBlocks;

    // Add words as basic blocks.
    PRGTextGroupWords::Subgroup subGrpWord;
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        const PRGTextGroupWords::Subgroup& subgroup = m_subgroupsWords[i];

        // Words in the subgroup.
        for( size_t j = 0 ; j < subgroup.group()->nbWords() ; ++j ) {
            subGrpWord.init( *subgroup.group(), false );
            const PRGTextWord* pWord = subgroup.word( j );

            // Add block corresponding to the word.
            if( pWord && pWord->type() == PRGTextWordType::Classic ) {
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

std::list<PRGTextLine::Block> PRGTextLine::horizontalBlocksList(double hDistance) const
{
    // Initialize the list of blocks with line classic words (no space...).
    std::list<Block> hBlocksList;
    PRGTextGroupWords::Subgroup subGrpWord;
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        const PRGTextGroupWords::Subgroup& subgroup = m_subgroupsWords[i];

        // Words in the subgroup.
        for( size_t j = 0 ; j < subgroup.group()->nbWords() ; ++j ) {
            subGrpWord.init( *subgroup.group(), false );
            const PRGTextWord* pWord = subgroup.word( j );

            // Add block corresponding to the word.
            if( pWord && pWord->type() == PRGTextWordType::Classic ) {
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

void PRGTextLine::modified()
{
    // Clear and sort subgroups
    this->clearEmptySubgroups();
    std::sort( m_subgroupsWords.begin(), m_subgroupsWords.end(), PRGTextGroupWords::Subgroup::compareGroupIndex );

    m_resetCachedData = true;
}
void PRGTextLine::clearEmptySubgroups()
{
    std::vector<PRGTextGroupWords::Subgroup>::iterator it;
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
}
void PRGTextLine::computeCacheData() const
{
    // Reset cache data.
    m_data.init();
    // Not element inside the line.
    if( m_subgroupsWords.empty() ) {
        m_resetCachedData = false;
        return;
    }

    // Compute transformation matrices/statistics and bbox.
    // Assume there is no LCL.
    this->computeTransMat();
    this->computeStatistics();
    this->computeBBox();
    this->computeRescaledData();

    // Detect LCL, and update information if necessary.
    this->findLargeCapitalLetter();
    if( m_data.idxLargeCapitalLetter != -1 ) {
        this->computeStatistics();
        this->computeBBox();
        this->computeRescaledData();
    }

    // Reset modified parameter.
    m_resetCachedData = false;
}
void PRGTextLine::computeTransMat() const
{
    // We assume that words have an angle = 0° between them.
    // That should be the case if we want a consistent line!

    // Use the rotation component of the first group to set the transformation matrix.
    PdfeORect unitRect;
    unitRect = m_subgroupsWords[0].group()->getGlobalTransMatrix().map( unitRect );
    PdfeVector unitVect = unitRect.direction();

    m_data.transMatPage(0,0) =  unitVect(0);    m_data.transMatPage(0,1) = unitVect(1);
    m_data.transMatPage(1,0) = -unitVect(1);    m_data.transMatPage(1,1) = unitVect(0);
}
void PRGTextLine::computeStatistics() const
{
    // Reset mean values.
    m_data.meanFontSize = 0.0;
    m_data.meanHeight = 0.0;

    size_t lengthCumul = 0;
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        PdfeORect bbox = m_subgroupsWords[i].bbox( PRGTextWordCoordinates::Page, true, true );
        size_t length = m_subgroupsWords[i].length( true );

        lengthCumul += length;
        m_data.meanFontSize += m_subgroupsWords[i].group()->fontSize() * length;
        m_data.meanHeight += bbox.height() * length;
    }
    m_data.meanFontSize = m_data.meanFontSize / lengthCumul;
    m_data.meanHeight = m_data.meanHeight / lengthCumul;
}
void PRGTextLine::findLargeCapitalLetter() const
{
    // Find the LCL: should be a single character closed to left coordinates
    // of the line bbox and sufficiently large.
    const size_t maxLength = 1;
    const double minScale = 1.7;
    const double meanWidth = 1. / m_data.transMatDocRescale(0,0);
    const PdfeMatrix invTransMat = m_data.transMatPage.inverse();

    // Look at every subgroup.
    m_data.idxLargeCapitalLetter = -1;
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        if( m_subgroupsWords[i].length( false ) <= maxLength ) {
            PdfeORect bbox = m_subgroupsWords[i].bbox( PRGTextWordCoordinates::Page, false, true );
            bbox = invTransMat.map( bbox );
            QString ustr = m_subgroupsWords[i].toUnicode( false, false );

            // Does the subgroup satisfy the necessary conditions?
            if( m_data.meanHeight * minScale <= bbox.height() &&
                    bbox.leftBottomX() <= m_data.bbox.GetLeft() + meanWidth &&
                    //bbox.width() >= meanWidth &&
                    QStringIsLettersNumbers( ustr ) ) {
                m_data.idxLargeCapitalLetter = i;
                return;
            }
        }
    }
}
void PRGTextLine::computeBBox() const
{
    PdfeMatrix invTransMat = m_data.transMatPage.inverse();

    // Left, Bottom, Right and Top coordinates of the bounding box.
    double left, bottom, right, top;
    left = bottom = std::numeric_limits<double>::max();
    right = top = -std::numeric_limits<double>::max();

    // Left, Bottom, Right and Top coordinates of the No LT bounding box.
    double leftNoLT, bottomNoLT, rightNoLT, topNoLT;
    leftNoLT = bottomNoLT = std::numeric_limits<double>::max();
    rightNoLT = topNoLT = -std::numeric_limits<double>::max();

    // Big loop on subgroups of words!
    PdfeORect bbox;
    for( long i = 0 ; i < long(m_subgroupsWords.size()) ; ++i ) {
        // Update line bbox coordinates.
        bbox = m_subgroupsWords[i].bbox( PRGTextWordCoordinates::Page, true, true );
        bbox = invTransMat.map( bbox );

        // Check if it is the large capital letter.
        if( i != m_data.idxLargeCapitalLetter ) {
            left = std::min( left, bbox.leftBottomX() );
            bottom = std::min( bottom,  bbox.leftBottomY() );
            right = std::max( right, bbox.rightTopX() );
            top = std::max( top, bbox.leftTopY() );
        }
        m_data.widthCumul += bbox.width();

        // Update no LT line bbox coordinates, if the width is positive.
        bbox = m_subgroupsWords[i].bbox( PRGTextWordCoordinates::Page, false, true );
        bbox = invTransMat.map( bbox );

        // Update LCL bounding box.
        if( i == m_data.idxLargeCapitalLetter ) {
            m_data.bboxLCL = PdfRect( bbox.leftBottomX(), bbox.leftBottomY(),
                                      bbox.width(), bbox.height() );
        }
        if( bbox.width() > 0 && bbox.height() > 0 &&
                i != m_data.idxLargeCapitalLetter ) {
            leftNoLT = std::min( leftNoLT, bbox.leftBottomX() );
            bottomNoLT = std::min( bottomNoLT, bbox.leftBottomY() );
            rightNoLT = std::max( rightNoLT, bbox.rightTopX() );
            topNoLT = std::max( topNoLT, bbox.leftTopY() );
        }
        if( !m_subgroupsWords[i].isSpace() ) {
            m_data.widthCumulNoSpaces += bbox.width();
        }
    }
    // Set bounding boxes.
    if( right >= left && top >= bottom ) {
        m_data.bbox.SetLeft( left );
        m_data.bbox.SetBottom( bottom );
        m_data.bbox.SetWidth( right - left );
        m_data.bbox.SetHeight( top - bottom );
    }
    else {
        m_data.bbox = PdfRect( 0,0,0,0 );
    }
    if( rightNoLT >= leftNoLT && topNoLT >= bottomNoLT ) {
        m_data.bboxNoLTSpaces.SetLeft( leftNoLT  );
        m_data.bboxNoLTSpaces.SetBottom( bottomNoLT );
        m_data.bboxNoLTSpaces.SetWidth( rightNoLT - leftNoLT );
        m_data.bboxNoLTSpaces.SetHeight( topNoLT - bottomNoLT );
    }
    else {
        m_data.bboxNoLTSpaces = PdfRect( 0,0,0,0 );
    }
}
void PRGTextLine::computeRescaledData() const
{
    // Translate line transformation matrix.
    double left = std::min( m_data.bbox.GetLeft(), m_data.bboxNoLTSpaces.GetLeft() );
    if( m_data.idxLargeCapitalLetter >= 0 ) {
        left = std::min( left, m_data.bboxLCL.GetLeft() );
    }
    PdfeMatrix translationMat;
    translationMat(2,0) = left;
    translationMat(2,1) = 0.0;      // TODO ?
    m_data.transMatPage = translationMat * m_data.transMatPage;

    // Update bounding boxes accordingly.
    m_data.bbox.SetLeft( m_data.bbox.GetLeft() - left );
    m_data.bboxNoLTSpaces.SetLeft( m_data.bboxNoLTSpaces.GetLeft() - left );
    if( m_data.idxLargeCapitalLetter >= 0 ) {
        m_data.bboxLCL.SetLeft( m_data.bboxLCL.GetLeft() - left );
    }

    // Document rescale transformation matrix.
    // Use Sub-document text statistics.
    m_data.transMatDocRescale.init();
    if( m_textPage ) {
        // Retrieve mean character width used for rescaling.
        const PRGTextStatistics& textStats = m_textPage->page()->parent()->textStatistics();
        const size_t minSamplingSize = 200;
        double meanWidth;
        // Letters and numbers are sufficient?
        if( textStats.variable( PRGTextVariables::CharLNWidth ).size() >= minSamplingSize ) {
            meanWidth = textStats.variable( PRGTextVariables::CharLNWidth ).mean();
        }
        // Take all characters.
        else {
            meanWidth = textStats.variable( PRGTextVariables::CharAllWidth ).mean();
        }
        // Line transformation matrix is only a rotation.
        // => do not need to apply it to get mean width in line coordinates.
        if( meanWidth ) {
            m_data.transMatDocRescale(0,0)
                    = m_data.transMatDocRescale(1,1)
                    = 1. / meanWidth;
        }
    }
}

bool PRGTextLine::compareGroupIndex( PRGTextLine* pLine1, PRGTextLine* pLine2 )
{
    // Compare minimum group index found in each line.
    return ( pLine1->minGroupIndex() < pLine2->minGroupIndex() );
}

//**********************************************************//
//                    PRGTextLine::Block                    //
//**********************************************************//
PRGTextLine::Block::Block()
{
    this->init();
}
PRGTextLine::Block::Block( const PRGTextLine* pLine,
                           const PRGTextGroupWords::Subgroup& subgroup,
                           bool leadTrailSpaces, bool useBottomCoord )
{
    // Initialize block.
    this->init( pLine, subgroup, leadTrailSpaces, useBottomCoord );
}

void PRGTextLine::Block::init()
{
    // Empty block.
    m_pLine = NULL;
    m_subgroupsInside.clear();
    m_subgroupsWords.clear();
    m_bbox = PdfRect( 0, 0, 0, 0 );
}
void PRGTextLine::Block::init( const PRGTextLine* pLine,
                               const PRGTextGroupWords::Subgroup& subgroup,
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
    m_pLine = const_cast<PRGTextLine*>( pLine );
    m_subgroupsInside.resize( pLine->nbSubgroups(), false );
    m_subgroupsInside.at( idxSubgroup ) = true;

    m_subgroupsWords.resize( pLine->nbSubgroups() );
    for( size_t i = 0 ; i < m_subgroupsWords.size() ; ++i ) {
        m_subgroupsWords[i].init( *pLine->subgroup( i ).group(), false );
    }
    m_subgroupsWords.at( idxSubgroup ) = PRGTextGroupWords::Subgroup::intersection( subgroup,
                                                                                    pLine->subgroup( idxSubgroup ) );

    // Compute bounding box.
    PdfeORect bbox = subgroup.bbox( PRGTextWordCoordinates::Page, leadTrailSpaces, useBottomCoord );
    bbox = m_pLine->transMatrix( PRGTextLineCoordinates::Page,
                                 PRGTextLineCoordinates::Line ).map( bbox );

    m_bbox.SetLeft( bbox.leftBottomX() );
    m_bbox.SetBottom( bbox.leftBottomY() );
    m_bbox.SetWidth( bbox.width() );
    m_bbox.SetHeight( bbox.height() );
}

void PRGTextLine::Block::merge( const PRGTextLine::Block& block2nd )
{
    // Check they have a common parent line.
    if( m_pLine != block2nd.m_pLine ) {
        return;
    }
    // Merge subgroups with the second block.
    for( size_t i = 0 ; i < m_subgroupsInside.size() ; ++i ) {
        m_subgroupsInside[i] = m_subgroupsInside[i] || block2nd.m_subgroupsInside[i];
        m_subgroupsWords[i] = PRGTextGroupWords::Subgroup::reunion( m_subgroupsWords[i],
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

PdfRect PRGTextLine::Block::bbox() const
{
    return m_bbox;
}
bool PRGTextLine::Block::horizontalComp( const PRGTextLine::Block& block1, const PRGTextLine::Block& block2 )
{
    return ( block1.m_bbox.GetLeft() < block2.m_bbox.GetLeft() );
}
bool PRGTextLine::Block::horizontalCompPtr( PRGTextLine::Block* pBlock1, PRGTextLine::Block* pBlock2 )
{
    return (  pBlock1->m_bbox.GetLeft() < pBlock2->m_bbox.GetLeft() );
}

//**********************************************************//
//                     PRGTextLine::Data                    //
//**********************************************************//
void PRGTextLine::Data::init()
{
    transMatPage.init();
    transMatDocRescale.init();
    bbox = PdfRect( 0,0,0,0 );
    bboxNoLTSpaces = PdfRect( 0,0,0,0 );
    bboxLCL = PdfRect( 0,0,0,0 );
    idxLargeCapitalLetter = -1;
    widthCumul = 0.0;
    widthCumulNoSpaces = 0.0;
    meanFontSize = 0.0;
}

}
