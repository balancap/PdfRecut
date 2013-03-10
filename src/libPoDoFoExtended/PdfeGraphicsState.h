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
#include "PdfeResources.h"

namespace PoDoFo {
    class PdfPage;
}

namespace PoDoFoExtended {

/** Class describing a text state in a PDF stream, i.e.
 * containing every information related to text rendering.
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
    /// Set rendering mode.
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

/** Structure describing a graphics state in a Pdf stream.
 * This structure does not reproduce all properties described the Pdf reference.
 */
struct PdfeGraphicsState
{
    /// Transformation matrix (set with cm).
    PdfeMatrix transMat;

    /// Clipping path.
    PdfePath clippingPath;

    /// Pdf Text graphics state.
    PdfeTextState textState;

    /// Line width.
    double lineWidth;
    /// Line cap style.
    int lineCap;
    /// Line join style.
    int lineJoin;
    /// Compatibility mode.
    bool compatibilityMode;

    /** Default constructor.
     */
    PdfeGraphicsState();

    /** Function which initializes members to default Pdf values.
     */
    void init();

    /** Import parameters from ExtGState.
     * \param resources Resources where to find to the GState.
     * \param gsName Name of the graphics state to import.
     * \return True if the extGState was found and loaded. False else.
     */
    bool importExtGState( const PdfeResources& resources, const std::string& gsName );

    /** Import the Pdf reference corresponding to the font object (with name fontName).
     * \param resources Resources where to find the font reference.
     */
    bool importFontReference( const PdfeResources& resources );
};

}

#endif // PDFEGRAPHICSSTATE_H
