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

#ifndef PRTEXTLINE_H
#define PRTEXTLINE_H

#include "PRTextWords.h"

namespace PdfRecut {

/** Class that represent a line of text in a PDF page.
 */
class PRTextLine
{
public:
    /** Default constructor.
     */
    PRTextLine();

    ~PRTextLine();

    /** Initialize to an empty line.
     */
    void init();

    /** Add a group of words to the line.
     * \param groupWords Pointer to the group of words to add.
     */
    void addGroupWords( PRTextGroupWords* pGroupWords );

    /** Get the vector of group of words.
     */
    std::vector<PRTextGroupWords*> groupsWords() const;

    /** Sort two lines using the index of their groups.
     * \param pLine1 Pointer to the first line.
     * \param pLine2 Pointer to the second line.
     * \return line1 < line2.
     */
    static bool sortLines( PRTextLine* pLine1, PRTextLine* pLine2 );

    /** Minimum index of group of words inside the line.
     * \return Minimum index found.
     */
    long minGroupIndex();
    /** Maximum index of group of words inside the line.
     * \return Maximum index found.
     */
    long maxGroupIndex();

protected:
    /// Index of the page to which belongs the line.
    long  m_pageIndex;
    /// Index of the line in the page.
    long  m_lineIndex;

    /// Vector of pointers to groups of words which constitute the line (objects do not belong the line).
    std::vector<PRTextGroupWords*>  m_pGroupsWords;
};

//**********************************************************//
//                     Inline PRTextLine                    //
//**********************************************************//
inline std::vector<PRTextGroupWords*> PRTextLine::groupsWords() const
{
    return m_pGroupsWords;
}

}

#endif // PRTEXTLINE_H
