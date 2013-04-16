/***************************************************************************
 * Copyright (C) Paul Balança - All Rights Reserved                        *
 *                                                                         *
 * NOTICE:  All information contained herein is, and remains               *
 * the property of Paul Balança. Dissemination of this information or      *
 * reproduction of this material is strictly forbidden unless prior        *
 * written permission is obtained from Paul Balança.                       *
 *                                                                         *
 * Written by Paul Balança <paul.balanca@gmail.com>, 2013                  *
 ***************************************************************************/

#include "PdfeGElement.h"
#include "PdfeGraphicsState.h"

namespace PoDoFoExtended {

PdfeGElement::PdfeGElement() :
    m_nodeID( NodeIDUndefined() ),
    m_pGState( NULL )
{
}
PdfeGElement::PdfeGElement( const PdfeGElement& rhs ) :
    m_nodeID( rhs.m_nodeID ),
    m_pGState( NULL )
{
    if( rhs.m_pGState ) {
        m_pGState = new PdfeGraphicsState( *rhs.m_pGState );
    }
}
PdfeGElement& PdfeGElement::operator=( const PdfeGElement& rhs )
{
    if( this != &rhs ) {
        m_nodeID = rhs.m_nodeID;
        if( rhs.m_pGState ) {
            m_pGState = new PdfeGraphicsState( *rhs.m_pGState );
        }
        else {
            m_pGState = NULL;
        }
    }
    return *this;
}
void PdfeGElement::init()
{
    m_nodeID = NodeIDUndefined();
    delete m_pGState;
    m_pGState = NULL;
}
PdfeGElement::~PdfeGElement()
{
    delete m_pGState;
}

bool PdfeGElement::hasGState() const
{
    return m_pGState;
}
const PdfeGraphicsState& PdfeGElement::gstate() const
{
    if( !m_pGState ) {
        m_pGState = new PdfeGraphicsState();
    }
    return (*m_pGState);
}
void PdfeGElement::setGState( const PdfeGraphicsState& gstate )
{
    if( !m_pGState ) {
        m_pGState = new PdfeGraphicsState();
    }
    m_pGState->operator=( gstate );
}
void PdfeGElement::clearGState()
{
    delete m_pGState;
    m_pGState = NULL;
}
void PdfeGElement::clearTextGState()
{
    if( m_pGState ) {
        m_pGState->clearTextState();
    }
}

}
