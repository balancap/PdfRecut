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

#ifndef PRTEXTWORDS_H
#define PRTEXTWORDS_H

#include "PdfeGraphicsState.h"
#include "PdfeFont.h"

namespace PoDoFo {
class PdfPage;
class PdfRect;
class PdfString;
class PdfVariant;
}

namespace PoDoFoExtended {
class PdfeStreamState;
}

// File that gathers different classes that represent text elements.
namespace PdfRecut {

class PRDocument;
class PRTextLine;

//**********************************************************//
//                         PRTextWord                       //
//**********************************************************//
namespace PRTextWordType {
/** Enumeration of the different types of word.
 */
enum Enum {
    Classic = 0,        /// Classic word.
    Space,              /// Space character 0x0020.
    PDFTranslation,     /// PDF translation (c.f. TJ operator).
    PDFTranslationCS,   /// PDF translation replacing char space.
    Unknown             /// Unknown...
};
}

/** Class that represents a single word from a PDF stream.
 */
class PRTextWord
{
public:
    /** Default constructor.
     */
    PRTextWord();
    /** Constructor used to set members.
     */
    PRTextWord( PRTextWordType::Enum type,
                long length,
                const PoDoFo::PdfRect& bbox,
                double lastCharSpace );

    /** Initialize to an empty word.
     */
    void init();

public:
    // Getters...
    long length() const                 {   return m_length;    }
    PRTextWordType::Enum type() const   {   return m_type;    }
    double lastCharSpace() const        {   return m_lastCharSpace;    }
    const PoDoFo::PdfString& pdfString() const  {   return m_pdfString; }
    const PdfeCIDString& cidString() const      {   return m_cidString; }

    // and setters.
    void setLength( long length )                   {   m_length = length;  }
    void setType( PRTextWordType::Enum type )       {   m_type = type;  }
    void setLastCharSpace( double lastCharSpace )   {   m_lastCharSpace = lastCharSpace;  }
    void setPdfString( const PoDoFo::PdfString& rhs )   {   m_pdfString = rhs;  }
    void setCIDString( const PdfeCIDString& rhs )       {   m_cidString = rhs;  }

public:
    /** Get word bounding box.
     * \param lastSpace Include last char space in width ? (default = true)
     * \param useBottomCoord Use the bottom coordinate of the bbox (unless set to 0).
     * \return Rectangle containing the bounding box.
     */
    PoDoFo::PdfRect bbox( bool lastSpace = true,
                          bool useBottomCoord = true ) const;

    /** Set the bounding box of the word (last char space included).
     * \param bbox New bounding box.
     */
    void setBBox( const PoDoFo::PdfRect& bbox );

    /** Get word width.
     * \param lastSpace Include last char space in width ? (default = true)
     * \return Width of the word.
     */
    double width( bool lastSpace ) const;

private:
    /// Pdf string corresponding to the word. TODO: not working !
    PoDoFo::PdfString  m_pdfString;
    /// CID string corresponding to the word.
    PdfeCIDString  m_cidString;

    /// Word type.
    PRTextWordType::Enum  m_type;
    /// Length: number of characters.
    long  m_length;

    /// Bounding box representing the word in local coordinates (includes last char space).
    /// Always assume the left coordinate of the word is set to 0.
    PoDoFo::PdfRect  m_bbox;
    /// Width of the last char space (usually zero...).
    double  m_lastCharSpace;
};

//**********************************************************//
//                      PRTextGroupWords                    //
//**********************************************************//
/** Class that represents a group of words read from a PDF stream.
 * A group can be split in subgroups seperated by a PDF translation word.
 */
class PRTextGroupWords
{
public:
    /// Embedded class that represents a subgroup.
    class Subgroup;

public:
    /** Default constructor.
     */
    PRTextGroupWords();
    /**  Construct a group of words from a PdfVariant (appended to the group).
     * \param variant Pdf variant to read (can be string or array).
     * \param transMatrix Transformation matrix from graphics state.
     * \param textState Corresponding text state.
     * \param pFont Font object used to compute words width.
     */
    PRTextGroupWords( const PoDoFo::PdfVariant& variant,
                      const PdfeMatrix& transMatrix,
                      const PoDoFoExtended::PdfeTextState& textState,
                      PoDoFoExtended::PdfeFont* pFont );
    /** Construct a group of words from a PDF stream state.
     * \param document Parent document.
     * \param streamState Stream state to consider (must correspond to a text showinG operator).
     */
    PRTextGroupWords( PRDocument* document,
                      const PoDoFoExtended::PdfeStreamState& streamState );

