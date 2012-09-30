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

#include "PdfeCanvasAnalysis.h"
#include "PdfeMisc.h"

#include "PRDocumentLayout.h"

namespace PoDoFo {
    class PdfPage;
    class PdfMemStream;
}

namespace PdfRecut {

/** Class used to generate a Pdf stream which corresponds to a given
 * document layout zone from a Pdf page.
 */
class PRStreamLayoutZone : public PoDoFoExtended::PdfeCanvasAnalysis
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
                        PoDoFoExtended::PdfeResources* resourcesOut,
                        const PRPageZone& zone,
                        const PRLayoutParameters& parameters,
                        const std::string& resSuffixe );

    void generateStream();

    void fGeneralGState( const PoDoFoExtended::PdfeStreamState& streamState );

    void fSpecialGState( const PoDoFoExtended::PdfeStreamState& streamState );

    void fPathConstruction( const PoDoFoExtended::PdfeStreamState& streamState,
                            const PoDoFoExtended::PdfePath& currentPath );

    void fPathPainting( const PoDoFoExtended::PdfeStreamState& streamState,
                        const PoDoFoExtended::PdfePath& currentPath );

    void fClippingPath( const PoDoFoExtended::PdfeStreamState& streamState,
                        const PoDoFoExtended::PdfePath& currentPath );

    void fTextObjects( const PoDoFoExtended::PdfeStreamState& streamState );

    void fTextState( const PoDoFoExtended::PdfeStreamState& streamState );

    void fTextPositioning( const PoDoFoExtended::PdfeStreamState& streamState );

    void fTextShowing( const PoDoFoExtended::PdfeStreamState& streamState );

    void fType3Fonts( const PoDoFoExtended::PdfeStreamState& streamState );

    void fColor( const PoDoFoExtended::PdfeStreamState& streamState );

    void fShadingPatterns( const PoDoFoExtended::PdfeStreamState& streamState );

    void fInlineImages( const PoDoFoExtended::PdfeStreamState& streamState );

    void fXObjects( const PoDoFoExtended::PdfeStreamState& streamState );

    void fMarkedContents( const PoDoFoExtended::PdfeStreamState& streamState );

    void fCompatibility( const PoDoFoExtended::PdfeStreamState& streamState );

    void fUnknown( const PoDoFoExtended::PdfeStreamState& streamState );

    void fFormBegin( const PoDoFoExtended::PdfeStreamState& streamState,
                     PoDoFo::PdfXObject* form );

    void fFormEnd( const PoDoFoExtended::PdfeStreamState& streamState,
                   PoDoFo::PdfXObject* form );

    /** Get the list of form objects from the page.
     */
    std::vector<PoDoFo::PdfObject*> getFormObjects();

protected:
    /** Copy variables to a buffer.
     */
    void copyVariables( const std::vector<std::string>& vecVariables, std::string& buffer );

    /** Add out resources key.
     */
    void addResourcesOutKey( PoDoFoExtended::PdfeResourcesType::Enum resourceType,
                             const std::string& key,
                             const PoDoFoExtended::PdfeResources& resourcesIn );

    /** Push form.
     */
    void pushForm();
    /** Pop form.
     */
    void popForm();

    /** Get suffixe.
     */
    std::string getSuffixe();

protected:
    /// Input page.
    PoDoFo::PdfPage*  m_pageIn;

    /// Output stream.
    PoDoFo::PdfMemStream*  m_streamOut;
    /// Output resources.
    PoDoFoExtended::PdfeResources*  m_resourcesOut;

    /// PDF page zone.
    PRPageZone  m_zone;

    /// Resource suffixe.
    std::string  m_resSuffixe;
    /// Form suffixe.
    std::string  m_formSuffixe;

    /// Form stack of indexes.
    std::vector<int>  m_formsStack;
    /// Number of forms.
    int  m_formsNb;
    /// Forms objects seen in the page.
    std::vector<PoDoFo::PdfObject*>  m_formObjects;

    /// Layout parameters.
    PRLayoutParameters m_parameters;

    /// Pdf stream buffer.
    PoDoFoExtended::PdfeOStringStream m_bufStream;
    /// String buffer.
    std::string m_bufString;

    // Temp variables used during analysis.
    /// Key/values of an inline image.
    std::vector<std::string> m_keyValuesII;
};

//**********************************************************//
//                      Inline methods                      //
//**********************************************************//
inline std::vector<PoDoFo::PdfObject*> PRStreamLayoutZone::getFormObjects()
{
    return m_formObjects;
}
inline void PRStreamLayoutZone::copyVariables( const std::vector<std::string>& vecVariables,
                                               std::string& buffer )
{
    for( size_t i = 0 ; i < vecVariables.size() ; i++ ) {
        buffer += vecVariables[i];
        buffer += " ";
    }
}
inline std::string PRStreamLayoutZone::getSuffixe()
{
    return m_resSuffixe + m_formSuffixe;
}

}

#endif // PRSTREAMLAYOUTZONE_H
