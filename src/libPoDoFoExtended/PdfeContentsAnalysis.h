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

#ifndef PDFECONTENTSANALYSIS_H
#define PDFECONTENTSANALYSIS_H

#include "PdfeContentsStream.h"

namespace PoDoFo {
    class PdfXObject;
}

namespace PoDoFoExtended {

/** Simple structure that gathers information related to the
 * current state of a stream.
 */
struct PdfeStreamState
{
    /// Contents stream object which is analysed.
    PdfeContentsStream*  pStream;
    /// Current node.
    PdfeContentsStream::Node*  pNode;

    /// Resources at the current state.
    PdfeResources  resources;
    /// Graphics states stack.
    std::vector<PdfeGraphicsState>  gstates;
};

/** Interface used for the analysis of a content stream,
 * represented by an object of type PdfeContentsStream.
 * The class is pure virtual.
 */
class PdfeContentsAnalysis
{
public:
    /** Default empty constructor.
     */
    PdfeContentsAnalysis();
    /** Copy constructor.
     */
    PdfeContentsAnalysis( const PdfeContentsAnalysis& rhs );
    /** Copy operator=.
     */
    PdfeContentsAnalysis& operator=( const PdfeContentsAnalysis& rhs );
    /** Default destructor.
     */
    virtual ~PdfeContentsAnalysis() = 0;

protected:
    /** Analyse a contents stream.
     * \param stream Contents stream to analyse.
     */
    void analyseContents( const PdfeContentsStream& stream );

    // PdfeContentsAnalysis interface.
    virtual void fGeneralGState( const PdfeStreamState& streamState );
    virtual void fSpecialGState( const PdfeStreamState& streamState );
    virtual void fPathConstruction( const PdfeStreamState& streamState,
                                    const PdfePath& currentPath );
    virtual void fPathPainting( const PdfeStreamState& streamState,
                                const PdfePath& currentPath );
    virtual void fClippingPath( const PdfeStreamState& streamState,
                                const PdfePath& currentPath );
    virtual void fTextObjects( const PdfeStreamState& streamState );
    virtual void fTextState( const PdfeStreamState& streamState );
    virtual void fTextPositioning( const PdfeStreamState& streamState );
    virtual PdfeVector fTextShowing( const PdfeStreamState& streamState );
    virtual void fType3Fonts( const PdfeStreamState& streamState );
    virtual void fColor( const PdfeStreamState& streamState );
    virtual void fShadingPatterns( const PdfeStreamState& streamState );
    virtual void fInlineImages( const PdfeStreamState& streamState );
    virtual void fXObjects( const PdfeStreamState& streamState );
    virtual void fMarkedContents( const PdfeStreamState& streamState );
    virtual void fCompatibility( const PdfeStreamState& streamState );
    virtual void fUnknown( const PdfeStreamState& streamState );
    virtual void fFormBegin( const PdfeStreamState& streamState,
                             PoDoFo::PdfXObject* form );
    virtual void fFormEnd( const PdfeStreamState& streamState,
                           PoDoFo::PdfXObject* form );

};

}

#endif // PDFECONTENTSANALYSIS_H