    /** Initialize to an empty group of words.
     */
    void init();
    /** Initialize a group of words from a PdfVariant (appended to the group).
     * \param variant Pdf variant to read (can be string or array).
     * \param transMatrix Transformation matrix from graphics state.
     * \param textState Corresponding text state.
     * \param pFont Font object used to compute words width.
     */
    void init( const PoDoFo::PdfVariant& variant,
               const PdfeMatrix& transMatrix,
               const PoDoFoExtended::PdfeTextState& textState,
               PoDoFoExtended::PdfeFont* pFont );
    /** Initialize a group of words from a PDF stream state.
     * \param document Parent document.
     * \param streamState Stream state to consider (must correspond to a text showinG operator).
     */
    void init( PRDocument* document,
               const PoDoFoExtended::PdfeStreamState& streamState );

    /** Read a group of words from a PdfVariant (appended to the group).
     * \param variant Pdf variant to read (can be string or array).
     * \param transMatrix Transformation matrix from graphics state.
     * \param textState Corresponding text state.
     * \param pFont Font object used to compute words width.
     */
    void readPdfVariant( const PoDoFo::PdfVariant& variant,
                         const PdfeMatrix& transMatrix,
                         const PoDoFoExtended::PdfeTextState& textState,
                         PoDoFoExtended::PdfeFont* pFont );

private:
    /** Read a group of words from a PdfString (appended to the group).
     * \param str Pdf string to read (can contain 0 characters !).
     * \param pFont Font object used to compute words width.
     */
    void readPdfString( const PoDoFo::PdfString& str,
                        PoDoFoExtended::PdfeFont* pFont );

public:
    /** Append a word to the group.,
     * \param word Word to append.
     */
    void appendWord( const PRTextWord& word );

    /** Compute the width of the group of words.
     * \param leadTrailSpaces Include leading and trailing spaces ?
     * \return Width of the group of words.
     */
    double width( bool leadTrailSpaces ) const;

    /** Compute the height of the group of words.
     * \return Height of the group of words.
     */
    double height() const;

    /** Compute the length of the group of words.
     * \param countSpaces Also count spaces?
     * \return Length of the group of words.
     */
    size_t length( bool countSpaces ) const;

    /** Get global transformation matrix (font + text + gState).
     * \return PdfeMatrix containing the transformation.
     */
    PdfeMatrix getGlobalTransMatrix() const;

    /** Get the bounding box of the group of words.
     * \param pageCoords In page coordinates (true) or local coordinates (false) ?
     * \param leadTrailSpaces Include leading and trailing spaces (for the all group) ?
     * \param useBottomCoord Use the bottom coordinate of the bbox (unless set botom to 0).
     * \return Oriented rectangle (PdfeORect object).
     */
    PdfeORect bbox( bool pageCoords,
                    bool leadTrailSpaces,
                    bool useBottomCoord ) const;

    /** Compute the displacement vector of the group of words.
     * \return Displacement vector (horizontal and vertical components).
     */
    PdfeVector displacement() const;

    /** Minimal distance between the object and another group.
     * Use subgroups to compute the distance.
     * Distance computed in the coordinate system of the object.
     * \param group Second group.
     * \return Distance computed.
     */
    double minDistance( const PRTextGroupWords& group ) const;

    /** Maximal distance between the object and another group.
     * Use subgroups to compute the distance.
     * Distance computed in the coordinate system of the object.
     * \param group Second group.
     * \return Distance computed.
     */
    double maxDistance( const PRTextGroupWords& group ) const;

    /** Add a text line. The (sub)group must belong to it.
     */
    void addTextLine( PRTextLine* pLine );
    /** Remove a text line. The (sub)group must not belong to it.
     */
    void rmTextLine( PRTextLine* pLine );

    /** Get lines the (sub)group belongs to.
     */
    std::vector<PRTextLine*> textLines() const;

public:
    /** Build the private vector of main subgroups.
     * Main subgroups are separated by PDF translation words.
     */
    void buildMainSubGroups();

public:
    // Getters.
    long pageIndex() const                          {   return m_pageIndex;     }
    long groupIndex() const                         {   return m_groupIndex;    }
    const PdfeMatrix& transMatrix() const           {   return m_transMatrix;   }
    const PoDoFoExtended::PdfeTextState& textState() const  {   return m_textState; }
    const PoDoFo::PdfRect& fontBBox() const         {   return m_fontBBox;  }
    PoDoFoExtended::PdfeFont* font() const          {   return m_pFont; }

