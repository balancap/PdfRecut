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

#include "PRPageStatistics.h"

#include <podofo/podofo.h>
#include <QtCore>
#include <QtGui>
#include<qrgb.h>

#include <fstream>

using namespace PoDoFo;

namespace PdfRecut {

PRPageStatistics::PRPageStatistics( long pageIndex,
                                    PdfPage* pageIn,
                                    PdfFontMetricsCache* fontMetricsCache ) :
    PRRenderPage( pageIn, fontMetricsCache )
{
    m_pageIndex = pageIndex;
    m_page = pageIn;
}

void PRPageStatistics::computeTextStatistics()
{
}

void PRPageStatistics::computeTextLines()
{
    // Rendering parameters used: only draw text.
    PRRenderParameters renderParameters;
    renderParameters.resolution = 2.0;

    renderParameters.textPB = PRRenderParameters::PRPenBrush();
    renderParameters.textSpacePB = PRRenderParameters::PRPenBrush();
    renderParameters.pathPB = PRRenderParameters::PRPenBrush();
    renderParameters.clippingPathPB = PRRenderParameters::PRPenBrush();
    renderParameters.inlineImagePB = PRRenderParameters::PRPenBrush();
    renderParameters.imagePB = PRRenderParameters::PRPenBrush();
    renderParameters.formPB = PRRenderParameters::PRPenBrush();

    renderParameters.textPB.fillBrush = new QBrush( Qt::blue );
    renderParameters.textSpacePB.fillBrush = new QBrush( Qt::blue );

    // Clear vectors content.
    m_textLines.clear();
    m_groupsWords.clear();
    m_groupsWordsLines.clear();

    // Render the page.
    this->renderPage( renderParameters );

    // Draw lines
    QColor txtColor;
    for( size_t idx = 0 ; idx < m_textLines.size() ; ++idx ) {
        txtColor.setHsv( idx*24 % 360, 255, 255 );
        m_renderParameters.textPB.fillBrush->setColor( txtColor );
        m_renderParameters.textSpacePB.fillBrush->setColor( txtColor );
        this->textDrawLine( m_textLines[idx] );
    }

}
void PRPageStatistics::fTextShowing( const PdfStreamState& streamState )
{
    // Update text transformation matrix.
    this->textUpdateTransMatrix( streamState );

    // Read the group of words.
    m_groupsWords.push_back( this->textReadGroupWords( streamState ) );
    m_groupsWordsLines.push_back( -1 );

//    std::cout << streamState.gOperands.back()
//              << " // " << streamState.gStates.back().textState.fontName << std::endl;

    // Try to find a line for the group of words.
    this->findTextLine( m_groupsWords.size()-1 );




    // Change the color used to fill text.
    //QRgb rgbColor = qRgb( 0, 255, 0);

    //QRgb rgbColor = RGB_MASK- RGB_MASK & (m_nbTextGroups + 1);
    QColor txtColor;
    txtColor.setHsv( m_nbTextGroups*6 % 360, 255, 255 );

    m_renderParameters.textPB.fillBrush->setColor( txtColor );
    //m_renderParameters.textSpacePB.fillBrush->setColor( txtColor );

    //std::cout << std::hex << rgbColor << std::endl;
    //std::cout << std::hex << (0xff000000|QRgb(m_nbTextGroups)) << std::endl;

    // Render the group of words.
    //this->textDrawGroupWords( m_groupsWords.back() );

    //this->drawPdfeORect( m_groupsWords.back().getOrientedRect() );

    //PRRenderPage::fTextShowing( streamState );
}



void PRPageStatistics::textDrawPdfeORect( const PdfeORect& orect )
{
    // Compute text rendering matrix.
    m_pagePainter->setTransform( m_pageImgTrans.toQTransform() );

    // Create polygon.
    QPolygonF polygon;
    PdfeVector point = orect.getLeftBottom();
    polygon << point.toQPoint();

    point = point + orect.getDirection() * orect.getWidth();
    polygon << point.toQPoint();

    point = point + orect.getDirection().rotate90() * orect.getHeight();
    polygon << point.toQPoint();

    point = orect.getLeftBottom() + orect.getDirection().rotate90() * orect.getHeight();
    polygon << point.toQPoint();

    m_renderParameters.textPB.setPenBrush( m_pagePainter );
    m_pagePainter->drawPolygon( polygon );
}

void PRPageStatistics::textDrawLine( const PRTextLine& line )
{
    const std::vector<PRTextGroupWords>& groupsWords = line.getGroupsWords();
    for( size_t idx = 0 ; idx < groupsWords.size() ; ++idx ) {
        this->textDrawGroupWords( groupsWords[idx] );
    }
}

size_t PRPageStatistics::findTextLine( size_t idxGroupWords )
{
    long MaxSearchGroupWords = 3;

    // Get the group of words.
    PRTextGroupWords& groupWords = m_groupsWords[idxGroupWords];
    PdfeORect orectGroup = groupWords.getOrientedRect();

    // Transformation matrix related to the group of words.
    PdfeVector lbGroup = orectGroup.getLeftBottom();
    PdfeVector direcGroup = orectGroup.getDirection();

    PdfeMatrix transMat, tmpMat;
    tmpMat(0,0) = direcGroup(0);      tmpMat(0,1) = -direcGroup(1);
    tmpMat(1,0) = direcGroup(1);      tmpMat(1,1) = direcGroup(0);
    tmpMat(2,0) = lbGroup(0);         tmpMat(2,1) = lbGroup(1);
    tmpMat.inverse( transMat );

    // Try to find a group of words that could belong to a same line.
    size_t idxGroupLimit = std::max( long(0), long(idxGroupWords) - MaxSearchGroupWords );

    for( size_t idx = idxGroupLimit ; idx < idxGroupWords ; ++idx ) {
        PdfeORect tmpORect = m_groupsWords[idx].getOrientedRect();
        double angle = PdfeVector::angle( orectGroup.getDirection(), tmpORect.getDirection() );

        // Get the angle between the two groups: should be less than ~5°.
        if( PdfeVector::angle( orectGroup.getDirection(), tmpORect.getDirection() ) <= 0.1  )
        {
            // Compute the two points that interest us (rb and rt).
            PdfeVector rbPoint, rtPoint;
            rbPoint = tmpORect.getLeftBottom() + tmpORect.getDirection() * tmpORect.getWidth();
            rtPoint = rbPoint + tmpORect.getDirection().rotate90() * tmpORect.getHeight();

            rbPoint = transMat.map( rbPoint );
            rtPoint = transMat.map( rtPoint );

            // Compute horizontal distance and vertical overlap (in % of height).
            double hDistance = ( rbPoint(0) ) / orectGroup.getHeight();
            double vOverlap = std::max( 0.0, std::min( orectGroup.getHeight(), rtPoint(1) ) - std::max( 0.0, rbPoint(1) ) ) / orectGroup.getHeight();

            // Conditions to satisfy...
            if( hDistance >= -4.0 && hDistance <= 4.0 && vOverlap >= 0.25 ) {
                // The current group already belongs to a line.
                if( m_groupsWordsLines[idx] >= 0 ) {
                    m_textLines[ m_groupsWordsLines[idx] ].addGroupWords( groupWords );
                    m_groupsWordsLines[idxGroupWords] = m_groupsWordsLines[idx];

                    return m_groupsWordsLines[idx];
                }
                // Create a line for both.
                else {
                    m_textLines.push_back( PRTextLine() );
                    m_textLines.back().addGroupWords( m_groupsWords[idx] );
                    m_textLines.back().addGroupWords( groupWords );

                    m_groupsWordsLines[idx] = m_textLines.size()-1;
                    m_groupsWordsLines[idxGroupWords] = m_textLines.size()-1;

                    return m_textLines.size()-1;
                }
            }
        }
    }

    // Arrived there : no line found ! Create a new one...
    m_textLines.push_back( PRTextLine() );
    m_textLines.back().addGroupWords( groupWords );
    return m_textLines.size()-1;
}

}
