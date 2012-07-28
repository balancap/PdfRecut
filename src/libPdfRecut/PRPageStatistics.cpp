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

    // Render the page.
    this->renderPage( renderParameters );
}
void PRPageStatistics::fTextShowing( const PdfStreamState& streamState )
{
    // Change the color used to fill text.
    //QRgb rgbColor = qRgb( 0, 255, 0);

    std::cout << streamState.gOperands.back()
              << " // " << streamState.gStates.back().textState.fontName << std::endl;
    QRgb rgbColor = RGB_MASK- RGB_MASK & (m_nbTextGroups + 1);
    QColor txtColor;
    txtColor.setHsv( m_nbTextGroups % 360, 255, 255 );

    m_renderParameters.textPB.fillBrush->setColor( txtColor );
    //m_renderParameters.textSpacePB.fillBrush->setColor( txtColor );

    //std::cout << std::hex << rgbColor << std::endl;
    //std::cout << std::hex << (0xff000000|QRgb(m_nbTextGroups)) << std::endl;

    // Render the group of words.
    m_groupsWords.push_back( this->textReadGroupWords( streamState ) );
    this->textDrawGroupWords( m_groupsWords.back() );

    //PRRenderPage::fTextShowing( streamState );
}

}
