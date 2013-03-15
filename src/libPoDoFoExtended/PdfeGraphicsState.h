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

#ifndef PDFEGRAPHICSSTATE_H
#define PDFEGRAPHICSSTATE_H

#include "podofo/base/PdfDefines.h"
#include "podofo/base/PdfReference.h"

#include "PdfePath.h"
#include "PdfeContentsStream.h"

namespace PoDoFo {
    class PdfPage;
}

namespace PoDoFoExtended {

class PdfeResources;

/** Class describing the text state in a PDF stream, i.e.
 * containing every information related to text rendering.
 * See PDF reference for my information on the topic.
 */
class PdfeTextState
{
public:
    /** Default constructor: initialize values as described in PDF reference.
     */
    PdfeTextState();
    /** Function which initializes members to default PDF values.
     */
    void init();
    /** Initialize only transformation matrices. Used at the beginning of
     * a text environnment (BT/ET operators).
     */
    void initMatrices();

public:
    // Getters...
    /// Get transformation matrix.
    const PdfeMatrix& transMat() const      {   return m_transMat;      }
    /// Get line transformation matrix.
    const PdfeMatrix& lineTransMat() const  {   return m_lineTransMat;  }

    /// Get font name.
    const std::string& fontName() const     {   return m_fontName;  }
    /// Get the reference to the font object.
    const PoDoFo::PdfReference& fontReference() const   {   return m_fontRef;    }

    /// Get font size.
    double fontSize() const     {   return m_fontSize;  }
    /// Get character space.
    double charSpace() const    {   return m_charSpace; }
    /// Get word space.
    double wordSpace() const    {   return m_wordSpace; }
    /// Get horizontal scale.
    double hScale() const       {   return m_hScale;    }
    /// Get lines leading.
    double leading() const      {   return m_leading;   }
    /// Get rendering mode.
    int render() const          {   return m_render;    }
    /// Get text rise.
    double rise() const         {   return m_rise;      }

    // and setters.
    /// Set transformation matrix.
    void setTransMat( const PdfeMatrix& rhs )      {   m_transMat = rhs;       }
    /// Set line transformation matrix.
    void setLineTransMat( const PdfeMatrix& rhs )  {   m_lineTransMat = rhs;   }

    /** Set font, identified by its reference.
     * \param name Name of font (optional).
     * \param ref Reference to the font object (required).
     */
    void setFont( const std::string& name,
                  const PoDoFo::PdfReference& ref );
    /** Set font, using its name and a resources object.
     * \param name Name of font (required).
     * \param resources Resources where to find the font object.
     * \return True if the font object has been found.
     */
    bool setFont( const std::string& name,
                  const PdfeResources& resources );

    /// Set font size.
    void setFontSize( double rhs )  {   m_fontSize = rhs;  }
    /// Set character space.
    void setCharSpace( double rhs ) {   m_charSpace = rhs; }
    /// Set word space.
    void setWordSpace( double rhs ) {   m_wordSpace = rhs; }
    /// Set horizontal scale.
    void setHScale( double rhs )    {   m_hScale = rhs;    }
    /// Set lines leading.
    void setLeading( double rhs)    {   m_leading = rhs;   }
    /// Set rendering mode (0-7).
    void setRender( int rhs )       {   m_render = rhs;    }
    /// Set text rise.
    void setRise( double rhs )      {   m_rise = rhs;      }

private:
    /// Text transformation matrix.
    PdfeMatrix  m_transMat;
    /// Line transformation matrix.
    PdfeMatrix  m_lineTransMat;

    /// Font name (as set in Font resources).
    std::string  m_fontName;
    /// Reference of the PDF font object.
    PoDoFo::PdfReference  m_fontRef;

