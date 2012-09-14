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

#include <podofo/podofo.h>
#include <QtCore>
#include <QtGui>
#include <qrgb.h>

#include <algorithm>
#include <fstream>

using namespace PoDoFo;

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
    // Groups of words.
    for( size_t i = 0 ; i < m_pGroupsWords.size() ; ++i ) {
        if( m_pGroupsWords[i] ) {
            delete m_pGroupsWords[i];
        }
    }
    m_pGroupsWords.clear();

    // Text lines.
    for( size_t i = 0 ; i < m_pTextLines.size() ; ++i ) {
        if( m_pTextLines[i] ) {
            delete m_pTextLines[i];
        }
    }
    m_pTextLines.clear();
}

void PRTextPageStructure::detectGroupsWords()
{
    // Set rendering parameters to empty.
    PRRenderParameters renderParameters;
    renderParameters.initToEmpty();
    renderParameters.resolution = 1.5;

    // Clear content.
    this->clearContent();

    // Analyse the page.
    this->renderPage( renderParameters );
}
void PRTextPageStructure::fTextShowing( const PdfStreamState& streamState )
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
    pGroup->setTextLine( NULL );

    m_pGroupsWords.push_back( pGroup );
}

void PRTextPageStructure::detectLines()
{
    //  Try to find a line for each group of words.
    for( size_t i = 0 ; i < m_pGroupsWords.size() ; ++i ) {
        this->findLine_Basic( i );
    }

    // Sort lines using group index.
    std::sort( m_pTextLines.begin(), m_pTextLines.end(), PRTextLine::sortLines );

    // Try to merge some lines.
    std::vector<PRTextLine*>::iterator it;
    PRTextLine* pLine;
    for( it = m_pTextLines.begin() ; it != m_pTextLines.end() ; ++it ) {
        pLine = this->findLine_Merge( *it );
        it = std::find( m_pTextLines.begin(), m_pTextLines.end(), pLine );
    }
}

PRTextLine* PRTextPageStructure::findLine_Basic( size_t idxGroupWords )
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
    PdfeORect rGroupBBox = rGroupWords.bbox( true );

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
        PdfeORect lGroupBBox = lGroupWords.bbox( true );
        lrGroupLink = false;

        // Get the angle between the two groups: should be less than ~5°.
        if( PdfeVector::angle( rGroupBBox.direction(), lGroupBBox.direction() ) > 0.1  ) {
            break;
        }

        // Investigate every subgroup of the left group.
        for( long i = lGroupWords.nbSubGroups()-1 ; i >=0 ; --i ) {
            // Subgroup bounding box in local coordinates.
            PdfeORect lSubGroupBBoxLocal = lGroupWords.bbox( true, i );
            lSubGroupBBoxLocal = rGroupTransMat.map( lSubGroupBBoxLocal );

            // Compare to every subgroup of the right group.
            for( size_t j = 0 ; j < rGroupWords.nbSubGroups() ; ++j ) {
                PdfeORect rSubGroupBBoxLocal = rGroupWords.bbox( false, j );

                // Estimate the horizontal distance and vertical overlap.
                PdfeVector lRBPoint, lRTPoint, rLBPoint, rLTPoint;

                lRBPoint = lSubGroupBBoxLocal.rightBottom();
                lRTPoint = lSubGroupBBoxLocal.rightTop();
                rLBPoint = rSubGroupBBoxLocal.leftBottom();
                rLTPoint = rSubGroupBBoxLocal.leftTop();

                double hDistance = lRBPoint(0) - rLBPoint(0);
                double vOverlap = std::max( 0.0, std::min( rLTPoint(1), lRTPoint(1) ) - std::max( rLBPoint(1), lRBPoint(1) ) );
                double vOverlapR = vOverlap / rSubGroupBBoxLocal.height();
                double vOverlapL = vOverlap / lSubGroupBBoxLocal.height();

                // Conditions to satisfy...
                lrGroupLink = ( hDistance >= MaxHDistanceLB ) &&
                        ( hDistance <= MaxHDistanceUB ) &&
                        ( vOverlapR >= MinVOverlap || vOverlapL >= MinVOverlap );

                // Add line, if not alrady in the vector.
                if( std::find( pLinesToMerge.begin(), pLinesToMerge.end(), lGroupWords.textLine() ) == pLinesToMerge.end()
                        && lrGroupLink ) {
                    pLinesToMerge.push_back( lGroupWords.textLine() );
                    break;
                }

            }
            // Line found: break...
            if( lrGroupLink ) {
                break;
            }
        }
        // Add group width (without spaces).
        lGroupsCumulWidth += lGroupWords.width( -1, false, false );
        if( lGroupsCumulWidth > MaxCumulWidth ) {
            break;
        }
    }

    // Add line corresponding to the right group.
    if( !rGroupWords.textLine() ) {
        m_pTextLines.push_back( new PRTextLine() );
        m_pTextLines.back()->addGroupWords( &rGroupWords );
        pLinesToMerge.push_back( m_pTextLines.back() );
    }

    // Sort lines to merge using the indexes of their groups.
    std::sort( pLinesToMerge.begin(), pLinesToMerge.end(), PRTextLine::sortLines );

    // Merge lines.
    return this->mergeLines( pLinesToMerge );
}

