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

#include "PRGTextPage.h"

#include "PRDocument.h"
#include "PRRenderPage.h"

#include "PRGDocument.h"
#include "PRGSubDocument.h"
#include "PRGPage.h"
#include "PRGTextLine.h"
#include "PRGTextWords.h"

#include "PdfeUtils.h"

#include "QsLog/QsLog.h"

#include <podofo/podofo.h>
#include <QtCore>
#include <QtGui>
#include <qrgb.h>

#include <algorithm>
#include <fstream>

using namespace PoDoFo;
using namespace PoDoFoExtended;

namespace PdfRecut {

PRGTextPage::PRGTextPage( PRGPage* page ) :
    QObject( page ),
    PdfeCanvasAnalysis(),
    m_page( page )
{
    // Clear vectors content.
    m_pGroupsWords.clear();
    m_pTextLines.clear();

    // Connect signals.
    QObject::connect( this, SIGNAL(dataLoaded(PRGPage*)),
                      page, SIGNAL(dataLoaded(PRGPage*)) );
}
PRGTextPage::~PRGTextPage()
{
    this->clear();
}
void PRGTextPage::loadData()
{
    // Analyse page content.
    m_nbGroupsStream = 0;
    m_nbGroupsPage = 0;
    this->analyseContents( m_page->podofoPage(), PdfeGraphicsState(), PdfeResources() );
    // Send signal.
    emit dataLoaded( this->page() );
}
void PRGTextPage::clearData()
{
    // Clear groups data.
    for( size_t  i = 0 ; i < m_pGroupsWords.size() ; ++i ) {
        m_pGroupsWords[i]->clearData();
    }
}
void PRGTextPage::clear()
{
    // Delete groups of words.
    std::for_each( m_pGroupsWords.begin(), m_pGroupsWords.end(), delete_ptr_fctor<PRGTextGroupWords>() );
    m_pGroupsWords.clear();
    m_nbGroupsStream = 0;
    m_nbGroupsPage = 0;

    // Delete text lines.
    std::for_each( m_pTextLines.begin(), m_pTextLines.end(), delete_ptr_fctor<PRGTextLine>() );
    m_pTextLines.clear();
}

void PRGTextPage::detectLines()
{
    QLOG_INFO() << QString( "<PRGTextPage> Detection of page's text lines (index: %1)." )
                   .arg( m_page->pageIndex() )
                   .toAscii().constData();

    // Clear existing lines.
    std::for_each( m_pTextLines.begin(), m_pTextLines.end(), delete_ptr_fctor<PRGTextLine>() );
    m_pTextLines.clear();

    // Create lines based on groups of words.
    // Simple merge between groups is performed.
    for( size_t i = 0 ; i < m_pGroupsWords.size() ; ++i ) {
        this->createLine_Basic( i );
    }
    // Sort lines using group index.
    std::sort( m_pTextLines.begin(), m_pTextLines.end(), PRGTextLine::compareGroupIndex );

    // Inside enlarge algorithm parameters.
    const double XEnlargeIn = 5.0;
    const double YEnlargeIn = 3.0;
    const double MinLineWidthIn = 2.0;

    // Try to merge some lines (inside elements).
    PRGTextLine* pLine;
    std::vector<PRGTextLine*>::iterator it;
    for( it = m_pTextLines.begin() ; it != m_pTextLines.end() ; ++it ) {
        pLine = this->mergeLines_EnlargeInside( *it,
                                                XEnlargeIn,
                                                YEnlargeIn,
                                                MinLineWidthIn );
        it = std::find( m_pTextLines.begin(), m_pTextLines.end(), pLine );
    }

    // Merge with elements outside a line.
    const size_t nLoops = 8;
    for( size_t n = 0 ; n <= nLoops ; ++n ) {
        // Algorithm parameters for this iteration.
        double ratio = double( n ) / double( nLoops );
        double xEnlarge = 5.0 * ratio;
        double yEnlarge = 2.0 * ratio;
        double MaxLineWidthCumul = 15.0 - 11. * ratio;

        for( it = m_pTextLines.begin() ; it != m_pTextLines.end() ; ++it ) {
            // Try to merge every line.
            pLine = this->mergeLines_EnlargeOutside( *it,
                                                     xEnlarge, yEnlarge,
                                                     MaxLineWidthCumul );
            it = std::find( m_pTextLines.begin(), m_pTextLines.end(), pLine );

            // Also merge inside elements in case they appear.
            pLine = this->mergeLines_EnlargeInside( *it,
                                                    XEnlargeIn,
                                                    YEnlargeIn,
                                                    MinLineWidthIn );
            it = std::find( m_pTextLines.begin(), m_pTextLines.end(), pLine );
        }
    }


/*
    // Split lines which have too large inside horizontal spaces.
    std::vector<PRGTextLine*> pLines;
    for( it = m_pTextLines.begin() ; it != m_pTextLines.end() ; ) {
        pLines = this->splitLines_hBlocks( *it );
        ++it;

        // Add new lines.
        m_pTextLines.insert( it, pLines.begin()+1, pLines.end() );
        it = std::find( m_pTextLines.begin(), m_pTextLines.end(), pLines.back() );
        ++it;
    }*/
}

PRGTextLine* PRGTextPage::createLine_Basic( size_t idxGroupWords )
{
    // Parameters of the algorithm, based on font statistics.
    const long MaxSearchGroupWords = 30;
    const double MaxHDistanceLB = -10.0;
    const double MaxHDistanceUB = 15.0;
    const double MaxCumulWidth = 25.0;
    const double MinVOverlap = 0.3;

    // Right group of words and its bbox.
    PRGTextGroupWords& rGroupWords = *( m_pGroupsWords[ idxGroupWords ] );
    PdfeORect rGroupBBox = rGroupWords.bbox( PRGTextWordCoordinates::Page, true, true );

    // Transformation from Page to word renormalized coordinates (using document statistics).
    PdfeMatrix rGroupTransMat = rGroupWords.transMatrix( PRGTextWordCoordinates::Page,
                                                         PRGTextWordCoordinates::WordDocRescaled );
    // Vector of lines to merge.
    std::vector<PRGTextLine*> pLinesToMerge;

    // Cumulative width.
    double lGroupsCumulWidth = 0;
    bool lrGroupLink = false;

    // Try to find a group of words that could belong to a same line.
    long idxGroupLimit = std::max( long(0), long(idxGroupWords) - MaxSearchGroupWords );
    for( long idx = idxGroupWords-1 ; idx >= idxGroupLimit ; --idx ) {
        // Left group of words.
        PRGTextGroupWords& lGroupWords = *( m_pGroupsWords[ idx ] );
        PdfeORect lGroupBBox = lGroupWords.bbox( PRGTextWordCoordinates::Page, true, true  );
        lrGroupLink = false;

        // Get the angle between the two groups: should be less than ~5°.
        const double maxAngle = 0.1;
        if( PdfeVector::angle( rGroupBBox.direction(), lGroupBBox.direction() ) > maxAngle ) {
            continue;
        }
        // At this stage, we can assume that the group is related to only one unique line. Add one if necessary.
        if( lGroupWords.textLines().empty() ) {
            m_pTextLines.push_back( new PRGTextLine( this ) );
            m_pTextLines.back()->addGroupWords( &lGroupWords );
        }
        // The line is already in the merging stack.
        std::vector<PRGTextLine*> lGroupLines = lGroupWords.textLines();
        if( std::find( pLinesToMerge.begin(), pLinesToMerge.end(), lGroupLines[0] ) != pLinesToMerge.end() ) {
            continue;
        }
        // Check if the left group corresponds to a LCL of a line.
        long idxLCL = lGroupLines[0]->hasLCL();
        if( idxLCL != -1 &&
                lGroupLines[0]->subgroup( idxLCL ).group() == m_pGroupsWords[ idx ] ) {
            continue;
        }

        // Investigate every subgroup of the left group.
        for( long i = lGroupWords.nbMSubgroups()-1 ; i >=0 ; --i ) {
            // Subgroup bounding box in local coordinates.
            PdfeORect lSubGroupBBoxLocal = lGroupWords.mSubgroup(i).bbox( PRGTextWordCoordinates::Page, true, true );
            lSubGroupBBoxLocal = rGroupTransMat.map( lSubGroupBBoxLocal );

            // Compare to every subgroup of the right group.
            for( size_t j = 0 ; j < rGroupWords.nbMSubgroups() ; ++j ) {
                PdfeORect rSubGroupBBoxLocal = rGroupWords.mSubgroup(j).bbox( PRGTextWordCoordinates::WordDocRescaled, true, true );

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
        // Add group width (without leading and trailing spaces).
        lGroupsCumulWidth += lGroupWords.bbox( PRGTextWordCoordinates::WordDocRescaled, false, true ).width();
        if( lGroupsCumulWidth > MaxCumulWidth ) {
            break;
        }
    }
    // Add line corresponding to the right group.
    if( rGroupWords.textLines().empty() ) {
        m_pTextLines.push_back( new PRGTextLine( this ) );
        m_pTextLines.back()->addGroupWords( &rGroupWords );
        pLinesToMerge.push_back( m_pTextLines.back() );
    }

    // Sort lines to merge using the indexes of their groups.
    std::sort( pLinesToMerge.begin(), pLinesToMerge.end(), PRGTextLine::compareGroupIndex );
    // Merge lines.
    return this->mergeVectorLines( pLinesToMerge );
}

PRGTextLine* PRGTextPage::mergeLines_EnlargeInside( PRGTextLine* pBaseLine,
                                                    double xEnlarge,
                                                    double yEnlarge,
                                                    double minLineWidth )
{
    // Scaling parameters used in the algorithm.
    const double ScaleXEnlarge = 4.;
    const double ScaleYEnlarge = 3.;

    // Min and max group indexes to consider.
    long minGrpIdx = pBaseLine->minGroupIndex();
    long maxGrpIdx = pBaseLine->maxGroupIndex();

    // Vector of lines to merge.
    std::vector<PRGTextLine*> pLinesToMerge;
    pLinesToMerge.push_back( pBaseLine );

    // Line bounding box (no spaces) and transformation matrix.
    PdfeORect baseLineOBBox = pBaseLine->bbox( PRGTextLineCoordinates::LineDocRescaled, false, false );
    PdfeMatrix baseTransMat = pBaseLine->transMatrix( PRGTextLineCoordinates::Page,
                                                      PRGTextLineCoordinates::LineDocRescaled );
    // Line too small or full of spaces, do not consider it...
    if( baseLineOBBox.width() <= minLineWidth || pBaseLine->isSpace() ) {
        return pBaseLine;
    }
    // Enlarge bounding box used.
    xEnlarge = std::min( xEnlarge, baseLineOBBox.height() * ScaleXEnlarge );
    yEnlarge = std::min( yEnlarge, baseLineOBBox.height() * ScaleYEnlarge );
    PdfRect enlargeBBox = baseLineOBBox.enlarge( xEnlarge, yEnlarge ).toPdfRect();

    // Search with groups in the interval ( lineMinGrpIdx,lineMaxGrpIdx )
    for( long i = minGrpIdx ; i <= maxGrpIdx ; ++i ) {
        // Assume the group of words only has one line!
        PRGTextLine* pLine = m_pGroupsWords[i]->textLines().at( 0 );

        // Line is already merged.
        if( std::find( pLinesToMerge.begin(), pLinesToMerge.end(), pLine ) != pLinesToMerge.end() ) {
            continue;
        }

        // Line bounding box in base coordinates system (check it is not empty).
        PdfeORect lineOBBox = pLine->bbox( PRGTextLineCoordinates::Page, false, false );
        if( lineOBBox.width() <= 0.0 || lineOBBox.height() <= 0.0 ) {
            continue;
        }
        lineOBBox = baseTransMat.map( lineOBBox );

        // Check the angle between the two lines: should be less than ~5°.
        if( PdfeVector::angle( baseLineOBBox.direction(), lineOBBox.direction() ) > 0.1  ) {
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
    std::sort( pLinesToMerge.begin(), pLinesToMerge.end(), PRGTextLine::compareGroupIndex );

    // Merge lines.
    return this->mergeVectorLines( pLinesToMerge );
}

PRGTextLine* PRGTextPage::mergeLines_EnlargeOutside(PRGTextLine* pBaseLine,
                                                            double xEnlarge,
                                                            double yEnlarge,
                                                            double maxLineWidthCumul )
{
    // Parameters of the algorithm.
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
    std::vector<PRGTextLine*> pLinesToMerge;
    pLinesToMerge.push_back( pBaseLine );

    // Line bounding box (no spaces) and transformation matrix.
    PdfeORect baseLineOBBox = pBaseLine->bbox( PRGTextLineCoordinates::LineDocRescaled, false, false );
    PdfeMatrix baseTransMat = pBaseLine->transMatrix( PRGTextLineCoordinates::Page,
                                                      PRGTextLineCoordinates::LineDocRescaled );
    // Line too small, do not consider it...
    if( baseLineOBBox.width() <= MinLineWidth ) {
        return pBaseLine;
    }
    // Enlarge bounding box used.
    PdfRect enlargeBBox = baseLineOBBox.enlarge( xEnlarge, yEnlarge ).toPdfRect();

    // Search groups before and after the line.
    for( size_t n = 0 ; n < 2 ; ++n ) {
        for( long i = searchBounds[n*2] ; i <= searchBounds[n*2+1] ; ++i ) {
            // Assume the group of words only has one line!
            PRGTextLine* pLine = m_pGroupsWords[i]->textLines().at( 0 );
            if( std::find( pLinesToMerge.begin(), pLinesToMerge.end(), pLine ) != pLinesToMerge.end() ) {
                continue;
            }
            // Line bounding box in base coordinates system (check it is not empty).
            PdfeORect lineOBBox = pLine->bbox( PRGTextLineCoordinates::Page, false, false );
            lineOBBox = baseTransMat.map( lineOBBox );
            if( lineOBBox.width() <= 0.0 || lineOBBox.height() <= 0.0 ||
                    pLine->widthCumulative( PRGTextLineCoordinates::LineDocRescaled, false ) >= maxLineWidthCumul ) {
                continue;
            }
            // Check the angle between the two lines: should be less than ~5°.
            if( PdfeVector::angle( baseLineOBBox.direction(), lineOBBox.direction() ) > 0.1  ) {
                continue;
            }
            // Check if the line bounding box is inside the enlarge base bbox.
            PdfRect lineBBox = lineOBBox.toPdfRect();
            if( PdfeORect::inside( enlargeBBox, lineBBox ) ) {
                pLinesToMerge.push_back( pLine );
            }
        }
    }
    // Sort lines to merge using the indexes of their groups.
    std::sort( pLinesToMerge.begin(), pLinesToMerge.end(), PRGTextLine::compareGroupIndex );

    // Merge lines.
    return this->mergeVectorLines( pLinesToMerge );
}
// DEPRECIATED.
PRGTextLine* PRGTextPage::mergeLines_Inside( PRGTextLine* pLine )
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
    std::vector<PRGTextLine*> pLinesToMerge;
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
                PRGTextLine* pLine1 = m_pGroupsWords[i]->textLines().at( 0 );

                // Look at it if the line is not already inside the vector of lines...
                if( std::find( pLinesToMerge.begin(), pLinesToMerge.end(), pLine1 ) == pLinesToMerge.end() ) {
                    // Begin and end minimal distances.
                    double distBegin = m_pGroupsWords[grpIdxBegin]->minDistance( *m_pGroupsWords[i] );
                    double distEnd = m_pGroupsWords[grpIdxEnd]->minDistance( *m_pGroupsWords[i] );

                    bool groupMerge = ( distBegin <= MaxDistanceInside ||
                                        distEnd <= MaxDistanceInside ) &&
                            ( m_pGroupsWords[i]->length( false ) <= MaxLengthInside ||
                              m_pGroupsWords[i]->bbox( PRGTextWordCoordinates::Word, false, true ).width() <= MaxWidthInside );

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
    std::sort( pLinesToMerge.begin(), pLinesToMerge.end(), PRGTextLine::compareGroupIndex );

    // Merge lines.
    return this->mergeVectorLines( pLinesToMerge );
}
// DEPRECIATED.
PRGTextLine *PRGTextPage::mergeLines_Small( PRGTextLine *pLine )
{
    // Parameters of the algorithm.
    double MaxDistance = 1.0;
    double MaxWidthLine = 3.0;
    size_t MaxLengthLine = 5;
    long MaxSearchGroupWords = 30;
    double MaxWidthCumul = 5.0;
    double MaxDistCumul = 5.0;

    // Consider elements in the line if and only if it is sufficiently small.
    if( pLine->width( PRGTextLineCoordinates::Line, true ) > MaxWidthLine ||
            pLine->length( false ) > MaxLengthLine ) {
        return pLine;
    }
    PdfeMatrix lineTransMat = pLine->transMatrix( PRGTextLineCoordinates::Page,
                                                  PRGTextLineCoordinates::Line );
    PdfeORect lineBBox = pLine->bbox( PRGTextLineCoordinates::Line, true, false );
    PdfeVector lineLBPoint = lineBBox.leftBottom();

    // Vector of lines to merge.
    std::vector<PRGTextLine*> pLinesToMerge;
    pLinesToMerge.push_back( pLine );

    // Look at groups inside the small line.
    for( size_t i = 0 ; i < pLine->nbSubgroups() ; ++i ) {
        double widthCumulBefore = 0;
        double widthCumulAfter = 0.0;
        double distMaxBefore = 0.0;
        double distMaxAfter = 0.0;
        const PRGTextGroupWords::Subgroup& subgroupLine = pLine->subgroup( i );
        PRGTextGroupWords* pGroupLine = subgroupLine.group();

        // Groups before the line one.
        long idxBefore = std::max( 0L, pGroupLine->groupIndex() - MaxSearchGroupWords );
        for( long j = idxBefore ; j < pGroupLine->groupIndex() ; ++j ) {
            PRGTextGroupWords* pGroup2nd = m_pGroupsWords[j];
            PRGTextLine* pLine2nd = pGroup2nd->textLines().at(0);

//            widthCumulBefore += pGroup2nd->width( true );
            widthCumulBefore += pGroup2nd->bbox( PRGTextWordCoordinates::Word, false, true ).width();

            // Consider the group ?
            if( widthCumulBefore <= MaxWidthCumul && distMaxBefore <= MaxDistCumul &&
                std::find( pLinesToMerge.begin(), pLinesToMerge.end(), pLine2nd ) == pLinesToMerge.end() ) {

                // Distance to the line group.
                double distMin = pGroupLine->minDistance( *pGroup2nd );
                distMaxBefore = std::max( distMaxBefore, pGroupLine->maxDistance( *pGroup2nd ) );

                // Line bounding box.
                PdfeORect lineBBox2nd = pLine2nd->bbox( PRGTextLineCoordinates::Page, true, false );
                lineBBox2nd = lineTransMat.map( lineBBox2nd );

                // Compute the expected height of the merge line.
                PdfeVector lineLBPoint2nd = lineBBox2nd.leftBottom();
                double heightMerge = std::max( lineLBPoint2nd(1)+lineBBox2nd.height(), lineLBPoint(1)+lineBBox.height() ) -
                         std::min( lineLBPoint2nd(1), lineLBPoint(1) ) ;

                bool merge = ( distMin <= MaxDistance ) && ( pLine2nd->width( PRGTextLineCoordinates::Line, true ) <= MaxWidthLine ||
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
            PRGTextGroupWords* pGroup2nd = m_pGroupsWords[j];
            PRGTextLine* pLine2nd = pGroup2nd->textLines().at((0));

            widthCumulAfter += pGroup2nd->bbox( PRGTextWordCoordinates::Word, false, true ).width();

            // Consider the group ?
            if( widthCumulAfter <= MaxWidthCumul && distMaxAfter <= MaxDistCumul &&
                std::find( pLinesToMerge.begin(), pLinesToMerge.end(), pLine2nd ) == pLinesToMerge.end() ) {

                // Distance to the line group.
                double distMin = pGroupLine->minDistance( *pGroup2nd );
                distMaxAfter = std::max( distMaxBefore, pGroupLine->maxDistance( *pGroup2nd ) );

                // Line bounding box.
                PdfeORect lineBBox2nd = pLine2nd->bbox( PRGTextLineCoordinates::Page, true, false );
                lineBBox2nd = lineTransMat.map( lineBBox2nd );

                // Compute the expected height of the merge line.
                PdfeVector lineLBPoint2nd = lineBBox2nd.leftBottom();
                double heightMerge = std::max( lineLBPoint2nd(1)+lineBBox2nd.height(), lineLBPoint(1)+lineBBox.height() ) -
                         std::min( lineLBPoint2nd(1), lineLBPoint(1) ) ;

                bool merge = ( distMin <= MaxDistance ) && ( pLine2nd->width( PRGTextLineCoordinates::Line, true ) <= MaxWidthLine ||
                                                             heightMerge <= lineBBox2nd.height() * 1.4 );

                if( merge ) {
                    pLinesToMerge.push_back( pGroup2nd->textLines().at(0) );
                }
            }
        }
    }

    // Sort lines to merge using the indexes of their groups.
    std::sort( pLinesToMerge.begin(), pLinesToMerge.end(), PRGTextLine::compareGroupIndex );

    // Merge lines.
    return this->mergeVectorLines( pLinesToMerge );
}

std::vector<PRGTextLine*> PRGTextPage::splitLines_hBlocks( PRGTextLine* pLine )
{
    // Parameters of the algorithm.
    double BlockHDistance = 2.0;

    // Vector of lines.
    std::vector<PRGTextLine*> pLines;
    pLines.push_back( pLine );

    // Get horizontal blocks inside the line.
    std::vector<PRGTextLine::Block*> hBlocks = pLine->horizontalBlocks( BlockHDistance );
    //std::list<PRGTextLine::Block> hBlocksList = pLine->horizontalBlocksList( BlockHDistance );

    // Size vector <= 1: nothing much to do...
    if( hBlocks.size() <= 1 ) {
        return pLines;
    }

    // Update the first line.
    PRGTextLine::Block& block0 = *hBlocks[0];
    for( size_t j = 0 ; j < block0.nbSubgroups() ; ++j ) {
        pLine->setSubgroup( j, block0.subgroup( j ) );
    }

    // Create new lines for other blocks.
    std::vector<PRGTextLine::Block*>::iterator it;
    for( it = ++hBlocks.begin() ; it != hBlocks.end() ; ++it ) {
        PRGTextLine::Block& block = *(*it);
        pLines.push_back( new PRGTextLine( this ) );

        // Add subgroups to the new line.
        for( size_t j = 0 ; j < block.nbSubgroups() ; ++j ) {
            pLines.back()->addSubgroupWords( block.subgroup( j ) );
        }
    }
    // Delete blocks.
    std::for_each( hBlocks.begin(), hBlocks.end(), delete_ptr_fctor<PRGTextLine::Block>() );

    return pLines;
}

PRGTextLine* PRGTextPage::mergeVectorLines( const std::vector<PRGTextLine*>& pLines )
{
    // Empty vector...
    if( pLines.empty() ) {
        return NULL;
    }

    // Base line.
    PRGTextLine* pBaseLine = pLines[0];
    PRGTextLine* pLine;

    // Merge every line with the first one.
    std::vector<PRGTextLine*>::iterator it;
    for( size_t i = 1 ; i < pLines.size() ; ++i ) {
        pLine = pLines[i];

        // Merge if and only if the line belongs to the page.
        it = std::find( m_pTextLines.begin(), m_pTextLines.end(), pLine );
        if( it != m_pTextLines.end() ) {

            // Copy the subgroups of words.
            for( size_t j = 0 ; j < pLine->nbSubgroups() ; ++j ) {
                const PRGTextGroupWords::Subgroup& subgroup = pLine->subgroup( j );

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

// Reimplement PdfeCanvasAnalysis interface.
PdfeVector PRGTextPage::fTextShowing( const PdfeStreamStateOld& streamState )
{
    PRGTextGroupWords* pGroup;
    // Create the group of words.
    if( m_nbGroupsPage >= long( m_pGroupsWords.size() ) ) {
        pGroup = new PRGTextGroupWords( this, streamState );
        pGroup->setGroupStreamID( m_nbGroupsStream );
        pGroup->setGroupIndex( m_nbGroupsPage );
        ++m_nbGroupsStream;

        // Empty group -> trash !
        if( !pGroup->nbWords() ) {
            delete pGroup;
            return PdfeVector();
        }
        // Append to the vector of text groups.
        m_pGroupsWords.push_back( pGroup );
        ++m_nbGroupsPage;
    }
    // Read data if necessary.
    else {
        pGroup = m_pGroupsWords[ m_nbGroupsPage ];
        if( pGroup->groupStreamID() == m_nbGroupsStream && !pGroup->isDataLoaded() ) {
            pGroup->readData( this, streamState );
            ++m_nbGroupsStream;
            ++m_nbGroupsPage;
        }
        else {
            ++m_nbGroupsStream;
            return PdfeVector();
        }
    }
    // Return text displacement.
    return pGroup->displacement();
}

// Rendering routines.
void PRGTextPage::renderGroupsWords( PRRenderPage& renderPage,
                                     bool renderGlyphs ) const
{
    // Rendering Pen/Brush.
    PRPenBrush textPB, spacePB, translationPB;
    textPB.setBrush( new QBrush( Qt::white ) );
    spacePB.setBrush( new QBrush( Qt::white ) );
    translationPB.setBrush( new QBrush( Qt::white ) );
//    renderParameters.textPB.drawPen = new QPen( Qt::blue );
//    renderParameters.textSpacePB.drawPen = new QPen( Qt::blue );
//    renderParameters.textPDFTranslationPB.drawPen = new QPen( Qt::blue );

    QColor groupColor;
    QColor groupColorSpace;

    // Draw groups of words.
    for( size_t idx = 0 ; idx < m_pGroupsWords.size() ; ++idx ) {
        // Group color.
        groupColor.setHsv( idx % 360, 255, 220 );
        groupColorSpace.setHsv( idx % 360, 100, 255 );

        // Modify rendering colors.
        textPB.brush()->setColor( groupColor );
        spacePB.brush()->setColor( groupColorSpace );
        translationPB.brush()->setColor( groupColorSpace );
        //        m_renderParameters.textPB.drawPen->setColor( groupColor );
        //        m_renderParameters.textSpacePB.drawPen->setColor( groupColorSpace );
        //        m_renderParameters.textPDFTranslationPB.drawPen->setColor( groupColorSpace );

        // Draw the group on the page.
        m_pGroupsWords[idx]->render( renderPage, textPB, spacePB, translationPB );
        if( renderGlyphs ) {
            m_pGroupsWords[idx]->renderGlyphs( renderPage );
        }
    }
    QLOG_INFO() << QString( "<PRGTextPage> Render page's groups of words (index: %1)." )
                   .arg( m_page->pageIndex() )
                   .toAscii().constData();
}
void PRGTextPage::renderLines( PRRenderPage& renderPage,
                               bool renderWords,
                               bool renderGlyphs ) const
{
    // Rendering Pen/Brush.
    PRPenBrush textPB, spacePB, translationPB, linePB;
    textPB.setBrush( new QBrush( Qt::white ) );
    spacePB.setBrush( new QBrush( Qt::white ) );
    translationPB.setBrush( new QBrush( Qt::white ) );
    linePB.setPen( new QPen( Qt::white ) );

    // Colors
    QColor lineColorWord, lineColorSpace, lineColorBBox;

    // Draw lines
    for( size_t idx = 0 ; idx < m_pTextLines.size() ; ++idx ) {
        PRGTextLine* pline = m_pTextLines[idx];
        // Set colors.
        lineColorWord.setHsv( idx*36 % 360, 255, 255 );
        lineColorSpace.setHsv( idx*36 % 360, 100, 255 );
        lineColorBBox.setHsv( idx*36 % 360, 255, 200 );

        // Modify rendering parameters.
        textPB.brush()->setColor( lineColorWord );
        spacePB.brush()->setColor( lineColorSpace );
        translationPB.brush()->setColor( lineColorSpace );
        linePB.pen()->setColor( lineColorBBox );

        if( pline ) {
            // Subgroups of words inside the line.
            for( size_t idx = 0 ; idx < pline->nbSubgroups() ; ++idx ) {
                if( renderWords ) {
                    pline->subgroup( idx ).render( renderPage, textPB, spacePB, translationPB );
                }
                if( renderGlyphs ) {
                    pline->subgroup( idx ).group()->renderGlyphs( renderPage );
                }
            }
            // Line bounding box.
            renderPage.drawPdfeORect( pline->bbox( PRGTextLineCoordinates::Page, false, false ), linePB );
            // Line first capital letter.
            if( pline->hasLCL() >= 0 ) {
                renderPage.drawPdfeORect( pline->bboxLCL( PRGTextLineCoordinates::Page ), spacePB );
                renderPage.drawPdfeORect( pline->bboxLCL( PRGTextLineCoordinates::Page ), linePB );
                //QLOG_INFO() << pline->subgroup( pline->hasLCL() ).toUnicode( true, true );
            }

            // Line blocks.
//            std::vector<PRGTextLine::Block*> hBlocks = m_pTextLines[idx]->horizontalBlocks( 2.0 );
//            PdfeMatrix transMat = m_pTextLines[idx]->transMatrix();
//            for( size_t j = 0 ; j < hBlocks.size() ; ++j ) {
//                PdfeORect bbox = transMat.map( hBlocks[j]->bbox() );
//                this->textDrawPdfeORect( bbox, linePen );
//            }
            // Line text.
            //QLOG_INFO() << pline->toUnicode( true, true );
        }
    }
    QLOG_INFO() << QString( "<PRGTextPage> Render page's text lines (index: %1)." )
                   .arg( m_page->pageIndex() )
                   .toAscii().constData();
}

PRGPage* PRGTextPage::parent() const
{
    return static_cast<PRGPage*>( this->QObject::parent() );
}

}
