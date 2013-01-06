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
#include "PdfeStreamTokenizer.h"

#include <QsLog/QsLog.h>
#include <podofo/podofo.h>

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
    this->deleteNodes();

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
    this->deleteNodes();
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
    this->deleteNodes();
}

PdfeContentsStream::Node* PdfeContentsStream::find( pdfe_nodeid nodeid ) const
{
    if( nodeid >= m_maxNodeID ) {
        return NULL;
    }
    // Look in the stream...
    Node* pnode = m_pFirstNode;
    while( pnode ) {
        if( pnode->id() == nodeid ) {
            return pnode;
        }
        pnode = pnode->next();
    }
    return NULL;
}
PdfeContentsStream::Node* PdfeContentsStream::insert( const PdfeContentsStream::Node& node,
                                                      PdfeContentsStream::Node* pNodePrev )
{
    // Copy the input node and set ID.
    Node* pNode = new Node( node );
    pNode->setID( m_maxNodeID );
    ++m_maxNodeID;
    ++m_nbNodes;

    Node* pNodeNext;
    // Case of the first node...
    if( !pNodePrev ) {
        pNodeNext = m_pFirstNode;
        m_pFirstNode = pNode;
    }
    else {
        pNodeNext = pNodePrev->next();
        pNodePrev->setNext( pNode );
        pNode->setPrev( pNodePrev );
    }
    // Case of the last node in the stream.
    if( !pNodeNext ) {
        m_pLastNode = pNode;
    }
    else {
        pNodeNext->setPrev( pNode );
        pNode->setNext( pNodeNext );
    }
    return pNode;
}
void PdfeContentsStream::erase( PdfeContentsStream::Node* pnode,
                                bool smartErase )
{
    // TODO...
}

