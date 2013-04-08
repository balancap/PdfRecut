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


#include <istream>
#include <limits>

#include <QByteArray>

#include "PdfeTypes.h"
#include "PdfeData.h"
#include "PdfeGraphicsOperators.h"
#include "PdfeResources.h"
#include "PdfeMisc.h"

namespace PoDoFo {
class PdfObject;
class PdfCanvas;
class PdfVariant;
}

namespace PoDoFoExtended {

class PdfeGraphicsState;

/// Node ID typedef.
typedef PoDoFo::pdf_uint32  pdfe_nodeid;
/// Node sub ID typedef.
typedef PoDoFo::pdf_uint16  pdfe_nodesubid;

/// Undefined node id value.
inline pdfe_nodeid NodeIDUndefined() {
    return std::numeric_limits<pdfe_nodeid>::max();
}
/// Undefined node sub id value.
inline pdfe_nodesubid NodeSubIDUndefined() {
    return std::numeric_limits<pdfe_nodesubid>::max();
}

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
    /** Copy constructor. Perform a deep copy.
     */
    PdfeContentsStream( const PdfeContentsStream& rhs );
    /** Assignment operator. Perform a deep copy.
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
     * \param smarterase Erase related nodes in the stream to
     * keep a consistent contents stream.
     * \return Return the node right after the erased one.
     */
    Node* erase( Node* pnode, bool smarterase );

public:
    /** Load the contents stream of a canvas (can be a page, a form,
     * a Type 3 font glyph,...).
     * \param pcanvas Canvas whose contents stream is loaded.
     * \param loadFormsStream Are streams from XObjects forms also loaded?
     * If yes, the stream is integrated into the parent stream. Otherwise,
     * only the graphics operator Do appears. Resources of the stream are completed
     * with Forms resources (their names are modified to avoid conflicts).
     * \param fixStream Fix mistakes detected in the stream.
     */
    void load( PoDoFo::PdfCanvas *pcanvas,
               bool loadFormsStream,
               bool fixStream );
    /** Save the stream into an existing canvas.
     * \param pcanvas Canvas whose contents stream is replaced.
     * Previous existing content is completely erased.
     */
    void save( PoDoFo::PdfCanvas* pcanvas );

    /** Get stream data, formatted as specified in PDF reference.
     * \return Byte array containing the stream.
     */
    QByteArray streamData() const;
    /** Export the stream into a text file. The text description
     * does not correspond to the official PDF reference but should be
     * easily human-readable.
     * \param filename Filename of the text file.
     */
    void exportToFile( QString filename ) const;

private:
    /** Private version of the canvas loading. Can be called recursively, in
     * particular to load form XObjects.
     * \param pNodePrev Node after which is loaded the form stream.
     * \param resSuffix Suffix to add to resources (form loading...).
     * \return Last node to be inserted.
     */
    Node* load( PoDoFo::PdfCanvas *pcanvas,
                bool loadFormsStream,
                bool fixStream,
                Node* pNodePrev,
                const std::string& resSuffix );

public:
    // Simples getters...
    /// Is the stream empty?
    bool isEmpty() const    {   return ( m_nbNodes == 0 );  }
    /// Get the numbers of nodes in the stream.
    size_t nbNodes() const  {   return m_nbNodes;           }
    /// First node of the stream.
    Node* firstNode() const {   return m_pFirstNode;        }
    /// Last node of the stream.
    Node* lastNode() const  {   return m_pLastNode;         }
    /// Initial graphics state.
    const PdfeGraphicsState& initialGState() const  {   return *m_pInitialGState;   }
    /// Resources used by the contents stream.
    const PdfeResources& resources() const          {   return m_resources;         }

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
    PdfeGraphicsState*  m_pInitialGState;
    /// Resources used by the contents stream.
    PdfeResources  m_resources;
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
          const PdfeGraphicOperator& goperator = PdfeGraphicOperator(),
          const std::vector<PdfeData>& goperands = std::vector<PdfeData>() );
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
    const PdfeGraphicOperator& goperator() const    {   return m_goperator;     }
    /// Graphics operands (represented by string).
    const std::vector<PdfeData>& operands() const   {   return m_goperands;     }
    /// Number of operands.
    size_t nbOperands() const               {   return m_goperands.size();      }
    /// Node type.
    PdfeGOperator::Enum type() const        {   return m_goperator.type();      }
    /// Node category.
    PdfeGCategory::Enum category() const    {   return m_goperator.category();  }
    /// Is the node empty, i.e. correspond to unknown type.
    bool isEmpty() const;

    /// Is it an opening node? i.e. BT, q, BI or BX and form XObject.
    bool isOpeningNode() const;
    /// Is it a closing node? i.e. ET, Q, EI or EX and form XObject.
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

    /// XObject type (Unknown if not a 'Do' node).
    PdfeXObjectType::Enum xobjectType() const;
    /// XObject PoDoFo::PdfObject. NULL if not a 'Do' node.
    PoDoFo::PdfObject* xobject() const;
    /// Is the XObject form loaded? False if not a form XObject.
    bool isFormXObjectLoaded() const;

