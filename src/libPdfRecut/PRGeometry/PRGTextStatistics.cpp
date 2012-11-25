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

#include "PdfeUtils.h"

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
    m_variables.resize( PRGTextVariables::number(), NULL );
    for( size_t i = 0 ; i < m_variables.size() ; ++i ) {
        m_variables[i] = new PRGTextStatistics::Variable();
    }
}
void PRGTextStatistics::clear()
{
    // Delete variables.
    std::for_each( m_variables.begin(), m_variables.end(), delete_ptr_fctor<PRGTextStatistics::Variable>() );
    m_variables.clear();
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
double PRGTextStatistics::Variable::variance()
{
    if( m_nbMeasures <= 1 ) {
        return 0.;
    }
    // Unbiased estimator.
    return ( m_sumSquaredValues - m_sumValues ) / ( m_nbMeasures - 1 );
}
void PRGTextStatistics::Variable::addValue( double val )
{
    ++m_nbMeasures;
    m_sumValues += val;
    m_sumSquaredValues += ( val*val );
}

}