    // Setters
    void setPageIndex( long pageIndex );
    void setGroupIndex( long groupIndex );
    void setTransMatrix( const PdfeMatrix& transMatrix );
    void setTextState( const PoDoFoExtended::PdfeTextState& textState );

public:
    /** Get the number of words in the group.
     * \return Number of words.
     */
    size_t nbWords() const;

    /** Get a constant reference to a word.
     * \param idx Index of the word.
     * \return Constant reference to a PRTextWord.
     */
    const PRTextWord& word( size_t idx ) const;

    /** Get the number of main subgroups.
     * \return Number of subgroups.
     */
    size_t nbMSubgroups() const;

    /** Get a main subgroup component.
     * \param idx Index of the subgroup.
     * \return Const reference to a PRTextGroupWords::SubGroup.
     */
    const Subgroup& mSubgroup( size_t idx ) const;

protected:
    /// Default space height used.
    static const double SpaceHeight = 0.5;
    /// Minimal height for a character.
    static const double MinimalHeight = 0.2;
    /// Max char space allowed inside a word: when greater, split the word and replace char space by PDF translation.
    static const double MaxWordCharSpace = -10.0;

protected:
    /// Index of the page to which belongs the group.
    long  m_pageIndex;
    /// Index of the group in the page.
    long  m_groupIndex;

    /// Font used for the group.
    PoDoFoExtended::PdfeFont*  m_pFont;
    /// Font bounding box.
    PoDoFo::PdfRect  m_fontBBox;

    /// Text lines the group (or a subgroup) belongs to.
    std::vector<PRTextLine*>  m_pTextLines;

    /// Transformation matrix of the graphics state.
    PdfeMatrix  m_transMatrix;
    /// Text state for this group of words.
    PoDoFoExtended::PdfeTextState  m_textState;

    /// Vector of words that make the group.
    std::vector<PRTextWord>  m_words;
    /// Main subgroups of words.
    std::vector<Subgroup>  m_mainSubgroups;
};

//**********************************************************//
//                 PRTextGroupWords::Subgroup               //
//**********************************************************//
/** Class that represents a subgroup of words.
 */
class PRTextGroupWords::Subgroup
{
public:
    /** Default empty constructor.
     */
    Subgroup();
    /** Constructor from a PRTextGroups: initialize to the complete subgroup (true).
     * \param group Constant reference to the group. Should not be a temp object !
     * \param allGroup Correspond to the complete group?
     */
    Subgroup( const PRTextGroupWords& group, bool allGroup = true );
    /** Copy constructor.
     * \param subgroup Subgroup to copy.
     */
    Subgroup( const Subgroup& subgroup );

    /** Initialize to the empty subgroup.
     */
    void init();
    /** Initialize to the complete subgroup using a given group.
     * \param pGroup Constant reference to the group. Should not be a temp object !
     * \param allGroup Correspond to the complete group?
     */
    void init( const PRTextGroupWords& group, bool allGroup = true );

    /** Get the parent group.
     * \return Pointer the parent group.
     */
    PRTextGroupWords* group() const;
    /** Set the parent group.
     * \param pGroup Pointer to the group.
     */
    void setGroup( PRTextGroupWords* pGroup );

    /** Is a word inside the subgroup? Can raise an exception.
     * \param idxWord Index of the word.
     * \return The word belongs to the subgroup?
     */
    bool inside( size_t idxWord ) const;
    /** Set if the word belongs, or not, to the subgroup.
     * Can raise an exception.
     * \param idxWord Index of the word.
     * \param inside Is the word inside the subgroup.
     */
    void setInside( size_t idxWord, bool inside );

    /** Get a pointer to a word. Can raise an exception.
     * \param idxWord Index of the word.
     * \return Pointer to the word. NULL if it does belong to the subgroup.
     */
    const PRTextWord* word( size_t idxWord ) const;