public:
    // Setters...
    /// Set the graphics operator of the node.
    void setGOperator( const PdfeGraphicOperator& rhs );
    /// Set graphics operands related to the node's operator.
    void setOperands( const std::vector<PdfeData>& rhs );

    /// Set graphics operands from a PdfeVector.
    void setOperands( const PdfeVector& rhs );
    /// Set graphics operands from a PdfeMatrix.
    void setOperands( const PdfeMatrix& rhs );

    /// Set opening node. Check the type before modification.
    void setOpeningNode( Node* pnode );
    /// Set closing node. Check the type before modification.
    void setClosingNode( Node* pnode );

    /// Set begin subpath node. Check it is a path construction node.
    void setBeginSubpathNode( Node* pnode );
    /// Set painting node. Check it is a path construction node.
    void setPaintingNode( Node* pnode );

    /// Set information on an XObject: type and xobject pointer.
    void setXObject( PdfeXObjectType::Enum type, PoDoFo::PdfObject* pXObject );
    /// Set loading/opening/closing information on a form XObject.
    void setFormXObject( bool isLoaded, bool isOpening, bool isClosing );

    /** Add suffix to (resources) names which appear in the node.
     * Does nothing if no name is involved in the node.
     * \param suffix Suffix to append.
     */
    void addSuffix( const std::string& suffix );

public:
    // Operands getters and setters...
    /** Get the value of an operand. Set the returned type
     * through the template parameter. Raise an exception if out of range.
     * \param idx Index of the operand.
     * \return Value, converted to a given type.
     */
    template <class T>
    const T operand( size_t idx ) const;
    /** Set the value of an operand. Resize the operands vector if necessary.
     * \param idx Index of the operand to set.
     * \param value New value to set.
     */
    template <class T>
    void setOperand( size_t idx, const T& value );

    // Special case of PdfeData.
    /// Get operand data value.
    const PdfeData& operand( size_t idx ) const;
    /// Set operand data.
    void setOperand( size_t idx, const PdfeData& data );

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
    std::vector<PdfeData>  m_goperands;

    union {
        /// Opening node. Only for operators BT/ET q/Q BI/EI BX/EX.
        Node*  m_pOpeningNode;
        /// Closing node. Only for operators BT/ET q/Q BI/EI BX/EX.
        Node*  m_pClosingNode;
        /// Painting node. Only for path construction nodes.
        Node*  m_pPaintingNode;
        /// XObject PoDoFo object.
        PoDoFo::PdfObject*  m_pXObject;
    };
    union {
        /// Node with which begins the subpath. Only for construction path nodes.
        Node*  m_pBeginSubpathNode;
        /// Structure gathering information on form XObjects.
        struct {
            /// XObject type (converted from PdfeXObjectType).
            unsigned char type;
            /// Is the form XObject loaded in the stream?
            bool  isLoaded;
            /// Does it correspond to the opening node, if loaded.
            bool  isOpening;
            /// Does it correspond to the closing node, if loaded.
            bool  isClosing;
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
            m_goperator.type() == PdfeGOperator::BX ||
            ( m_goperator.type() == PdfeGOperator::Do &&
              m_formXObject.isOpening );
}
inline bool PdfeContentsStream::Node::isClosingNode() const
{
    return  m_goperator.type() == PdfeGOperator::ET ||
            m_goperator.type() == PdfeGOperator::Q  ||
            m_goperator.type() == PdfeGOperator::EI ||
            m_goperator.type() == PdfeGOperator::EX ||
            ( m_goperator.type() == PdfeGOperator::Do &&
              m_formXObject.isClosing );
}
inline bool PdfeContentsStream::Node::isBeginSubpathNode() const
{
    return  m_goperator.category() == PdfeGCategory::PathConstruction &&
            this == m_pBeginSubpathNode;
}

// Operand's getter / setter (template).
template <class T>
inline const T PdfeContentsStream::Node::operand( size_t idx ) const
{
    T value;
    std::istringstream istream( m_goperands.at( idx ).to_string() );
    if( !( istream >> value ) ) {
        PODOFO_RAISE_ERROR_INFO( PoDoFo::ePdfError_InvalidDataType, m_goperands.at( idx ).to_string().c_str() );
    }
    return value;
}
template <class T>
inline void PdfeContentsStream::Node::setOperand( size_t idx, const T& val )
{
    if( idx >= m_goperands.size() ) {
        m_goperands.resize( idx + 1 );
    }
    PdfeOStringStream ostream;
    ostream << val;
    m_goperands.at( idx ) = ostream.str();
}
// Operand's getter / setter (PdfeData).
inline const PdfeData& PdfeContentsStream::Node::operand( size_t idx ) const {
    return m_goperands.at( idx );
}
/// Set operand data.
inline void PdfeContentsStream::Node::setOperand( size_t idx, const PdfeData& data ) {
    if( idx >= m_goperands.size() ) {
        m_goperands.resize( idx + 1 );
    }
    m_goperands.at( idx ) = data;
}
// PoDoFo::PdfVariant specialization.
template <>
const PoDoFo::PdfVariant PdfeContentsStream::Node::operand( size_t idx ) const;
template <>
void PdfeContentsStream::Node::setOperand( size_t idx, const PoDoFo::PdfVariant& value );
// PoDoFo::PdfName specialization (no check...).
template <>
inline const PoDoFo::PdfName PdfeContentsStream::Node::operand( size_t idx ) const {
    const PdfeData& goperand = m_goperands.at( idx );
    return PoDoFo::PdfName( goperand.data()+1, goperand.size()-1 );
}
template <>
inline void PdfeContentsStream::Node::setOperand( size_t idx, const PoDoFo::PdfName& name ) {
    if( idx >= m_goperands.size() ) {
        m_goperands.resize( idx + 1 );
    }
    PdfeData& goperand = m_goperands.at( idx );
    goperand << "/" << name.GetName();
}

}

#endif // PDFECONTENTSSTREAM_H
