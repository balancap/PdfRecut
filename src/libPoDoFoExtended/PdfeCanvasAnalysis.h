/***************************************************************************
 *   Copyright (C) 2011 by Paul Balança                                    *
 *   paul.balanca@gmail.com                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef PDFESTREAMANALYSIS_H
#define PDFESTREAMANALYSIS_H

#include "PdfeGraphicsState.h"
#include "PdfeResources.h"
#include "podofo/podofo.h"

namespace PoDoFo {
    class PdfPage;
    class PdfCanvas;
    class PdfXObject;
}

namespace PdfRecut {

/** Simple structure that gathers information related to the current state of a stream.
 */
struct PdfeStreamState
{
    /// Canvas analysed.
    PoDoFo::PdfCanvas* canvas;

    /// PdfResources associated.
    PdfeResources resources;

    /// Graphics operator read.
    PdfeGraphicOperator gOperator;
    /// Graphics operands associated to the operator.
    std::vector<std::string> gOperands;

    /// Graphics state stack.
    std::vector<PdfeGraphicsState> gStates;
};

/** Basic class used to analysis contents of a PDF canvas.
 */
class PdfeCanvasAnalysis
{
public:
    /** Default empty constructor.
     */
    PdfeCanvasAnalysis();

    /** Default destructor.
     */
    virtual ~PdfeCanvasAnalysis();

    /** Analyse the content stream of a canvas.
     * \param canvas Canvas to analyse.
     * \param initialGState Initial graphics state to use.
     * \param initialResources Initial resources to use.
     */
    void analyseContents( PoDoFo::PdfCanvas* canvas,
                          const PdfeGraphicsState& initialGState,
                          const PdfeResources& initialResources );

    virtual void fGeneralGState( const PdfeStreamState& streamState ) = 0;

    virtual void fSpecialGState( const PdfeStreamState& streamState ) = 0;

    virtual void fPathConstruction( const PdfeStreamState& streamState,
                                    const PdfePath& currentPath ) = 0;

    virtual void fPathPainting( const PdfeStreamState& streamState,
                                const PdfePath& currentPath ) = 0;

    virtual void fClippingPath( const PdfeStreamState& streamState,
                                const PdfePath& currentPath ) = 0;

    virtual void fTextObjects( const PdfeStreamState& streamState ) = 0;

    virtual void fTextState( const PdfeStreamState& streamState ) = 0;

    virtual void fTextPositioning( const PdfeStreamState& streamState ) = 0;

    virtual void fTextShowing( const PdfeStreamState& streamState ) = 0;

    virtual void fType3Fonts( const PdfeStreamState& streamState ) = 0;

    virtual void fColor( const PdfeStreamState& streamState ) = 0;

    virtual void fShadingPatterns( const PdfeStreamState& streamState ) = 0;

    virtual void fInlineImages( const PdfeStreamState& streamState ) = 0;

    virtual void fXObjects( const PdfeStreamState& streamState ) = 0;

    virtual void fMarkedContents( const PdfeStreamState& streamState ) = 0;

    virtual void fCompatibility( const PdfeStreamState& streamState ) = 0;

    virtual void fUnknown( const PdfeStreamState& streamState ) = 0;

    virtual void fFormBegin( const PdfeStreamState& streamState,
                             PoDoFo::PdfXObject* form ) = 0;

    virtual void fFormEnd( const PdfeStreamState& streamState,
                           PoDoFo::PdfXObject* form ) = 0;

protected:
    /** Read an integer from a string.
     * \param str String to analyse.
     * \param value Value read.
     */
    void readValue( const std::string& str, int& value );

    /** Read a double from a string.
     * \param str String to analyse.
     * \param value Value read.
     */
    void readValue( const std::string& str, double& value );

protected:
    /// Istringstream used in conversion string -> number.
    std::istringstream  m_iStrStream;
};

//**********************************************************//
//                      Inline methods                      //
//**********************************************************//
inline void PdfeCanvasAnalysis::readValue( const std::string& str, int& value )
{
    // Clear error state and set string.
    m_iStrStream.clear();
    m_iStrStream.str( str );
    if( !( m_iStrStream >> value ) )
    {
        std::cout << "Error: " << str << std::endl;
        m_iStrStream.clear(); // clear error state
        PODOFO_RAISE_ERROR_INFO( PoDoFo::ePdfError_InvalidDataType, str.c_str() );
    }
}
inline void PdfeCanvasAnalysis::readValue( const std::string& str, double& value )
{
    // Clear error state and set string.
    m_iStrStream.clear();
    m_iStrStream.str( str );
    if( !( m_iStrStream >> value ) )
    {
        std::cout << "Error: " << str << std::endl;
        m_iStrStream.clear(); // clear error state
        PODOFO_RAISE_ERROR_INFO( PoDoFo::ePdfError_InvalidDataType, str.c_str() );
    }
}

}

#endif // PDFESTREAMANALYSIS_H
