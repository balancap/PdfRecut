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

#ifndef PDFECONTENTSSTREAM_H
#define PDFECONTENTSSTREAM_H

#include "PdfeGraphicsState.h"
#include "PdfeResources.h"

namespace PoDoFo {
class PdfObject;
}

namespace PoDoFoExtended {

/// Node ID typedef.
typedef PoDoFo::pdf_uint32  pdfe_nodeid;

//**********************************************************//
//                     PdfeContentsStream                   //
//**********************************************************//
/** Class that represents a contents stream. The description of the stream
 * remains at an intermediate level of abstraction, in order to perform some
 * basic operations on the stream but nevertheless keep a small memory print.
 * The stream itself is represented by a doubly-linked list of nodes, each node
 * containing a graphics operator.
 */
class PdfeContentsStream
{
public:
    /// Node in the stream.
    class Node;
public:
    /** Basic constructor. Create an empty stream.
     */
    PdfeContentsStream();
    /** Initialize the contents stream to an empty object.
     */
    void init();
    /** Copy constructor.
     */
    PdfeContentsStream( const PdfeContentsStream& rhs );
    /** Assignement operator.
     */
    PdfeContentsStream& operator=( const PdfeContentsStream& rhs );
    /** Destructor.
     */
    ~PdfeContentsStream();

private:
    /** Deep copy of nodes from another contents stream.
     */
    void copyNodes( const PdfeContentsStream& stream );
    /** Remove contents nodes.
     */
    void rmNodes();

private:
    /// Pointer to the first node of the stream.
    Node*  m_pFirstNode;
    /// Pointer to the last node of the stream.
    Node*  m_pLastNode;

    /// Numbers of nodes in the stream.
    size_t  m_nbNodes;
    /// Maximal node ID in the stream.
    pdfe_nodeid  m_maxNodeID;

    /// Initial graphics state used for the stream.
    PdfeGraphicsState  m_initialGState;
    /// Initial resources used for the stream.
    PdfeResources  m_initialResources;
};

//**********************************************************//
//                  PdfeContentsStream::Node                //
//**********************************************************//
/** Class representing a node inside a contents stream. A node
 * corresponds to a graphics operator, possibly associated to
 * some operands. It might contain some additional information,
 * depending on the node type. It also has a unique ID in a stream
 * that should never change, whatever other modifications in the stream.
 * Every node has a previous and a next node (unless
 * corresponding to starting or ending nodes of the stream).
 */
class PdfeContentsStream::Node
{
public:
    // PdfeContentsStream friend class...
    friend class PdfeContentsStream;

    /** Basic constructor. Create an empty node.
     */
    Node();
    /** Initialize the node to an empty object.
     */
    void init();
    /** Copy constructor.
     */
    Node( const Node& rhs );
    /** Assignement operator.
     */
    Node& operator=( const Node& rhs );
    /** Destructor.
     */
    ~Node();

public:
    /** Clear the node contents (empty operands and operator).
     */
    void clear();

public:
    // Getters and...
    /// Get the node ID in the stream.
    pdfe_nodeid id() const  {   return m_nodeID;        }
    /// Previous node (pointer, can be NULL).
    Node* prev() const      {   return m_pPrevNode;     }
    /// Nex node (pointer, can be NULL).
    Node* next() const      {   return m_pNextNode;     }
    /// Graphics operator (const reference).
    const PdfeGraphicOperator& goperator() const        {   return m_goperator;     }
    /// Graphics operands (represented by string).
    const std::vector<std::string>& goperands() const   {   return m_goperands;     }
    /// Node type.
    PdfeGOperator::Enum type() const        {   return m_goperator.code;    }
    /// Node category.
    PdfeGCategory::Enum category() const    {   return m_goperator.cat;     }

    /// Is the node empty, i.e. correspond to unknown type.
    bool isEmpty() const;
    /// Is it an opening node? i.e. BT, q, BI or BX.
    bool isOpeningNode() const;
    /// Is it a closing node? i.e. ET, Q, EI or EX.
    bool isClosingNode() const;
    /// Associated opening node. Need to be an operator ET, Q, EI or EX. NULL otherwise.
    Node* openingNode() const;
    /// Associated closing node. Need to be an operator BT, q, BI or BX. NULL otherwise.
    Node* closingNode() const;
    /// Painting node. NULL if not a path construction node.
    Node* paintingNode() const;
    /// Resources object. Only for XObjects forms. NULL otherwise.
    PoDoFo::PdfObject* resources() const;

public:
    // Setters...
    /// Set operator of the node.
    void setOperator( const PdfeGraphicOperator& rhs );
    /// Set operands related to the node's operator.
    void setOperands( const std::vector<std::string>& rhs );

private:
    // Setters... Keep them private for now...
    /// Set node ID in the stream.
    void setID( pdfe_nodeid nodeid );
    /// Set previous node in the stream.
    void setPrev( Node* pnode );
    /// Set next node in the stream.
    void setNext( Node* pnode );

    /// Set opening node. Check the type before modification.
    void setOpeningNode( Node* pnode );
    /// Set closing node. Check the type before modification.
    void setClosingNode( Node* pnode );
    /// Set painting node. Check it is a path construction node.
    void setPaintingNode( Node* pnode );
    /// Set resources. Check it is an XObjects form before modification.
    void setResources( PoDoFo::PdfObject* presources );

private:
    /// ID of the node in the stream. Should be unique.
    pdfe_nodeid  m_nodeID;
    /// Previous node in the stream.
    Node*  m_pPrevNode;
    /// Next node in the stream.
    Node*  m_pNextNode;

    /// Graphics operator corresponding to the node.
    PdfeGraphicOperator  m_goperator;
    /// Graphics operands associated to the operator.
    std::vector<std::string>  m_goperands;

    union {
        /// Opening node. Only for operators BT/ET q/Q BI/EI BX/EX.
        Node*  m_pOpeningNode;
        /// Closing node. Only for operators BT/ET q/Q BI/EI BX/EX.
        Node*  m_pClosingNode;
        /// Painting node. Only for path construction nodes.
        Node*  m_pPaintingNode;
        /// Resources object. Only for XObjects forms (Do).
        PoDoFo::PdfObject*  m_pResourcesObj;
    };
};

//**********************************************************//
//              Inline PdfeContentsStream::Node             //
//**********************************************************//
inline bool PdfeContentsStream::Node::isEmpty() const
{
    return m_goperator.code == PdfeGOperator::Unknown;
}
inline bool PdfeContentsStream::Node::isOpeningNode() const
{
    return  m_goperator.code == PdfeGOperator::BT ||
            m_goperator.code == PdfeGOperator::q  ||
            m_goperator.code == PdfeGOperator::BI ||
            m_goperator.code == PdfeGOperator::BX;
}
inline bool PdfeContentsStream::Node::isClosingNode() const
{
    return  m_goperator.code == PdfeGOperator::ET ||
            m_goperator.code == PdfeGOperator::Q  ||
            m_goperator.code == PdfeGOperator::EI ||
            m_goperator.code == PdfeGOperator::EX;
}

}

#endif // PDFECONTENTSSTREAM_H