PRTextLine* PRTextPageStructure::findLine_Merge( PRTextLine* pLine )
{
    // Parameters of the algorithm.
    double MaxDistanceInside = 1.0;
    double MaxWidthInside = 3.0;
    size_t MaxLengthInside = 5;

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

        // Find a group in the interval that does not belong to the line.
        while( grpIdxBegin <= lineMaxGrpIdx &&
               m_pGroupsWords[ grpIdxBegin ]->textLine() == pLine ) {
            ++grpIdxBegin;
        }

        // Found one...
        if(  grpIdxBegin <= lineMaxGrpIdx ) {
            // Find the next group from the original line.
            grpIdxEnd = grpIdxBegin;
            while( grpIdxEnd <= lineMaxGrpIdx &&
                   m_pGroupsWords[ grpIdxEnd ]->textLine() != pLine ) {
                ++grpIdxEnd;
            }
            grpIdxBegin--;

            // Let study group indexes such that: grpIdxBegin < idx < grpIdxEnd.
            for( long i = grpIdxBegin+1 ; i < grpIdxEnd ; ++i ) {
                PRTextLine* pLine1 = m_pGroupsWords[ i ]->textLine();

                // Look at it if the line is not already inside the vector of lines...
                if( std::find( pLinesToMerge.begin(), pLinesToMerge.end(), pLine1 ) == pLinesToMerge.end() ) {
                    // Begin and end minimal distances.
                    double distBegin = PRTextGroupWords::minDistance( *m_pGroupsWords[ grpIdxBegin ], *m_pGroupsWords[ i ] );
                    double distEnd = PRTextGroupWords::minDistance( *m_pGroupsWords[ grpIdxEnd ], *m_pGroupsWords[ i ] );

                    bool groupMerge = ( distBegin <= MaxDistanceInside ||
                                        distEnd <= MaxDistanceInside ) &&
                            ( m_pGroupsWords[i]->length(-1, false) <= MaxLengthInside ||
                              m_pGroupsWords[i]->width(-1, true, true) <= MaxWidthInside );

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

    // Search groups before and after the line.
    /*long MaxSearchGroupWords = 2;
    double MaxDistanceOutside = 0.8;
    double MaxWidthOutside = 3.0;
    double MaxWidthCumul = 5.0;
    size_t MaxLengthOutside = 5;

    // Before the beginning of the line.
    grpIdxBegin = std::max( 0L, lineMinGrpIdx - MaxSearchGroupWords );
    for( long i = lineMinGrpIdx-1 ; i >= grpIdxBegin ; --i ) {
        // Width and line of the group.
        double width = m_pGroupsWords[i]->width( -1, true, true );
        PRTextLine* pLine1 = m_pGroupsWords[ i ]->textLine();

        // Look at it if the line is not already inside the vector of lines...
        if( std::find( pLinesToMerge.begin(), pLinesToMerge.end(), pLine1 ) == pLinesToMerge.end() ) {
            // Distance to the first group of the line.
            double dist = PRTextGroupWords::minDistance( *m_pGroupsWords[ lineMinGrpIdx ], *m_pGroupsWords[i] );

            bool groupMerge = dist <= MaxDistanceOutside &&
                    ( m_pGroupsWords[i]->length(-1, false) <= MaxLengthOutside ||
                      m_pGroupsWords[i]->width(-1, true, true) <= MaxWidthOutside );
            // Merge lines...
            if( groupMerge ) {
                pLinesToMerge.push_back( pLine1 );
            }
        }
    }
    // Idem: after the end of the line.
    grpIdxEnd = std::min( static_cast<long>( m_pTextLines.size() )-1, lineMaxGrpIdx + MaxSearchGroupWords );
    for( long i = lineMaxGrpIdx+1 ; i <= grpIdxEnd ; ++i ) {
        // Width and line of the group.

        PRTextLine* pLine1 = m_pGroupsWords[i]->textLine();


        // Look at it if the line is not already inside the vector of lines...
        if( std::find( pLinesToMerge.begin(), pLinesToMerge.end(), pLine1 ) == pLinesToMerge.end() ) {
            // Distance to the first group of the line.
            double dist = PRTextGroupWords::minDistance( *m_pGroupsWords[ lineMinGrpIdx ], *m_pGroupsWords[i] );

            bool groupMerge = dist <= MaxDistanceOutside &&
                    ( m_pGroupsWords[i]->length(-1, false) <= MaxLengthOutside ||
                      m_pGroupsWords[i]->width(-1, true, true) <= MaxWidthOutside );
            // Merge lines...
            if( groupMerge ) {
//                pLinesToMerge.push_back( pLine1 );
            }
        }
    }*/

    // Sort lines to merge using the indexes of their groups.
    std::sort( pLinesToMerge.begin(), pLinesToMerge.end(), PRTextLine::sortLines );

    // Merge lines.
    return this->mergeLines( pLinesToMerge );
}

PRTextLine *PRTextPageStructure::mergeLines( const std::vector<PRTextLine*>& pLines )
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

            // Copy the groups of words.
            std::vector<PRTextGroupWords*> pGroups = pLine->groupsWords();
            for( size_t j = 0 ; j < pGroups.size() ; ++j ) {
                pBaseLine->addGroupWords( pGroups[j] );
            }

            // Remove line from the page vector and delete object.
            m_pTextLines.erase( it );
            delete pLine;
        }
    }
    return pBaseLine;
}

