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

#include "PdfeContentsStream.h"

using namespace PoDoFo;

namespace PoDoFoExtended {

//**********************************************************//
//                     PdfeContentsStream                   //
//**********************************************************//
PdfeContentsStream::PdfeContentsStream() :
    m_pFirstNode( NULL ), m_pLastNode( NULL ),
    m_nbNodes( 0 ), m_maxNodeID( 0 ),
    m_initialGState(), m_initialResources()
{
}
void PdfeContentsStream::init()
{
    // Clear contents nodes.
    this->rmNodes();

    m_pFirstNode = NULL;
    m_pLastNode = NULL;
    m_nbNodes = 0;
    m_maxNodeID = 0;
    m_initialGState.init();
    m_initialResources.init();
}
PdfeContentsStream::PdfeContentsStream( const PdfeContentsStream& rhs ) :
    m_pFirstNode( NULL ), m_pLastNode( NULL ),
    m_nbNodes( rhs.m_nbNodes ),
    m_maxNodeID( rhs.m_maxNodeID ),
    m_initialGState( rhs.m_initialGState ),
    m_initialResources( rhs.m_initialResources )
{
    // Copy nodes.
    this->copyNodes( rhs );
}
PdfeContentsStream& PdfeContentsStream::operator=( const PdfeContentsStream& rhs )
{
    // Clear existing contents nodes.
    this->rmNodes();
    // Copy members and nodes.
    m_nbNodes = rhs.m_nbNodes;
    m_maxNodeID = rhs.m_maxNodeID;
    m_initialGState = rhs.m_initialGState;
    m_initialResources = rhs.m_initialResources;
    this->copyNodes( rhs );

    return *this;
}
PdfeContentsStream::~PdfeContentsStream()
{
    // Clear contents nodes.
    this->rmNodes();
}

void PdfeContentsStream::copyNodes( const PdfeContentsStream& stream )
{
    m_pFirstNode = m_pLastNode = NULL;
    m_nbNodes = stream.m_nbNodes;
    m_maxNodeID = stream.m_maxNodeID;
    if( stream.m_pFirstNode ) {
        // Nodes map, for specific links.
        std::map<Node*,Node*> nodesOpening;
        std::map<Node*,std::vector<Node*> > nodesPath;

        // First node.
        Node* pNodePrev = NULL;
        Node* pNodeNext = NULL;
        Node* pNodeOut = stream.m_pFirstNode;
        // Deep copy of nodes.
        while( pNodeOut ) {
            pNodeNext = new Node( *pNodeOut );
            if( pNodePrev ) {
                pNodePrev->setNext( pNodeNext );
                pNodeNext->setPrev( pNodePrev );
            }
            if( !m_pFirstNode ) {
                m_pFirstNode = pNodeNext;
            }

            // Opening/closing nodes.
            if( pNodeOut->isOpeningNode() ) {
                nodesOpening.at( pNodeOut ) = pNodeNext;
            }
            else if( pNodeOut->isClosingNode() ) {
                std::map<Node*,Node*>::iterator it =
                        nodesOpening.find( pNodeOut->openingNode() );
                if( it != nodesOpening.end() ) {
                    pNodeNext->setOpeningNode( it->second );
                    it->second->setClosingNode( pNodeNext );
                    nodesOpening.erase( it );
                }
            }
            // Path construction/painting nodes.
            else if( pNodeOut->category() == PdfeGCategory::PathConstruction ) {
                if( pNodeOut->paintingNode() ) {
                    nodesPath.at( pNodeOut->paintingNode() ).push_back( pNodeNext );
                }
            }
            else if( pNodeOut->category() == PdfeGCategory::PathPainting ) {
                std::map<Node*,std::vector<Node*> >::iterator it =
                        nodesPath.find( pNodeOut );
                if( it != nodesPath.end() ) {
                    std::vector<Node*>& pnodes = it->second;
                    for( size_t i = 0 ; i < pnodes.size() ; ++i ) {
                        pnodes[i]->setPaintingNode( pNodeNext );
                    }
                    nodesPath.erase( it );
                }
            }
            // Update nodes pointers.
            pNodePrev = pNodeNext;
            pNodeOut = pNodeOut->next();
        }
        m_pLastNode = pNodeNext;
    }
}
void PdfeContentsStream::rmNodes()
{
    // Remove every node in the chain.
    Node* pNode = m_pFirstNode;
    Node* pNodeNext;
    while( pNode ) {
        pNodeNext = pNode->next();
        delete pNode;
        pNode = pNodeNext;
    }
    m_pFirstNode = NULL;
    m_pLastNode = NULL;
    m_nbNodes = 0;
    m_maxNodeID = 0;
}

//**********************************************************//
//                  PdfeContentsStream::Node                //
//**********************************************************//
PdfeContentsStream::Node::Node() :
    m_pPrevNode( NULL ), m_pNextNode( NULL ), m_pOpeningNode( NULL )
{
}
void PdfeContentsStream::Node::init()
{
    m_pPrevNode = m_pNextNode = m_pOpeningNode = NULL;
    m_goperator.init();
    m_goperands.clear();
}
PdfeContentsStream::Node::Node( const PdfeContentsStream::Node& rhs ) :
    m_nodeID( rhs.m_nodeID ),
    m_pPrevNode( NULL ), m_pNextNode( NULL ),
    m_goperator( rhs.m_goperator ), m_goperands( rhs.m_goperands )
{
    if( m_goperator.code == PdfeGOperator::Do ) {
        m_pResourcesObj = rhs.m_pResourcesObj;
    }
    else {
        m_pResourcesObj = NULL;
    }
}
PdfeContentsStream::Node& PdfeContentsStream::Node::operator=( const PdfeContentsStream::Node& rhs )
{
    m_nodeID = rhs.m_nodeID;
    m_pPrevNode = m_pNextNode = NULL;
    m_goperator = rhs.m_goperator;
    m_goperands = rhs.m_goperands;
    if( m_goperator.code == PdfeGOperator::Do ) {
        m_pResourcesObj = rhs.m_pResourcesObj;
    }
    else {
        m_pResourcesObj = NULL;
    }
    return *this;
}

PdfeContentsStream::Node::~Node()
{
    // None...
}

void PdfeContentsStream::Node::clear()
{
    m_goperator.init();
    m_goperands.clear();
    m_pOpeningNode = NULL;
}

// Getters...
PdfeContentsStream::Node* PdfeContentsStream::Node::openingNode() const
{
    if( this->closingNode() ) {
        return m_pOpeningNode;
    }
    return NULL;
}
PdfeContentsStream::Node* PdfeContentsStream::Node::closingNode() const
{
    if( this->openingNode() ) {
        return m_pClosingNode;
    }
    return NULL;
}
PdfeContentsStream::Node* PdfeContentsStream::Node::paintingNode() const
{
    if( m_goperator.cat == PdfeGCategory::PathConstruction ) {
        return m_pPaintingNode;
    }
    return NULL;
}
PdfObject* PdfeContentsStream::Node::resources() const
{
    if( m_goperator.code == PdfeGOperator::Do ) {
        return m_pResourcesObj;
    }
    return NULL;
}

// Setters...
void PdfeContentsStream::Node::setID( pdfe_nodeid nodeid )
{
    m_nodeID = nodeid;
}
void PdfeContentsStream::Node::setPrev( PdfeContentsStream::Node* pnode )
{
    m_pPrevNode = pnode;
}
void PdfeContentsStream::Node::setNext( PdfeContentsStream::Node* pnode )
{
    m_pNextNode = pnode;
}
void PdfeContentsStream::Node::setOperator( const PdfeGraphicOperator& rhs )
{
    m_goperator = rhs;
}
void PdfeContentsStream::Node::setOperands( const std::vector<std::string>& rhs )
{
    m_goperands = rhs;
}
void PdfeContentsStream::Node::setOpeningNode( PdfeContentsStream::Node* pnode )
{
    if( this->isClosingNode() ) {
        m_pOpeningNode = pnode;
    }
}
void PdfeContentsStream::Node::setClosingNode( PdfeContentsStream::Node* pnode )
{
    if( this->isOpeningNode() ) {
        m_pClosingNode = pnode;
    }
}
void PdfeContentsStream::Node::setPaintingNode( PdfeContentsStream::Node* pnode )
{
    if( m_goperator.cat == PdfeGCategory::PathConstruction ) {
        m_pPaintingNode = pnode;
    }
}
void PdfeContentsStream::Node::setResources( PdfObject* presources )
{
    if( m_goperator.code == PdfeGOperator::Do ) {
        m_pResourcesObj = presources;
    }
}

}
