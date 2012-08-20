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

#ifndef PRPAGESTATISTICS_H
#define PRPAGESTATISTICS_H

#include "PRTextStructure.h"
#include "PRRenderPage.h"

namespace PoDoFo {
    class PdfPage;
    class PdfRect;
    class PdfMemDocument;
    class PdfFontMetrics;
    class PdfString;
}

namespace PdfRecut {

/** Structure which gathers the different statistics computed.
 * Expressed in the coordinate system of the page.
 */
struct PRPageStatisticsData
{
    /** Page media box.
     */
    PoDoFo::PdfRect pageMediaBox;
    /** Page crop box (checked to be inside media box).
     */
    PoDoFo::PdfRect pageCropBox;

    /** Text bounding box.
     */
    PoDoFo::PdfRect textBoundingBox;

    /** Path bounding box.
     */
    PoDoFo::PdfRect pathBoundingBox;

    /** Image bounding box.
     */
    PoDoFo::PdfRect imageBoundingBox;
};

/** Class which estimates different statistics in a page: text/path/image positions...
 */
class PRPageStatistics : public PRRenderPage
{
public:
    /** Default constructor
     * \param noPage Page number.
     * \param pageIn Input page to analyse.
     * \param fontMetricsCache Cache object for font metrics.
     */
    PRPageStatistics( PRDocument* pDocument,
                      long pageIndex,
                      PoDoFo::PdfPage* pageIn );

    /** Compute page text statistics.
     */
    void computeTextStatistics();

    /** Compute the information relative text lines in the page.
     */
    void computeTextLines();

    /** Reimplementation of text showing from PRRenderPage.
     */
    virtual void fTextShowing( const PdfStreamState& streamState );

    void textDrawPdfeORect( const PdfeORect& orect );

    void textDrawLine( const PRTextLine& line );

protected:
    /** Find a line for a group of words. Line created if necessary.
     * \param idxGroupWords Index of the group of words.
     * \return Index of the line.
     */
    size_t findTextLine( size_t idxGroupWords );

public:
    /** Max of group of words used for a search of line.
     */

protected:
    /** Page index.
     */
    long m_pageIndex;
    /** Page object.
     */
    PoDoFo::PdfPage* m_page;

    /** Text lines that belongs to the page.
     */
    std::vector<PRTextLine> m_textLines;

    /** Group of words that belongs to the page.
     */
    std::vector<PRTextGroupWords> m_groupsWords;
    /** Lines assigned to group of words that belongs to the page.
     */
    std::vector<long>   m_groupsWordsLines;

    /** Page statistics data.
     */
    //PRPageStatisticsData m_data;

};

}

#endif // PRPAGESTATISTICS_H