    /** Get the bounding box of the subgroup of words.
     * \param pageCoords In page coordinates (true) or local coordinates (false) ?
     * \param leadTrailSpaces Include leading and trailing spaces ?
     * \param useBottomCoord Use the bottom coordinate of the bbox (unless set botom to 0).
     * \return Oriented rectangle (PdfeORect object).
     */
    PdfeORect bbox( bool pageCoords = true,
                    bool leadTrailSpaces = true,
                    bool useBottomCoord = true ) const;

    /** Compute the width of the subgroup of words.
     * \param leadTrailSpaces Include leading and trailing spaces ?
     * \return Width of the subgroup of words.
     */
    double width( bool leadTrailSpaces = true ) const;

    /** Compute the height of the subgroup of words.
     * \return Height of the subgroup of words.
     */
    double height() const;

    /** Compute the length of the subgroup of words.
     * \param countSpaces Also count spaces?
     * \return Length of the subgroup of words.
     */
    size_t length( bool countSpaces = true ) const;

    /** Is the subgroup empty? Meaning to that no word belongs to it.
     * \return True if empty, false otherwise.
     */
    bool isEmpty() const;

    /** Intersection of two subgroups.
     * The parent group must be the same (return empty subgroup else).
     * \param subgroup1 First subgroup.
     * \param subgroup2 Second subgroup.
     * \return New subgroup corresponding to the intersection.
     */
    static Subgroup intersection( const Subgroup& subgroup1, const Subgroup& subgroup2 );

    /** Reunion of two subgroups.
     * The parent group must be the same (return empty subgroup else).
     * \param subgroup1 First subgroup.
     * \param subgroup2 Second subgroup.
     * \return New subgroup corresponding to the union.
     */
    static Subgroup reunion( const Subgroup& subgroup1, const Subgroup& subgroup2 );

private:
    /// Pointer to the group.
    PRTextGroupWords*  m_pGroup;
    /// Vector of boolean telling if words of the group belongs to the subgroup.
    std::vector<bool>  m_wordsInside;
};

//**********************************************************//
//                 Inline PRTextGroupWords                  //
//**********************************************************//
inline void PRTextGroupWords::setPageIndex( long pageIndex )
{
    m_pageIndex = pageIndex;
}
inline void PRTextGroupWords::setGroupIndex( long groupIndex )
{
    m_groupIndex = groupIndex;
}
inline std::vector<PRTextLine*> PRTextGroupWords::textLines() const
{
    return m_pTextLines;
}
inline void PRTextGroupWords::setTransMatrix( const PdfeMatrix& transMatrix )
{
    m_transMatrix = transMatrix; m_words.size();
}
inline void PRTextGroupWords::setTextState( const PoDoFoExtended::PdfeTextState& textState )
{
    m_textState = textState;
}

inline size_t PRTextGroupWords::nbWords() const
{
    return m_words.size();
}
inline const PRTextWord& PRTextGroupWords::word( size_t idx ) const
{
    return m_words.at(idx);     // Throw out of range exception if necessary.
}
inline size_t PRTextGroupWords::nbMSubgroups() const
{
    return m_mainSubgroups.size();
}
inline const PRTextGroupWords::Subgroup& PRTextGroupWords::mSubgroup( size_t idx ) const
{
    return m_mainSubgroups.at( idx );
}

//**********************************************************//
//            Inline PRTextGroupWords::SubGroup             //
//**********************************************************//
inline PRTextGroupWords* PRTextGroupWords::Subgroup::group() const
{
    return const_cast<PRTextGroupWords*>( m_pGroup );
}
inline void PRTextGroupWords::Subgroup::setGroup( PRTextGroupWords* pGroup )
{
    m_pGroup = pGroup;
}
inline bool PRTextGroupWords::Subgroup::inside( size_t idxWord ) const
{
    return m_wordsInside.at( idxWord );
}
inline void PRTextGroupWords::Subgroup::setInside( size_t idxWord, bool inside )
{
    m_wordsInside.at( idxWord ) = inside;
}
inline const PRTextWord* PRTextGroupWords::Subgroup::word( size_t idxWord ) const
{
    if( m_wordsInside.at( idxWord ) ) {
        return &( m_pGroup->word( idxWord ) );
    }
    return NULL;
}
inline bool PRTextGroupWords::Subgroup::isEmpty() const
{
    for( size_t i = 0 ; i < m_wordsInside.size() ; ++i ) {
        if( m_wordsInside[i] ) {
            return false;
        }
    }
    return true;
}

}

#endif // PRTEXTWORDS_H