PRTextLine* PRTextPageStructure::findLine_Basic2( size_t idxGroupWords )
{
    long MaxSearchGroupWords = 8;
    size_t MaxCharSimpleGroup = 3;
    double MaxHDistanceLB = -5.0;
    double MaxHDistanceUP = 3.0;

    // Get the group of words with the corresponding index: considered as the group at the right.
    PRTextGroupWords& rGroupWords = *( m_pGroupsWords[ idxGroupWords ] );
    PdfeORect rGroupORect = rGroupWords.bbox( true );
    PdfeORect rGroupORectLocal = rGroupWords.bbox( false );
    bool rGroupSimple = ( rGroupWords.length( false ) <= MaxCharSimpleGroup );

    // Local transformation matrix related to the right group of words.
    PdfeMatrix rGroupTransMat = rGroupWords.getGlobalTransMatrix().inverse();
    //PdfeMatrix rGroupTransMat = rGroupORect.localTransMatrix();

    // Try to find a group of words that could belong to a same line.
    long idxGroupLimit = std::max( long(0), long(idxGroupWords) - MaxSearchGroupWords );
    std::vector<PRTextLine*> lrGroupsLines;
    double lGroupsMaxDist = 0;

    for( long idx = idxGroupWords-1 ; idx >= idxGroupLimit ; --idx ) {
        //  Group of words at the left.
        PRTextGroupWords& lGroupWords = *( m_pGroupsWords[ idx ] );
        PdfeORect lGroupORect = lGroupWords.bbox( true );
        PdfeORect lGroupORectLocal = rGroupTransMat.map( lGroupORect );
        bool lGroupSimple = ( lGroupWords.length( false ) <= MaxCharSimpleGroup );

        bool lrGroupLink = false;

        // Get the angle between the two groups: should be less than ~5°.
        if( PdfeVector::angle( rGroupORect.direction(), lGroupORect.direction() ) <= 0.1  )
        {
            //-- Treat the difference combinaisons which lead to different criteria. --//

            // Both simple groups: look at the minimal distance between them.
            if( lGroupSimple && rGroupSimple ) {

                // Estimate of the minimal distance between them.
                //double distORects = PdfeORect::minDistance( lGroupORectLocal, rGroupORectLocal );

                // Is the left group linked to rGroupWords ?
                //lrGroupLink = ( distORects <= 1.0 ) && ( lGroupsMaxDist <= 6.0 );
            }
            // Left normal group and right simple group.
            /*else if( !lGroupSimple && rGroupSimple ) {
                // Estimate the distance between the right side of lGroup and rGroup.
                double dist = std::min( PdfeORect::minDistance( rGroupORectLocal, lGroupORectLocal.rightBottom() ),
                                        PdfeORect::minDistance( rGroupORectLocal, lGroupORectLocal.rightTop() ) );

                // Is the left group linked to rGroupWords ?
                lrGroupLink = ( dist <= 1.0 );
            }
            // Left simple group and right normal group.
            else if( lGroupSimple && !rGroupSimple ) {
                // Estimate the distance between the left side of rGroup and lGroup.
                double dist = std::min( PdfeORect::minDistance( lGroupORectLocal, rGroupORectLocal.leftBottom() ),
                                        PdfeORect::minDistance( lGroupORectLocal, rGroupORectLocal.leftTop() ) );

                // Is the left group linked to rGroupWords ?
                lrGroupLink = ( dist <= 1.0 );
            }*/


            // Haven't work yet => try another configuration.
            if( !lrGroupLink ) {
                PdfeVector lRBPoint, lRTPoint, rLBPoint, rLTPoint;

                lRBPoint = lGroupORectLocal.rightBottom();
                lRTPoint = lGroupORectLocal.rightTop();
                rLBPoint = rGroupORectLocal.leftBottom();
                rLTPoint = rGroupORectLocal.leftTop();

                double hDistance = lRBPoint(0);
                double vOverlapR = std::max( 0.0, std::min( rLTPoint(1), lRTPoint(1) ) - std::max( rLBPoint(1), lRBPoint(1) ) ) / rGroupORectLocal.height();
                double vOverlapL = std::max( 0.0, std::min( rLTPoint(1), lRTPoint(1) ) - std::max( rLBPoint(1), lRBPoint(1) ) ) / lGroupORectLocal.height();

                // Conditions to satisfy...
                lrGroupLink = ( hDistance >= MaxHDistanceLB ) &&
                        ( hDistance <= MaxHDistanceUP ) &&
                        ( vOverlapR >= 0.3 || vOverlapL >= 0.3 );
            }

            // Line found: add to the list...
            if( lrGroupLink ) {
                lrGroupsLines.push_back( lGroupWords.textLine() );
            }

            // Update maximal distance.
            lGroupsMaxDist = std::max( lGroupsMaxDist, PdfeORect::maxDistance( lGroupORectLocal, rGroupORectLocal ) );
        }
    }

    // Some interesting lines have been found !
    if( lrGroupsLines.size() ) {
        // Sort and remove duplicates.
        std::vector<PRTextLine*>::iterator it, itEnd;
        std::sort( lrGroupsLines.begin(), lrGroupsLines.end() );
        itEnd = std::unique( lrGroupsLines.begin(), lrGroupsLines.end() );
        lrGroupsLines.resize( itEnd-lrGroupsLines.begin() );

        // Merge lines of with the first one.
        PRTextLine* pTextLine = lrGroupsLines[0];
        PRTextLine* pTextLine2;
        for( size_t i = 1 ; i < lrGroupsLines.size() ; ++i) {
            pTextLine2 = lrGroupsLines[i];

            // Copy groups of words.
            std::vector<PRTextGroupWords*> groups = pTextLine2->groupsWords();
            for( size_t i = 0 ; i < groups.size() ; ++i ) {
                pTextLine->addGroupWords( groups[i] );
            }

            // Remove line from the page vector.
            it = std::find( m_pTextLines.begin(), m_pTextLines.end(), pTextLine2 );
            m_pTextLines.erase( it );
            delete pTextLine2;
        }

        // Assign right group of words
        pTextLine->addGroupWords( &rGroupWords );
        return pTextLine;
    }
    // No line found ! Create a new one...
    else {
        m_pTextLines.push_back( new PRTextLine() );
        m_pTextLines.back()->addGroupWords( &rGroupWords );
        return m_pTextLines.back();
    }
}


