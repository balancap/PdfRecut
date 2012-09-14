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

#ifndef PRTEXTPAGESTRUCTURE_H
#define PRTEXTPAGESTRUCTURE_H

#include "PRTextWords.h"
#include "PRTextLine.h"
#include "PRRenderPage.h"

namespace PoDoFo {
class PdfPage;
class PdfRect;
class PdfMemDocument;
class PdfString;
class PdfVariant;
}

namespace PdfRecut {

/** Class that analyse the text structure of a PDF page.
 */
class PRTextPageStructure : public PRRenderPage
{
public:
    /** Default constructor
     * \param document Input document.
     * \param pageIndex Index of the page to render.
     */
    PRTextPageStructure( PRDocument* document,
                         long pageIndex );

    /** Destructor.
     */
    virtual ~PRTextPageStructure();

protected:
    /** Clear content of the object (i.e. different vectors).
     */
    void clearContent();

public:
    /** Reimplementation of text showing function from PRRenderPage.
     * Used to read text groups of words.
     */
    virtual void fTextShowing( const PdfStreamState& streamState );

    /** Detect the groups of words in the page.
     */
    void detectGroupsWords();
    /** Detect text lines structure of the page.
     */
    void detectLines();

protected:
    /** Find a line for a group of words. Line created if necessary.
     * Basic algorithm used.
     * \param idxGroupWords Index of the group of words.
     * \return Pointer to the line object.
     */
    PRTextLine* findLine_Basic( size_t idxGroupWords );
    PRTextLine* findLine_Basic2( size_t idxGroupWords );

    /** Find lines algorithm: try to merge existing lines.
     * \param pLine Pointer of the line to consider.
     * \return Pointer the merged line.
     */
    PRTextLine* findLine_Merge( PRTextLine* pLine );

    /** Merge a vector of lines into a single one.
     * Use the first element as base (other lines are deleted).
     * \param pLines Vector of pointer of lines to merge.
     * \return Pointer to the resulting line.
     */
    PRTextLine* mergeLines( const std::vector<PRTextLine*>& pLines );

    /** No copy constructor allowed.
     */
    PRTextPageStructure( const PRTextPageStructure& obj );

public:
    // Drawing routines.
    /** Render the groups of words in the page.
     */
    void renderTextGroupsWords();
    /** Render lines of text in the page.
     */
    void renderTextLines();

protected:
    /** Draw a PdfeORect.
     */
    void textDrawPdfeORect( const PdfeORect& orect );
    /** Draw subgroups of a PRTextGroupWords.
     */
    void textDrawSubGroups( const PRTextGroupWords& groupWords );
    /** Draw a line of text.
     */
    void textDrawLine( const PRTextLine& line );

protected:
    /// Groups of words that belong to the page (vector of pointers).
    std::vector<PRTextGroupWords*>  m_pGroupsWords;

    /// Text lines that belong to the page (vector of pointers).
    std::vector<PRTextLine*>  m_pTextLines;
};

}

#endif // PRTEXTPAGESTRUCTURE_H
