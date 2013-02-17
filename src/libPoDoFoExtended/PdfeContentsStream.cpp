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
#include "PdfeUtils.h"

#include <QtCore>
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
    m_initialGState(), m_resources()
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
    m_resources.init();
}
PdfeContentsStream::PdfeContentsStream( const PdfeContentsStream& rhs ) :
    m_pFirstNode( NULL ), m_pLastNode( NULL ),
    m_nbNodes( rhs.m_nbNodes ),
    m_maxNodeID( rhs.m_maxNodeID ),
    m_initialGState( rhs.m_initialGState ),
    m_resources( rhs.m_resources )
{
    // Copy nodes.
    this->copyNodes( rhs );
}
PdfeContentsStream& PdfeContentsStream::operator=( const PdfeContentsStream& rhs )
{
    if( this == &rhs ) {
        return *this;
    }
    // Clear existing contents nodes.
    this->deleteNodes();
    // Copy members and nodes.
    m_nbNodes = rhs.m_nbNodes;
    m_maxNodeID = rhs.m_maxNodeID;
    m_initialGState = rhs.m_initialGState;
    m_resources = rhs.m_resources;
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
PdfeContentsStream::Node* PdfeContentsStream::erase( PdfeContentsStream::Node* pnode,
                                                     bool smarterase )
{
    if( !pnode ) {
        return NULL ;
    }
    // Simple erase of the node.
    if( !smarterase ) {
        // Case of the first node...
        if( !pnode->prev() ) {
            m_pFirstNode = pnode->next();
        }
        else {
            pnode->prev()->setNext( pnode->next() );
        }
        // Case of the last node in the stream.
        if( !pnode->next() ) {
            m_pLastNode = pnode->prev();
        }
        else {
            pnode->next()->setPrev( pnode->prev() );
        }
        --m_nbNodes;
        Node* pNodeNext = pnode->next();
        delete pnode;
        return pNodeNext;
    }
    // Smart erase: study the structure of the stream and remove other related nodes.
    if( pnode->category() == PdfeGCategory::SpecialGState && pnode->type() != PdfeGOperator::cm ) {
        // Erase the entire content between 'q' and 'Q'.
        Node* pNodeFirst;
        Node* pNodeLast;
        if( pnode->type() == PdfeGOperator::q ) {
            pNodeFirst = pnode;
            pNodeLast = pnode->closingNode();
        }
        else {
            pNodeFirst = pnode->openingNode();
            pNodeLast = pnode;
        }
        if( pNodeFirst && pNodeLast ) {
            pnode = pNodeFirst;
            while( pnode != pNodeLast ) {
                pnode = this->erase( pnode, false );
            }
            pNodeLast = pNodeLast->next();
            this->erase( pnode, false );
            return pNodeLast;
        }
        else {
            QLOG_WARN() << QString( "<PdfeContentsStream> Can not erase completely the contents between 'q' and 'Q' nodes (node ID: %1)." )
                           .arg( pnode->id() ).toAscii().constData();
            return this->erase( pnode, false );
        }
    }
    else if( pnode->category() == PdfeGCategory::PathConstruction ) {
        Node* pNodeBegin = pnode->beginSubpathNode();
        if( pNodeBegin ) {
            // Erase the subpath it belongs to.
            pnode = pNodeBegin;
            while( pnode->beginSubpathNode() == pNodeBegin ) {
                pnode = this->erase( pnode, false );
            }
            // Erase painting operator if nothing else left.
            if( pnode->category() != PdfeGCategory::PathConstruction && ( !pnode->prev() ||
                    pnode->prev()->category() != PdfeGCategory::PathConstruction ) ) {
                // Clipping operator.
                if( pnode->category() == PdfeGCategory::ClippingPath ) {
                    pnode = this->erase( pnode, false );
                }
                if( pnode->category() == PdfeGCategory::PathPainting ) {
                    return this->erase( pnode, false );
                }
                else {
                    QLOG_WARN() << QString( "<PdfeContentsStream> No path painting node to erase (node ID: %1)." )
                                   .arg( pnode->id() ).toAscii().constData();
                    return pnode;
                }
            }
        }
        else {
            QLOG_WARN() << QString( "<PdfeContentsStream> No beginning node defined for the subpath (node ID: %1)." )
                           .arg( pnode->id() ).toAscii().constData();
            return this->erase( pnode, false );
        }
    }
    else if( pnode->category() == PdfeGCategory::PathPainting ) {
        // Remove painting operator and path construction.
        Node* pNodePrev = pnode->prev();
        // Clipping operator.
        if( pNodePrev && pNodePrev->category() == PdfeGCategory::ClippingPath ) {
            this->erase( pNodePrev, false );
            pNodePrev = pnode->prev();
        }
        // Path construction.
        while( pNodePrev && pNodePrev->category() == PdfeGCategory::PathConstruction ) {
            this->erase( pNodePrev, false );
            pNodePrev = pnode->prev();
        }
        return this->erase( pnode, false );
    }
    else if( pnode->category() == PdfeGCategory::ClippingPath ) {
        // Erase node and complete path.
        pnode = this->erase( pnode, false );
        if( pnode->category() == PdfeGCategory::PathPainting ) {
            return this->erase( pnode, true );
        }
        else {
            QLOG_WARN() << QString( "<PdfeContentsStream> No path painting node associated to the clipping path (node ID: %1)." )
                           .arg( pnode->id() ).toAscii().constData();
            return pnode;
        }
    }
    else if( pnode->category() == PdfeGCategory::TextObjects ) {
        // Erase the entire content between 'BT' and 'ET'.
        Node* pNodeFirst;
        Node* pNodeLast;
        if( pnode->type() == PdfeGOperator::BT ) {
            pNodeFirst = pnode;
            pNodeLast = pnode->closingNode();
        }
        else {
            pNodeFirst = pnode->openingNode();
            pNodeLast = pnode;
        }
        if( pNodeFirst && pNodeLast ) {
            pnode = pNodeFirst;
            while( pnode != pNodeLast ) {
                pnode = this->erase( pnode, false );
            }
            pNodeLast = pNodeLast->next();
            this->erase( pnode, false );
            return pNodeLast;
        }
        else {
            QLOG_WARN() << QString( "<PdfeContentsStream> Can not erase completely the contents between 'BT' and 'ET' nodes (node ID: %1)." )
                           .arg( pnode->id() ).toAscii().constData();
            return this->erase( pnode, false );
        }
    }
    else if( pnode->category() == PdfeGCategory::InlineImages ) {
        // Erase BI, ID and EI. TODO: check...
        if( pnode->type() == PdfeGOperator::BI ) {
            this->erase( pnode->next()->next(), false );
            this->erase( pnode->next(), false );
            return this->erase( pnode, false );
        }
        else if( pnode->type() == PdfeGOperator::ID ) {
            this->erase( pnode->prev(), false );
            this->erase( pnode->next(), false );
            return this->erase( pnode, false );
        }
        else {
            this->erase( pnode->prev()->prev(), false );
            this->erase( pnode->prev(), false );
            return this->erase( pnode, false );
        }
    }
    else if( pnode->category() == PdfeGCategory::XObjects ) {
        // Is it a loaded form?
        if( pnode->xobjectType() == PdfeXObjectType::Form && pnode->isFormXObjectLoaded() ) {
            // Erase 'Do' node and the content which follows 'q/Q'.
            if( pnode->next()->type() == PdfeGOperator::q ) {
                this->erase( pnode->next(), true );
            }
            else {
                QLOG_WARN() << QString( "<PdfeContentsStream> Form XObject's content not loaded (and thus not erased) (node ID: %1)." )
                               .arg( pnode->id() ).toAscii().constData();
            }
            // Erase the two 'Do' nodes (opening and closing).
            pnode = this->erase( pnode, false );
            return this->erase( pnode, false );
        }
        else {
            // Simple treatment.
            return this->erase( pnode, false );
        }
    }
    else if( pnode->category() == PdfeGCategory::MarkedContents ) {
        // TODO?
    }
    else if( pnode->category() == PdfeGCategory::Compatibility ) {
        // Erase the entire content between 'BX' and 'EX'.
        Node* pNodeFirst;
        Node* pNodeLast;
        if( pnode->type() == PdfeGOperator::BX ) {
            pNodeFirst = pnode;
            pNodeLast = pnode->closingNode();
        }
        else {
            pNodeFirst = pnode->openingNode();
            pNodeLast = pnode;
        }
        if( pNodeFirst && pNodeLast ) {
            pnode = pNodeFirst;
            while( pnode != pNodeLast ) {
                pnode = this->erase( pnode, false );
            }
            pNodeLast = pNodeLast->next();
            this->erase( pnode, false );
            return pNodeLast;
        }
        else {
            QLOG_WARN() << QString( "<PdfeContentsStream> Can not erase completely the contents between 'BX' and 'EX' nodes (node ID: %1)." )
                           .arg( pnode->id() ).toAscii().constData();
            return this->erase( pnode, false );
        }
    }
    // Simple erase as default behaviour.
    return this->erase( pnode, false );
}

void PdfeContentsStream::load( PdfCanvas* pcanvas, bool loadFormsStream, bool fixStream )
{
    // Reinitialize the contents stream.
    this->init();
    // Load canvas and set initial resources.
    this->load( pcanvas, loadFormsStream, fixStream, NULL, std::string() );
}
PdfeContentsStream::Node* PdfeContentsStream::load( PdfCanvas* pcanvas,
                                                    bool loadFormsStream,
                                                    bool fixStream,
                                                    PdfeContentsStream::Node* pNodePrev,
                                                    const std::string& resSuffix )
{
    // Contents stream tokenizer.
    PdfeStreamTokenizer tokenizer( pcanvas );
    // Tmp variable to store node informations.
    EPdfContentsType tokenType;
    std::string strVariant;
    PdfeGraphicOperator goperator;
    std::vector<std::string> goperands;
    size_t nbForms = 0;

    // Nodes stacks, for specific links.
    std::vector<Node*> pNodes_BT;
    std::vector<Node*> pNodes_q;
    std::vector<Node*> pNodes_BI;
    std::vector<Node*> pNodes_BX;
    std::vector<Node*> pNodes_path;
    Node* pNode_BeginSubpath = NULL;
    // Temp nodes pointers.
    Node* pNode = NULL;

    // Add canvas resources.
    PdfeResources resourcesCanvas( pcanvas->GetResources() );
    resourcesCanvas.addSuffix( resSuffix );
    m_resources.append( resourcesCanvas );
    //PdfeResources resources( resSuffix );
    //resources.push_back( pcanvas->GetResources() );
    //resources.append( PdfeResources( pcanvas->GetResources() ) );

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
            pNode->addSuffix( resSuffix );

            // Create specific links.
            if( goperator.category() == PdfeGCategory::SpecialGState ) {
                if( goperator.type() == PdfeGOperator::q ) {
                    pNodes_q.push_back( pNode );
                }
                else if( goperator.type() == PdfeGOperator::Q ) {
                    if( !pNodes_q.empty() ) {
                        pNodes_q.back()->setClosingNode( pNode );
                        pNode->setOpeningNode( pNodes_q.back() );
                        pNodes_q.pop_back();
                    }
                    else {
                        QLOG_WARN() << QString( "<PdfeContentsStream> Missing closing 'Q' operator in a contents stream (node ID: %1)." )
                                       .arg( pNode->id() ).toAscii().constData();
                    }
                }
            }
            else if( goperator.category() == PdfeGCategory::PathConstruction ) {
                // Begin of subpath.
                if( !pNode_BeginSubpath ) {
                    pNode_BeginSubpath = pNode;
                    if( goperator.type() != PdfeGOperator::m &&
                            goperator.type() != PdfeGOperator::re ) {
                        QLOG_WARN() << QString( "<PdfeContentsStream> Begin subpath a construction operator different from 'm/re' (node ID: %1)." )
                                       .arg( pNode->id() ).toAscii().constData();
                        // TODO: keep track of the current point and add a node with 'm'.
                    }
                }
                pNode->setBeginSubpathNode( pNode_BeginSubpath );
                if( goperator.type() == PdfeGOperator::h ) {
                    pNode_BeginSubpath = NULL;
                }
                // Path painting nodes stack.
                pNodes_path.push_back( pNode );
            }
            else if( goperator.category() == PdfeGCategory::PathPainting ) {
                if( !pNodes_path.empty() ) {
                    // Update path construction nodes.
                    for( size_t i = 0 ; i < pNodes_path.size() ; ++i ) {
                        pNodes_path[i]->setPaintingNode( pNode );
                    }
                    pNodes_path.clear();
                }
                else {
                    QLOG_WARN() << QString( "<PdfeContentsStream> Path painting with no path defined (node ID: %1)." )
                                   .arg( pNode->id() ).toAscii().constData();
                }
            }
            else if( goperator.category() == PdfeGCategory::TextObjects ) {
                if( goperator.type() == PdfeGOperator::BT ) {
                    pNodes_BT.push_back( pNode );
                }
                else if( goperator.type() == PdfeGOperator::ET ) {
                    if( !pNodes_BT.empty() ) {
                        pNodes_BT.back()->setClosingNode( pNode );
                        pNode->setOpeningNode( pNodes_BT.back() );
                        pNodes_BT.pop_back();
                    }
                    else {
                        QLOG_WARN() << QString( "<PdfeContentsStream> Missing closing 'BT' operator in a contents stream (node ID: %1)." )
                                       .arg( pNode->id() ).toAscii().constData();
                    }
                }
            }
            else if( goperator.category() == PdfeGCategory::InlineImages ) {
                if( goperator.type() == PdfeGOperator::BI ) {
                    pNodes_BI.push_back( pNode );
                }
                else if( goperator.type() == PdfeGOperator::EI ) {
                    if( !pNodes_BI.empty() ) {
                        pNodes_BI.back()->setClosingNode( pNode );
                        pNode->setOpeningNode( pNodes_BI.back() );
                        pNodes_BI.pop_back();
                    }
                    else {
                        QLOG_WARN() << QString( "<PdfeContentsStream> Missing closing 'BI' operator in a contents stream (node ID: %1)." )
                                       .arg( pNode->id() ).toAscii().constData();
                    }
                }
            }
            else if( goperator.category() == PdfeGCategory::Compatibility ) {
                if( goperator.type() == PdfeGOperator::BX ) {
                    pNodes_BX.push_back( pNode );
                }
                else if( goperator.type() == PdfeGOperator::EX ) {
                    if( !pNodes_BX.empty() ) {
                        pNodes_BX.back()->setClosingNode( pNode );
                        pNode->setOpeningNode( pNodes_BX.back() );
                        pNodes_BX.pop_back();
                    }
                    else {
                        QLOG_WARN() << QString( "<PdfeContentsStream> Missing closing 'BX' operator in a contents stream (node ID: %1)." )
                                       .arg( pNode->id() ).toAscii().constData();
                    }
                }
            }
            // XObjects forms: resources and loading.
            else if( goperator.category() == PdfeGCategory::XObjects ) {
                // Get XObject pointer and subtype.
                std::string xobjName = goperands.back().substr( 1 ) + resSuffix;
                PdfObject* pXObject = m_resources.getIndirectKey( PdfeResourcesType::XObject, xobjName );
                std::string xobjSubtype = pXObject->GetIndirectKey( "Subtype" )->GetName().GetName();

                // Form XObject.
                if( xobjSubtype == "Form" ) {
                    // PdfXObject corresponding created and update node information.
                    PdfXObject xobject( pXObject );
                    pNode->setXObject( PdfeXObjectType::Form, pXObject );

                    // Load form XObject.
                    if( loadFormsStream ) {
                        pNode->setFormXObject( true, true, false );
                        // Save the current graphics state on the stack 'q'.
                        pNode = this->insert( Node( 0, PdfeGraphicOperator( PdfeGOperator::q ),
                                                    std::vector<std::string>() ),
                                              pNode );
                        // Get transformation matrix of the form.
                        PdfeMatrix formTransMat;
                        if( pXObject->GetDictionary().HasKey( "Matrix" ) ) {
                            PdfArray& mat = pXObject->GetIndirectKey( "Matrix" )->GetArray();
                            formTransMat(0,0) = mat[0].GetReal();    formTransMat(0,1) = mat[1].GetReal();
                            formTransMat(1,0) = mat[2].GetReal();    formTransMat(1,1) = mat[3].GetReal();
                            formTransMat(2,0) = mat[4].GetReal();    formTransMat(2,1) = mat[5].GetReal();
                            // Insert in the stream it if not the identity.
                            if( formTransMat != PdfeMatrix() ) {
                                std::vector<std::string> goperands_cm( 6 );
                                mat[0].ToString( goperands_cm[0] );
                                mat[1].ToString( goperands_cm[1] );
                                mat[2].ToString( goperands_cm[2] );
                                mat[3].ToString( goperands_cm[3] );
                                mat[4].ToString( goperands_cm[4] );
                                mat[5].ToString( goperands_cm[5] );
                                pNode = this->insert( Node( 0, PdfeGraphicOperator( PdfeGOperator::cm ),
                                                            goperands_cm ),
                                                      pNode );
                            }
                        }
                        // Load form XObject, with new suffix.
                        std::ostringstream  suffixStream;
                        suffixStream << resSuffix << "_form" << nbForms;
                        pNode = this->load( &xobject, loadFormsStream, fixStream, pNode, suffixStream.str() );
                        // Restore the current graphics state on the stack 'Q'.
                        pNode = this->insert( Node( 0, PdfeGraphicOperator( PdfeGOperator::Q ),
                                                    std::vector<std::string>() ),
                                              pNode );
                        // Closing form XObject node.
                        pNode = this->insert( Node( 0, goperator, goperands ),
                                              pNode );
                        pNode->setXObject( PdfeXObjectType::Form, pXObject );
                        pNode->setFormXObject( true, false, true );
                        pNode->addSuffix( resSuffix );
                    }
                    else {
                        pNode->setFormXObject( false, false, false );
                    }
                    ++nbForms;
                }
                else if( xobjSubtype == "PS" ) {
                    pNode->setXObject( PdfeXObjectType::PS, pXObject );
                    QLOG_WARN() << QString( "<PdfeContentsStream> Postscript XObject inserted. Currently not supported. (node ID: %1)." )
                                   .arg( pNode->id() ).toAscii().constData();
                }
                else if( xobjSubtype == "Image" ) {
                    pNode->setXObject( PdfeXObjectType::Image, pXObject );
                }
                else {
                    pNode->setXObject( PdfeXObjectType::Unknown, pXObject );
                    QLOG_WARN() << QString( "<PdfeContentsStream> Unknown XObject type. (node ID: %1)." )
                                   .arg( pNode->id() ).toAscii().constData();
                }
            }
            else if( goperator.type() == PdfeGOperator::Unknown ) {
                QLOG_WARN() << QString( "<PdfeContentsStream> Unknown graphics operator in the contents stream (node ID: %1)." )
                               .arg( pNode->id() ).toAscii().constData();
            }

            // Clear path construction stack.
            if( goperator.category() != PdfeGCategory::PathConstruction &&
                    goperator.category() != PdfeGCategory::PathPainting &&
                    goperator.category() != PdfeGCategory::ClippingPath &&
                    !pNodes_path.empty() ) {
                pNodes_path.clear();
                QLOG_WARN() << QString( "<PdfeContentsStream> Path constructed but not painted (node ID: %1)." )
                               .arg( pNode->id() ).toAscii().constData();

            }
            if( goperator.category() != PdfeGCategory::PathConstruction ) {
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
    return pNodePrev;
}

void PdfeContentsStream::save( PdfCanvas* pcanvas )
{
    // Clean existing contents.
    PdfObject* pContentsObj = pcanvas->GetContents();
    PdfStream* pstream;
    if( pContentsObj->IsArray() ) {
        // Clear streams present in the array.
        PdfArray& contentsArray = pContentsObj->GetArray();
        for( size_t i = 0 ; i < contentsArray.size() ; ++i ) {
            PdfObject* pStreamObj = PdfeIndirectObject( &contentsArray[i],
                                                        pContentsObj->GetOwner() );
            pstream = pStreamObj->GetStream();
            pstream->BeginAppend( true );
            pstream->EndAppend();
        }
    }
    else {
        // Simply clear the stream.
        pstream = pContentsObj->GetStream();
        pstream->BeginAppend( true );
        pstream->EndAppend();
    }
    // Set stream contents.
    QByteArray streamData = this->streamData();
    TVecFilters vecFilters;
    vecFilters.push_back( ePdfFilter_FlateDecode );
    pstream->Set( streamData.constData(), streamData.size(), vecFilters );

    // Save contents resources.
    m_resources.save( pcanvas->GetResources() );
}

QByteArray PdfeContentsStream::streamData() const
{
    QByteArray data;
    // Write nodes description.
    Node* pnode = m_pFirstNode;
    while( pnode ) {
        // Case of a loaded form: do not write done.
        if( !pnode->isFormXObjectLoaded() ) {
            for( size_t i = 0 ; i < pnode->operands().size() ; ++i ) {
                const std::string& operand = pnode->operands()[i];
                data.append( operand.c_str(), operand.length() );
                data.append( ' ' );
            }
            data.append( pnode->goperator().str(), -1 );
            data.append( '\n' );
        }
        pnode = pnode->next();
    }
    return data;
}
void PdfeContentsStream::exportToFile( QString filename ) const
{
    QFile data( filename );
    if( data.open( QFile::WriteOnly | QFile::Truncate ) ) {
        QTextStream out( &data );

        Node* pnode = m_pFirstNode;
        while( pnode ) {
            // Write node description.
            out << qSetFieldWidth( 5 ) << left
                << pnode->id()
                << pnode->goperator().str()
                << qSetFieldWidth( 0 ) << left;
            for( size_t i = 0 ; i < pnode->operands().size() ; ++i ) {
                out << pnode->operands().at( i ).c_str() << " ";
            }
            out << endl;

            pnode = pnode->next();
        }
    }
    else {
        QLOG_WARN() << QString( "<PdfeContentsStream> Can not open file (%1) to write stream description." )
                       .arg( filename ).toAscii().constData();
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
                pNodesOpening[ pNodeIn ] = pNodeNext;
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
                    pNodesBeginSubpath[ pNodeIn ] = pNodeNext;
                }
                std::map<Node*,Node*>::iterator it =
                        pNodesBeginSubpath.find( pNodeIn->beginSubpathNode() );
                if( it != pNodesBeginSubpath.end() ) {
                    pNodeNext->setBeginSubpathNode( it->second );
                }
                // Painting nodes associated.
                if( pNodeIn->paintingNode() ) {
                    pNodesPath[ pNodeIn->paintingNode() ].push_back( pNodeNext );
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
    // TODO: check the number of operands.
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
    if( m_goperator.type() == PdfeGOperator::Do ) {
        m_pXObject = rhs.m_pXObject;
        m_formXObject = rhs.m_formXObject;
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
    if( m_goperator.type() == PdfeGOperator::Do ) {
        m_pXObject = rhs.m_pXObject;
        m_formXObject = rhs.m_formXObject;
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
    if( this->isClosingNode() ) {
        return m_pOpeningNode;
    }
    return NULL;
}
PdfeContentsStream::Node* PdfeContentsStream::Node::closingNode() const
{
    if( this->isOpeningNode() ) {
        return m_pClosingNode;
    }
    return NULL;
}

PdfeContentsStream::Node* PdfeContentsStream::Node::beginSubpathNode() const
{
    if( m_goperator.category() == PdfeGCategory::PathConstruction ) {
        return m_pBeginSubpathNode;
    }
    return NULL;
}
PdfeContentsStream::Node* PdfeContentsStream::Node::paintingNode() const
{
    if( m_goperator.category() == PdfeGCategory::PathConstruction ) {
        return m_pPaintingNode;
    }
    return NULL;
}

PdfeXObjectType::Enum PdfeContentsStream::Node::xobjectType() const
{
    if( m_goperator.type() == PdfeGOperator::Do ) {
        return PdfeXObjectType::Enum( m_formXObject.type );
    }
    return PdfeXObjectType::Unknown;
}
PdfObject* PdfeContentsStream::Node::xobject() const
{
    if( m_goperator.type() == PdfeGOperator::Do ) {
        return m_pXObject;
    }
    return NULL;
}
bool PdfeContentsStream::Node::isFormXObjectLoaded() const
{
    return ( m_goperator.type() == PdfeGOperator::Do && m_formXObject.isLoaded );
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
void PdfeContentsStream::Node::setGOperator( const PdfeGraphicOperator& rhs )
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

void PdfeContentsStream::Node::setXObject( PdfeXObjectType::Enum type, PdfObject* pXObject )
{
    if( m_goperator.type() == PdfeGOperator::Do ) {
        m_pXObject = pXObject;
        m_formXObject.type = static_cast<unsigned char>( type );
        if( type != PdfeXObjectType::Form ) {
            m_formXObject.isOpening
                    = m_formXObject.isClosing
                    = false;
        }
    }
}
void PdfeContentsStream::Node::setFormXObject( bool isLoaded, bool isOpening, bool isClosing )
{
    if( m_goperator.type() == PdfeGOperator::Do ) {
        m_formXObject.isLoaded = isLoaded;
        m_formXObject.isOpening = isOpening;
        m_formXObject.isClosing = isClosing;
    }
}

void PdfeContentsStream::Node::addSuffix( const std::string& suffix )
{
    if( suffix.empty() ) {
        return;
    }

    if( m_goperator.type() == PdfeGOperator::gs ) {
        m_goperands.back() = m_goperands.back() + suffix;
    }
    else if( m_goperator.type() == PdfeGOperator::Tf ) {
        m_goperands[0] = m_goperands[0] + suffix;
    }
    else if( m_goperator.type() == PdfeGOperator::sh ) {
        m_goperands.back() = m_goperands.back() + suffix;
    }
    else if( m_goperator.type() == PdfeGOperator::ID ) {
        // TODO: color space.
    }
    else if( m_goperator.type() == PdfeGOperator::Do ) {
        m_goperands.back() = m_goperands.back() + suffix;
    }
    else if( m_goperator.category() == PdfeGCategory::Color ) {
        // Color space.
        if( m_goperator.type() == PdfeGOperator::CS ||
               m_goperator.type() == PdfeGOperator::cs ) {
            std::string colorSpace = m_goperands.back().substr( 1 );
            // TODO: improve a bit this ugly code!
            if( colorSpace != "DeviceGray" && colorSpace != "DeviceRGB" &&
                   colorSpace != "DeviceCMYK" && colorSpace != "Pattern" ) {
                m_goperands.back() = m_goperands.back() + suffix;
            }
        }
        // Color, in case of pattern.
        else if( m_goperator.type() == PdfeGOperator::scn ||
                 m_goperator.type() == PdfeGOperator::SCN ) {
            if( m_goperands.back()[0] == '/' ) {
                m_goperands.back() = m_goperands.back() + suffix;
            }
        }
    }
}

// PoDoFo::PdfVariant specialization.
template <>
const PoDoFo::PdfVariant PdfeContentsStream::Node::operand( size_t idx ) const
{
    PdfVariant variant;
    PdfTokenizer tokenizer( m_goperands.at( idx ).c_str(), m_goperands.at( idx ).length() );
    tokenizer.GetNextVariant( variant, NULL );
    return variant;
}
template <>
void PdfeContentsStream::Node::setOperand( size_t idx, const PoDoFo::PdfVariant& value )
{
    if( idx >= m_goperands.size() ) {
        m_goperands.resize( idx + 1 );
    }
    value.ToString( m_goperands.at( idx ) ,ePdfWriteMode_Compact );
}

}
