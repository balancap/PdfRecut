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

#include "PRTextWords.h"

#include <podofo/podofo.h>
#include <limits>

using namespace PoDoFo;
using namespace PoDoFoExtended;

namespace PdfRecut {

//**********************************************************//
//                         PRTextWord                       //
//**********************************************************//
PRTextWord::PRTextWord()
{
    this->init();
}
PRTextWord::PRTextWord( PRTextWordType::Enum type,
                        long length,
                        const PdfRect& bbox,
                        double lastCharSpace ):
    m_type(type), m_length(length), m_bbox(bbox), m_lastCharSpace(lastCharSpace)
{
}
void PRTextWord::init()
{
    m_length = 0;
    m_bbox = PdfRect(0,0,0,0);
    m_lastCharSpace = 0;
    m_type = PRTextWordType::Unknown;
}

PdfRect PRTextWord::bbox( bool lastSpace,
                          bool useBottomCoord ) const
{
    PdfRect bbox = m_bbox;
    // Remove last character space.
    if( !lastSpace ) {
        bbox.SetWidth( bbox.GetWidth() - m_lastCharSpace );
    }
    // Set bottom to zero.
    if( !useBottomCoord ) {
        bbox.SetHeight( bbox.GetHeight() + bbox.GetBottom() );
        bbox.SetBottom( 0.0 );
    }
    return bbox;
}
double PRTextWord::width( bool lastSpace ) const
{
    double width  = m_bbox.GetWidth();
    if( !lastSpace ) {
        width -= m_lastCharSpace;
    }
    return width;
}
void PRTextWord::setBBox( const PdfRect& bbox )
{
    m_bbox = bbox;
}

//**********************************************************//
//                     PRTextGroupWords                     //
//**********************************************************//
PRTextGroupWords::PRTextGroupWords()
{
    this->init();
}
void PRTextGroupWords::init()
{
    // Initialize members to default values.
    m_pageIndex = -1;
    m_groupIndex = -1;
    m_pTextLine = NULL;

    m_transMatrix.init();
    m_textState.init();
    m_fontBBox = PdfRect( 0,0,0,0 );

    m_words.clear();
    m_subGroups.clear();
}

void PRTextGroupWords::readPdfVariant( const PdfVariant& variant,
                                       const PdfeMatrix& transMatrix,
                                       const PdfTextState& textState,
                                       PdfeFont *pFont )
{
    // Set transformation matrix and textstate.
    m_transMatrix = transMatrix;
    m_textState = textState;

    // Get font bounding box.
    PdfArray bbox = pFont->fontBBox();
    m_fontBBox.SetLeft( bbox[0].GetReal() / 1000. );
    m_fontBBox.SetBottom( bbox[1].GetReal() / 1000. );
    m_fontBBox.SetWidth( ( bbox[2].GetReal() - bbox[0].GetReal() ) / 1000. );
    m_fontBBox.SetHeight( ( bbox[3].GetReal() - bbox[1].GetReal() ) / 1000. );

    // Variant is a string.
    if( variant.IsString() || variant.IsHexString() ) {
        this->readPdfString( variant.GetString(), pFont );
    }
    // Variant is an array (c.f. operator TJ).
    else if( variant.IsArray() ) {
        const PdfArray& array = variant.GetArray();
        for( size_t i = 0 ; i < array.size() ; i++ ) {
            if( array[i].IsString() || array[i].IsHexString() ) {
                // Read string.
                this->readPdfString( array[i].GetString(), pFont );
            }
            else if( array[i].IsReal() || array[i].IsNumber() ) {
                // Read pdf space.
                PRTextWord textWord( PRTextWordType::PDFTranslation,
                                     1,
                                     PdfRect( 0.0, 0.0, -array[i].GetReal() / 1000.0, this->SpaceHeight ),
                                     0.0 );

                m_words.push_back( textWord );
            }
        }
    }
    // Construct subgroups vector.
    this->buildSubGroups();
}

void PRTextGroupWords::appendWord( const PRTextWord& word )
{
    // Append the word to the vector.
    m_words.push_back( word );

    // Construct subgroups vector.
    this->buildSubGroups();
}

double PRTextGroupWords::width( long idxSubGroup,
                                bool countSpaces,
                                bool lastCharSpace ) const
{
    double width = 0;
    long idxFirst = 0;
    long idxLast = static_cast<long>( m_words.size() ) - 1;

    // Consider a specific subgroup.
    if( idxSubGroup >= 0 && idxSubGroup < static_cast<long>( m_subGroups.size() ) ) {
        idxFirst = m_subGroups[ idxSubGroup ].idxFirstWord;
        idxLast = m_subGroups[ idxSubGroup ].idxLastWord;
    }

    // Compute group width.
    for( long i = idxFirst ; i <= idxLast ; ++i ) {
        // Add word width ?
        if( countSpaces || m_words[i].type() == PRTextWordType::Classic ) {
            // Last character space of last word.
            if( i == idxLast ) {
                width += m_words[i].width( lastCharSpace );
            }
            else {
                width += m_words[i].width( true );
            }
        }
    }
    return width;
}
double PRTextGroupWords::height( long idxSubGroup ) const
{
    double height = 0;
    long idxFirst = 0;
    long idxLast = static_cast<long>( m_words.size() ) - 1;

    // Consider a specific subgroup.
    if( idxSubGroup >= 0 && idxSubGroup < static_cast<long>( m_subGroups.size() ) ) {
        idxFirst = m_subGroups[ idxSubGroup ].idxFirstWord;
        idxLast = m_subGroups[ idxSubGroup ].idxLastWord;
    }
    // Compute the height.
    for( long i = idxFirst ; i <= idxLast ; ++i ) {
        height = std::max( m_words[i].bbox().GetHeight(), height );
    }
    return height;
}

size_t PRTextGroupWords::length( long idxSubGroup, bool countSpaces )
{
    size_t length = 0;
    long idxFirst = 0;
    long idxLast = static_cast<long>( m_words.size() ) - 1;

    // Consider a specific subgroup.
    if( idxSubGroup >= 0 && idxSubGroup < static_cast<long>( m_subGroups.size() ) ) {
        idxFirst = m_subGroups[ idxSubGroup ].idxFirstWord;
        idxLast = m_subGroups[ idxSubGroup ].idxLastWord;
    }

    // Compute length.
    for( long i = idxFirst ; i <= idxLast ; ++i ) {
        if( m_words[i].type() == PRTextWordType::Classic ||
            ( m_words[i].type() == PRTextWordType::Space && countSpaces ) ) {
            length += m_words[i].length();
        }
    }
    return length;
}

PdfeMatrix PRTextGroupWords::getGlobalTransMatrix() const
{
    // Compute text rendering matrix.
    PdfeMatrix tmpMat, textMat;
    tmpMat(0,0) = m_textState.fontSize * ( m_textState.hScale / 100. );
    //tmpMat(1,1) = m_textState.fontSize * m_words.back().height;
    tmpMat(1,1) = m_textState.fontSize;
    tmpMat(2,1) = m_textState.rise;
    textMat = tmpMat * m_textState.transMat * m_transMatrix;

    return textMat;
}

PdfeORect PRTextGroupWords::bbox( long idxSubGroup,
                                  bool pageCoords,
                                  bool leadTrailSpaces,
                                  bool useBottomCoord ) const
{
    // Handle the case of an empty group.
    PdfeORect bbox( 0.0, 0.0 );
    if( !m_words.size() ) {
        return bbox;
    }

    // First and last indexes (limits).
    long idxFirst = 0;
    long idxLast = static_cast<long>( m_words.size() ) - 1;

    // Subgroup.
    if( idxSubGroup >= 0 && idxSubGroup < static_cast<long>( m_subGroups.size() ) ) {
        idxFirst = m_subGroups[ idxSubGroup ].idxFirstWord;
        idxLast = m_subGroups[ idxSubGroup ].idxLastWord;
    }
    // Remove leading and trailing spaces.
    else if( !leadTrailSpaces ) {
        // Leading spaces.
        while( idxFirst < static_cast<long>( m_words.size() ) &&
               m_words[idxFirst].type() != PRTextWordType::Classic ) {
            ++idxFirst;
        }
        // Trailing spaces.
        while( idxLast >= 0L &&
               m_words[idxLast].type() != PRTextWordType::Classic ) {
            --idxLast;
        }
    }

    // Check that the indexes. If wrong, return empty bbox.
    if( idxFirst >= static_cast<long>( m_words.size() ) || idxLast < 0L || idxFirst > idxLast ) {
        return bbox;
    }

    // Compute width of words before the current group.
    double leftWidth = 0.0;
    for( long i = 0 ; i < idxFirst ; ++i ) {
        leftWidth += m_words[i].width( true );
    }

    // Compute width and vertical coordinates.
    double width = 0.0;
    double bottom = std::numeric_limits<double>::max();
    double top = std::numeric_limits<double>::min();

    for( long i = idxFirst ; i <= idxLast ; ++i ) {
        PdfRect bboxWord = m_words[i].bbox( true, useBottomCoord );

        width += bboxWord.GetWidth();
        bottom = std::min( bottom, bboxWord.GetBottom() );
        top = std::max( top, bboxWord.GetBottom() + bboxWord.GetHeight() );
    }
    bbox.setWidth( std::max( 0.0, width ) );
    bbox.setLeftBottom( PdfeVector( leftWidth, bottom ) );
    bbox.setHeight( std::max( 0.0, top - bottom ) );

    // Apply global transform if needed.
    if( pageCoords ) {
        PdfeMatrix textMat = this->getGlobalTransMatrix();
        bbox = textMat.map( bbox );
    }
    return bbox;
}

double PRTextGroupWords::minDistance( const PRTextGroupWords& group ) const
{
    PdfeORect grp1BBox, grp2BBox;
    double dist = std::numeric_limits<double>::max();

    // Inverse Transformation matrix of the first group.
    PdfeMatrix grp1TransMat = this->getGlobalTransMatrix().inverse();

    // Loop on subgroups of the second one.
    for( size_t j = 0 ; j < group.nbSubGroups() ; ++j ) {
        // Subgroup bounding box.
        grp2BBox = grp1TransMat.map( group.bbox( j, true, true, true ) );

        // Subgroups of the first one.
        for( size_t i = 0 ; i < this->nbSubGroups() ; ++i ) {
            grp1BBox = this->bbox( i, false, true, true );

            dist = std::min( dist, PdfeORect::minDistance( grp1BBox, grp2BBox ) );
        }
    }
    return dist;
}

double PRTextGroupWords::maxDistance(const PRTextGroupWords &group) const
{
    PdfeORect grp1BBox, grp2BBox;
    double dist = 0.0;

    // Inverse Transformation matrix of the first group.
    PdfeMatrix grp1TransMat = this->getGlobalTransMatrix().inverse();

    // Loop on subgroups of the second one.
    for( size_t j = 0 ; j < group.nbSubGroups() ; ++j ) {
        // Subgroup bounding box.
        grp2BBox = grp1TransMat.map( group.bbox( j, true, true, true ) );

        // Subgroups of the first one.
        for( size_t i = 0 ; i < this->nbSubGroups() ; ++i ) {
            grp1BBox = this->bbox( i, false, true, true );

            dist = std::max( dist, PdfeORect::maxDistance( grp1BBox, grp2BBox ) );
        }
    }
    return dist;
}

void PRTextGroupWords::readPdfString( const PoDoFo::PdfString& str,
                                      PoDoFoExtended::PdfeFont* pFont )
{
    // To CID string.
    PdfeCIDString cidstr = pFont->toCIDString( str );
    std::string bufstr = str.GetString();

//    if( bufstr.find( "Here all" ) != std::string::npos ) {
//        std::cout << bufstr << std::endl;
//        qDebug() << pFont->toUnicode( str );
//    }
//    qDebug() << pFont->toUnicode( str );

    // Text parameters.
    double fontSize = m_textState.fontSize;
    double charSpace = m_textState.charSpace / fontSize;
    double wordSpace = m_textState.wordSpace / fontSize;

    // Set font parameters (not necessary in theory).
    pFont->setCharSpace( 0.0 );
    pFont->setFontSize( 1.0 );
    pFont->setHScale( 100. );

    PdfRect cbbox;
    //double cwidth = 1.0;

    // Read characters from the string.
    size_t i = 0;
    while( i < cidstr.length() ) {

        // Length and bbox of the next word.
        PRTextWordType::Enum type;
        long length = 0;
        double width = 0.0;
        double bottom = std::numeric_limits<double>::max();
        double top = std::numeric_limits<double>::min();

        // Read space word: use SpaceHeight for the character height.
        if( pFont->isSpace( cidstr[i] ) ) {

            // Read spaces (can be multiple...)
            while( i < cidstr.length() && pFont->isSpace( cidstr[i] ) )
            {
                // Get bounding box of the character.
                cbbox = pFont->bbox( cidstr[i], false );
                //cwidth = pFont->width( cidstr[i], false );

                // Word space to add.
                width += cbbox.GetWidth();
                if( pFont->isSpace( cidstr[i] ) == PdfeFontSpace::Code32 ) {
                    width += wordSpace;
                }

                length++;
                i++;

                // Char space too large: divide the word and replace with pdf translation.
                if( charSpace > MaxWordCharSpace ) {
                    break;
                }
                else {
                    width += charSpace;
                }
            }
            bottom = 0.0;
            top = this->SpaceHeight;
            type = PRTextWordType::Space;
        }
        // Read classic word.
        else {
            // Read word characters.
            while( i < cidstr.length() && !pFont->isSpace( cidstr[i] ) )
            {
                // Get bounding box of the character.
                cbbox = pFont->bbox( cidstr[i], false );
                // cwidth = pFont->width( cidstr[i], false );

                width += cbbox.GetWidth();

                // Update bottom and top.
                bottom = std::min( bottom, cbbox.GetBottom() );
                top = std::max( top, cbbox.GetBottom() + cbbox.GetHeight() );

                length++;
                i++;

                // Char space too large: divide the word and replace with pdf translation.
                if( charSpace > MaxWordCharSpace ) {
                    break;
                }
                else {
                    width += charSpace;
                }
            }
            // Minimal height: add half for bottom and top.
            if( top-bottom <= this->MinimalHeight ) {
                top += this->MinimalHeight / 2;
                bottom -= this->MinimalHeight / 2;
            }
            type = PRTextWordType::Classic;
        }
        // Add word to the collection !
        m_words.push_back( PRTextWord( type,
                                       length,
                                       PdfRect( 0.0, bottom, width, top-bottom ),
                                       charSpace ) );

        // Char space too large: replace with pdf translation.
        if( charSpace > MaxWordCharSpace ) {
            // Reset last char space to zero.
            m_words.back().setLastCharSpace( 0.0 );

            // Push back PDF translation word.
            m_words.push_back( PRTextWord( PRTextWordType::PDFTranslationCS,
                                           1,
                                           PdfRect( 0.0, 0.0, charSpace, this->SpaceHeight ),
                                           0.0 ) );
        }
    }
}

void PRTextGroupWords::buildSubGroups()
{
    m_subGroups.clear();

    size_t idx = 0;
    while( idx < m_words.size() ) {
        // Remove PDF translation words.
        while( idx < m_words.size() &&
               ( m_words[idx].type() == PRTextWordType::PDFTranslation ||
                 m_words[idx].type() == PRTextWordType::PDFTranslationCS ) ) {
            idx++;
        }

        // Words in the subgroup.
        SubGroup subgroup;
        subgroup.idxFirstWord = idx;
        while( idx < m_words.size() &&
               m_words[idx].type() != PRTextWordType::PDFTranslation &&
               m_words[idx].type() != PRTextWordType::PDFTranslationCS ) {
            idx++;
        }
        subgroup.idxLastWord = idx-1;

        // Add subgroup to the list.
        if( subgroup.idxFirstWord != static_cast<long>( m_words.size() ) ) {
            m_subGroups.push_back( subgroup );
        }
    }
}

}
