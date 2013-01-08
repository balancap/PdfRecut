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
class PdfCanvas;
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
public:
    // Basic modifications of the contents stream.
    /** Find a node with a given ID.
     * \param nodeID ID of the node to find.
     * \return Pointer to the node. NULL if not found.
     */
    Node* find( pdfe_nodeid nodeid ) const;
    /** Insert a node in the stream.
     * \param node Node to insert (will be copied).
     * \param pNodePrev Pointer to the node which corresponds
     * to the previous node in the stream. If NULL, pNodeOutinsert the node
     * at the beginning of the stream.
     * \return Pointer to the newly inserted node.
     */
    Node* insert( const Node& node, Node* pNodePrev );
    /** Erase a node in the stream.
     * \param pnode Pointer to the node to erase.
     * \param smartErase Erase relative nodes in the stream to
     * keep a consistent contents stream.
     */
    void erase( Node* pnode, bool smartErase );

public:
    /** Load the contents stream of a canvas (can be a page, a form,
     * a Type 3 font glyph,...).
     * \param pCanvas Canvas whose contents stream is loaded.
     * \param loadFormsStream Are streams from XObjects forms also loaded?
     * If yes, the stream is integrated into the parent stream. Otherwise,
     * only the graphics operator Do appears.
     */
    void load( PoDoFo::PdfCanvas* pcanvas,
               bool loadFormsStream );

private:
    /** Private version of the canvas loading. Can be called recursively, in
     * particular to load form XObjects.
     * \param pNodePrev Node after which is loaded the form stream.
     * \param iniResources Resources to use.
     * \return Last node to be inserted.
     */
    Node* load( PoDoFo::PdfCanvas* pcanvas,
                bool loadFormsStream,
                Node* pNodePrev,
                const PdfeResources& iniResources );

public:
    // Simples getters...
    /// Is the stream empty?
    bool isEmpty() const    {   return (m_nbNodes == 0);    }
    /// Get the numbers of nodes in the stream.
    size_t nbNodes() const  {   return m_nbNodes;           }

private:
    /** Deep copy of nodes from another contents stream.
     */
    void copyNodes( const PdfeContentsStream& stream );
    /** Delete contents nodes.
     */
    void deleteNodes();

private:
    /// Pointer to the first node of the stream.
    Node*  m_pFirstNode;
    /// Pointer to the last node of the stream.
    Node*  m_pLastNode;

    /// Numbers of nodes in the stream.
    size_t  m_nbNodes;
    /// Maximal node ID + 1 in the stream.
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
    /** Constructor. Create a node with given ID,
     * operator and operands.
     */
    Node( pdfe_nodeid nodeid,
          const PdfeGraphicOperator& goperator,
          const std::vector<std::string>& goperands );
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
    PdfeGOperator::Enum type() const        {   return m_goperator.type();    }
    /// Node category.
    PdfeGCategory::Enum category() const    {   return m_goperator.category();     }
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

    /// Does the node correspond to a the beginning of a subpath (m/re)?
    bool isBeginSubpathNode() const;
    /// Node corresponding to the beginning of the subpath.
    Node* beginSubpathNode() const;
    /// Painting node. NULL if not a path construction node.
    Node* paintingNode() const;

    /// Does the node corresponds to a form XObject?
    bool isFormXObject() const;
    /// Is the XObject form loaded? False if not a form XObject.
    bool isFormXObjectLoaded() const;
    /// Resources object. Only for XObjects forms. NULL otherwise.
    PoDoFo::PdfObject* resources() const;

public:
    // Setters...
    /// Set operator of the node.
    void setOperator( const PdfeGraphicOperator& rhs );
    /// Set operands related to the node's operator.
    void setOperands( const std::vector<std::string>& rhs );

    /// Set opening node. Check the type before modification.
    void setOpeningNode( Node* pnode );
    /// Set closing node. Check the type before modification.
    void setClosingNode( Node* pnode );

    /// Set begin subpath node. Check it is a path construction node.
    void setBeginSubpathNode( Node* pnode );
    /// Set painting node. Check it is a path construction node.
    void setPaintingNode( Node* pnode );

    /// Set information on a form XObject: isLoaded and resources.
    void setFormXObject( bool isloaded,
                         PoDoFo::PdfObject* presources );
    /// Set form resources. Check it is an XObjects form before modification.
    void setFormResources( PoDoFo::PdfObject* presources );

private:
    // Setters... Keep them private for now...
    /// Set node ID in the stream.
    void setID( pdfe_nodeid nodeid );
    /// Set previous node in the stream.
    void setPrev( Node* pnode );
    /// Set next node in the stream.
    void setNext( Node* pnode );

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
        PoDoFo::PdfObject*  m_pFormResources;
    };
    union {
        /// Node with which begins the subpath. Only for construction path nodes.
        Node*  m_pBeginSubpathNode;
        struct {
            /// Is the node an XObjects form?
            bool  isForm;
            /// Is an XObjects form loaded?
            bool  isLoaded;
        } m_formXObject;
    };
};

//**********************************************************//
//              Inline PdfeContentsStream::Node             //
//**********************************************************//
inline bool PdfeContentsStream::Node::isEmpty() const
{
    return m_goperator.type() == PdfeGOperator::Unknown;
}
inline bool PdfeContentsStream::Node::isOpeningNode() const
{
    return  m_goperator.type() == PdfeGOperator::BT ||
            m_goperator.type() == PdfeGOperator::q  ||
            m_goperator.type() == PdfeGOperator::BI ||
            m_goperator.type() == PdfeGOperator::BX;
}
inline bool PdfeContentsStream::Node::isClosingNode() const
{
    return  m_goperator.type() == PdfeGOperator::ET ||
            m_goperator.type() == PdfeGOperator::Q  ||
            m_goperator.type() == PdfeGOperator::EI ||
            m_goperator.type() == PdfeGOperator::EX;
}
inline bool PdfeContentsStream::Node::isBeginSubpathNode() const
{
    return  m_goperator.category() == PdfeGCategory::PathConstruction &&
            this == m_pBeginSubpathNode;
}

}

#endif // PDFECONTENTSSTREAM_H
