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

#ifndef PRTEXTELEMENTS_H
#define PRTEXTELEMENTS_H

#include "PdfGraphicsState.h"
#include "PdfeFont.h"

namespace PoDoFo {
class PdfPage;
class PdfRect;
class PdfString;
class PdfVariant;
}

// File that gathers different classes that represent text elements.

namespace PdfRecut {

class PRTextLine;

//**********************************************************//
//                         PRTextWord                       //
//**********************************************************//
namespace PRTextWordType {
/** Enumeration of the different types of word.
 */
enum Enum {
    Classic = 0,    /// Classic word.
    Space,          /// Space character 0x0020.
    PDFTranslation, /// PDF translation (c.f. TJ operator).
    Unknown         /// Unknown...
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
    // Getters and setters...
    long length() const             {   return m_length;    }
    void setLength( long length )   {   m_length = length;  }

    PRTextWordType::Enum type() const           {   return m_type;    }
    void setType( PRTextWordType::Enum type )   {   m_type = type;  }

    double lastCharSpace() const                    {   return m_lastCharSpace;    }
    void setLastCharSpace( double lastCharSpace )   {   m_lastCharSpace = lastCharSpace;  }

public:
    /** Get word bounding box.
     * \param lastSpace Include last char space in width ? (default = true)
     * \return Rectangle containing the bounding box.
     */
    PoDoFo::PdfRect bbox( bool lastSpace = true ) const;

    /** Set the bounding box of the word (last char space included).
     * \param bbox New bounding box.
     */
    void setBBox( const PoDoFo::PdfRect& bbox );

    /** Get word width.
     * \param lastSpace Include last char space in width ? (default = true)
     * \return Width of the word.
     */
    double width( bool lastSpace = true ) const;

protected:
    /// Length: number of characters.
    long  m_length;

    /// Bounding box representing the word in local coordinates (includes last char space).
    /// Always assume the left coordinate of the word is set to 0.
    PoDoFo::PdfRect  m_bbox;

    /// Width of the last char space (usually zero...).
    double  m_lastCharSpace;

    /// Word type.
    PRTextWordType::Enum  m_type;
};

//**********************************************************//
//                      PRTextGroupWords                    //
//**********************************************************//
/** Class that represents a group of words read from a PDF stream.
 */
class PRTextGroupWords
{
public:
    /** Default constructor.
     */
    PRTextGroupWords();

    /** Initialize to an empty group of words.
     */
    void init();

    /** Read a group of words from a PdfVariant (appended to the group).
     * \param variant Pdf variant to read (can be string or array).
     * \param pFont Font object used to compute words width.
     */
    void readPdfVariant( const PoDoFo::PdfVariant& variant,
                         PoDoFoExtended::PdfeFont* pFont );

    /** Append a word to the group.,
     * \param word Word to append.
     */
    void appendWord( const PRTextWord& word );

    /** Compute the width of the group of words.
     * \return Width of the group of words.
     */
    double width() const;
    /** Compute the height of the group of words.
     * \return Height of the group of words.
     */
    double height() const;
    /** Compute the length of the group of words.
     * \param countSpaces Also count spaces ?
     * \return Length of the group of words.
     */
    size_t length( bool countSpaces = true );

    /** Get global transformation matrix (font + text + gState).
     */
    PdfeMatrix getGlobalTransMatrix() const;

    /** Get the oriented rectangle which represents the group of words.
     * \param In page coordinates or local coordinates ?
     * \return PdfeORect object.
     */
    PdfeORect getOrientedRect( bool pageCoords = true ) const;

protected:
    /** Read a group of words from a PdfString (appended to the group).
     * \param str Pdf string to read (can contain 0 characters !).
     * \param pFont Font object used to compute words width.
     */
    void readPdfString( const PoDoFo::PdfString& str,
                        PoDoFoExtended::PdfeFont *pFont );

public:
    // Getters and setters...
    long pageIndex() const;
    void setPageIndex( long pageIndex );

    long groupIndex() const;
    void setGroupIndex( long groupIndex );

    PRTextLine* textLine() const;
    void setTextLine( PRTextLine* pTextLine );

    PdfeMatrix transMatrix() const;
    void setTransMatrix( const PdfeMatrix& transMatrix );

    PdfTextState textState() const;
    void setTextState( const PdfTextState& textState );

    const double* boundingBox() const;

    /** Get a constant reference to the vector of group of words.
     * \return Constant reference to an std::vector.
     */
    const std::vector<PRTextWord>& words() const;

protected:
    /// Default space height used.
    static const double SpaceHeight = 0.5;
    /// Minimal height for a character.
    static const double MinimalHeight = 0.2;
    /// Max char space allowed inside a word.
    static const double MaxWordCharSpace = 0.1;

protected:
    /// Index of the page to which belongs the group.
    long  m_pageIndex;
    /// Index of the group in the page.
    long  m_groupIndex;
    /// Line it belongs to (pointer).
    PRTextLine*  m_pTextLine;

    /// Vector of words that make the group.
    std::vector<PRTextWord>  m_words;

    /// Transformation matrix of the graphics state.
    PdfeMatrix  m_transMatrix;
    /// Text state for this group of words.
    PdfTextState  m_textState;

    /// Font bounding box.
    double  m_boundingBox[4];
};

//**********************************************************//
//                        PRTextLine                        //
//**********************************************************//
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

protected:
    /// Index of the page to which belongs the line.
    long  m_pageIndex;
    /// Index of the line in the page.
    long  m_lineIndex;

    /// Vector of pointers to groups of words which constitute the line (objects do not belong the line).
    std::vector<PRTextGroupWords*>  m_pGroupsWords;
};


//**********************************************************//
//                 Inline PRTextGroupWords                  //
//**********************************************************//
inline long PRTextGroupWords::pageIndex() const
{
    return m_pageIndex;
}
inline void PRTextGroupWords::setPageIndex( long pageIndex )
{
    m_pageIndex = pageIndex;
}
inline long PRTextGroupWords::groupIndex() const
{
    return m_groupIndex;
}
inline void PRTextGroupWords::setGroupIndex( long groupIndex )
{
    m_groupIndex = groupIndex;
}
inline PRTextLine* PRTextGroupWords::textLine() const
{
    return m_pTextLine;
}
inline void PRTextGroupWords::setTextLine( PRTextLine* pTextLine )
{
    m_pTextLine = pTextLine;
}
inline PdfeMatrix PRTextGroupWords::transMatrix() const
{
    return m_transMatrix;
}
inline void PRTextGroupWords::setTransMatrix( const PdfeMatrix& transMatrix )
{
    m_transMatrix = transMatrix;
}
inline PdfTextState PRTextGroupWords::textState() const
{
    return m_textState;
}
inline void PRTextGroupWords::setTextState( const PdfTextState& textState )
{
    m_textState = textState;
}
inline const double* PRTextGroupWords::boundingBox() const
{
    return m_boundingBox;
}
inline const std::vector<PRTextWord>& PRTextGroupWords::words() const
{
    return m_words;
}

//**********************************************************//
//                     Inline PRTextLine                    //
//**********************************************************//
inline std::vector<PRTextGroupWords*> PRTextLine::groupsWords() const
{
    return m_pGroupsWords;
}

}


#endif // PRTEXTELEMENTS_H