//**********************************************************//
//                      Drawing routines                    //
//**********************************************************//
void PRTextPageStructure::renderTextGroupsWords()
{
    // Rendering parameters.
    PRRenderParameters renderParameters;
    renderParameters.initToEmpty();
    renderParameters.textPB.fillBrush = new QBrush( Qt::blue );
    renderParameters.textSpacePB.fillBrush = new QBrush( Qt::blue );
    m_renderParameters = renderParameters;

    // Draw groups of words.
    QColor groupColor;
    QColor groupColorSpace;
    for( size_t idx = 0 ; idx < m_pGroupsWords.size() ; ++idx ) {
        groupColor.setHsv( idx % 360, 255, 220 );
        groupColorSpace.setHsv( idx % 360, 100, 255 );

        m_renderParameters.textPB.fillBrush->setColor( groupColor );
        m_renderParameters.textSpacePB.fillBrush->setColor( groupColorSpace );

        this->textDrawGroupWords( *m_pGroupsWords[idx] );
//        this->textDrawPdfeORect( m_pGroupsWords[idx]->bbox() );
    }
}
void PRTextPageStructure::renderTextLines()
{
    // Rendering parameters.
    PRRenderParameters renderParameters;
    renderParameters.initToEmpty();
    renderParameters.textPB.fillBrush = new QBrush( Qt::blue );
    renderParameters.textSpacePB.fillBrush = new QBrush( Qt::blue );
    //renderParameters.textPDFTranslationPB.fillBrush = new QBrush( Qt::blue );
    m_renderParameters = renderParameters;

    // Draw lines
    QColor lineColor;
    QColor lineColorSpace;
    for( size_t idx = 0 ; idx < m_pTextLines.size() ; ++idx ) {
        lineColor.setHsv( idx*36 % 360, 255, 255 );
        lineColorSpace.setHsv( idx*36 % 360, 100, 255 );

        m_renderParameters.textPB.fillBrush->setColor( lineColor );
        m_renderParameters.textSpacePB.fillBrush->setColor( lineColorSpace );
        //m_renderParameters.textPDFTranslationPB.fillBrush->setColor( lineColorSpace );

        if( m_pTextLines[idx] ) {
            this->textDrawLine( *m_pTextLines[idx] );
        }
    }
}
void PRTextPageStructure::textDrawPdfeORect( const PdfeORect& orect )
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

    m_renderParameters.textPB.applyToPainter( m_pagePainter );
    m_pagePainter->drawPolygon( polygon );
}

void PRTextPageStructure::textDrawSubGroups( const PRTextGroupWords& groupWords )
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
    for( size_t i = 0 ; i < groupWords.nbSubGroups() ; i++ )
    {
        // Set pen & brush
        m_renderParameters.textPB.applyToPainter( m_pagePainter );

        // Paint word, if the width is positive !
        PdfeORect bbox = groupWords.bbox( false, i );
        PdfeVector lb = bbox.leftBottom();
        m_pagePainter->drawRect( QRectF( lb(0) , lb(1), bbox.width(), bbox.height() ) );
    }
}

void PRTextPageStructure::textDrawLine( const PRTextLine& line )
{
    // Draw the different groups of words that belong to the line.
    std::vector<PRTextGroupWords*> pGroupsWords = line.groupsWords();
    for( size_t idx = 0 ; idx < pGroupsWords.size() ; ++idx ) {
        this->textDrawGroupWords( *( pGroupsWords[idx] ) );
//        this->textDrawSubGroups( *( pGroupsWords[idx] ) );
//        this->textDrawPdfeORect( pGroupsWords[idx]->bbox( true ) );
    }
}


}
