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
PRTextWord::PRTextWord( const PdfeCIDString& cidstr, PRTextWordType::Enum type, PdfeFont* pFont )
{
    this->init( cidstr, type, pFont );
}
PRTextWord::PRTextWord( double spaceWidth, double spaceHeight, PRTextWordType::Enum type )
{
    this->init( spaceWidth, spaceHeight, type );
}

void PRTextWord::init()
{
    m_pdfString = PoDoFo::PdfString();
    m_cidString = PdfeCIDString();
    m_type = PRTextWordType::Unknown;

    m_advance = PdfeVector();
    m_bbox = PdfRect( 0,0,0,0 );
    m_charSpace = 0.0;
}
void PRTextWord::init( const PdfeCIDString& cidstr, PRTextWordType::Enum type, PdfeFont* pFont )
{
    this->init();

    // Copy members.
    m_cidString = cidstr;
    m_type = type;
    m_charSpace = pFont->charSpace();

    // Compute advance vector and bbox.
    m_advance = pFont->advance( cidstr );
    m_bbox = pFont->bbox( cidstr );

    // Check size for space words.
    if( type == PRTextWordType::Space ) {
        m_bbox.SetBottom( 0.0 );
        m_bbox.SetHeight( pFont->spaceHeight() );

        m_bbox.SetLeft( 0.0 );
        m_bbox.SetWidth( m_advance(0) );
    }
    else {
        // Check minimal height.
        double minHeight = pFont->spaceHeight() * MinimalHeightScale;
        if( m_bbox.GetHeight() < minHeight ) {
            m_bbox.SetBottom( m_bbox.GetBottom() - minHeight / 4 );
            m_bbox.SetHeight( m_bbox.GetHeight() + minHeight );
        }
    }
}
void PRTextWord::init( double spaceWidth, double spaceHeight, PRTextWordType::Enum type )
{
    this->init();

    // Set type, advance vector and bbox.
    m_type = type;
    m_advance = PdfeVector( spaceWidth, 0.0 );
    m_bbox = PdfRect( 0.0, 0.0, spaceWidth, spaceHeight );
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

    m_pFont = NULL;
    m_fontBBox = PdfRect( 0,0,0,0 );
    m_fontNormTransMatrix.init();

    m_transMatrix.init();
    m_textState.init();

    m_words.clear();
    m_mainSubgroups.clear();
}
void PRTextGroupWords::init( const PdfVariant& variant,
                             const PdfeMatrix& transMatrix,
                             const PdfeTextState& textState, PdfeFont* pFont )
{
    this->init();

    // Set transformation matrix and text state.
    m_transMatrix = transMatrix;
    m_textState = textState;

    // Read group of words from variant.
    this->readPdfVariant( variant, pFont );
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

    // Set transformation matrix and text state.
    m_transMatrix = gState.transMat;
    m_textState = gState.textState;

    // Get variant from string.
    PdfVariant variant;
    PdfTokenizer tokenizer( gOperands.back().c_str(), gOperands.back().length() );
    tokenizer.GetNextVariant( variant, NULL );

    // Get font metrics.
    PdfeFont* pFont = document->fontCache( gState.textState.fontRef );

    // Read group of words.
    this->readPdfVariant( variant, pFont );
}

