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

#ifndef PRGTEXTWORDS_H
#define PRGTEXTWORDS_H

#include "podofo/base/PdfString.h"
#include "PdfeGraphicsState.h"

namespace PoDoFo {
class PdfPage;
class PdfRect;
class PdfString;
class PdfVariant;
}

namespace PoDoFoExtended {
class PdfeFont;
class PdfeStreamState;
}

// File that gathers different classes that represent text elements.
namespace PdfRecut {

class PRDocument;
class PRGDocument;
class PRGTextLine;

class PRRenderPage;
class PRPenBrush;

//**********************************************************//
//                        PRGTextWord                       //
//**********************************************************//
namespace PRGTextWordType {
/** Enumeration of the different types of word.
 */
enum Enum {
    Classic = 0,        /// Classic word.
    Space,              /// Space characters (c.f. PdfeFont space characters).
    PDFTranslation,     /// PDF translation (c.f. TJ operator).
    PDFTranslationCS,   /// PDF translation replacing char space.
    Unknown             /// Unknown...
};
}

namespace PRGTextWordCoordinates {
/** Enumeration of the different coordinates system
 * a word bounding box can be expressed in.
 */
enum Enum {
    Font = 0,           /// Default font coordinates.
    FontNormalized,     /// Renormalized font coordinates.
    Page                /// Page coordinates.
};
}

/** Class that represents a single word from a PDF stream.
 * It is characterized by a corresponding CID string.
 * Metrics (advance and bbox) are given in renormalized units, i.e.
 * Fontsize = 1.0, hScale = 100. and char/word space are renormalized using fontsize.
 */
class PRGTextWord
{
public:
    /** Default empty constructor.
     */
    PRGTextWord();
    /** Constructor based on a CID string (classic or space words).
     * \param cidstr CID string representing the word.
     * \param type Type of the word.
     * \param pFont Pointer to the font object used for the word.
     */
    PRGTextWord( const PdfeCIDString& cidstr,
                 PRGTextWordType::Enum type,
                 PoDoFoExtended::PdfeFont* pFont );
    /** Constructor of PDF translation space.
     * \param spaceWidth Space width.
     * \param spaceHeight Space height.
     * \param type Type of the space.
     */
    PRGTextWord( double spaceWidth,
                 double spaceHeight,
                 PRGTextWordType::Enum type );

    /** Initialize to an empty word.
     */
    void init();
    /** Initialization based on a CID string.
     * \param cidstr CID string representing the word.
     * \param type Type of the word.
     * \param pFont Pointer to the font object used for the word.
     */
    void init( const PdfeCIDString& cidstr,
               PRGTextWordType::Enum type,
               PoDoFoExtended::PdfeFont* pFont );
    /** Initialization of PDF translation space.
     * \param spaceWidth Space width.
     * \param spaceHeight Space height.
     * \param type Type of the space.
     */
    void init( double spaceWidth,
               double spaceHeight,
               PRGTextWordType::Enum type );

public:
    // Simple getters...
    long length() const                 {   return m_cidString.length();    }
    double charSpace() const            {   return m_charSpace;     }
    PRGTextWordType::Enum type() const   {   return m_type;    }
    const PoDoFo::PdfString& pdfString() const  {   return m_pdfString; }
    const PdfeCIDString& cidString() const      {   return m_cidString; }

public:
    /** Get word advance/displacement vector.
     * \return Advance vector of the word.
     */
    PdfeVector advance() const {
        return m_advance;
    }
    /** Get word bounding box.
     * \param useBottomCoord Use the bottom coordinate of bbox?
     * \return Rectangle containing the bounding box.
     */
    PoDoFo::PdfRect bbox( bool useBottomCoord ) const;

private:
    /// Minimal height scale factor for a word (compared to space height).
    static const double MinimalHeightScale = 0.3;

private:
    /// Pdf string corresponding to the word. TODO: not working !
    PoDoFo::PdfString  m_pdfString;
    /// CID string corresponding to the word.
    PdfeCIDString  m_cidString;
    /// Word type.
    PRGTextWordType::Enum  m_type;

    /// Advance vector of the word.
    PdfeVector  m_advance;
    /// Bounding box representing the word in local coordinates.
    PoDoFo::PdfRect  m_bbox;

