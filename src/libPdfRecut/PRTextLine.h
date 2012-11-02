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

namespace PRTextLineCoordinates {
/** Enumeration of the different coordinates system
 * a line bounding box can be expressed in.
 */
enum Enum {
    Line = 0,           /// Default line coordinates.
    Page                /// Page coordinates.
};
}

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
    /** Set a subgroup which is inside the line.
     * Can raise an out of range exception.
     * \param idx Index of the subgroup to modify.
     * \param subgroup Constant reference to the new subgroup.
     */
    void setSubgroup( size_t idx, const PRTextGroupWords::Subgroup& subgroup );    
    /** Get a subgroup which is inside the line.
     * Can raise an exception.
     * \param idx Index of the subgroup.
     * \return Constant reference to the subgroup.
     */
    const PRTextGroupWords::Subgroup& subgroup( size_t idx ) const;
    /** Get the number of subgroups in the line.
     * \return Number of subgroups.
     */
    size_t nbSubgroups() const;
    /** Clear empty subgroups in the line.
     */
    void clearEmptySubgroups();
    /** Does the group belongs to the line.
     * \param pGroup Pointer to the group.
     * \return Index of the corresponding subgroup (-1 if not found).
     */
    long hasGroupWords( PRTextGroupWords* pGroup ) const;

public:
    /** Minimum index of group of words inside the line.
     * \return Minimum index found.
     */
    long minGroupIndex() const;
    /** Maximum index of group of words inside the line.
     * \return Maximum index found.
     */
    long maxGroupIndex() const;

    /** Get the line bounding box.
     * \param lineCoords Coordinates system in which the bounding box is expressed.
     * \param leadTrailSpaces Include leading and trailing spaces ?
     * \return Oriented rectangle containing the bounding box.
     */
    PdfeORect bbox( PRTextLineCoordinates::Enum lineCoords,
                    bool leadTrailSpaces );
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

    /** Get a transformation matrix from a starting coordinate
     * system to an ending one.
     * \param startCoord Starting coordinate system.
     * \param endCoord Ending coordinate system.
     * \return Transformation matrix.
     */
    PdfeMatrix transMatrix( PRTextLineCoordinates::Enum startCoord,
                            PRTextLineCoordinates::Enum endCoord );

    /** Mean font size of the line.
     */
    double meanFontSize();

    /** Is the line empty?
     * \return Answer!
     */
    bool isEmpty() const;
    /** Is the line uniquelly composed of spaces?
     * \return Answer!
     */
    bool isSpace() const;

public:
    /// Embedded class that represent a block of a line.
    class Block;

    /** Obtain horizontal blocks defined by the line.
     * \param hDistance Horizontal distance used for blocks merging.
     * \return Vector of PRTextLine::Block*. Objects are owned by the user.
     */
    std::vector<PRTextLine::Block*> horizontalBlocks( double hDistance = 0.0 ) const;

    /** Obtain horizontal blocks defined by the line.
     * \param hDistance Horizontal distance used for blocks merging.
     * \return List of PRTextLine::Block.
     */
    std::list<PRTextLine::Block> horizontalBlocksList( double hDistance = 0.0 ) const;

private:
    /// Compute inside cache data of the line (bbox, first capital letter, ...).
    void computeCacheData();

    /// Compute bounding boxes and the transformation matrix.
    void computeBBoxes() const;

public:
    /** Sort two lines using the index of their groups.
     * \param pLine1 Pointer to the first line.
     * \param pLine2 Pointer to the second line.
     * \return line1 < line2.
     */
    static bool compareGroupIndex( PRTextLine* pLine1, PRTextLine* pLine2 );

