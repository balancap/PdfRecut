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

void PRTextPageStructure::analyseGroupsWords()
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
    if( !pGroup->words().size() ) {
        delete pGroup;
        return;
    }
    //group.setGroupIndex( m_groupsWords.size() );  // Already set by PRRenderPage.
    pGroup->setPageIndex( m_pageIndex );
    pGroup->setTextLine( NULL );

    m_pGroupsWords.push_back( pGroup );
    //m_groupsWordsLines.push_back( -1 );


}

void PRTextPageStructure::analyseLines()
{
    //  Try to find a line for each group of words.
    for( size_t i = 0 ; i < m_pGroupsWords.size() ; ++i ) {
        this->findTextLine( i );
    }
}


PRTextLine* PRTextPageStructure::findTextLine( size_t idxGroupWords )
{
    long MaxSearchGroupWords = 5;
    size_t MaxCharSimpleGroup = 3;
    double MaxHDistanceLB = -5.0;
    double MaxHDistanceUP = 3.0;

    // Get the group of words with the corresponding index: considered as the group at the right.
    PRTextGroupWords& rGroupWords = *( m_pGroupsWords[ idxGroupWords ] );
    PdfeORect rGroupORect = rGroupWords.getOrientedRect( true );
    PdfeORect rGroupORectLocal = rGroupWords.getOrientedRect( false );
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
        PdfeORect lGroupORect = lGroupWords.getOrientedRect( true );
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
                double distORects = PdfeORect::minDistance( lGroupORectLocal, rGroupORectLocal );

                // Is the left group linked to rGroupWords ?
                lrGroupLink = ( distORects <= 1.0 ) && ( lGroupsMaxDist <= 6.0 );
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

                /*
                // Compute the two points that interest us (rb and rt).
                PdfeVector rbPoint0, rtPoint0, rbPoint, rtPoint;
                rbPoint0 = lGroupORect.leftBottom() + lGroupORect.direction() * lGroupORect.width();
                rtPoint0 = rbPoint0 + lGroupORect.direction().rotate90() * lGroupORect.height();

                //PdfeVector dir = lGroupORect.direction().rotate90();

                rbPoint = rGroupTransMat.map( rbPoint0 );
                rtPoint = rGroupTransMat.map( rtPoint0 );

                //rbPoint = lGroupORect.rightBottom();
                //rtPoint = lGroupORect.rightTop();

                // Compute horizontal distance and vertical overlap (in % of height).
                double hDistance = rbPoint(0);
                double vOverlapR = std::max( 0.0, std::min( rGroupORectLocal.height(), rtPoint(1) ) - std::max( 0.0, rbPoint(1) ) ) / rGroupORectLocal.height();
                double vOverlapL = std::max( 0.0, std::min( rGroupORectLocal.height(), rtPoint(1) ) - std::max( 0.0, rbPoint(1) ) ) / ( rtPoint(1) - rbPoint(1) );

                */


//                double hDistance = rbPoint(0);
//                double vOverlapR = std::max( 0.0, std::min( rGroupORect.height(), rtPoint(1) ) - std::max( 0.0, rbPoint(1) ) ) / rGroupORect.height();
//                double vOverlapL = std::max( 0.0, std::min( rGroupORect.height(), rtPoint(1) ) - std::max( 0.0, rbPoint(1) ) ) / ( rtPoint(1) - rbPoint(1) );

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
//        this->textDrawPdfeORect( m_pGroupsWords[idx]->getOrientedRect() );
    }
}
void PRTextPageStructure::renderTextLines()
{
    // Rendering parameters.
    PRRenderParameters renderParameters;
    renderParameters.initToEmpty();
    renderParameters.textPB.fillBrush = new QBrush( Qt::blue );
    renderParameters.textSpacePB.fillBrush = new QBrush( Qt::blue );
    m_renderParameters = renderParameters;

    // Draw lines
    QColor lineColor;
    QColor lineColorSpace;
    for( size_t idx = 0 ; idx < m_pTextLines.size() ; ++idx ) {
        lineColor.setHsv( idx*36 % 360, 255, 255 );
        lineColorSpace.setHsv( idx*36 % 360, 100, 255 );

        m_renderParameters.textPB.fillBrush->setColor( lineColor );
        m_renderParameters.textSpacePB.fillBrush->setColor( lineColorSpace );

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

void PRTextPageStructure::textDrawLine( const PRTextLine& line )
{
    // Draw the different groups of words that belong to the line.
    std::vector<PRTextGroupWords*> pGroupsWords = line.groupsWords();
    for( size_t idx = 0 ; idx < pGroupsWords.size() ; ++idx ) {
        this->textDrawGroupWords( *( pGroupsWords[idx] ) );
    }
}


}
