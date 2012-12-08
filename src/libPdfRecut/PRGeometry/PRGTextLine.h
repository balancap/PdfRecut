/***************************************************************************
 * Copyright (C) Paul Balança - All Rights Reserved                        *
 *                                                                         *
 * NOTICE:  All information contained herein is, and remains               *
 * the property of Paul Balança. Dissemination of this information or      *
 * reproduction of this material is strictly forbidden unless prior        *
 * written permission is obtained from Paul Balança.                       *
 *                                                                         *
 * Written by Paul Balança <paul.balanca@gmail.com>, 2012                  *
 ***************************************************************************/

#ifndef PRGTEXTLINE_H
#define PRGTEXTLINE_H

#include "PRGTextWords.h"

namespace PdfRecut {

class PRGTextPage;

namespace PRGTextLineCoordinates {
/** Enumeration of the different coordinates system
 * a line bounding box can be expressed in.
 */
enum Enum {
    Line = 0,           /// Default line coordinates.
    LineDocRescaled,    /// Rescaled word coordinates using document statistics.
    Page                /// Page coordinates.
};
}

/** Class that represent a line of text inside a PDF page.
 */
class PRGTextLine
{
public:
    /** Default constructor.
     * \param textPage Parent text page object.
     */
    PRGTextLine( PRGTextPage* textPage );
    /** Destructor...
      */
    ~PRGTextLine();
    /** Initialize to an empty line.
     */
    void init();

    /** Add a group of words to the line.
     * \param groupWords Pointer to the group of words to add.
     */
    void addGroupWords( PRGTextGroupWords* pGroupWords );
    /** Add a subgroup of words to the line.
     * \param subgroup Reference to the subgroup of words to add.
     */
    void addSubgroupWords( const PRGTextGroupWords::Subgroup& subgroup );
    /** Remove a group of words from the line.
     * \param groupWords Pointer to the group of words to remove.
     */
    void rmGroupWords( PRGTextGroupWords* pGroupWords );
    /** Set a subgroup which is inside the line.
     * Can raise an out of range exception.
     * \param idx Index of the subgroup to modify.
     * \param subgroup Constant reference to the new subgroup.
     */
    void setSubgroup( size_t idx, const PRGTextGroupWords::Subgroup& subgroup );
    /** Get a subgroup which is inside the line.
     * Can raise an exception.
     * \param idx Index of the subgroup.
     * \return Constant reference to the subgroup.
     */
    const PRGTextGroupWords::Subgroup& subgroup( size_t idx ) const;
    /** Get the number of subgroups in the line.
     * \return Number of subgroups.
     */
    size_t nbSubgroups() const;
    /** Does the group belongs to the line.
     * \param pGroup Pointer to the group.
     * \return Index of the corresponding subgroup (-1 if not found).
     */
    long hasGroupWords( PRGTextGroupWords* pGroup ) const;

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
     * \param endCoords Coordinates system in which the bounding box is expressed.
     * \param leadTrailSpaces Include leading and trailing spaces ?
     * \return Oriented rectangle containing the bounding box.
     */
    PdfeORect bbox( PRGTextLineCoordinates::Enum endCoords,
                    bool leadTrailSpaces ) const;
    /** Get the width of the line.
     * \param endCoords Coordinates system in which the width is expressed.
     * \param leadTrailSpaces Include leading and trailing spaces ?
     * \return Width in local coordinates.
     */
    double width( PRGTextLineCoordinates::Enum endCoords,
                  bool leadTrailSpaces ) const;
    /** Get the cumulative width, i.e. sum of words width.
     * \param endCoords Coordinates system in which the width is expressed.
     * \param incSpaces Include spaces in the computation.
     */
    double widthCumulative( PRGTextLineCoordinates::Enum endCoords,
                            bool incSpaces ) const;
    /** Get the length of the line.
     * \param countSpaces Also count spaces?
     * \return Length of the line.
     */
    size_t length( bool countSpaces ) const;

    /** Get a transformation matrix from a starting coordinate
     * system to an ending one.
     * \param startCoord Starting coordinate system.
     * \param endCoord Ending coordinate system.
     * \return Transformation matrix.
     */
    PdfeMatrix transMatrix( PRGTextLineCoordinates::Enum startCoord,
                            PRGTextLineCoordinates::Enum endCoord ) const;

    /** Mean font size of the line.
     */
    double meanFontSize() const;
    /** Is the line empty?
     * \return Answer!
     */
    bool isEmpty() const;
    /** Is the line uniquelly composed of spaces?
     * \return Answer!
     */
    bool isSpace() const;

public:
    // Getters.
    PRGTextPage* textPage() const   {   return m_textPage;      }
    long lineIndex() const          {   return m_lineIndex;     }

