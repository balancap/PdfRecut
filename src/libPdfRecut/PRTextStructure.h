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

#ifndef PRTEXTSTRUCTURE_H
#define PRTEXTSTRUCTURE_H

#include "PdfGraphicsState.h"
#include "PdfFontMetricsCache.h"

namespace PoDoFo {
    class PdfPage;
    class PdfRect;
    class PdfFontMetrics;
    class PdfString;
    class PdfVariant;
}

namespace PdfRecut {

/** Class that represents a group of words read from a pdf stream.
 */
class PRTextGroupWords
{
public:
    /** Structure that gathers information about a word.
     */
    struct Word
    {
        /** Length: number of characters.
         */
        int length;

        /** Width in word coordinate system.
         */
        double width;

        /** Height in word coordinate system.
         */
        double height;

        /** Word type :
         * 0: classic word.
         * 1: classic space.
         * 2: pdf space (c.f. TJ operator).
         */
        unsigned char type;

        /** Default constructor.
         */
        Word() : length(0), width(0), height(0), type(0) { }

        /** Constructor with parameters.
         */
        Word( int _length, double _width, double _height, unsigned char _type ) :
            length(_length), width(_width), height(_height), type(_type) { }
    };
public:
    /** Default constructor.
     */
    PRTextGroupWords();

    /** Initialize to an empty group of words.
     */
    void init();


    /** Read a group of words from a PdfVariant (appended to the group).
     * \param variant Pdf variant to read (can be string or array).
     * \param fontMetrics Font metrics object used to compute width of words.
     */
    void readPdfVariant( const PoDoFo::PdfVariant& variant,
                        PoDoFo::PdfFontMetrics* fontMetrics );

    /** Append a word to the group.
     * \param word Word to append.
     */
    void appendWord( const Word& word );

protected:
    /** Read a group of words from a PdfString (appended to the group).
     * \param str Pdf string to read.
     * \param charHeight Default height of characters.
     * \param fontMetrics Font metrics object used to compute width of words.
     */
    void readPdfString( const PoDoFo::PdfString& str,
                        double charHeight,
                        PoDoFo::PdfFontMetrics* fontMetrics );

public:
    /** Getters and setters...
     */
    long getPageIndex() const;
    void setPageIndex( long pageIndex );

    long getGroupIndex() const;
    void setGroupIndex( long groupIndex );

    PdfMatrix getTransMatrix() const;
    void setTransMatrix( const PdfMatrix& transMatrix );

    PdfTextState getTextState() const;
    void setTextState( const PdfTextState& textState );

    const std::vector<PRTextGroupWords::Word>& getWords() const;

protected:
    /** Index of the page to which belongs the group.
     */
    long m_pageIndex;
    /** Index of the group in the page.
     */
    long m_groupIndex;

    /** Vector of words that make the group.
     */
    std::vector<PRTextGroupWords::Word> m_words;

    /** Transformation matrix of the graphics state.
     */
    PdfMatrix m_transMatrix;
    /** Text state for this group of words.
     */
    PdfTextState m_textState;
};

//**********************************************************//
//                Inline getters and setters                //
//**********************************************************//
inline long PRTextGroupWords::getPageIndex() const
{
    return m_pageIndex;
}
inline void PRTextGroupWords::setPageIndex( long pageIndex )
{
    m_pageIndex = pageIndex;
}
inline long PRTextGroupWords::getGroupIndex() const
{
    return m_groupIndex;
}
inline void PRTextGroupWords::setGroupIndex( long groupIndex )
{
    m_groupIndex = groupIndex;
}
inline PdfMatrix PRTextGroupWords::getTransMatrix() const
{
    return m_transMatrix;
}
inline void PRTextGroupWords::setTransMatrix( const PdfMatrix& transMatrix )
{
    m_transMatrix = transMatrix;
}
inline PdfTextState PRTextGroupWords::getTextState() const
{
    return m_textState;
}
inline void PRTextGroupWords::setTextState( const PdfTextState& textState )
{
    m_textState = textState;
}
inline const std::vector<PRTextGroupWords::Word>& PRTextGroupWords::getWords() const
{
    return m_words;
}

/** Text structure... Still empty !
 */
class PRTextStructure
{
public:
    PRTextStructure();
};

}

#endif // PRTEXTSTRUCTURE_H