void PdfeContentsStream::load( PdfCanvas* pcanvas, bool loadFormsStream )
{
    // Reinitialize the contents stream.
    this->init();

    // Contents stream tokenizer.
    PdfeStreamTokenizer tokenizer( pcanvas );
    // Tmp variable to store node informations.
    EPdfContentsType tokenType;
    std::string strVariant;
    PdfeGraphicOperator goperator;
    std::vector<std::string> goperands;

    // Nodes stacks, for specific links.
    std::vector<Node*> pNodes_BT;
    std::vector<Node*> pNodes_q;
    std::vector<Node*> pNodes_BI;
    std::vector<Node*> pNodes_BX;
    std::vector<Node*> pNodes_path;
    Node* pNode_BeginSubpath;

    // Temp nodes pointers.
    Node* pNode = NULL;
    Node* pNodePrev = NULL;

    // Analyse page stream / Also known as the big dirty loop !
    while( tokenizer.ReadNext( tokenType, goperator, strVariant ) ) {
        // Variant: store it in the operands stack.
        if ( tokenType == ePdfContentsType_Variant ) {
            goperands.push_back( strVariant );
        }
        // Keyword: insert the node.
        else if( tokenType == ePdfContentsType_Keyword ) {
            pNode = this->insert( Node( 0, goperator, goperands ),
                                  pNodePrev );

            // Create specific links.
            if( goperator.cat == PdfeGCategory::SpecialGState ) {
                if( goperator.code == PdfeGOperator::q ) {
                    pNodes_q.push_back( pNode );
                }
                else if( goperator.code == PdfeGOperator::Q ) {
                    if( !pNodes_q.empty() ) {
                        pNodes_q.back()->setClosingNode( pNode );
                        pNode->setOpeningNode( pNodes_q.back() );
                        pNodes_q.pop_back();
                    }
                    else {
                        QLOG_WARN() << QString( "<PdfeContentsStream> Missing closing 'Q' operator in a contents stream (node ID: )." )
                                       .arg( pNode->id() ).toAscii().constData();
                    }
                }
            }
            else if( goperator.cat == PdfeGCategory::PathConstruction ) {
                // Begin of subpath.
                if( !pNode_BeginSubpath ) {
                    pNode_BeginSubpath = pNode;
                    if( goperator.code != PdfeGOperator::m &&
                            goperator.code != PdfeGOperator::re ) {
                        QLOG_WARN() << QString( "<PdfeContentsStream> Begin subpath a construction operator different from 'm/re' (node ID: )." )
                                       .arg( pNode->id() ).toAscii().constData();
                        // TODO: keep track of the current point and add a node with 'm'.
                    }
                }
                pNode->setBeginSubpathNode( pNode_BeginSubpath );
                // Path painting nodes stack.
                pNodes_path.push_back( pNode );
            }
            else if( goperator.cat == PdfeGCategory::PathPainting ) {
                if( !pNodes_path.empty() ) {
                    // Update path construction nodes.
                    for( size_t i = 0 ; i < pNodes_path.size() ; ++i ) {
                        pNodes_path[i]->setPaintingNode( pNode );
                    }
                    pNodes_path.clear();
                }
                else {
                    QLOG_WARN() << QString( "<PdfeContentsStream> Path painting with no path defined (node ID: )." )
                                   .arg( pNode->id() ).toAscii().constData();
                }
            }
            else if( goperator.cat == PdfeGCategory::TextObjects ) {
                if( goperator.code == PdfeGOperator::BT ) {
                    pNodes_BT.push_back( pNode );
                }
                else if( goperator.code == PdfeGOperator::ET ) {
                    if( !pNodes_BT.empty() ) {
                        pNodes_BT.back()->setClosingNode( pNode );
                        pNode->setOpeningNode( pNodes_BT.back() );
                        pNodes_BT.pop_back();
                    }
                    else {
                        QLOG_WARN() << QString( "<PdfeContentsStream> Missing closing 'BT' operator in a contents stream (node ID: )." )
                                       .arg( pNode->id() ).toAscii().constData();
                    }
                }
            }
            else if( goperator.cat == PdfeGCategory::InlineImages ) {
                if( goperator.code == PdfeGOperator::BI ) {
                    pNodes_BI.push_back( pNode );
                }
                else if( goperator.code == PdfeGOperator::EI ) {
                    if( !pNodes_BI.empty() ) {
                        pNodes_BI.back()->setClosingNode( pNode );
                        pNode->setOpeningNode( pNodes_BI.back() );
                        pNodes_BI.pop_back();
                    }
                    else {
                        QLOG_WARN() << QString( "<PdfeContentsStream> Missing closing 'BI' operator in a contents stream (node ID: )." )
                                       .arg( pNode->id() ).toAscii().constData();
                    }
                }
            }
            else if( goperator.cat == PdfeGCategory::Compatibility ) {
                if( goperator.code == PdfeGOperator::BX ) {
                    pNodes_BX.push_back( pNode );
                }
                else if( goperator.code == PdfeGOperator::EX ) {
                    if( !pNodes_BX.empty() ) {
                        pNodes_BX.back()->setClosingNode( pNode );
                        pNode->setOpeningNode( pNodes_BX.back() );
                        pNodes_BX.pop_back();
                    }
                    else {
                        QLOG_WARN() << QString( "<PdfeContentsStream> Missing closing 'BX' operator in a contents stream (node ID: )." )
                                       .arg( pNode->id() ).toAscii().constData();
                    }
                }
            }
            // XObjects forms: resources and loading.
            else if( goperator.cat == PdfeGCategory::XObjects ) {
                // TODO...
            }

            // Clear path construction stack.
            if( goperator.cat != PdfeGCategory::PathConstruction &&
                    goperator.cat != PdfeGCategory::PathPainting &&
                    goperator.cat != PdfeGCategory::ClippingPath &&
                    !pNodes_path.empty() ) {
                pNodes_path.clear();
                QLOG_WARN() << QString( "<PdfeContentsStream> Path constructed but not painted (node ID: )." )
                               .arg( pNode->id() ).toAscii().constData();

            }
            if( goperator.cat != PdfeGCategory::PathConstruction ) {
                pNode_BeginSubpath = NULL;
            }
            // Clear operands stack and update previous node.
            goperands.clear();
            pNodePrev = pNode;
        }
        else if ( tokenType == ePdfContentsType_ImageData ) {
            // Copy inline image data in the variables vector. TODO?
            goperands.push_back( strVariant );
        }
    }
}