    /// Char space parameter used for the word.
    double  m_charSpace;
};

//**********************************************************//
//                     PRGTextGroupWords                    //
//**********************************************************//
/** Class that represents a group of words read from a PDF stream.
 * A group can be split in subgroups seperated by a PDF translation word.
 */
class PRGTextGroupWords
{
public:
    /// Embedded class that represents a subgroup.
    class Subgroup;

public:
    /** Default constructor.
     */
    PRGTextGroupWords();
    /**  Construct a group of words from a PdfVariant (appended to the group).
     * \param variant Pdf variant to read (can be string or array).
     * \param transMatrix Transformation matrix from graphics state.
     * \param textState Corresponding text state.
     * \param pFont Font object used to compute words width.
     */
    PRGTextGroupWords( const PoDoFo::PdfVariant& variant,
                       const PdfeMatrix& transMatrix,
                       const PoDoFoExtended::PdfeTextState& textState,
                       PoDoFoExtended::PdfeFont* pFont );
    /** Construct a group of words from a PDF stream state.
     * \param document Parent document.
     * \param streamState Stream state to consider (must correspond to a text showinG operator).
     */
    PRGTextGroupWords( PRDocument* document,
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
     * \param pFont Font object used to compute words width.
     */
    void readPdfVariant( const PoDoFo::PdfVariant& variant,
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
    void appendWord( const PRGTextWord& word );

    /** Compute the length of the group of words.
     * \param countSpaces Also count spaces?
     * \return Length of the group of words.
     */
    size_t length( bool countSpaces ) const;

    /** Get the advance vector of the group of words.
     * \return Advance vector of the group.
     */
    PdfeVector advance() const;
    /** Get the displacement vector of the group of words.
     * \return Displacement vector of the group.
     */
    PdfeVector displacement() const;

    /** Get the bounding box of the group of words.
     * \param wordCoord Word coordinates system in which the bbox is expressed.
     * \param leadTrailSpaces Include leading and trailing spaces (for the all group) ?
     * \param useBottomCoord Use the bottom coordinate of the bbox (unless set botom to 0).
     * \return Oriented rectangle (PdfeORect object).
     */
    PdfeORect bbox( PRGTextWordCoordinates::Enum wordCoord,
                    bool leadTrailSpaces,
                    bool useBottomCoord ) const;
    /** Estimate the global font size used for this group.
     * \return Global font size.
     */
    double fontSize() const;

    /** Get a transformation matrix from a starting coordinate
     * system to an ending one.
     * \param startCoord Starting coordinate system.
     * \param endCoord Ending coordinate system.
     * \return Transformation matrix.
     */
    PdfeMatrix transMatrix( PRGTextWordCoordinates::Enum startCoord,
                            PRGTextWordCoordinates::Enum endCoord );

    /** Get global transformation matrix (font + text + gState).
     * \return PdfeMatrix containing the transformation.
     */
    PdfeMatrix getGlobalTransMatrix() const;

    /** Minimal distance between the object and another group.
     * Use subgroups to compute the distance.
     * Distance computed in the coordinate system of the object.
     * \param group Second group.
     * \return Distance computed.
     */
    double minDistance( const PRGTextGroupWords& group ) const;
    /** Maximal distance between the object and another group.
     * Use subgroups to compute the distance.
     * Distance computed in the coordinate system of the object.
     * \param group Second group.
     * \return Distance computed.
     */
    double maxDistance( const PRGTextGroupWords& group ) const;

    /** Add a text line. The (sub)group must belong to it.
     * \param pLine Pointer to the line to add.
     */
    void addTextLine( PRGTextLine* pLine );
    /** Remove a text line. The (sub)group must not belong to it.
     * \param pLine Pointer to the line to remove.
     */
    void rmTextLine( PRGTextLine* pLine );
    /** Get lines the (sub)group belongs to.
     * \return Vector of pointers to PRGTextLines.
     */
    std::vector<PRGTextLine*> textLines() const;

    /** Is the group composed uniquely of space characters?
     */
    bool isSpace() const;

public:
    /** Build the private vector of main subgroups.
     * Main subgroups are separated by PDF translation words.
     */
    void buildMainSubGroups();

public:
    // Rendering routines.
    /** Render a group of words inside a page.
     * \param renderPage Object on which words are rendered.
     * \param textPB Pen/Brush used for normal text.
     * \param spacePB Pen/Brush used for space characters.
     * \param translationPB Pen/Brush used for PDF translation.
     */
    void render( PRRenderPage& renderPage,
                 const PRPenBrush& textPB,
                 const PRPenBrush& spacePB,
                 const PRPenBrush& translationPB ) const;
    /** Render words glyphs.
     * \param renderPage Object on which glyphs are rendered.
     */
    void renderGlyphs( PRRenderPage& renderPage ) const;

public:
    // Getters.
    long pageIndex() const                          {   return m_pageIndex;     }
    long groupIndex() const                         {   return m_groupIndex;    }
    const PdfeMatrix& gsTransMatrix() const         {   return m_transMatrix;   }
    const PoDoFoExtended::PdfeTextState& textState() const  {   return m_textState; }
    const PoDoFo::PdfRect& fontBBox() const         {   return m_fontBBox;  }
    PoDoFoExtended::PdfeFont* font() const          {   return m_pFont; }

    // Setters
    void setPageIndex( long pageIndex );
    void setGroupIndex( long groupIndex );
    void setGSTransMatrix( const PdfeMatrix& transMatrix );
    void setTextState( const PoDoFoExtended::PdfeTextState& textState );

public:
    /// Get the number of words in the group.
    size_t nbWords() const;
    /// Get a constant reference to a word.
    const PRGTextWord& word( size_t idx ) const;
    /// Get the number of main subgroups.
    size_t nbMSubgroups() const;
    /// Get a main subgroup component.
    const Subgroup& mSubgroup( size_t idx ) const;

private:
    /// Max char space allowed inside a word: when greater, split the word and replace char space by PDF translation.
    /// Compared to font mean width.
    static const double MaxCharSpaceScale = 0.4;

private:
    /// Index of the page to which belongs the group.
    long  m_pageIndex;
    /// Index of the group in the page.
    long  m_groupIndex;

    /// Font used for the group.
    PoDoFoExtended::PdfeFont*  m_pFont;
    /// Font bounding box.
    PoDoFo::PdfRect  m_fontBBox;
    /// Font renormalization transformation matrix (font coord to renorm coord)..
    PdfeMatrix  m_fontNormTransMatrix;

    /// Transformation matrix of the graphics state.
    PdfeMatrix  m_transMatrix;
    /// Text state for this group of words.
    PoDoFoExtended::PdfeTextState  m_textState;

    /// Vector of words that make the group.
    std::vector<PRGTextWord>  m_words;
    /// Main subgroups of words.
    std::vector<Subgroup>  m_mainSubgroups;

    /// Text lines the group (or a subgroup) belongs to.
    std::vector<PRGTextLine*>  m_pTextLines;
};

//**********************************************************//
//                PRGTextGroupWords::Subgroup               //
//**********************************************************//
/** Class that represents a subgroup of words.
 */
class PRGTextGroupWords::Subgroup
{
public:
    /** Default empty constructor.
     */
    Subgroup();
    /** Constructor from a PRTextGroups: initialize to the complete subgroup (true).
     * \param group Constant reference to the group. Should not be a temp object !
     * \param allGroup Correspond to the complete group?
     */
    Subgroup( const PRGTextGroupWords& group, bool allGroup = true );
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
    void init( const PRGTextGroupWords& group, bool allGroup = true );

    /** Get the parent group.
     * \return Pointer the parent group.
     */
    PRGTextGroupWords* group() const;
    /** Set the parent group.
     * \param pGroup Pointer to the group.
     */
    void setGroup( PRGTextGroupWords* pGroup );

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
    const PRGTextWord* word( size_t idxWord ) const;

    /** Compute the length of the subgroup of words.
     * \param countSpaces Also count spaces?
     * \return Length of the subgroup of words.
     */
    size_t length( bool countSpaces = true ) const;

    /** Get the advance/displacement vector of the subgroup of words.
     * Consider advance vectors of every words inside the interval [first,last].
     * \param useGroupOrig Use the first word of the group as origin.
     * \return Advance vector of the group.
     */
    PdfeVector advance( bool useGroupOrig ) const;

    /** Get the bounding box of the subgroup of words.
     * \param wordCoord Word coordinates system in which the bbox is expressed.
     * \param leadTrailSpaces Include leading and trailing spaces ?
     * \param useBottomCoord Use the bottom coordinate of the bbox (unless set botom to 0).
     * \return Oriented rectangle (PdfeORect object).
     */
    PdfeORect bbox( PRGTextWordCoordinates::Enum wordCoord,
                    bool leadTrailSpaces = true,
                    bool useBottomCoord = true ) const;

    /** Is the subgroup empty? Meaning to that no word belongs to it.
     * \return True if empty, false otherwise.
     */
    bool isEmpty() const;
    /** Is the subgroup uniquely composed of space characters?
     * \return Answer!
     */
    bool isSpace() const;

public:
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

public:
    // Rendering routines.
    /** Render a group of words inside a page.
     * \param renderPage Object on which words are rendered.
     * \param textPB Pen/Brush used for normal text.
     * \param spacePB Pen/Brush used for space characters.
     * \param translationPB Pen/Brush used for PDF translation.
     */
    void render( PRRenderPage& renderPage,
                 const PRPenBrush& textPB,
                 const PRPenBrush& spacePB,
                 const PRPenBrush& translationPB ) const;
    /** Render words glyphs.
     * \param renderPage Object on which glyphs are rendered.
     */
    void renderGlyphs( PRRenderPage& renderPage ) const;

private:
    /// Pointer to the group.
    PRGTextGroupWords*  m_pGroup;
    /// Vector of boolean telling if words of the group belongs to the subgroup.
    std::vector<bool>  m_wordsInside;

    // Cache private data.
    /// Cache Bounding box (with spaces and bottom). TODO: improve?
    mutable PdfeORect  m_bboxCache;
    /// Is the bounding box cache?
    mutable bool  m_isBBoxCache;
};

//**********************************************************//
//                    Inline PRGTextWord                     //
//**********************************************************//
inline PoDoFo::PdfRect PRGTextWord::bbox( bool useBottomCoord ) const
{
    if( useBottomCoord ) {
        return m_bbox;
    }
    // Set bottom to zero.
    PoDoFo::PdfRect bbox = m_bbox;
    bbox.SetHeight( bbox.GetHeight() + bbox.GetBottom() );
    bbox.SetBottom( 0.0 );
    return bbox;
}

//**********************************************************//
//                 Inline PRGTextGroupWords                  //
//**********************************************************//
inline void PRGTextGroupWords::setPageIndex( long pageIndex )
{
    m_pageIndex = pageIndex;
}
inline void PRGTextGroupWords::setGroupIndex( long groupIndex )
{
    m_groupIndex = groupIndex;
}
inline std::vector<PRGTextLine*> PRGTextGroupWords::textLines() const
{
    return m_pTextLines;
}
inline void PRGTextGroupWords::setGSTransMatrix( const PdfeMatrix& transMatrix )
{
    m_transMatrix = transMatrix; m_words.size();
}
inline void PRGTextGroupWords::setTextState( const PoDoFoExtended::PdfeTextState& textState )
{
    m_textState = textState;
}

inline size_t PRGTextGroupWords::nbWords() const
{
    return m_words.size();
}
inline const PRGTextWord& PRGTextGroupWords::word( size_t idx ) const
{
    return m_words.at(idx);     // Throw out of range exception if necessary.
}
inline size_t PRGTextGroupWords::nbMSubgroups() const
{
    return m_mainSubgroups.size();
}
inline const PRGTextGroupWords::Subgroup& PRGTextGroupWords::mSubgroup( size_t idx ) const
{
    return m_mainSubgroups.at( idx );
}

//**********************************************************//
//            Inline PRGTextGroupWords::SubGroup             //
//**********************************************************//
inline PRGTextGroupWords* PRGTextGroupWords::Subgroup::group() const
{
    return const_cast<PRGTextGroupWords*>( m_pGroup );
}
inline void PRGTextGroupWords::Subgroup::setGroup( PRGTextGroupWords* pGroup )
{
    m_pGroup = pGroup;
}
inline bool PRGTextGroupWords::Subgroup::inside( size_t idxWord ) const
{
    return m_wordsInside.at( idxWord );
}
inline void PRGTextGroupWords::Subgroup::setInside( size_t idxWord, bool inside )
{
    m_wordsInside.at( idxWord ) = inside;
    m_isBBoxCache = false;
}
inline const PRGTextWord* PRGTextGroupWords::Subgroup::word( size_t idxWord ) const
{
    if( m_wordsInside.at( idxWord ) ) {
        return &( m_pGroup->word( idxWord ) );
    }
    return NULL;
}
inline bool PRGTextGroupWords::Subgroup::isEmpty() const
{
    for( size_t i = 0 ; i < m_wordsInside.size() ; ++i ) {
        if( m_wordsInside[i] ) {
            return false;
        }
    }
    return true;
}

}

#endif // PRGTEXTWORDS_H