    // Setters
    void setLineIndex( long lineIndex ) {   m_lineIndex = lineIndex;    }

public:
    /// Embedded class that represent a block of a line.
    class Block;
    /** Obtain horizontal blocks defined by the line.
     * \param hDistance Horizontal distance used for blocks merging.
     * \return Vector of PRGTextLine::Block*. Objects are owned by the user.
     */
    std::vector<PRGTextLine::Block*> horizontalBlocks( double hDistance = 0.0 ) const;
    /** Obtain horizontal blocks defined by the line.
     * \param hDistance Horizontal distance used for blocks merging.
     * \return List of PRGTextLine::Block.
     */
    std::list<PRGTextLine::Block> horizontalBlocksList( double hDistance = 0.0 ) const;

public:
    /** Compare two lines using the index of their groups.
     * \param pLine1 Pointer to the first line.
     * \param pLine2 Pointer to the second line.
     * \return line1 < line2.
     */
    static bool compareGroupIndex( PRGTextLine* pLine1, PRGTextLine* pLine2 );

private:
    struct Data;
    /// Get private data member. Re-compute data if necessary.
    Data* data() const;
    /// Line has been modified.
    void modified();
    /// Clear empty subgroups in the line.
    void clearEmptySubgroups();

    /// Compute inside cache data of the line (bbox, first capital letter, ...).
    void computeCacheData() const;
    /// Compute bounding boxes and transformation matrices.
    void computeBBoxes() const;

private:
    /// Pointer to the text page it belongs to.
    PRGTextPage*  m_textPage;
    /// Index of the line in the page.
    long  m_lineIndex;

    /// Vector of words subgroups which constitute the line.
    std::vector<PRGTextGroupWords::Subgroup>  m_subgroupsWords;

    // Cache data.
    /// Does the cache data need to be re-computed?
    mutable bool  m_resetCachedData;
    /** Structure that gathers line cached data.
     */
    struct Data {
        /// Transformation matrix (into page coordinates). Related to the bounding box.
        PdfeMatrix  transMatPage;
        /// Document statistics transformation matrix (line coord to doc rescaled coord)..
        PdfeMatrix  transMatDocRescale;

        /// Line bounding box.
        PoDoFo::PdfRect  bbox;
        /// Line bounding box (with no leading and trailing spaces).
        PoDoFo::PdfRect  bboxNoLTSpaces;

        /// Cumulative width.
        double  widthCumul;
        /// Cumulative width, without spaces.
        double  widthCumulNoSpaces;
        /// Mean font size of the line.
        double  meanFontSize;

        /// Initialize to default values.
        void init();
    };
    /// Cached data.
    mutable Data  m_data;
};

/** Block inside a line: represent a generic collection of subgroups
 * that must belong to a common line.
 */
class PRGTextLine::Block
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
    Block( const PRGTextLine* pLine,
           const PRGTextGroupWords::Subgroup& subgroup,
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
    void init( const PRGTextLine* pLine,
               const PRGTextGroupWords::Subgroup& subgroup,
               bool leadTrailSpaces,
               bool useBottomCoord );

    /** Get the parent line.
     * \return Pointer the parent line.
     */
    PRGTextLine* line() const;

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
    const PRGTextGroupWords::Subgroup& subgroup( size_t idx ) const;

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
    PRGTextLine*  m_pLine;

    /// Vector of boolean telling if a line subgroup (or a subset) belongs to the block.
    std::vector<bool>  m_subgroupsInside;
    /// Vector of words subgroups which constitute the block.
    std::vector<PRGTextGroupWords::Subgroup>  m_subgroupsWords;

    /// Bounding box.
    PoDoFo::PdfRect  m_bbox;
};

//**********************************************************//
//                     Inline PRGTextLine                   //
//**********************************************************//
inline size_t PRGTextLine::nbSubgroups() const
{
    return m_subgroupsWords.size();
}
inline const PRGTextGroupWords::Subgroup& PRGTextLine::subgroup( size_t idx ) const
{
    return m_subgroupsWords.at( idx );
}
inline PRGTextLine::Data* PRGTextLine::data() const
{
    if( m_resetCachedData ) {
        this->computeCacheData();
        m_resetCachedData = false;
    }
    return &m_data;
}
inline void PRGTextLine::modified()
{
    this->clearEmptySubgroups();
    m_resetCachedData = true;
}

//**********************************************************//
//                  Inline PRGTextLine::Block                //
//**********************************************************//
inline PRGTextLine* PRGTextLine::Block::line() const
{
    return m_pLine;
}
inline bool PRGTextLine::Block::inside( size_t idx ) const
{
    return m_subgroupsInside.at( idx );
}
inline void PRGTextLine::Block::setInside( size_t idx, bool inside )
{
    m_subgroupsInside.at( idx ) = inside;
}
inline size_t PRGTextLine::Block::nbSubgroups() const
{
    return m_subgroupsWords.size();
}
inline const PRGTextGroupWords::Subgroup& PRGTextLine::Block::subgroup( size_t idx ) const
{
    return m_subgroupsWords.at( idx );
}

}

#endif // PRGTEXTLINE_H
