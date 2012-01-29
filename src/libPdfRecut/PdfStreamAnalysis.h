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

#ifndef PDFSTREAMANALYSIS_H
#define PDFSTREAMANALYSIS_H

#include "PdfGraphicsState.h"
#include "podofo/podofo.h"

namespace PoDoFo {
    class PdfPage;
}

namespace PdfeBooker {

/** Basic class used to analysis the contents of a page's streams.
 */
class PdfStreamAnalysis
{
public:
    /** Default constructor, with a page assigned.
     * \param page  PoDoFo page to be analysed.
     */
    PdfStreamAnalysis( PoDoFo::PdfPage* page );

    /** Analysing the content streams of the page.
     */
    void analyse();

    virtual void fGeneralGState( const PdfGraphicOperator& gOperator,
                                 const std::vector<std::string>& vecVariables,
                                 const std::vector<PdfGraphicsState>& vecGStates ) = 0;

    virtual void fSpecialGState( const PdfGraphicOperator& gOperator,
                                 const std::vector<std::string>& vecVariables,
                                 const std::vector<PdfGraphicsState>& vecGStates ) = 0;

    virtual void fPathConstruction( const PdfGraphicOperator& gOperator,
                                    const std::vector<std::string>& vecVariables,
                                    const std::vector<PdfGraphicsState>& vecGStates,
                                    const PdfPath& currentPath ) = 0;

    virtual void fPathPainting( const PdfGraphicOperator& gOperator,
                                const std::vector<std::string>& vecVariables,
                                const std::vector<PdfGraphicsState>& vecGStates,
                                const PdfPath& currentPath ) = 0;

    virtual void fClippingPath( const PdfGraphicOperator& gOperator,
                                const std::vector<std::string>& vecVariables,
                                const std::vector<PdfGraphicsState>& vecGStates,
                                const PdfPath& currentPath ) = 0;

    virtual void fTextObjects( const PdfGraphicOperator& gOperator,
                               const std::vector<std::string>& vecVariables,
                               const std::vector<PdfGraphicsState>& vecGStates ) = 0;

    virtual void fTextState( const PdfGraphicOperator& gOperator,
                             const std::vector<std::string>& vecVariables,
                             const std::vector<PdfGraphicsState>& vecGStates ) = 0;

    virtual void fTextPositioning( const PdfGraphicOperator& gOperator,
                                   const std::vector<std::string>& vecVariables,
                                   const std::vector<PdfGraphicsState>& vecGStates ) = 0;

    virtual void fTextShowing( const PdfGraphicOperator& gOperator,
                               const std::vector<std::string>& vecVariables,
                               const std::vector<PdfGraphicsState>& vecGStates ) = 0;

    virtual void fType3Fonts( const PdfGraphicOperator& gOperator,
                              const std::vector<std::string>& vecVariables,
                              const std::vector<PdfGraphicsState>& vecGStates ) = 0;

    virtual void fColor( const PdfGraphicOperator& gOperator,
                         const std::vector<std::string>& vecVariables,
                         const std::vector<PdfGraphicsState>& vecGStates ) = 0;

    virtual void fShadingPatterns( const PdfGraphicOperator& gOperator,
                                   const std::vector<std::string>& vecVariables,
                                   const std::vector<PdfGraphicsState>& vecGStates ) = 0;

    virtual void fInlineImages( const PdfGraphicOperator& gOperator,
                                const std::vector<std::string>& vecVariables,
                                const std::vector<PdfGraphicsState>& vecGStates ) = 0;

    virtual void fXObjects( const PdfGraphicOperator& gOperator,
                            const std::vector<std::string>& vecVariables,
                            const std::vector<PdfGraphicsState>& vecGStates ) = 0;

    virtual void fMarkedContents( const PdfGraphicOperator& gOperator,
                                  const std::vector<std::string>& vecVariables,
                                  const std::vector<PdfGraphicsState>& vecGStates ) = 0;

    virtual void fCompatibility( const PdfGraphicOperator& gOperator,
                                 const std::vector<std::string>& vecVariables,
                                 const std::vector<PdfGraphicsState>& vecGStates ) = 0;

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
    /** Page analysed.
     */
    PoDoFo::PdfPage* m_page;

    /** Istringstream used in conversion string->number.
     */
    std::istringstream m_iStrStream;
};

//**********************************************************//
//                      Inline methods                      //
//**********************************************************//
inline void PdfStreamAnalysis::readValue( const std::string& str, int& value )
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
inline void PdfStreamAnalysis::readValue( const std::string& str, double& value )
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

#endif // PDFSTREAMANALYSIS_H