protected:
    /// Index of the page to which belongs the line.
    long  m_pageIndex;
    /// Index of the line in the page.
    long  m_lineIndex;

    /// Vector of words subgroups which constitute the line.
    std::vector<PRTextGroupWords::Subgroup>  m_subgroupsWords;

    // Cache computed data.
    /// Boolean value used to detect if the line have been modified (add words, ...).
    mutable bool  m_modified;

    /// Transformation matrix (into page coordinates). Related to the bounding box.
    mutable PdfeMatrix  m_transMatrix;
    /// Line bounding box.
    mutable PoDoFo::PdfRect  m_bbox;
    /// Line bounding box (with no leading and trailing spaces).
    mutable PoDoFo::PdfRect  m_bboxNoLTSpaces;
    /// Mean font size of the line.
    mutable double  m_meanFontSize;
};

/** Block inside a line: represent a generic collection of subgroups
 * that must belong to a common line.
 */
class PRTextLine::Block
{
public:
    /** Default constructor.
     */
    Block();
    /** Constructor: initialize with a given line and subgroup.
     * \param pLine Pointer to the parent line.
     * \param subgroup Subgroup which must be a subset of a line subgroup.
     * \param leadTrailSpaces Include leading and trailing spaces for BBox ?
     * \param useBottomCoord Use bottom coordinates for the BBox ?
     */
    Block( const PRTextLine* pLine,
           const PRTextGroupWords::Subgroup& subgroup,
           bool leadTrailSpaces,
           bool useBottomCoord );

    /** Initialize to an empty block.
     */
    void init();
    /** Initialize a block with a given line and subgroup of words.
     * \param pLine Pointer to the parent line.
     * \param subgroup Subgroup which must be a subset of a line subgroup.
     * \param leadTrailSpaces Include leading and trailing spaces for BBox ?
     * \param useBottomCoord Use bottom coordinates for the BBox ?
     */
    void init( const PRTextLine* pLine,
               const PRTextGroupWords::Subgroup& subgroup,
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
    void setInside( size_t idx, bool inside );

    /** Get the number of subgroups in the block.
     * \return Number of subgroups.
     */
    size_t nbSubgroups() const;

    /** Get a subgroup of the block.
     * Can raise an exception.
     * \param idx Index of the subgroup.
     * \return Constant reference to the subgroup.
     */
    const PRTextGroupWords::Subgroup& subgroup( size_t idx ) const;

    /** Merge the block with another one.
     * \param block2nd Second block (not modified).
     */
    void merge( const Block& block2nd );

    /** Get the bounding box, in the line coordinate system.
     * \return PdfRect containing the bounding box.
     */
    PoDoFo::PdfRect bbox() const;

    /** Horizontally compare two blocks.
     * \param block1 First block.
     * \param block2 Second block.
     */
    static bool horizontalComp( const Block& block1, const Block& block2 );

    /** Horizontally compare two block (use pointers).
     * \param block1 Pointer to the first block.
     * \param block2 Pointer to the second block.
     */
    static bool horizontalCompPtr( Block* pBlock1, Block* pBlock2 );

protected:
    /// Pointer to the parent line.
    PRTextLine*  m_pLine;

    /// Vector of boolean telling if a line subgroup (or a subset) belongs to the block.
    std::vector<bool>  m_subgroupsInside;
    /// Vector of words subgroups which constitute the block.
    std::vector<PRTextGroupWords::Subgroup>  m_subgroupsWords;

    /// Bounding box.
    PoDoFo::PdfRect  m_bbox;
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
    return m_pLine;
}
inline bool PRTextLine::Block::inside( size_t idx ) const
{
    return m_subgroupsInside.at( idx );
}
inline void PRTextLine::Block::setInside( size_t idx, bool inside )
{
    m_subgroupsInside.at( idx ) = inside;
}
inline size_t PRTextLine::Block::nbSubgroups() const
{
    return m_subgroupsWords.size();
}
inline const PRTextGroupWords::Subgroup& PRTextLine::Block::subgroup( size_t idx ) const
{
    return m_subgroupsWords.at( idx );
}

}

#endif // PRTEXTLINE_H
