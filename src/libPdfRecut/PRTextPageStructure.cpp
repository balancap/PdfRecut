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

#include "PRTextPageStructure.h"
#include "PdfeUtils.h"

#include <podofo/podofo.h>
#include <QtCore>
#include <QtGui>
#include <qrgb.h>

#include <algorithm>
#include <fstream>

using namespace PoDoFo;
using namespace PoDoFoExtended;

namespace PdfRecut {

PRTextPageStructure::PRTextPageStructure( PRDocument* document,
                                          long pageIndex ) :
    PRRenderPage( document, pageIndex )
{
    // Clear vectors content.
    m_pGroupsWords.clear();
    m_pTextLines.clear();
}

PRTextPageStructure::~PRTextPageStructure()
{
    this->clearContent();
}

void PRTextPageStructure::clearContent()
{
    // Delete groups of words.
    std::for_each( m_pGroupsWords.begin(), m_pGroupsWords.end(), delete_ptr_fctor<PRTextGroupWords>() );

    // Delete text lines.
    std::for_each( m_pTextLines.begin(), m_pTextLines.end(), delete_ptr_fctor<PRTextLine>() );
}

void PRTextPageStructure::detectGroupsWords()
{
    // Set rendering parameters to empty.
    PRRenderParameters renderParameters;
    renderParameters.initToEmpty();
    renderParameters.resolution = 1.05;

    // Clear content.
    this->clearContent();

    // Analyse the page.
    this->renderPage( renderParameters );
}
void PRTextPageStructure::fTextShowing( const PdfeStreamState& streamState )
{
    // Update text transformation matrix.
    this->textUpdateTransMatrix( streamState );

    // Read the group of words.
    PRTextGroupWords* pGroup = new PRTextGroupWords( this->textReadGroupWords( streamState ) );

    // Empty group -> trash !
    if( !pGroup->nbWords() ) {
        delete pGroup;
        return;
    }
    //group.setGroupIndex( m_groupsWords.size() );  // Already set by PRRenderPage.
    pGroup->setPageIndex( m_pageIndex );

    m_pGroupsWords.push_back( pGroup );
}

void PRTextPageStructure::detectLines()
{
    //  Try to create a line for each group of words.
    for( size_t i = 0 ; i < m_pGroupsWords.size() ; ++i ) {
        this->createLine_Basic( i );
    }

    // Sort lines using group index.
    std::sort( m_pTextLines.begin(), m_pTextLines.end(), PRTextLine::sortLines );

    // Try to merge some lines (inside elements).
    PRTextLine* pLine;
    std::vector<PRTextLine*>::iterator it;
    for( it = m_pTextLines.begin() ; it != m_pTextLines.end() ; ++it ) {
        pLine = this->mergeLines_EnlargeInside( *it );
        it = std::find( m_pTextLines.begin(), m_pTextLines.end(), pLine );
    }

    // Try to merge with elements outside a line.
    /*const size_t nLoops = 8;
    for( size_t n = 0 ; n <= nLoops ; ++n ) {
        // Algorithm parameters for this iteration.
        double ScaleXEnlarge = 0.15 * n;
        double ScaleYEnlarge = 0.05 * n;
        double MaxLineWidth = 8.0 - 0.7*n;

        for( it = m_pTextLines.begin() ; it != m_pTextLines.end() ; ++it ) {
            // Try to merge every line.
            pLine = this->mergeLines_EnlargeOutside( *it,
                                                     ScaleXEnlarge,
                                                     ScaleYEnlarge,
                                                     MaxLineWidth );

            it = std::find( m_pTextLines.begin(), m_pTextLines.end(), pLine );

            // Also merge inside elements in case they appear.
            pLine = this->mergeLines_EnlargeInside( *it );
            it = std::find( m_pTextLines.begin(), m_pTextLines.end(), pLine );
        }
    }

    // Now split lines, using horizontal blocks.
    std::vector<PRTextLine*> pLines;
    for( it = m_pTextLines.begin() ; it != m_pTextLines.end() ; ) {
        pLines = this->splitLines_hBlocks( *it );
        ++it;

        // Add new lines.
        m_pTextLines.insert( it, pLines.begin()+1, pLines.end() );
        it = std::find( m_pTextLines.begin(), m_pTextLines.end(), pLines.back() );
        ++it;
    }*/
}

PRTextLine* PRTextPageStructure::createLine_Basic( size_t idxGroupWords )
{
    // Parameters of the algorithm.
    long MaxSearchGroupWords = 20;
    double MaxHDistanceLB = -5.0;
    double MaxHDistanceUB = 3.0;
    double MaxCumulWidth = 15.0;
    double MinVOverlap = 0.3;

    // Local transformation matrix related to the right group of words.
    PRTextGroupWords& rGroupWords = *( m_pGroupsWords[ idxGroupWords ] );
    PdfeMatrix rGroupTransMat = rGroupWords.getGlobalTransMatrix().inverse();
    PdfeORect rGroupBBox = rGroupWords.bbox( true, true, true );

    // Vector of lines to merge.
    std::vector<PRTextLine*> pLinesToMerge;

    // Cumulative width.
    double lGroupsCumulWidth = 0;
    bool lrGroupLink = false;

    // Try to find a group of words that could belong to a same line.
    long idxGroupLimit = std::max( long(0), long(idxGroupWords) - MaxSearchGroupWords );
    for( long idx = idxGroupWords-1 ; idx >= idxGroupLimit ; --idx ) {
        // Left group of words.
        PRTextGroupWords& lGroupWords = *( m_pGroupsWords[ idx ] );
        PdfeORect lGroupBBox = lGroupWords.bbox( true, true, true  );
        lrGroupLink = false;

        // Get the angle between the two groups: should be less than ~5°.
        if( PdfeVector::angle( rGroupBBox.direction(), lGroupBBox.direction() ) > 0.1  ) {
            continue;
        }

        // At this stage, we can assume that the group has only one unique line. Add one if necessary.
        if( lGroupWords.textLines().empty() ) {
            m_pTextLines.push_back( new PRTextLine() );
            m_pTextLines.back()->addGroupWords( &lGroupWords );
        }
        std::vector<PRTextLine*> lGroupLines = lGroupWords.textLines();
        if( std::find( pLinesToMerge.begin(), pLinesToMerge.end(), lGroupLines[0] ) != pLinesToMerge.end() ) {
            continue;
        }

        // Investigate every subgroup of the left group.
        for( long i = lGroupWords.nbMSubgroups()-1 ; i >=0 ; --i ) {
            // Subgroup bounding box in local coordinates.
            PdfeORect lSubGroupBBoxLocal = lGroupWords.mSubgroup(i).bbox( true, true, true );
            lSubGroupBBoxLocal = rGroupTransMat.map( lSubGroupBBoxLocal );

            // Compare to every subgroup of the right group.
            for( size_t j = 0 ; j < rGroupWords.nbMSubgroups() ; ++j ) {
                PdfeORect rSubGroupBBoxLocal = rGroupWords.mSubgroup(j).bbox( false, true, true );

                // Estimate the horizontal distance and vertical overlap.
                PdfeVector lRBPoint, lRTPoint, rLBPoint, rLTPoint;

                lRBPoint = lSubGroupBBoxLocal.rightBottom();
                lRTPoint = lSubGroupBBoxLocal.rightTop();
                rLBPoint = rSubGroupBBoxLocal.leftBottom();
                rLTPoint = rSubGroupBBoxLocal.leftTop();

                double hDistance = rLBPoint(0) - lRBPoint(0);
                double vOverlap = std::max( 0.0, std::min( rLTPoint(1), lRTPoint(1) ) - std::max( rLBPoint(1), lRBPoint(1) ) );
                double vOverlapR = vOverlap / rSubGroupBBoxLocal.height();
                double vOverlapL = vOverlap / lSubGroupBBoxLocal.height();

                // Conditions to satisfy...
                lrGroupLink = ( hDistance >= MaxHDistanceLB ) &&
                        ( hDistance <= MaxHDistanceUB ) &&
                        ( vOverlapR >= MinVOverlap || vOverlapL >= MinVOverlap );

                // Add line.
                if( lrGroupLink ) {
                    pLinesToMerge.push_back( lGroupLines[0] );
                    break;
                }

            }
            // Line found: break...
            if( lrGroupLink ) {
                break;
            }
        }
        // Add group width (without spaces).
        lGroupsCumulWidth += lGroupWords.width( false );
        if( lGroupsCumulWidth > MaxCumulWidth ) {
            break;
        }
    }

    // Add line corresponding to the right group.
    if( rGroupWords.textLines().empty() ) {
        m_pTextLines.push_back( new PRTextLine() );
        m_pTextLines.back()->addGroupWords( &rGroupWords );
        pLinesToMerge.push_back( m_pTextLines.back() );
    }

    // Sort lines to merge using the indexes of their groups.
    std::sort( pLinesToMerge.begin(), pLinesToMerge.end(), PRTextLine::sortLines );

    // Merge lines.
    return this->mergeVectorLines( pLinesToMerge );
}

PRTextLine *PRTextPageStructure::mergeLines_EnlargeInside( PRTextLine* pBaseLine )
{
    // Parameters of the algorithm.
    const double MinBaseHeight = 0.8;
    const double MaxBaseHeight = 2.0;
    const double ScaleXEnlarge = 1.0;
    const double ScaleYEnlarge = 0.7;
    const double MinLineWidth = 3.0;

    // Min and max group indexes to consider.
    long minGrpIdx = pBaseLine->minGroupIndex();
    long maxGrpIdx = pBaseLine->maxGroupIndex();

    // Vector of lines to merge.
    std::vector<PRTextLine*> pLinesToMerge;
    pLinesToMerge.push_back( pBaseLine );

    // Line bounding box (no spaces) and transformation matrix.
    PdfeORect baseLineOBBox = pBaseLine->bbox( false, false, true );
    PdfeMatrix baseTransMat = pBaseLine->transMatrix().inverse();

    // Line too small, do not consider it...
    if( baseLineOBBox.width() <= MinLineWidth ) {
        return pBaseLine;
    }

    // Enlarge bounding box used.
    PdfeORect enlargeOBBox = baseLineOBBox;
    double baseHeight = std::min( std::max( baseLineOBBox.height(), MinBaseHeight ),
                                  MaxBaseHeight );
    enlargeOBBox.enlarge( baseHeight * ScaleXEnlarge,
                          baseHeight * ScaleYEnlarge );
    PdfRect enlargeBBox = enlargeOBBox.toPdfRect();

    // Search with groups in the interval (lineMinGrpIdx,lineMaxGrpIdx)
    for( long i = minGrpIdx ; i <= maxGrpIdx ; ++i ) {
        // Assume the group of words only has one line!
        PRTextLine* pLine = m_pGroupsWords[i]->textLines().at( 0 );

        // Line is already merged.
        if( std::find( pLinesToMerge.begin(), pLinesToMerge.end(), pLine ) != pLinesToMerge.end() ) {
            continue;
        }

        // Line bounding box in base coordinates system (check it is not empty).
        PdfeORect lineOBBox = pLine->bbox( true, false, true );
        if( lineOBBox.width() <= 0.0 || lineOBBox.height() <= 0.0 ) {
            continue;
        }
        lineOBBox = baseTransMat.map( lineOBBox );

        // Check the angle between the two lines: should be less than ~5°.
        if( PdfeVector::angle( enlargeOBBox.direction(), lineOBBox.direction() ) > 0.1  ) {
            continue;
        }

        // Can now approximate with a PdfRect...
        PdfRect lineBBox = lineOBBox.toPdfRect();

        // Check if the line bounding box is inside the enlarge base bbox.
        if( PdfeORect::inside( enlargeBBox, lineBBox ) ) {
            pLinesToMerge.push_back( pLine );
        }
    }
    // Sort lines to merge using the indexes of their groups.
    std::sort( pLinesToMerge.begin(), pLinesToMerge.end(), PRTextLine::sortLines );

    // Merge lines.
    return this->mergeVectorLines( pLinesToMerge );
}

PRTextLine *PRTextPageStructure::mergeLines_EnlargeOutside( PRTextLine* pBaseLine,
                                                            double ScaleXEnlarge,
                                                            double ScaleYEnlarge,
                                                            double MaxLineWidth )
{
    // Parameters of the algorithm.
    const double MinBaseHeight = 0.7;
    const double MaxBaseHeight = 2.0;
    const double MinLineWidth = 2.5;
    const long MaxOutsideGroups = 10;

    // Min and max group indexes to consider.
    long minGrpIdx = std::max( pBaseLine->minGroupIndex() - MaxOutsideGroups, 0L );
    long maxGrpIdx = std::min( pBaseLine->maxGroupIndex() + MaxOutsideGroups,
                               static_cast<long>( m_pGroupsWords.size() )-1 );

    // Vector gathering search bounds.
    std::vector<long>  searchBounds;
    searchBounds.push_back( minGrpIdx );
    searchBounds.push_back( pBaseLine->minGroupIndex() );
    searchBounds.push_back( pBaseLine->maxGroupIndex() );
    searchBounds.push_back( maxGrpIdx );

    // Vector of lines to merge.
    std::vector<PRTextLine*> pLinesToMerge;
    pLinesToMerge.push_back( pBaseLine );

    // Line bounding box (no spaces) and transformation matrix.
    PdfeORect baseLineOBBox = pBaseLine->bbox( false, false, true );
    PdfeMatrix baseTransMat = pBaseLine->transMatrix().inverse();

    // Line too small, do not consider it...
    if( baseLineOBBox.width() <= MinLineWidth ) {
        return pBaseLine;
    }

    // Enlarge bounding box used.
    PdfeORect enlargeOBBox = baseLineOBBox;
    double baseHeight = std::min( std::max( baseLineOBBox.height(), MinBaseHeight ),
                                  MaxBaseHeight );
    enlargeOBBox.enlarge( baseHeight * ScaleXEnlarge,
                          baseHeight * ScaleYEnlarge );
    PdfRect enlargeBBox = enlargeOBBox.toPdfRect();

    // Search groups before and after the line.
    for( size_t n = 0 ; n < 2 ; ++n ) {
        for( long i = searchBounds[n*2] ; i <= searchBounds[n*2+1] ; ++i ) {
            // Assume the group of words only has one line!
            PRTextLine* pLine = m_pGroupsWords[i]->textLines().at( 0 );
            if( std::find( pLinesToMerge.begin(), pLinesToMerge.end(), pLine ) != pLinesToMerge.end() ) {
                continue;
            }
            // Line bounding box in base coordinates system (check it is not empty or too big).
            PdfeORect lineOBBox = pLine->bbox( true, false, true );
            lineOBBox = baseTransMat.map( lineOBBox );
            if( lineOBBox.width() <= 0.0 || lineOBBox.height() <= 0.0 || lineOBBox.width() >= MaxLineWidth ) {
                continue;
            }

            // Check the angle between the two lines: should be less than ~5°.
            if( PdfeVector::angle( enlargeOBBox.direction(), lineOBBox.direction() ) > 0.1  ) {
                //continue;
            }
            // Check if the line bounding box is inside the enlarge base bbox.
            PdfRect lineBBox = lineOBBox.toPdfRect();
            if( PdfeORect::inside( enlargeBBox, lineBBox ) ) {
                pLinesToMerge.push_back( pLine );
            }
        }
    }
    // Sort lines to merge using the indexes of their groups.
    std::sort( pLinesToMerge.begin(), pLinesToMerge.end(), PRTextLine::sortLines );

    // Merge lines.
    return this->mergeVectorLines( pLinesToMerge );
}

PRTextLine* PRTextPageStructure::mergeLines_Inside( PRTextLine* pLine )
{
    // Parameters of the algorithm.
    double MaxDistanceInside = 1.0;
    double MaxWidthInside = 4.0;
    size_t MaxLengthInside = 6;

    // Line min and max indexes.
    long lineMinGrpIdx = pLine->minGroupIndex();
    long lineMaxGrpIdx = pLine->maxGroupIndex();

    // Loop inside the line...
    long grpIdxBegin = lineMinGrpIdx;
    long grpIdxEnd = lineMinGrpIdx;

    // Vector of lines to merge.
    std::vector<PRTextLine*> pLinesToMerge;
    pLinesToMerge.push_back( pLine );

    while( grpIdxBegin <= lineMaxGrpIdx ) {
        // At this stage, we can assume that the group has only one unique line. Add one if necessary.

        // Find a group in the interval that does not belong to the line.
        while( grpIdxBegin <= lineMaxGrpIdx &&
               m_pGroupsWords[ grpIdxBegin ]->textLines().at(0) == pLine ) {
            ++grpIdxBegin;
        }

        // Found one...
        if(  grpIdxBegin <= lineMaxGrpIdx ) {
            // Find the next group from the original line.
            grpIdxEnd = grpIdxBegin;
            while( grpIdxEnd <= lineMaxGrpIdx && m_pGroupsWords[ grpIdxEnd ]->textLines().at(0) != pLine ) {
                ++grpIdxEnd;
            }
            grpIdxBegin--;

            // Let study group indexes such that: grpIdxBegin < idx < grpIdxEnd.
            for( long i = grpIdxBegin+1 ; i < grpIdxEnd ; ++i ) {
                PRTextLine* pLine1 = m_pGroupsWords[i]->textLines().at( 0 );

                // Look at it if the line is not already inside the vector of lines...
                if( std::find( pLinesToMerge.begin(), pLinesToMerge.end(), pLine1 ) == pLinesToMerge.end() ) {
                    // Begin and end minimal distances.
                    double distBegin = m_pGroupsWords[grpIdxBegin]->minDistance( *m_pGroupsWords[i] );
                    double distEnd = m_pGroupsWords[grpIdxEnd]->minDistance( *m_pGroupsWords[i] );

                    bool groupMerge = ( distBegin <= MaxDistanceInside ||
                                        distEnd <= MaxDistanceInside ) &&
                            ( m_pGroupsWords[i]->length( false ) <= MaxLengthInside ||
                              m_pGroupsWords[i]->width( true ) <= MaxWidthInside );

                    // Merge lines...
                    if( groupMerge ) {
                        pLinesToMerge.push_back( pLine1 );
                    }
                }
            }
            // Update index.
            grpIdxBegin = grpIdxEnd;
        }
    }
    // Sort lines to merge using the indexes of their groups.
    std::sort( pLinesToMerge.begin(), pLinesToMerge.end(), PRTextLine::sortLines );

    // Merge lines.
    return this->mergeVectorLines( pLinesToMerge );
}

PRTextLine *PRTextPageStructure::mergeLines_Small( PRTextLine *pLine )
{
    // Parameters of the algorithm.
    double MaxDistance = 1.0;
    double MaxWidthLine = 3.0;
    size_t MaxLengthLine = 5;
    long MaxSearchGroupWords = 30;
    double MaxWidthCumul = 5.0;
    double MaxDistCumul = 5.0;

    // Consider elements in the line if and only if it is sufficiently small.
    if( pLine->width( true ) > MaxWidthLine || pLine->length( false ) > MaxLengthLine ) {
        return pLine;
    }
    PdfeMatrix lineTransMat = pLine->transMatrix().inverse();
    PdfeORect lineBBox = pLine->bbox( false, true, true );
    PdfeVector lineLBPoint = lineBBox.leftBottom();

    // Vector of lines to merge.
    std::vector<PRTextLine*> pLinesToMerge;
    pLinesToMerge.push_back( pLine );

    // Look at groups inside the small line.
    for( size_t i = 0 ; i < pLine->nbSubgroups() ; ++i ) {
        double widthCumulBefore = 0;
        double widthCumulAfter = 0.0;
        double distMaxBefore = 0.0;
        double distMaxAfter = 0.0;
        const PRTextGroupWords::Subgroup& subgroupLine = pLine->subgroup( i );
        PRTextGroupWords* pGroupLine = subgroupLine.group();

        // Groups before the line one.
        long idxBefore = std::max( 0L, pGroupLine->groupIndex() - MaxSearchGroupWords );
        for( long j = idxBefore ; j < pGroupLine->groupIndex() ; ++j ) {
            PRTextGroupWords* pGroup2nd = m_pGroupsWords[j];
            PRTextLine* pLine2nd = pGroup2nd->textLines().at(0);

            widthCumulBefore += pGroup2nd->width( true );

            // Consider the group ?
            if( widthCumulBefore <= MaxWidthCumul && distMaxBefore <= MaxDistCumul &&
                std::find( pLinesToMerge.begin(), pLinesToMerge.end(), pLine2nd ) == pLinesToMerge.end() ) {

                // Distance to the line group.
                double distMin = pGroupLine->minDistance( *pGroup2nd );
                distMaxBefore = std::max( distMaxBefore, pGroupLine->maxDistance( *pGroup2nd ) );

                // Line bounding box.
                PdfeORect lineBBox2nd = pLine2nd->bbox( true, true, true );
                lineBBox2nd = lineTransMat.map( lineBBox2nd );

                // Compute the expected height of the merge line.
                PdfeVector lineLBPoint2nd = lineBBox2nd.leftBottom();
                double heightMerge = std::max( lineLBPoint2nd(1)+lineBBox2nd.height(), lineLBPoint(1)+lineBBox.height() ) -
                         std::min( lineLBPoint2nd(1), lineLBPoint(1) ) ;

                bool merge = ( distMin <= MaxDistance ) && ( pLine2nd->width( true ) <= MaxWidthLine ||
                                                          heightMerge <= lineBBox2nd.height() * 1.4 );


                if( merge ) {
                    pLinesToMerge.push_back( pGroup2nd->textLines().at(0) );
                }
            }
        }

        // Groups after the line one.
        long idxAfter = std::min( static_cast<long>( m_pGroupsWords.size() )-1,
                                pGroupLine->groupIndex() + MaxSearchGroupWords );
        for( long j = pGroupLine->groupIndex()+1 ; j <= idxAfter  ; ++j ) {
            PRTextGroupWords* pGroup2nd = m_pGroupsWords[j];
            PRTextLine* pLine2nd = pGroup2nd->textLines().at((0));

            widthCumulAfter += pGroup2nd->width( true );

            // Consider the group ?
            if( widthCumulAfter <= MaxWidthCumul && distMaxAfter <= MaxDistCumul &&
                std::find( pLinesToMerge.begin(), pLinesToMerge.end(), pLine2nd ) == pLinesToMerge.end() ) {

                // Distance to the line group.
                double distMin = pGroupLine->minDistance( *pGroup2nd );
                distMaxAfter = std::max( distMaxBefore, pGroupLine->maxDistance( *pGroup2nd ) );

                // Line bounding box.
                PdfeORect lineBBox2nd = pLine2nd->bbox( true, true, true );
                lineBBox2nd = lineTransMat.map( lineBBox2nd );

                // Compute the expected height of the merge line.
                PdfeVector lineLBPoint2nd = lineBBox2nd.leftBottom();
                double heightMerge = std::max( lineLBPoint2nd(1)+lineBBox2nd.height(), lineLBPoint(1)+lineBBox.height() ) -
                         std::min( lineLBPoint2nd(1), lineLBPoint(1) ) ;

                bool merge = ( distMin <= MaxDistance ) && ( pLine2nd->width( true ) <= MaxWidthLine ||
                                                             heightMerge <= lineBBox2nd.height() * 1.4 );

                if( merge ) {
                    pLinesToMerge.push_back( pGroup2nd->textLines().at(0) );
                }
            }
        }
    }

    // Sort lines to merge using the indexes of their groups.
    std::sort( pLinesToMerge.begin(), pLinesToMerge.end(), PRTextLine::sortLines );

    // Merge lines.
    return this->mergeVectorLines( pLinesToMerge );
}

std::vector<PRTextLine*> PRTextPageStructure::splitLines_hBlocks( PRTextLine* pLine )
{
    // Parameters of the algorithm.
    double BlockHDistance = 2.0;

    // Vector of lines.
    std::vector<PRTextLine*> pLines;
    pLines.push_back( pLine );

    // Get horizontal blocks inside the line.
    std::vector<PRTextLine::Block*> hBlocks = pLine->horizontalBlocks( BlockHDistance );
    //std::list<PRTextLine::Block> hBlocksList = pLine->horizontalBlocksList( BlockHDistance );

    // Size vector <= 1: nothing much to do...
    if( hBlocks.size() <= 1 ) {
        return pLines;
    }

    // Update the first line.
    PRTextLine::Block& block0 = *hBlocks[0];
    for( size_t j = 0 ; j < block0.nbSubgroups() ; ++j ) {
        pLine->setSubgroup( j, block0.subgroup( j ) );
    }
    pLine->clearEmptySubgroups();

    // Create new lines for other blocks.
    std::vector<PRTextLine::Block*>::iterator it;
    for( it = ++hBlocks.begin() ; it != hBlocks.end() ; ++it ) {
        PRTextLine::Block& block = *(*it);
        pLines.push_back( new PRTextLine() );

        // Add subgroups to the new line.
        for( size_t j = 0 ; j < block.nbSubgroups() ; ++j ) {
            pLines.back()->addSubgroupWords( block.subgroup( j ) );
        }
        pLines.back()->clearEmptySubgroups();
    }
    // Delete blocks.
    std::for_each( hBlocks.begin(), hBlocks.end(), delete_ptr_fctor<PRTextLine::Block>() );

    return pLines;
}

PRTextLine* PRTextPageStructure::mergeVectorLines( const std::vector<PRTextLine*>& pLines )
{
    // Empty vector...
    if( pLines.empty() ) {
        return NULL;
    }

    // Base line.
    PRTextLine* pBaseLine = pLines[0];
    PRTextLine* pLine;

    // Merge every line with the first one.
    std::vector<PRTextLine*>::iterator it;
    for( size_t i = 1 ; i < pLines.size() ; ++i ) {
        pLine = pLines[i];

        // Merge if and only if the line belongs to the page.
        it = std::find( m_pTextLines.begin(), m_pTextLines.end(), pLine );
        if( it != m_pTextLines.end() ) {

            // Copy the subgroups of words.
            for( size_t j = 0 ; j < pLine->nbSubgroups() ; ++j ) {
                const PRTextGroupWords::Subgroup& subgroup = pLine->subgroup( j );

                // Add subgroup to the base line.
                pBaseLine->addSubgroupWords( subgroup );

                // Remove the reference to the line in the group.
                subgroup.group()->rmTextLine( pLine );
            }
            // Remove line from the page vector and delete object.
            m_pTextLines.erase( it );
            delete pLine;
        }
    }
    return pBaseLine;
}

//**********************************************************//
//                      Drawing routines                    //
//**********************************************************//
void PRTextPageStructure::renderTextGroupsWords()
{
    // Rendering parameters.
    PRRenderParameters renderParameters;
    renderParameters.initToEmpty();
//    renderParameters.textPB.fillBrush = new QBrush( Qt::blue );
//    renderParameters.textSpacePB.fillBrush = new QBrush( Qt::blue );
//    renderParameters.textPDFTranslationPB.fillBrush = new QBrush( Qt::blue );

    renderParameters.textPB.drawPen = new QPen( Qt::blue );
    renderParameters.textSpacePB.drawPen = new QPen( Qt::blue );
    renderParameters.textPDFTranslationPB.drawPen = new QPen( Qt::blue );

    m_renderParameters = renderParameters;

    // Draw groups of words.
    QColor groupColor;
    QColor groupColorSpace;
    for( size_t idx = 0 ; idx < m_pGroupsWords.size() ; ++idx ) {
        groupColor.setHsv( idx % 360, 255, 220 );
        groupColorSpace.setHsv( idx % 360, 100, 255 );

//        m_renderParameters.textPB.fillBrush->setColor( groupColor );
//        m_renderParameters.textSpacePB.fillBrush->setColor( groupColorSpace );
//        m_renderParameters.textPDFTranslationPB.fillBrush->setColor( groupColorSpace );

        m_renderParameters.textPB.drawPen->setColor( groupColor );
        m_renderParameters.textSpacePB.drawPen->setColor( groupColorSpace );
        m_renderParameters.textPDFTranslationPB.drawPen->setColor( groupColorSpace );

        this->textDrawGroupWords( *m_pGroupsWords[idx] );
//        this->textDrawMainSubgroups( *m_pGroupsWords[idx] );
//        this->textDrawPdfeORect( m_pGroupsWords[idx]->bbox() );
    }
}
void PRTextPageStructure::renderTextLines()
{
    // Words rendering parameters.
    PRRenderParameters renderParameters;
    renderParameters.initToEmpty();
    renderParameters.textPB.fillBrush = new QBrush( Qt::blue );
    renderParameters.textSpacePB.fillBrush = new QBrush( Qt::blue );
//    renderParameters.textPDFTranslationPB.fillBrush = new QBrush( Qt::blue );
    m_renderParameters = renderParameters;

    // Line rendering pen.
    PRRenderParameters::PRPenBrush linePen;
    linePen.drawPen = new QPen( Qt::blue );
    //linePen.drawPen->setWidthF( 2.0 );

    // Draw lines
    QColor lineColorWord, lineColorSpace, lineColorBBox;

    for( size_t idx = 0 ; idx < m_pTextLines.size() ; ++idx ) {
        lineColorWord.setHsv( idx*36 % 360, 255, 255 );
        lineColorSpace.setHsv( idx*36 % 360, 100, 255 );
        lineColorBBox.setHsv( idx*36 % 360, 255, 200 );

        m_renderParameters.textPB.fillBrush->setColor( lineColorWord );
        m_renderParameters.textSpacePB.fillBrush->setColor( lineColorSpace );
//        m_renderParameters.textPDFTranslationPB.fillBrush->setColor( lineColorSpace );
        linePen.drawPen->setColor( lineColorBBox );

        if( m_pTextLines[idx] ) {
            // Line words.
            this->textDrawLineWords( *m_pTextLines[idx] );

            // Line bounding box.
            this->textDrawPdfeORect( m_pTextLines[idx]->bbox( true, false, true ), linePen );

            // Line blocks.
//            std::vector<PRTextLine::Block*> hBlocks = m_pTextLines[idx]->horizontalBlocks( 2.0 );
//            PdfeMatrix transMat = m_pTextLines[idx]->transMatrix();
//            for( size_t j = 0 ; j < hBlocks.size() ; ++j ) {
//                PdfeORect bbox = transMat.map( hBlocks[j]->bbox() );
//                this->textDrawPdfeORect( bbox, linePen );
//            }
        }
    }
}
void PRTextPageStructure::textDrawPdfeORect( const PdfeORect& orect,
                                             const PRRenderParameters::PRPenBrush& penBrush )
{
    // Compute text rendering matrix.
    m_pagePainter->setTransform( m_pageImgTrans.toQTransform() );

    // Create polygon.
    QPolygonF polygon;
    PdfeVector point = orect.leftBottom();
    polygon << point.toQPoint();

    point = point + orect.direction() * orect.width();
    polygon << point.toQPoint();

    point = point + orect.direction().rotate90() * orect.height();
    polygon << point.toQPoint();

    point = orect.leftBottom() + orect.direction().rotate90() * orect.height();
    polygon << point.toQPoint();

    penBrush.applyToPainter( m_pagePainter );
    m_pagePainter->drawPolygon( polygon );
}

void PRTextPageStructure::textDrawMainSubgroups( const PRTextGroupWords& groupWords )
{
    // Nothing to draw...
    if( !groupWords.nbWords() ) {
        return;
    }

    // Compute text rendering matrix.
    PdfeMatrix textMat;
    textMat = groupWords.getGlobalTransMatrix() * m_pageImgTrans;
    m_pagePainter->setTransform( textMat.toQTransform() );

    // Paint subgroups.
    for( size_t i = 0 ; i < groupWords.nbMSubgroups() ; i++ )
    {
        // Set pen & brush
        m_renderParameters.textPB.applyToPainter( m_pagePainter );

        // Paint word, if the width is positive !
        PdfeORect bbox = groupWords.mSubgroup(i).bbox( false, true, true );
        PdfeVector lb = bbox.leftBottom();
        m_pagePainter->drawRect( QRectF( lb(0) , lb(1), bbox.width(), bbox.height() ) );
    }
}

void PRTextPageStructure::textDrawLineWords( const PRTextLine& line )
{
    // Draw the subgroups of words that belong to the line.
    for( size_t idx = 0 ; idx < line.nbSubgroups() ; ++idx ) {
        this->textDrawSubgroupWords( line.subgroup( idx ) );
//        this->textDrawMainSubgroups( *line.subgroup( idx ).group() );
    }
}


}
