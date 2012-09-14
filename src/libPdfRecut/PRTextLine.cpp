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
}

void PRTextLine::addGroupWords( PRTextGroupWords* pGroupWords )
{
    if( pGroupWords ) {
        // Set group textline (assume there is only one !).
        pGroupWords->setTextLine( this );
        m_pGroupsWords.push_back( pGroupWords );
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

}
