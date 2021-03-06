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

    void fGeneralGState( const PoDoFoExtended::PdfeStreamStateOld& streamState );

    void fSpecialGState( const PoDoFoExtended::PdfeStreamStateOld& streamState );

    void fPathConstruction( const PoDoFoExtended::PdfeStreamStateOld& streamState,
                            const PoDoFoExtended::PdfePath& currentPath );

    void fPathPainting( const PoDoFoExtended::PdfeStreamStateOld& streamState,
                        const PoDoFoExtended::PdfePath& currentPath );

    void fClippingPath( const PoDoFoExtended::PdfeStreamStateOld& streamState,
                        const PoDoFoExtended::PdfePath& currentPath );

    void fTextObjects( const PoDoFoExtended::PdfeStreamStateOld& streamState );

    void fTextState( const PoDoFoExtended::PdfeStreamStateOld& streamState );

    void fTextPositioning( const PoDoFoExtended::PdfeStreamStateOld& streamState );

    PdfeVector fTextShowing( const PoDoFoExtended::PdfeStreamStateOld& streamState );

    void fType3Fonts( const PoDoFoExtended::PdfeStreamStateOld& streamState );

    void fColor( const PoDoFoExtended::PdfeStreamStateOld& streamState );

    void fShadingPatterns( const PoDoFoExtended::PdfeStreamStateOld& streamState );

    void fInlineImages( const PoDoFoExtended::PdfeStreamStateOld& streamState );

    void fXObjects( const PoDoFoExtended::PdfeStreamStateOld& streamState );

    void fMarkedContents( const PoDoFoExtended::PdfeStreamStateOld& streamState );

    void fCompatibility( const PoDoFoExtended::PdfeStreamStateOld& streamState );

    void fUnknown( const PoDoFoExtended::PdfeStreamStateOld& streamState );

    void fFormBegin( const PoDoFoExtended::PdfeStreamStateOld& streamState,
                     PoDoFo::PdfXObject* form );

    void fFormEnd( const PoDoFoExtended::PdfeStreamStateOld& streamState,
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
