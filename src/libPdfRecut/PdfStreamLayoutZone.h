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

#ifndef PDFSTREAMLAYOUTZONE_H
#define PDFSTREAMLAYOUTZONE_H

#include "PdfStreamAnalysis.h"
#include "PdfDocumentLayout.h"
#include "PdfMisc.h"

namespace PoDoFo {
    class PdfPage;
    class PdfMemStream;
}

namespace PdfeBooker {

/** Class used to generate a Pdf stream which corresponds to a given
 * document layout zone from a Pdf page.
 */
class PdfStreamLayoutZone : public PdfStreamAnalysis
{
public:
    /** Default constructor.
     * \param pageIn Input page to analyse.
     * \param streamOut Output stream to generate.
     * \param zone Pdf zone corresponding to the output stream.
     * \param Resource prefix to be used in the output stream.
     */
    PdfStreamLayoutZone( PoDoFo::PdfPage* pageIn,
                         PoDoFo::PdfStream* streamOut,
                         const PdfPageZone& zone,
                         const PdfLayoutParameters& parameters,
                         const std::string& resPrefix );

    void generateStream();

    void fGeneralGState( const PdfGraphicOperator& gOperator,
                         const std::vector<std::string>& vecVariables,
                         const std::vector<PdfGraphicsState>& vecGStates );

    void fSpecialGState( const PdfGraphicOperator& gOperator,
                         const std::vector<std::string>& vecVariables,
                         const std::vector<PdfGraphicsState>& vecGStates );

    void fPathConstruction( const PdfGraphicOperator& gOperator,
                            const std::vector<std::string>& vecVariables,
                            const std::vector<PdfGraphicsState>& vecGStates,
                            const PdfPath& currentPath );

    void fPathPainting( const PdfGraphicOperator& gOperator,
                        const std::vector<std::string>& vecVariables,
                        const std::vector<PdfGraphicsState>& vecGStates,
                        const PdfPath& currentPath );

    void fClippingPath( const PdfGraphicOperator& gOperator,
                        const std::vector<std::string>& vecVariables,
                        const std::vector<PdfGraphicsState>& vecGStates,
                        const PdfPath& currentPath );

    void fTextObjects( const PdfGraphicOperator& gOperator,
                       const std::vector<std::string>& vecVariables,
                       const std::vector<PdfGraphicsState>& vecGStates );

    void fTextState( const PdfGraphicOperator& gOperator,
                     const std::vector<std::string>& vecVariables,
                     const std::vector<PdfGraphicsState>& vecGStates );

    void fTextPositioning( const PdfGraphicOperator& gOperator,
                           const std::vector<std::string>& vecVariables,
                           const std::vector<PdfGraphicsState>& vecGStates );

    void fTextShowing( const PdfGraphicOperator& gOperator,
                       const std::vector<std::string>& vecVariables,
                       const std::vector<PdfGraphicsState>& vecGStates );

    void fType3Fonts( const PdfGraphicOperator& gOperator,
                      const std::vector<std::string>& vecVariables,
                      const std::vector<PdfGraphicsState>& vecGStates );

    void fColor( const PdfGraphicOperator& gOperator,
                 const std::vector<std::string>& vecVariables,
                 const std::vector<PdfGraphicsState>& vecGStates );

    void fShadingPatterns( const PdfGraphicOperator& gOperator,
                           const std::vector<std::string>& vecVariables,
                           const std::vector<PdfGraphicsState>& vecGStates );

    void fInlineImages( const PdfGraphicOperator& gOperator,
                        const std::vector<std::string>& vecVariables,
                        const std::vector<PdfGraphicsState>& vecGStates );

    void fXObjects( const PdfGraphicOperator& gOperator,
                    const std::vector<std::string>& vecVariables,
                    const std::vector<PdfGraphicsState>& vecGStates );

    void fMarkedContents( const PdfGraphicOperator& gOperator,
                          const std::vector<std::string>& vecVariables,
                          const std::vector<PdfGraphicsState>& vecGStates );

    void fCompatibility( const PdfGraphicOperator& gOperator,
                         const std::vector<std::string>& vecVariables,
                         const std::vector<PdfGraphicsState>& vecGStates );

protected:
    /** Copy variables to a buffer.
     */
    void copyVariables( const std::vector<std::string>& vecVariables, std::string& buffer );

protected:
    /** Output stream.
     */
    PoDoFo::PdfMemStream* m_streamOut;
    /** Pdf page zone.
     */
    PdfPageZone m_zone;
    /** Resource prefix.
     */
    std::string m_resPrefix;
    /** Layout parameters.
     */
    PdfLayoutParameters m_parameters;

    /** Pdf stream buffer.
     */
    PdfOStringStream m_bufStream;
    /** String buffer.
     */
    std::string m_bufString;

    // Temp variables used during analysis.
    /** Key/values of an inline image.
     */
    std::vector<std::string> m_keyValuesII;
};

//**********************************************************//
//                      Inline methods                      //
//**********************************************************//
inline void PdfStreamLayoutZone::copyVariables( const std::vector<std::string>& vecVariables,
                                                std::string& buffer )
{
    for( size_t i = 0 ; i < vecVariables.size() ; i++ ) {
        buffer += vecVariables[i];
        buffer += " ";
    }
}

}

#endif // PDFSTREAMLAYOUTZONE_H
