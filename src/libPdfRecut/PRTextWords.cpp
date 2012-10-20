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
#include "PRDocument.h"

#include "PdfeCanvasAnalysis.h"

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
PRTextGroupWords::PRTextGroupWords( const PdfVariant& variant,
                                    const PdfeMatrix& transMatrix,
                                    const PdfeTextState& textState,
                                    PdfeFont* pFont )
{
    this->init( variant, transMatrix, textState, pFont );
}

PRTextGroupWords::PRTextGroupWords( PRDocument* document,
                                    const PoDoFoExtended::PdfeStreamState& streamState )
{
    this->init( document, streamState );
}

void PRTextGroupWords::init()
{
    // Initialize members to default values.
    m_pageIndex = -1;
    m_groupIndex = -1;
    m_pTextLines.clear();

    m_transMatrix.init();
    m_textState.init();
    m_fontBBox = PdfRect( 0,0,0,0 );

    m_words.clear();
    m_mainSubgroups.clear();
}
void PRTextGroupWords::init( const PdfVariant& variant,
                             const PdfeMatrix& transMatrix,
                             const PdfeTextState& textState, PdfeFont* pFont )
{
    this->init();
    this->readPdfVariant( variant, transMatrix, textState, pFont );
}

void PRTextGroupWords::init( PRDocument* document, const PoDoFoExtended::PdfeStreamState& streamState )
{
    // Initialize to empty.
    this->init();

    // Check it is a text showing operator.
    if( streamState.gOperator.cat != ePdfGCategory_TextShowing ) {
        return;
    }

    // Simpler references.
    const std::vector<std::string>& gOperands = streamState.gOperands;
    const PdfeGraphicsState& gState = streamState.gStates.back();

    // Get variant from string.
    PdfVariant variant;
    PdfTokenizer tokenizer( gOperands.back().c_str(), gOperands.back().length() );
    tokenizer.GetNextVariant( variant, NULL );

    // Get font metrics.
    PdfeFont* pFont = document->fontCache( gState.textState.fontRef );

    // Read group of words.
    this->readPdfVariant( variant,
                          streamState.gStates.back().transMat,
                          streamState.gStates.back().textState,
                          pFont );

}

