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
    /** Destructor...
      */
    ~PRTextLine();

    /** Initialize to an empty line.
     */
    void init();

    /** Add a group of words to the line.
     * \param groupWords Pointer to the group of words to add.
     */
    void addGroupWords( PRTextGroupWords* pGroupWords );

    /** Remove a group of words from the line.
     * \param groupWords Pointer to the group of words to remove.
     */
    void rmGroupWords( PRTextGroupWords* pGroupWords );

    /** Analyse the line (compute bbox, first capital letter, ...).
     */
    void analyse();

public:
    /** Block inside a line: correspond to a vector of groups of words.
     */
    class Block
    {
    public:
        /** Default constructor.
         */
        Block();

        /** Constructor: initialize with a given group.
         * \param pGroupWords Pointer to the group of words.
         * \param pLineTransMat Pointer to the line transformation matrix.
         */
        Block( PRTextGroupWords* pGroupWords,
               const PdfeMatrix* pLineTransMat );

        /** Initialize a block with a given group of words.
         * \param pGroupWords Pointer to the group of words.
         * \param pLineTransMat Pointer to the line transformation matrix.
         */
        void init( PRTextGroupWords* pGroupWords,
                   const PdfeMatrix* pLineTransMat );

        /** Merge the block with another one.
         * \param block2nd Second block (not modified).
         */
        void merge( const Block& block2nd );

        /** Get the bounding box, in the line coordinate system.
         * \return PdfRect containing the bounding box.
         */
        PoDoFo::PdfRect bbox() const;

        /** Get the vector of group of words that belong to the block.
         * \return Vector of pointers to groups of words.
         */
        std::vector<PRTextGroupWords*> groupsWords() const;

        /** Sort horizontally two blocks.
         * \param block1 First block.
         * \param block2 Second block.
         */
        static bool horizontalSort( const Block& block1, const Block& block2 );

    protected:
        /// Vector of groups of words.
        std::vector<PRTextGroupWords*>  m_pGroupsWords;
        /// Bounding box.
        PoDoFo::PdfRect  m_bbox;
        /// Line transformation matrix.
        const PdfeMatrix*  m_pLineTransMat;
    };

public:
    /** Minimum index of group of words inside the line.
     * \return Minimum index found.
     */
    long minGroupIndex();
    /** Maximum index of group of words inside the line.
     * \return Maximum index found.
     */
    long maxGroupIndex();

    /** Get the line bounding box.
     * \param pageCoords BBox in page coordinates (true) or local coordinates (false) ?
     * \param useBottomCoord Use bottom coordinates for the rectangle?
     * \return Oriented rectangle containing the bounding box.
     */
    PdfeORect bbox(bool pageCoords = true,
                   bool useBottomCoord = true );

    /** Get the width of the line.
     * \return Width in local coordinates.
     */
    double width();

    /** Get the length of the line.
     * \param countSpaces Also count spaces?
     * \return Length of the line.
     */
    size_t length( bool countSpaces );

    /** Get the transformation matrix associated to line coordinates.
     * \return PdfeMatrix representing the transformation.
     */
    PdfeMatrix transMatrix();

    /** Get the vector of group of words.
     * \return Vector of pointers to groups of words.
     */
    std::vector<PRTextGroupWords*> groupsWords() const;

    /** Obtain horizontal blocks defined by the line.
     * \param hDistance Horizontal distance used for blocks merging.
     * \return Vector of PRTextLine:: Block.
     */
    std::vector<Block> horizontalBlocks( double hDistance = 0.0 ) const;

protected:
    /** Compute the bounding box and the transformation matrix.
     */
    void computeBBox();

public:
    /** Sort two lines using the index of their groups.
     * \param pLine1 Pointer to the first line.
     * \param pLine2 Pointer to the second line.
     * \return line1 < line2.
     */
    static bool sortLines( PRTextLine* pLine1, PRTextLine* pLine2 );

protected:
    /// Index of the page to which belongs the line.
    long  m_pageIndex;
    /// Index of the line in the page.
    long  m_lineIndex;
    /// Boolean value used to detect if the line have been modified (add words, ...).
    bool  m_modified;

    /// Vector of pointers to groups of words which constitute the line (objects do not belong the line).
    std::vector<PRTextGroupWords*>  m_pGroupsWords;

    /// Transformation matrix (into page coordinates). Related to the bounding box.
    PdfeMatrix  m_transMatrix;
    /// Line bounding box.
    PoDoFo::PdfRect  m_bbox;
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