void PRTextGroupWords::readPdfVariant( const PdfVariant& variant,
                                       PdfeFont* pFont )
{
    // Related font members
    m_pFont = pFont;
    m_fontBBox = pFont->fontBBox();

    // Font renormalization matrix.
    PdfeFont::Statistics stats = pFont->statistics( true );
    m_fontNormTransMatrix.init();
    m_fontNormTransMatrix(0,0) = 1 / stats.meanBBox.GetWidth();
    m_fontNormTransMatrix(1,1) = 1 / stats.meanBBox.GetHeight();

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
                // Add PDF translation space.
               m_words.push_back( PRTextWord( -array[i].GetReal() / 1000.0,
                                               pFont->spaceHeight(),
                                               PRTextWordType::PDFTranslation ) );
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

    //std::string bufstr = str.GetString();
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

    double maxCharSpace = pFont->statistics( true ).meanBBox.GetWidth() * MaxCharSpaceScale;

    // Set font parameters.
    if( charSpace > maxCharSpace ) {
        pFont->setCharSpace( 0.0 );         // Char spaces are replaced.
    }
    else {
        pFont->setCharSpace( charSpace );   // Keep char spaces.
    }
    pFont->setWordSpace( wordSpace );
    pFont->setFontSize( 1.0 );
    pFont->setHScale( 100. );

    // Read characters from the string.
    size_t idx = 0;
    while( idx < cidstr.length() ) {
        // Save the first character index.
        size_t idxFirst = idx;

        // Read space word: use SpaceHeight for the character height.
        if( pFont->isSpace( cidstr[idx] ) ) {

            // Read spaces (can be multiple...)
            while( idx < cidstr.length() && pFont->isSpace( cidstr[idx] ) ) {
                ++idx;
            }
            // Create associated word.
            m_words.push_back( PRTextWord( cidstr.substr( idxFirst, idx-idxFirst ),
                                           PRTextWordType::Space,
                                           pFont ) );
        }
        // Read classic word.
        else {
            // Create two words when char space is too large.
            if( charSpace > maxCharSpace ) {
                m_words.push_back( PRTextWord( cidstr.substr( idxFirst, 1 ),
                                               PRTextWordType::Classic,
                                               pFont ) );
                m_words.push_back( PRTextWord( charSpace,
                                               pFont->spaceHeight(),
                                               PRTextWordType::PDFTranslationCS ) );
                ++idx;
            }
            // Create classic word
            else {
                // Read word characters.
                while( idx < cidstr.length() && !pFont->isSpace( cidstr[idx] ) ) {
                    ++idx;
                }
                m_words.push_back( PRTextWord( cidstr.substr( idxFirst, idx-idxFirst ),
                                               PRTextWordType::Classic,
                                               pFont ) );
            }
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

size_t PRTextGroupWords::length( bool countSpaces ) const
{
    // Length of the complete subgroup.
    Subgroup globalSubgroup( *this );
    return globalSubgroup.length( countSpaces );
}

PdfeVector PRTextGroupWords::advance() const
{
    // Advance vector of the complete subgroup.
    Subgroup globalSubgroup( *this );
    return globalSubgroup.advance( true );
}
PdfeVector PRTextGroupWords::displacement() const
{
    PdfeVector displacement = this->advance();
    displacement(0) = displacement(0) * m_textState.fontSize * ( m_textState.hScale / 100. );
    displacement(1) = displacement(1) * m_textState.fontSize;
    return displacement;
}
PdfeORect PRTextGroupWords::bbox( PRTextWordCoordinates::Enum wordCoord,
                                  bool leadTrailSpaces,
                                  bool useBottomCoord ) const
{
    // Bounding box of the complete subgroup.
    Subgroup globalSubgroup( *this );
    return globalSubgroup.bbox( wordCoord, leadTrailSpaces, useBottomCoord );
}

double PRTextGroupWords::fontSize() const
{
    PdfeFont::Statistics stats = m_pFont->statistics( true );

    // Apply global transformation matrix on meanBBox.
    PdfeORect meanBBox( stats.meanBBox );
    meanBBox = this->getGlobalTransMatrix().map( meanBBox );

    // Estimation of the font size...
    return ( meanBBox.height() / stats.meanBBox.GetHeight() );
}

PdfeMatrix PRTextGroupWords::transMatrix( PRTextWordCoordinates::Enum startCoord,
                                          PRTextWordCoordinates::Enum endCoord )
{
    // Compute global rendering matrix (font -> page).
    PdfeMatrix tmpMat, globalTransMat;
    tmpMat(0,0) = m_textState.fontSize * ( m_textState.hScale / 100. );
    tmpMat(1,1) = m_textState.fontSize;
    tmpMat(2,1) = m_textState.rise;
    globalTransMat = tmpMat * m_textState.transMat * m_transMatrix;

    if( startCoord == PRTextWordCoordinates::Font &&
        endCoord == PRTextWordCoordinates::Page ) {
        return globalTransMat;
    }
    else if( startCoord == PRTextWordCoordinates::Font &&
             endCoord == PRTextWordCoordinates::FontNormalized ) {
        return m_fontNormTransMatrix;
    }
    else if( startCoord == PRTextWordCoordinates::Page &&
             endCoord == PRTextWordCoordinates::Font ) {
        return globalTransMat.inverse();
    }
    else if( startCoord == PRTextWordCoordinates::Page &&
             endCoord == PRTextWordCoordinates::FontNormalized ) {
        return ( globalTransMat.inverse() * m_fontNormTransMatrix );
    }
    else if( startCoord == PRTextWordCoordinates::FontNormalized &&
             endCoord == PRTextWordCoordinates::Font ) {
        return m_fontNormTransMatrix.inverse();
    }
    else if( startCoord == PRTextWordCoordinates::FontNormalized &&
             endCoord == PRTextWordCoordinates::Page ) {
        return ( m_fontNormTransMatrix.inverse() * globalTransMat );
    }
    // Identity in other cases.
    return PdfeMatrix();
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
        grp2BBox = grp1TransMat.map( subGrp2.bbox( PRTextWordCoordinates::Page, true, true ) );

        // Subgroups of the first one.
        for( size_t i = 0 ; i < this->nbMSubgroups() ; ++i ) {
            const Subgroup& subGrp1 = this->mSubgroup( i );
            grp1BBox = subGrp1.bbox( PRTextWordCoordinates::Font, true, true );

            dist = std::min( dist, PdfeORect::minDistance( grp1BBox, grp2BBox ) );
        }
    }
    return dist;
}

double PRTextGroupWords::maxDistance( const PRTextGroupWords& group ) const
{
    PdfeORect grp1BBox, grp2BBox;
    double dist = 0.0;

    // Inverse Transformation matrix of the first group.
    PdfeMatrix grp1TransMat = this->getGlobalTransMatrix().inverse();

    // Loop on subgroups of the second one.
    for( size_t j = 0 ; j < group.nbMSubgroups() ; ++j ) {
        // Subgroup bounding box.
        const Subgroup& subGrp2 = group.mSubgroup( j );
        grp2BBox = grp1TransMat.map( subGrp2.bbox( PRTextWordCoordinates::Page, true, true ) );

        // Subgroups of the first one.
        for( size_t i = 0 ; i < this->nbMSubgroups() ; ++i ) {
            const Subgroup& subGrp1 = this->mSubgroup( i );
            grp1BBox = subGrp1.bbox( PRTextWordCoordinates::Font, true, true );

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

bool PRTextGroupWords::isSpace() const
{
    // Advance vector of the complete subgroup.
    Subgroup globalSubgroup( *this );
    return globalSubgroup.isSpace();
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
    // Copy members.
    m_pGroup = subgroup.m_pGroup;
    m_wordsInside = subgroup.m_wordsInside;
    m_bboxCache = subgroup.m_bboxCache;
    m_isBBoxCache = subgroup.m_isBBoxCache;
}

void PRTextGroupWords::Subgroup::init()
{
    m_pGroup = NULL;
    m_wordsInside.clear();
    m_isBBoxCache = false;
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
    m_isBBoxCache = false;
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
PdfeVector PRTextGroupWords::Subgroup::advance( bool useGroupOrig ) const
{
    // Handle the case of an empty group.
    PdfeVector advance;
    if( !m_wordsInside.size() ) {
        return advance;
    }

    // First and last indexes to consider.
    long idxFirst = 0;
    if( !useGroupOrig ) {
        while( idxFirst < static_cast<long>( m_wordsInside.size() ) && !m_wordsInside[idxFirst] ) {
            ++idxFirst;
        }
    }
    long idxLast = static_cast<long>( m_wordsInside.size() ) - 1;
    while( idxLast >= 0L && !m_wordsInside[idxLast] ) {
        --idxLast;
    }
    // Check indexes. If wrong, return 0 advance vector
    if( idxFirst >= static_cast<long>( m_wordsInside.size() ) || idxLast < 0L || idxFirst > idxLast ) {
        return advance;
    }

    // Compute advance vector.
    for( long i = idxFirst ; i <= idxLast ; ++i ) {
        const PRTextWord& word = m_pGroup->word( i );
        advance += word.advance();
    }
    return advance;
}

PdfeORect PRTextGroupWords::Subgroup::bbox( PRTextWordCoordinates::Enum wordCoord, bool leadTrailSpaces, bool useBottomCoord ) const
{
    // Handle the case of an empty group.
    if( !m_wordsInside.size() ) {
        return PdfeORect( 0.0, 0.0 );
    }
    // BBox cache.
    if( m_isBBoxCache && leadTrailSpaces && useBottomCoord ) {
        PdfeORect bbox( m_bboxCache );
        // Apply transformation if needed.
        if( wordCoord != PRTextWordCoordinates::Font ) {
            PdfeMatrix transMat = m_pGroup->transMatrix( PRTextWordCoordinates::Font, wordCoord );
            bbox = transMat.map( bbox );
        }
        return bbox;
    }

    // Else: classic computation.
    PdfeORect bbox( 0.0, 0.0 );

    // First and last indexes of the subgroup.
    long idxFirst = 0;
    while( idxFirst < static_cast<long>( m_wordsInside.size() ) && !m_wordsInside[idxFirst] ) {
         ++idxFirst;
    }
    long idxLast = static_cast<long>( m_wordsInside.size() ) - 1;
    while( idxLast >= 0L && !m_wordsInside[idxLast] ) {
        --idxLast;
    }
    // Check indexes. If wrong, return empty bbox.
    if( idxFirst >= static_cast<long>( m_wordsInside.size() ) || idxLast < 0L || idxFirst > idxLast ) {
        return bbox;
    }
    // Bounding box coordinates.
    double left = std::numeric_limits<double>::max();
    double right = -std::numeric_limits<double>::max();
    double bottom = std::numeric_limits<double>::max();
    double top = -std::numeric_limits<double>::max();

    // Compute bounding box.
    PdfeVector advance;
    for( long i = 0 ; i < static_cast<long>( m_wordsInside.size() ) ; ++i ) {
        // Current word and its bounding box.
        const PRTextWord& word = m_pGroup->word( i );
        PdfRect wbbox = word.bbox( useBottomCoord );

        // If inside the subgroup, update coordinates (remove spaces if needed).
        if( this->inside( i ) && i >= idxFirst && i <= idxLast &&
                ( leadTrailSpaces || word.type() == PRTextWordType::Classic )  ) {
            left = std::min( left, advance(0) + wbbox.GetLeft() );
            bottom = std::min( bottom, advance(1) + wbbox.GetBottom() );
            right = std::max( right, advance(0) + wbbox.GetLeft() + wbbox.GetWidth() );
            top = std::max( top, advance(1) + wbbox.GetBottom() + wbbox.GetHeight() );
        }
        advance += word.advance();
    }
    // Strange coordinates, return empty bbox.
    if( left > right || bottom > top ) {
        return bbox;
    }
    // Set bounding box coordinates.
    bbox.setWidth( right - left );
    bbox.setLeftBottom( PdfeVector( left, bottom ) );
    bbox.setHeight( top - bottom );

    // Cache result
    if( !m_isBBoxCache && leadTrailSpaces && useBottomCoord ) {
        m_bboxCache = bbox;
        m_isBBoxCache = true;
    }

    // Apply transformation if needed.
    if( wordCoord != PRTextWordCoordinates::Font ) {
        PdfeMatrix transMat = m_pGroup->transMatrix( PRTextWordCoordinates::Font, wordCoord );
        bbox = transMat.map( bbox );
    }
    return bbox;
}

bool PRTextGroupWords::Subgroup::isSpace() const
{
    // Check words.
    for( size_t i = 0 ; i < this->m_wordsInside.size() ; ++i ) {
        if( m_wordsInside[i] && ( this->m_pGroup->word(i).type() == PRTextWordType::Classic ) ) {
            return false;
        }
    }
    return true;
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
