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

#ifndef PRSTREAMLAYOUTZONE_H
#define PRSTREAMLAYOUTZONE_H

#include "PRStreamAnalysis.h"
#include "PRDocumentLayout.h"
#include "PdfMisc.h"

namespace PoDoFo {
    class PdfPage;
    class PdfMemStream;
}

namespace PdfRecut {

/** Class used to generate a Pdf stream which corresponds to a given
 * document layout zone from a Pdf page.
 */
class PRStreamLayoutZone : public PRStreamAnalysis
{
public:
    /** Default constructor.
     * \param pageIn Input page to analyse.
     * \param streamOut Output stream to generate.
     * \param zone Pdf zone corresponding to the output stream.
     * \param Resource prefix to be used in the output stream.
     */
    PRStreamLayoutZone( PoDoFo::PdfPage* pageIn,
                        PoDoFo::PdfStream* streamOut,
                        const PRPageZone& zone,
                        const PRLayoutParameters& parameters,
                        const std::string& resPrefix );

    void generateStream();

    void fGeneralGState( const PdfStreamState& streamState );

    void fSpecialGState( const PdfStreamState& streamState );

    void fPathConstruction( const PdfStreamState& streamState,
                            const PdfPath& currentPath );

    void fPathPainting( const PdfStreamState& streamState,
                        const PdfPath& currentPath );

    void fClippingPath( const PdfStreamState& streamState,
                        const PdfPath& currentPath );

    void fTextObjects( const PdfStreamState& streamState );

    void fTextState( const PdfStreamState& streamState );

    void fTextPositioning( const PdfStreamState& streamState );

    void fTextShowing( const PdfStreamState& streamState );

    void fType3Fonts( const PdfStreamState& streamState );

    void fColor( const PdfStreamState& streamState );

    void fShadingPatterns( const PdfStreamState& streamState );

    void fInlineImages( const PdfStreamState& streamState );

    void fXObjects( const PdfStreamState& streamState );

    void fMarkedContents( const PdfStreamState& streamState );

    void fCompatibility( const PdfStreamState& streamState );



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
    PRPageZone m_zone;
    /** Resource prefix.
     */
    std::string m_resPrefix;
    /** Layout parameters.
     */
    PRLayoutParameters m_parameters;

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
inline void PRStreamLayoutZone::copyVariables( const std::vector<std::string>& vecVariables,
                                               std::string& buffer )
{
    for( size_t i = 0 ; i < vecVariables.size() ; i++ ) {
        buffer += vecVariables[i];
        buffer += " ";
    }
}

}

#endif // PRSTREAMLAYOUTZONE_H