void PdfeContentsStream::copyNodes( const PdfeContentsStream& stream )
{
    m_pFirstNode = m_pLastNode = NULL;
    m_nbNodes = stream.m_nbNodes;
    m_maxNodeID = stream.m_maxNodeID;
    if( stream.m_pFirstNode ) {
        // Nodes map, for specific links.
        std::map<Node*,Node*> pNodesOpening;
        std::map<Node*,Node*> pNodesBeginSubpath;
        std::map<Node*,std::vector<Node*> > pNodesPath;

        // First node.
        Node* pNodePrev = NULL;
        Node* pNodeNext = NULL;
        Node* pNodeIn = stream.m_pFirstNode;
        // Deep copy of nodes.
        while( pNodeIn ) {
            pNodeNext = new Node( *pNodeIn );
            if( pNodePrev ) {
                pNodePrev->setNext( pNodeNext );
                pNodeNext->setPrev( pNodePrev );
            }
            if( !m_pFirstNode ) {
                m_pFirstNode = pNodeNext;
            }

            // Opening/closing nodes.
            if( pNodeIn->isOpeningNode() ) {
                pNodesOpening.at( pNodeIn ) = pNodeNext;
            }
            else if( pNodeIn->isClosingNode() ) {
                std::map<Node*,Node*>::iterator it =
                        pNodesOpening.find( pNodeIn->openingNode() );
                if( it != pNodesOpening.end() ) {
                    pNodeNext->setOpeningNode( it->second );
                    it->second->setClosingNode( pNodeNext );
                    pNodesOpening.erase( it );
                }
            }
            // Path construction/painting nodes.
            else if( pNodeIn->category() == PdfeGCategory::PathConstruction ) {
                // Beginning of the subpath.
                if( pNodeIn->isBeginSubpathNode() ) {
                    pNodesBeginSubpath.at( pNodeIn ) = pNodeNext;
                }
                std::map<Node*,Node*>::iterator it =
                        pNodesBeginSubpath.find( pNodeIn->beginSubpathNode() );
                if( it != pNodesBeginSubpath.end() ) {
                    pNodeNext->setBeginSubpathNode( it->second );
                }
                // Painting nodes associated.
                if( pNodeIn->paintingNode() ) {
                    pNodesPath.at( pNodeIn->paintingNode() ).push_back( pNodeNext );
                }
            }
            else if( pNodeIn->category() == PdfeGCategory::PathPainting ) {
                std::map<Node*,std::vector<Node*> >::iterator it =
                        pNodesPath.find( pNodeIn );
                if( it != pNodesPath.end() ) {
                    std::vector<Node*>& pnodes = it->second;
                    for( size_t i = 0 ; i < pnodes.size() ; ++i ) {
                        pnodes[i]->setPaintingNode( pNodeNext );
                    }
                    pNodesPath.erase( it );
                }
            }
            // Update nodes pointers.
            pNodePrev = pNodeNext;
            pNodeIn = pNodeIn->next();
        }
        m_pLastNode = pNodeNext;
    }
}
void PdfeContentsStream::deleteNodes()
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
    m_nodeID( 0 ),
    m_pPrevNode( NULL ), m_pNextNode( NULL ),
    m_goperator(), m_goperands(),
    m_pOpeningNode( NULL ),
    m_pBeginSubpathNode( NULL )
{
}
PdfeContentsStream::Node::Node( pdfe_nodeid nodeid,
                                const PdfeGraphicOperator& goperator,
                                const std::vector<std::string>& goperands ) :
    m_nodeID( nodeid ),
    m_pPrevNode( NULL ), m_pNextNode( NULL ),
    m_goperator( goperator ), m_goperands( goperands ),
    m_pOpeningNode( NULL ),
    m_pBeginSubpathNode( NULL )
{
}
void PdfeContentsStream::Node::init()
{
    m_nodeID = 0;
    m_pPrevNode = m_pNextNode = NULL;
    m_pOpeningNode = NULL;
    m_pBeginSubpathNode = NULL;
    m_goperator.init();
    m_goperands.clear();
}
PdfeContentsStream::Node::Node( const PdfeContentsStream::Node& rhs ) :
    m_nodeID( rhs.m_nodeID ),
    m_pPrevNode( NULL ), m_pNextNode( NULL ),
    m_goperator( rhs.m_goperator ), m_goperands( rhs.m_goperands ),
    m_pOpeningNode( NULL ),
    m_pBeginSubpathNode( NULL )
{
    if( m_goperator.code == PdfeGOperator::Do ) {
        m_pFormResources = rhs.m_pFormResources;
        m_formXObject.isForm = rhs.m_formXObject.isForm;
        m_formXObject.isLoaded = rhs.m_formXObject.isLoaded;
    }
}
PdfeContentsStream::Node& PdfeContentsStream::Node::operator=( const PdfeContentsStream::Node& rhs )
{
    m_nodeID = rhs.m_nodeID;
    m_pPrevNode = m_pNextNode = NULL;
    m_goperator = rhs.m_goperator;
    m_goperands = rhs.m_goperands;
    m_pOpeningNode = NULL;
    m_pBeginSubpathNode = NULL;
    if( m_goperator.code == PdfeGOperator::Do ) {
        m_pFormResources = rhs.m_pFormResources;
        m_formXObject.isForm = rhs.m_formXObject.isForm;
        m_formXObject.isLoaded = rhs.m_formXObject.isLoaded;
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
    m_pBeginSubpathNode = NULL;
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

PdfeContentsStream::Node* PdfeContentsStream::Node::beginSubpathNode() const
{
    if( m_goperator.cat == PdfeGCategory::PathConstruction ) {
        return m_pBeginSubpathNode;
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

bool PdfeContentsStream::Node::isFormXObject() const
{
    return ( m_goperator.code == PdfeGOperator::Do && m_formXObject.isForm );
}
bool PdfeContentsStream::Node::isFormXObjectLoaded() const
{
    return ( m_goperator.code == PdfeGOperator::Do && m_formXObject.isLoaded );
}
PdfObject* PdfeContentsStream::Node::resources() const
{
    if( m_goperator.code == PdfeGOperator::Do && m_formXObject.isForm ) {
        return m_pFormResources;
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
    if( this->isClosingNode() && pnode->isOpeningNode() ) {
        m_pOpeningNode = pnode;
    }
}
void PdfeContentsStream::Node::setClosingNode( PdfeContentsStream::Node* pnode )
{
    if( this->isOpeningNode() && pnode->isClosingNode() ) {
        m_pClosingNode = pnode;
    }
}
void PdfeContentsStream::Node::setBeginSubpathNode( PdfeContentsStream::Node* pnode )
{
    if( this->category() == PdfeGCategory::PathConstruction &&
            pnode->category() == PdfeGCategory::PathConstruction ) {
        m_pBeginSubpathNode = pnode;
    }
}
void PdfeContentsStream::Node::setPaintingNode( PdfeContentsStream::Node* pnode )
{
    if( this->category() == PdfeGCategory::PathConstruction &&
            pnode->category() == PdfeGCategory::PathPainting ) {
        m_pPaintingNode = pnode;
    }
}

void PdfeContentsStream::Node::setFormXObject( bool isloaded, PdfObject* presources )
{
    if( m_goperator.code == PdfeGOperator::Do ) {
        m_formXObject.isForm = true;
        m_formXObject.isLoaded = isloaded;
        m_pFormResources = presources;
    }
}
void PdfeContentsStream::Node::setFormResources( PdfObject* presources )
{
    if( m_goperator.code == PdfeGOperator::Do && m_formXObject.isForm ) {
        m_pFormResources = presources;
    }
}

}
