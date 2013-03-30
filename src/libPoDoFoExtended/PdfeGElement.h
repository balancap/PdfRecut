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

#include "PdfeTypes.h"
#include "PdfeGraphicsOperators.h"
#include "PdfeContentsStream.h"

#ifndef PDFEGELEMENT_H
#define PDFEGELEMENT_H

namespace PoDoFoExtended {

class PdfeGraphicsState;


namespace PdfeGElementSave {
/// Type of saving policy used for PdfeGElement object.
enum Enum {
    Replace = 0,    /// Replace existing content.
    PushBack,       /// Push the element after existing ones.
    PushFront       /// Push the element in front of existing ones.
};
}

/** Class that represents a common PDF graphical element stored
 * in a contents stream (i.e. path, text, image, ...).
 * This parent interface is used to stored some common information
 * such as node ID and graphics state.
 */
class PdfeGElement
{
public:
    /** Create an empty element, with no node ID defined and
     * no graphics state stored.
     */
    PdfeGElement();
    /** Initialize the graphic element to an empty object.
     */
    void init();
    /** Destructor: release resources.
     */
    ~PdfeGElement();

public:
    // Node ID.
    /// Get node ID of the element.
    pdfe_nodeid nodeID() const          {   return m_nodeID;    }
    /// Set node ID of the element.
    void setNodeID( pdfe_nodeid id )    {   m_nodeID = id;      }

public:
    // Graphics state.
    /// Does the element a graphics state defined?
    bool hasGState() const;
    /// Get the graphics state (initialize to default if none exists).
    const PdfeGraphicsState& gstate() const;
    /// Set the graphics state of the element.
    void setGState( const PdfeGraphicsState& gstate );
    /// Clear the graphics state of the element.
    void clearGState();

private:
    /// Node ID corresponding to this element.
    pdfe_nodeid  m_nodeID;
    /// Corresponding graphics state of the element (can be undefined).
    mutable PdfeGraphicsState*  m_pGState;
};

}

#endif // PDFEGELEMENT_H
