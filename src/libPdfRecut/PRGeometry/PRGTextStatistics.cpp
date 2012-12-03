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

#include "PRGTextStatistics.h"
#include "PRGTextWords.h"

#include "PdfeUtils.h"
#include "PdfeFont.h"

#include <algorithm>

using namespace PoDoFoExtended;

namespace PdfRecut {

//**********************************************************//
//                     PRGTextStatistics                    //
//**********************************************************//
PRGTextStatistics::PRGTextStatistics()
{
    this->init();
}
PRGTextStatistics::~PRGTextStatistics()
{
    this->clear();
}
void PRGTextStatistics::init()
{
    this->clear();
    // Create variable vector.
    m_pVariables.resize( PRGTextVariables::number(), NULL );
    for( size_t i = 0 ; i < m_pVariables.size() ; ++i ) {
        m_pVariables[i] = new PRGTextStatistics::Variable();
    }
}
void PRGTextStatistics::clear()
{
    // Delete variables.
    std::for_each( m_pVariables.begin(), m_pVariables.end(), delete_ptr_fctor<PRGTextStatistics::Variable>() );
    m_pVariables.clear();
}

void PRGTextStatistics::addGroupWords( const PRGTextGroupWords& group )
{
    // Group transformation matrix and font.
    PdfeMatrix transMat = group.transMatrix( PRGTextWordCoordinates::Word,
                                             PRGTextWordCoordinates::Page );
    PdfeFont* pFont = group.font();
    for( size_t i = 0 ; i < group.nbWords() ; ++i ) {
        // Word to study.
        const PRGTextWord& word = group.word( i );
        if( word.type() == PRGTextWordType::Classic ||
                word.type() == PRGTextWordType::Space ) {

            for( size_t j = 0 ; j < word.cidString().length() ; ++j ) {
                pdfe_cid c = word.cidString()[j];
                QString utfc = pFont->toUnicode( c );
                // Character bounding box.
                PdfeORect bbox( pFont->bbox( c, false ) );
                bbox = transMat.map( bbox );

                // Add character statistics.
                m_pVariables[ PRGTextVariables::CharAllWidth ]->addValue( bbox.width() );
                m_pVariables[ PRGTextVariables::CharAllHeight ]->addValue( bbox.height() );
                // Letters and numbers characters.
                if( utfc.length() == 1 && utfc[0].isLetterOrNumber() ) {
                    m_pVariables[ PRGTextVariables::CharLNWidth]->addValue( bbox.width() );
                    m_pVariables[ PRGTextVariables::CharLNHeight ]->addValue( bbox.height() );
                }
            }
        }
    }
}

//**********************************************************//
//                PRGTextStatistics::Variable               //
//**********************************************************//
PRGTextStatistics::Variable::Variable()
{
    this->init();
}
void PRGTextStatistics::Variable::init()
{
    m_nbMeasures = 0;
    m_sumValues = 0;
    m_sumSquaredValues = 0;
}
double PRGTextStatistics::Variable::mean() const
{
    if( !m_nbMeasures ) {
        return 0.;
    }
    // Classic estimator.
    return ( m_sumValues / m_nbMeasures );
}
double PRGTextStatistics::Variable::variance() const
{
    if( m_nbMeasures <= 1 ) {
        return 0.;
    }
    // Unbiased estimator.
    return ( m_sumSquaredValues - m_sumValues ) / ( m_nbMeasures - 1 );
}
size_t PRGTextStatistics::Variable::size() const
{
    return m_nbMeasures;
}
void PRGTextStatistics::Variable::addValue( double val )
{
    ++m_nbMeasures;
    m_sumValues += val;
    m_sumSquaredValues += ( val*val );
}

}
