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
    /** Add a subgroup of words to the line.
     * \param subgroup Reference to the subgroup of words to add.
     */
    void addSubgroupWords( const PRTextGroupWords::Subgroup& subgroup );
    /** Remove a group of words from the line.
     * \param groupWords Pointer to the group of words to remove.
     */
    void rmGroupWords( PRTextGroupWords* pGroupWords );

public:
    /** Block inside a line: represent a generic collection of subgroups
     * that must belong to a common line.
     */
    class Block
    {
    public:
        /** Default constructor.
         */
        Block();

        /** Constructor: initialize with a given line and subgroup.
         * \param pLine Pointer to the parent line.
         * \param idxSubgroup Index of the subgroup in the line.
         * \param leadTrailSpaces Include leading and trailing spaces for BBox ?
         * \param useBottomCoord Use bottom coordinates for the BBox ?
         */
        Block( const PRTextLine* pLine,
               size_t idxSubgroup,
               bool leadTrailSpaces,
               bool useBottomCoord );

        /** Initialize a block with a given line and subgroup of words.
         * \param pLine Pointer to the parent line.
         * \param idxSubgroup Index of the subgroup in the line.
         * \param leadTrailSpaces Include leading and trailing spaces for BBox ?
         * \param useBottomCoord Use bottom coordinates for the BBox ?
         */
        void init( const PRTextLine* pLine,
                   size_t idxSubgroup,
                   bool leadTrailSpaces,
                   bool useBottomCoord );

        /** Get the parent line.
         * \return Pointer the parent line.
         */
        PRTextLine* line() const;

        /** Is a line subgroup inside the block? Can raise an exception.
         * \param idx Index of the subgroup.
         * \return The subgroup belongs to the block?
         */
        bool inside( size_t idx ) const;
        /** Set if a subgroup belongs, or not, to the line.
         * Can raise an exception.
         * \param idx Index of the subgroup.
         * \param inside Is the subgroup inside the block.
         */
        void setInside(size_t idx, bool inside );

        /** Get a pointer to a subgroup. Can raise an exception.
         * \param idx Index of the subgroup.
         * \return Pointer to the subgroup. NULL if it does belong to the line.
         */
        const PRTextGroupWords::Subgroup* subgroup( size_t idx ) const;

        /** Merge the block with another one.
         * \param block2nd Second block (not modified).
         */
        void merge( const Block& block2nd );

        /** Get the bounding box, in the line coordinate system.
         * \return PdfRect containing the bounding box.
         */
        PoDoFo::PdfRect bbox() const;

        /** Sort horizontally two blocks.
         * \param block1 First block.
         * \param block2 Second block.
         */
        static bool horizontalSort( const Block& block1, const Block& block2 );

    protected:
        /// Pointer to the parent line.
        PRTextLine*  m_pLine;
        /// Vector of boolean telling if a subgroup belongs to the block.
        std::vector<bool>  m_subgroupsInside;

        /// Bounding box.
        PoDoFo::PdfRect  m_bbox;
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
     * \param leadTrailSpaces Include leading and trailing spaces ?
     * \param useBottomCoord Use bottom coordinates for the rectangle?
     * \return Oriented rectangle containing the bounding box.
     */
    PdfeORect bbox( bool pageCoords,
                    bool leadTrailSpaces,
                    bool useBottomCoord );

    /** Get the width of the line.
     * \param leadTrailSpaces Include leading and trailing spaces ?
     * \return Width in local coordinates.
     */
    double width( bool leadTrailSpaces );

    /** Get the length of the line.
     * \param countSpaces Also count spaces?
     * \return Length of the line.
     */
    size_t length( bool countSpaces );

    /** Get the transformation matrix associated to line coordinates.
     * \return PdfeMatrix representing the transformation.
     */
    PdfeMatrix transMatrix();

    /** Does the group belongs to the line.
     * \param pGroup Pointer to the group.
     * \return Index of the corresponding subgroup (-1 if not found).
     */
    long hasGroupWords( PRTextGroupWords* pGroup ) const;

    /** Get the number of subgroups in the line.
     * \return Number of subgroups.
     */
    size_t nbSubgroups() const;

    /** Get a subgroup which is inside the line.
     * Can raise an exception.
     * \param idx Index of the subgroup.
     * \return Constant reference to the subgroup.
     */
    const PRTextGroupWords::Subgroup& subgroup( size_t idx ) const;

    /** Obtain horizontal blocks defined by the line.
     * \param hDistance Horizontal distance used for blocks merging.
     * \return Vector of PRTextLine:: Block.
     */
    std::vector<Block> horizontalBlocks( double hDistance = 0.0 ) const;

protected:
    /** Compuet cache data of the line (bbox, first capital letter, ...).
     */
    void computeData();

    /** Compute bounding boxes and the transformation matrix.
     */
    void computeBBoxes();

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

    /// Vector of words subgroups which constitute the line.
    std::vector<PRTextGroupWords::Subgroup>  m_subgroupsWords;

    // Cache data for the line.

    /// Transformation matrix (into page coordinates). Related to the bounding box.
    PdfeMatrix  m_transMatrix;
    /// Line bounding box.
    PoDoFo::PdfRect  m_bbox;
    /// Line bounding box (with no leading and trailing spaces).
    PoDoFo::PdfRect  m_bboxNoLTSpaces;

};

//**********************************************************//
//                     Inline PRTextLine                    //
//**********************************************************//
inline size_t PRTextLine::nbSubgroups() const
{
    return m_subgroupsWords.size();
}
inline const PRTextGroupWords::Subgroup& PRTextLine::subgroup( size_t idx ) const
{
    return m_subgroupsWords.at( idx );
}

//**********************************************************//
//                  Inline PRTextLine::Block                //
//**********************************************************//

inline PRTextLine* PRTextLine::Block::line() const
{
    return const_cast<PRTextLine*>( m_pLine );
}
inline bool PRTextLine::Block::inside( size_t idx ) const
{
    return m_subgroupsInside.at( idx );
}
inline void PRTextLine::Block::setInside( size_t idx, bool inside )
{
    m_subgroupsInside.at( idx ) = inside;
}
inline const PRTextGroupWords::Subgroup* PRTextLine::Block::subgroup( size_t idx ) const
{
    if( m_subgroupsInside.at( idx ) ) {
        m_pLine->subgroup( idx );
    }
    else {
        return NULL;
    }
}

}

#endif // PRTEXTLINE_H