    // Text state parameters.
    /// Font size.
    double  m_fontSize;
    /// Character space.
    double  m_charSpace;
    /// Word space.
    double  m_wordSpace;
    /// Horizontal scale.
    double  m_hScale;
    /// Leading (vertical space between lines).
    double  m_leading;
    /// Render (rendering mode 0-7).
    int  m_render;
    /// Rise (move baseline up or down).
    double  m_rise;
};

/** Class describing the graphics state in a PDF stream.
 * It gathers several parameters related to page rendering.
 *
 * Note that currently, the class does not implement all the
 * PDF reference (in particular parameters that might be set
 * in an ExtGState object).
 */
class PdfeGraphicsState
{
public:
    /** Default constructor. Initialize the state to default values.
     */
    PdfeGraphicsState();
    /** Copy constructor.
     */
    PdfeGraphicsState( const PdfeGraphicsState& rhs );
    /** Assignement operator.
     */
    PdfeGraphicsState& operator=( const PdfeGraphicsState& rhs );
    /** Initialize members to default values specified in the PDF reference.
     */
    void init();
    /** Destructor.
     */
    ~PdfeGraphicsState();

public:
    /** Update graphics state accordingly to a node contained
     * in a contents stream.
     * \param pnode Pointer to the node.
     * \param currentPath Current path in the stream. Used to update
     * the clipping path.
     * \param resources Resources of the stream.
     */
    void update( const PdfeContentsStream::Node* pnode,
                 const PdfePath& currentPath,
                 const PdfeResources& resources );

    /** Load parameters from an extern graphics state object.
     * \param gstateName Name of the graphics state to load.
     * \param resources Resources object where to find to the GState.
     * \return True if the extGState was found and loaded. False else.
     */
    bool loadExtGState( const std::string& gstateName, const PdfeResources& resources );

public:
    // Text graphics state.
    /** Get a reference to the text graphics state. Can be used
     * to modify the text graphics state object.
     */
    PdfeTextState& textState();
    const PdfeTextState& textState() const;
    /** Clear text graphics state object. Can be useful
     * when no text drawing information is needed (e.g. paths...).
     */
    void clearTextState();

    // Getters...
    /// Get transformation matrix.
    const PdfeMatrix& transMat() const      {   return m_transMat;      }
    /// Get clipping path.
    const PdfePath& clippingPath() const    {   return m_clippingPath;  }
    /// Get line's width.
    double lineWidth() const    {   return m_lineWidth;     }
    /// Get line's cap style.
    int lineCap() const         {   return m_lineCap;       }
    /// Get line's join style.
    int lineJoin() const        {   return m_lineJoin;      }
    /// Get miter limit.
    double miterLimit() const   {   return m_miterLimit;    }

    /// Compatibility mode (BX/EX operators)?
    bool compatibilityMode() const  {   return m_compatibilityMode;     }

    // and setters.
    /// Set transformation matrix.
    void setTransMat( const PdfeMatrix& rhs )       {   m_transMat = rhs;       }
    /// Set clipping path.
    void setClippingPath( const PdfePath& rhs )     {   m_clippingPath = rhs;   }
    /// Set line's width.
    void setLineWidth( double rhs ) {   m_lineWidth = rhs;  }
    /// Set line's cap style.
    void setLineCap( int rhs )      {   m_lineCap = rhs;    }
    /// Set line's join style.
    void setLineJoin( int rhs )     {   m_lineJoin = rhs;   }
    /// Set miter limit.
    void setMiterLimit( double rhs ){   m_miterLimit = rhs; }

    /// Set compatibility mode.
    void setCompatibilityMode( bool rhs )   {   m_compatibilityMode = rhs;  }

private:
    /// PDF text graphics state.
    mutable PdfeTextState*  m_pTextState;

    /// Transformation matrix (op: cm).
    PdfeMatrix  m_transMat;
    /// Clipping path.
    PdfePath  m_clippingPath;
    /// Color space.
    // TODO.
    /// Color.
    // TODO.
    /// Line width (op: w).
    double  m_lineWidth;
    /// Line cap style (EPdfLineCapStyle) (op: J).
    int  m_lineCap;
    /// Line join style (EPdfLineJoinStyle) (op: j).
    int  m_lineJoin;
    /// maximum length of mitered line joins for stroked paths (op: M).
    double  m_miterLimit;
    /// Color rendering intent.
    // TODO.
    /// Flatness.
    // TODO.

    /// Compatibility mode (BX/EX operator).
    bool  m_compatibilityMode;
};

}

#endif // PDFEGRAPHICSSTATE_H