void PRTextGroupWords::readPdfVariant( const PdfVariant& variant,
                                       const PdfeMatrix& transMatrix,
                                       const PdfeTextState& textState,
                                       PdfeFont* pFont )
{
    // Set transformation matrix and textstate.
    m_transMatrix = transMatrix;
    m_textState = textState;

    // Get font bounding box.
    m_fontBBox = pFont->fontBBox();

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
                                     PdfRect( 0.0, 0.0, -array[i].GetReal() / 1000.0, pFont->spaceHeight() ),
                                     0.0 );

                m_words.push_back( textWord );
            }
        }
    }
    // Construct subgroups vector.
    this->buildMainSubGroups();
}
void PRTextGroupWords::readPdfString( const PoDoFo::PdfString& str,
                                      PoDoFoExtended::PdfeFont* pFont )
{
    // To CID string.
    PdfeCIDString cidstr = pFont->toCIDString( str );
    std::string bufstr = str.GetString();

    //if( pFont->type() ==  PdfeFontType::Type0 )
//    {
//        QString ustr = pFont->toUnicode( str );
//        qDebug() << pFont->type()
//                 << " : " << ustr << " / "
//                 << cidstr[0] << " / "
//                 << cidstr[4];
//    }


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
            top = pFont->spaceHeight();
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

void PRTextGroupWords::appendWord( const PRTextWord& word )
{
    // Append the word to the vector.
    m_words.push_back( word );

    // Construct subgroups vector.
    this->buildMainSubGroups();
}

double PRTextGroupWords::width( bool leadTrailSpaces ) const
{
    // Width of the complete subgroup.
    Subgroup globalSubgroup( *this );
    return globalSubgroup.width( leadTrailSpaces );
}
double PRTextGroupWords::height() const
{
    // Height of the complete subgroup.
    Subgroup globalSubgroup( *this );
    return globalSubgroup.height();
}

size_t PRTextGroupWords::length( bool countSpaces ) const
{
    // Length of the complete subgroup.
    Subgroup globalSubgroup( *this );
    return globalSubgroup.length( countSpaces );
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

PdfeORect PRTextGroupWords::bbox(bool pageCoords,
                                 bool leadTrailSpaces,
                                 bool useBottomCoord ) const
{
    // Bounding box of the complete subgroup.
    Subgroup globalSubgroup( *this );
    return globalSubgroup.bbox( pageCoords, leadTrailSpaces, useBottomCoord );
}
PdfeVector PRTextGroupWords::displacement() const
{
    PdfeVector textDispl;

    // Compute displacement. TODO: vertical component.
    textDispl(0) = this->width( true ) * m_textState.fontSize * ( m_textState.hScale / 100. );
    textDispl(1) = 0.0;

    return textDispl;
}

double PRTextGroupWords::minDistance( const PRTextGroupWords& group ) const
{
    PdfeORect grp1BBox, grp2BBox;
    double dist = std::numeric_limits<double>::max();

    // Inverse Transformation matrix of the first group.
    PdfeMatrix grp1TransMat = this->getGlobalTransMatrix().inverse();

    // Loop on subgroups of the second one.
    for( size_t j = 0 ; j < group.nbMSubgroups() ; ++j ) {
        // Subgroup bounding box.
        const Subgroup& subGrp2 = group.mSubgroup( j );
        grp2BBox = grp1TransMat.map( subGrp2.bbox( true, true, true ) );

        // Subgroups of the first one.
        for( size_t i = 0 ; i < this->nbMSubgroups() ; ++i ) {
            const Subgroup& subGrp1 = this->mSubgroup( i );
            grp1BBox = subGrp1.bbox( false, true, true );

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
    for( size_t j = 0 ; j < group.nbMSubgroups() ; ++j ) {
        // Subgroup bounding box.
        const Subgroup& subGrp2 = group.mSubgroup( j );
        grp2BBox = grp1TransMat.map( subGrp2.bbox( true, true, true ) );

        // Subgroups of the first one.
        for( size_t i = 0 ; i < this->nbMSubgroups() ; ++i ) {
            const Subgroup& subGrp1 = this->mSubgroup( i );
            grp1BBox = subGrp1.bbox( false, true, true );

            dist = std::max( dist, PdfeORect::maxDistance( grp1BBox, grp2BBox ) );
        }
    }
    return dist;
}
void PRTextGroupWords::addTextLine( PRTextLine* pLine )
{
    std::vector<PRTextLine*>::iterator it;
    it = std::find( m_pTextLines.begin(), m_pTextLines.end(), pLine );

    // Add if not found.
    if( it == m_pTextLines.end() ) {
        m_pTextLines.push_back( pLine );
    }
}
void PRTextGroupWords::rmTextLine( PRTextLine* pLine )
{
    std::vector<PRTextLine*>::iterator it;
    it = std::find( m_pTextLines.begin(), m_pTextLines.end(), pLine );

    // Remove if found.
    if( it != m_pTextLines.end() ) {
        m_pTextLines.erase( it );
    }
}

void PRTextGroupWords::buildMainSubGroups()
{
    m_mainSubgroups.clear();

    size_t idx = 0;
    while( idx < m_words.size() ) {
        // Remove PDF translation words.
        while( idx < m_words.size() &&
               ( m_words[idx].type() == PRTextWordType::PDFTranslation ||
                 m_words[idx].type() == PRTextWordType::PDFTranslationCS ) ) {
            idx++;
        }

        // Create a subgroup.
        if( idx < m_words.size() ) {
            Subgroup subgroup( *this, false );
            while( idx < m_words.size() &&
                   m_words[idx].type() != PRTextWordType::PDFTranslation &&
                   m_words[idx].type() != PRTextWordType::PDFTranslationCS ) {
                subgroup.setInside( idx, true );
                idx++;
            }
            m_mainSubgroups.push_back( subgroup );
        }
    }
}

//**********************************************************//
//                PRTextGroupWords::SubGroup                //
//**********************************************************//
PRTextGroupWords::Subgroup::Subgroup()
{
    // Initialize to empty subgroup.
    this->init();
}
PRTextGroupWords::Subgroup::Subgroup( const PRTextGroupWords& group , bool allGroup )
{
    // Initialize to complete subgroup.
    this->init( group, allGroup );
}
PRTextGroupWords::Subgroup::Subgroup( const PRTextGroupWords::Subgroup& subgroup )
{
    // Comy members.
    m_pGroup = subgroup.m_pGroup;
    m_wordsInside = subgroup.m_wordsInside;
}

void PRTextGroupWords::Subgroup::init()
{
    m_pGroup = NULL;
    m_wordsInside.clear();
}

void PRTextGroupWords::Subgroup::init( const PRTextGroupWords& group, bool allGroup )
{
    // Complete subgroup.
    m_pGroup = const_cast<PRTextGroupWords*>( &group );
    if( allGroup ) {
        m_wordsInside.assign( group.nbWords(), true );
    }
    else {
        m_wordsInside.assign( group.nbWords(), false );
    }
}

PdfeORect PRTextGroupWords::Subgroup::bbox( bool pageCoords, bool leadTrailSpaces, bool useBottomCoord ) const
{
    // Handle the case of an empty group.
    PdfeORect bbox( 0.0, 0.0 );
    if( !m_wordsInside.size() ) {
        return bbox;
    }

    // First and last indexes of the subgroup.
    long idxFirst = 0;
    while( idxFirst < static_cast<long>( m_wordsInside.size() ) && !m_wordsInside[idxFirst] ) {
         ++idxFirst;
    }
    long idxLast = static_cast<long>( m_wordsInside.size() ) - 1;
    while( idxLast >= 0L && !m_wordsInside[idxLast] ) {
        --idxLast;
    }

    // Check the indexes. If wrong, return empty bbox.
    if( idxFirst >= static_cast<long>( m_wordsInside.size() ) || idxLast < 0L || idxFirst > idxLast ) {
        return bbox;
    }
    // Bounding box coordinates.
    double left = std::numeric_limits<double>::max();
    double right = std::numeric_limits<double>::min();
    double bottom = std::numeric_limits<double>::max();
    double top = std::numeric_limits<double>::min();

    // Sum of words width.
    double widthSum = 0.0;

    // Compute bounding box.
    for( long i = 0 ; i < static_cast<long>( m_wordsInside.size() ) ; ++i ) {
        // Current word and its bounding box.
        const PRTextWord& word = m_pGroup->word( i );
        PdfRect bboxWord = word.bbox( true, useBottomCoord );

        // If inside the subgroup, update coordinates (remove spaces if needed).
        if( this->inside( i ) && i >= idxFirst && i <= idxLast &&
                ( leadTrailSpaces || word.type() == PRTextWordType::Classic )  ) {
            left = std::min( left, widthSum + bboxWord.GetLeft() );
            right = std::max( right, widthSum + bboxWord.GetLeft() + bboxWord.GetWidth() );
            bottom = std::min( bottom, bboxWord.GetBottom() );
            top = std::max( top, bboxWord.GetBottom() + bboxWord.GetHeight() );

        }
        widthSum += bboxWord.GetWidth();
    }

    // Strange coordinates, return empty bbox.
    if( left > right || bottom > top ) {
        return bbox;
    }

    // Set bounding box coordinates.
    bbox.setWidth( right - left );
    bbox.setLeftBottom( PdfeVector( left, bottom ) );
    bbox.setHeight( top - bottom );

    // Apply global transform if needed.
    if( pageCoords ) {
        PdfeMatrix textMat = m_pGroup->getGlobalTransMatrix();
        bbox = textMat.map( bbox );
    }
    return bbox;
}
double PRTextGroupWords::Subgroup::width( bool leadTrailSpaces ) const
{
    // Get bounding box.
    PdfeORect bbox = this->bbox( false, leadTrailSpaces, true );
    return bbox.width();
}
double PRTextGroupWords::Subgroup::height() const
{
    // Get bounding box.
    PdfeORect bbox = this->bbox( false, true, true );
    return bbox.height();
}
size_t PRTextGroupWords::Subgroup::length( bool countSpaces ) const
{
    size_t length = 0;
    for( size_t i = 0 ; i < m_wordsInside.size() ; ++i ) {
        // Current word.
        const PRTextWord& word = m_pGroup->word( i );
        if( m_wordsInside[i] &&
            ( word.type() == PRTextWordType::Classic ||
            ( word.type() == PRTextWordType::Space && countSpaces ) ) ) {
            length += word.length();
        }
    }
    return length;
}

PRTextGroupWords::Subgroup PRTextGroupWords::Subgroup::intersection( const PRTextGroupWords::Subgroup& subgroup1,
                                                                     const PRTextGroupWords::Subgroup& subgroup2 )
{
    Subgroup subgroup;
    // Check the parent group is the same.
    if( subgroup1.m_pGroup == subgroup2.m_pGroup ) {
        subgroup.m_pGroup = subgroup1.m_pGroup;
        subgroup.m_wordsInside.resize( subgroup1.m_wordsInside.size(), false );

        // Intersection...
        for( size_t i = 0 ; i < subgroup1.m_wordsInside.size() ; ++i ) {
            subgroup.m_wordsInside[i] = subgroup1.m_wordsInside[i] && subgroup2.m_wordsInside[i];
        }
    }
    return subgroup;
}
PRTextGroupWords::Subgroup PRTextGroupWords::Subgroup::reunion( const PRTextGroupWords::Subgroup& subgroup1,
                                                                const PRTextGroupWords::Subgroup& subgroup2 )
{
    Subgroup subgroup;
    // Check the parent group is the same.
    if( subgroup1.m_pGroup == subgroup2.m_pGroup ) {
        subgroup.m_pGroup = subgroup1.m_pGroup;
        subgroup.m_wordsInside.resize( subgroup1.m_wordsInside.size(), false );

        // Union...
        for( size_t i = 0 ; i < subgroup1.m_wordsInside.size() ; ++i ) {
            subgroup.m_wordsInside[i] = subgroup1.m_wordsInside[i] || subgroup2.m_wordsInside[i];
        }
    }
    return subgroup;
}

}
