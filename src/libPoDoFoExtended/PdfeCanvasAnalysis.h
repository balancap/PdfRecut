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

namespace PoDoFoExtended {

/** Simple structure that gathers information related to the current state of a stream.
 */
struct PdfeStreamStateOld
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
    /** Copy constructor.
     * \param rhs Object to copy.
     */
    PdfeCanvasAnalysis( const PdfeCanvasAnalysis& rhs );
    /** Copy operator=.
     * \param rhs Object to copy.
     */
    PdfeCanvasAnalysis& operator=( const PdfeCanvasAnalysis& rhs );

    /** Default destructor.
     */
    virtual ~PdfeCanvasAnalysis();

protected:
    /** Analyse the content stream of a canvas.
     * \param canvas Canvas to analyse.
     * \param initialGState Initial graphics state to use.
     * \param initialResources Initial resources to use.
     */
    void analyseContents( PoDoFo::PdfCanvas* canvas,
                          const PdfeGraphicsState& initialGState,
                          const PdfeResources& initialResources );

    // PdfeCanvasAnalysis interface.
    virtual void fGeneralGState( const PdfeStreamStateOld& streamState ) = 0;

    virtual void fSpecialGState( const PdfeStreamStateOld& streamState ) = 0;

    virtual void fPathConstruction( const PdfeStreamStateOld& streamState,
                                    const PdfePath& currentPath ) = 0;

    virtual void fPathPainting( const PdfeStreamStateOld& streamState,
                                const PdfePath& currentPath ) = 0;

    virtual void fClippingPath( const PdfeStreamStateOld& streamState,
                                const PdfePath& currentPath ) = 0;

    virtual void fTextObjects( const PdfeStreamStateOld& streamState ) = 0;

    virtual void fTextState( const PdfeStreamStateOld& streamState ) = 0;

    virtual void fTextPositioning( const PdfeStreamStateOld& streamState ) = 0;

    virtual PdfeVector fTextShowing( const PdfeStreamStateOld& streamState ) = 0;

    virtual void fType3Fonts( const PdfeStreamStateOld& streamState ) = 0;

    virtual void fColor( const PdfeStreamStateOld& streamState ) = 0;

    virtual void fShadingPatterns( const PdfeStreamStateOld& streamState ) = 0;

    virtual void fInlineImages( const PdfeStreamStateOld& streamState ) = 0;

    virtual void fXObjects( const PdfeStreamStateOld& streamState ) = 0;

    virtual void fMarkedContents( const PdfeStreamStateOld& streamState ) = 0;

    virtual void fCompatibility( const PdfeStreamStateOld& streamState ) = 0;

    virtual void fUnknown( const PdfeStreamStateOld& streamState ) = 0;

    virtual void fFormBegin( const PdfeStreamStateOld& streamState,
                             PoDoFo::PdfXObject* form ) = 0;

    virtual void fFormEnd( const PdfeStreamStateOld& streamState,
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

private:
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
