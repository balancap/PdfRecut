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

#include <podofo/podofo.h>
#include "PRStreamLayoutZone.h"

#define BUFFER_SIZE 4096

using namespace PoDoFo;
using namespace PoDoFoExtended;

namespace PdfRecut {

PRStreamLayoutZone::PRStreamLayoutZone( PoDoFo::PdfPage* pageIn,
                                        PoDoFo::PdfStream* streamOut,
                                        PdfeResources* resourcesOut,
                                        const PRPageZone &zone,
                                        const PRLayoutParameters &parameters, const std::string &resSuffixe ) :
    PdfeCanvasAnalysis(), m_pageIn( pageIn ), m_zone ( zone ), m_resSuffixe ( resSuffixe ), m_parameters( parameters )
{
    // Downcasting to PdfMemStream pointer.resource
    m_streamOut = dynamic_cast<PdfMemStream*>( streamOut );
    m_resourcesOut = resourcesOut;

    m_bufString.reserve( BUFFER_SIZE );
}

void PRStreamLayoutZone::generateStream()
{
    // Zone Out coordinates and transformation.
    PdfeMatrix zoneOutTrMatrix;
    zoneOutTrMatrix(2,0) = m_zone.leftZoneOut - m_zone.zoneIn.GetLeft();
    zoneOutTrMatrix(2,1) = m_zone.bottomZoneOut - m_zone.zoneIn.GetBottom();

    // Form initialization.
    m_formsNb = 0;
    m_formObjects.clear();
    this->pushForm();

    // Initialize output stream
    TVecFilters vecFilters;
    vecFilters.push_back( ePdfFilter_FlateDecode );
    m_streamOut->BeginAppend( vecFilters, true, true );
    m_streamOut->Append("q\n");

    // Add clipping path which corresponds to zone.
    if( m_parameters.zoneClippingPath )
    {
        m_bufStream << m_zone.leftZoneOut << " "
                    << m_zone.bottomZoneOut << " "
                    << m_zone.zoneIn.GetWidth() << " "
                    << m_zone.zoneIn.GetHeight() << " re W n\n";
        m_streamOut->Append( m_bufStream.str() );
        m_bufStream.str( "" );
    }
    // First transformation matrix, related to zone coordinates.
    m_bufStream << zoneOutTrMatrix(0,0) << " " << zoneOutTrMatrix(0,1) << " "
                << zoneOutTrMatrix(1,0) << " " << zoneOutTrMatrix(1,1) << " "
                << zoneOutTrMatrix(2,0) << " " << zoneOutTrMatrix(2,1) << " cm\n";
    m_streamOut->Append( m_bufStream.str() );
    m_bufStream.str( "" );

    // Perform the analysis.
    this->analyseContents( m_pageIn, PdfeGraphicsState(), PdfeResources() );

    // Close stream.
    m_streamOut->Append("Q\n");
    m_streamOut->EndAppend();


    //    std::ostringstream bufs;
    //    PdfOutputDevice out(&bufs);

    //    m_streamOut->Write( &out );
    //    std::cout << bufs.str() << std::endl;
}

void PRStreamLayoutZone::fGeneralGState( const PdfeStreamStateOld& streamState )
{
    // Simpler references.
    const PdfeGraphicOperator& gOperator = streamState.gOperator;
    const std::vector<std::string>& gOperands = streamState.gOperands;

    m_bufStream.str("");
    m_bufString.clear();

    if( gOperator.type() == PdfeGOperator::gs ) {
        // Specific case of command "gs": add resource suffixe.
        m_bufString += "/";
        m_bufString += gOperands.back().substr( 1 );
        m_bufString += this->getSuffixe();
        m_bufString += " gs\n";

        // Add key to out resources.
        this->addResourcesOutKey( PdfeResourcesType::ExtGState,
                                  gOperands.back().substr( 1 ),
                                  streamState.resources );
    }
    else {
        // Copy variables and operator.
        this->copyVariables( gOperands, m_bufString );
        m_bufString += gOperator.str();
        m_bufString += "\n";
    }
    m_streamOut->Append( m_bufString );
}

void PRStreamLayoutZone::fSpecialGState( const PdfeStreamStateOld& streamState )
{
    // Simpler references.
    const PdfeGraphicOperator& gOperator = streamState.gOperator;
    const std::vector<std::string>& gOperands = streamState.gOperands;

    m_bufString.clear();

    // Copy variables and operator.
    this->copyVariables( gOperands, m_bufString );
    m_bufString += gOperator.str();
    m_bufString += "\n";
    m_streamOut->Append( m_bufString );
}

void PRStreamLayoutZone::fPathConstruction( const PdfeStreamStateOld& streamState,
                                            const PdfePath& currentPath )
{
    // Everything is done in painting function !
}

void PRStreamLayoutZone::fPathPainting( const PdfeStreamStateOld& streamState,
                                        const PdfePath& currentPath )
{
    // Simpler references.
    const PdfeGraphicOperator& gOperator = streamState.gOperator;
    const PdfeGraphicsState& gState = streamState.gStates.back();

    // Subpaths from the current path.
    std::vector<PdfeSubPath> subpaths = currentPath.subpaths();
    std::vector<PdfeSubPath> subpathsBack( subpaths );

    // Temp variable, including inverse transformation matrix.
    bool inZone;
    size_t idx = 0;
    PdfeMatrix invTransMat;
    gState.transMat.inverse( invTransMat );

    // Keep subpaths which intersect the zone, and reduce them if necessary.
    std::vector<PdfeSubPath>::iterator it;
    for( it = subpaths.begin() ; it != subpaths.end() ; )
    {
        // Apply transformation matrix.
        it->applyTransMatrix( gState.transMat );

        // Does the subpath intersect the zone.
        inZone = it->intersectZone( m_zone.zoneIn, m_parameters.pathStrictlyInside );

        if( inZone ) {
            // Reduce to zone if asked, and if not already strictly inside the zone.
            if( m_parameters.pathReduce && !m_parameters.pathStrictlyInside ) {
                it->reduceToZone( m_zone.zoneIn );

                // Apply inverse matrix to get points in current space.
                it->applyTransMatrix( invTransMat );
            }
            else {
                // Simply get back original subpath.
                (*it) = subpathsBack[idx];
            }

            it++;
        }
        else {
            // Remove the subpath.
            it = subpaths.erase( it );
        }
        idx++;
    }

    // Draw subpaths kept in the vector.
    m_bufStream.str( "" );
    for( size_t i = 0 ; i < subpaths.size() ; i++ ) {
        // Points in the subpath.
        for( size_t j = 0 ; j < subpaths[i].nbPoints() ; j++ ) {
            // Distinction between different painting operators.
            if( subpaths[i].pointOp(j) == PdfePathOperators::m ) {
                m_bufStream << subpaths[i].pointCoord(j)(0) << " ";
                m_bufStream << subpaths[i].pointCoord(j)(1) << " ";
                m_bufStream << "m\n";
            }
            else if( subpaths[i].pointOp(j) == PdfePathOperators::l ) {
                m_bufStream << subpaths[i].pointCoord(j)(0) << " ";
                m_bufStream << subpaths[i].pointCoord(j)(1) << " ";
                m_bufStream << "l\n";
            }
            else if( subpaths[i].pointOp(j) == PdfePathOperators::c ) {
                m_bufStream << subpaths[i].pointCoord(j)(0) << " ";
                m_bufStream << subpaths[i].pointCoord(j)(1) << " ";
                m_bufStream << subpaths[i].pointCoord(j+1)(0) << " ";
                m_bufStream << subpaths[i].pointCoord(j+1)(1) << " ";
                m_bufStream << subpaths[i].pointCoord(j+2)(0) << " ";
                m_bufStream << subpaths[i].pointCoord(j+2)(1) << " ";
                m_bufStream << "c\n";
                j+=2;
            }
            else if( subpaths[i].pointOp(j) == PdfePathOperators::v ) {
                m_bufStream << subpaths[i].pointCoord(j)(0) << " ";
                m_bufStream << subpaths[i].pointCoord(j)(1) << " ";
                m_bufStream << subpaths[i].pointCoord(j+1)(0) << " ";
                m_bufStream << subpaths[i].pointCoord(j+1)(1) << " ";
                m_bufStream << "v\n";
                j+=1;
            }
            else if( subpaths[i].pointOp(j) == PdfePathOperators::y ) {
                m_bufStream << subpaths[i].pointCoord(j)(0) << " ";
                m_bufStream << subpaths[i].pointCoord(j)(1) << " ";
                m_bufStream << subpaths[i].pointCoord(j+1)(0) << " ";
                m_bufStream << subpaths[i].pointCoord(j+1)(1) << " ";
                m_bufStream << "y\n";
                j+=1;
            }
            else if( subpaths[i].pointOp(j) == PdfePathOperators::h ) {
                m_bufStream << "h\n";
            }
            else if( subpaths[i].pointOp(j) == PdfePathOperators::re ) {
                m_bufStream << subpaths[i].pointCoord(j)(0) << " ";
                m_bufStream << subpaths[i].pointCoord(j)(1) << " ";
                m_bufStream << ( subpaths[i].pointCoord(j+2)(0)-subpaths[i].pointCoord(j)(0) ) << " ";
                m_bufStream << ( subpaths[i].pointCoord(j+2)(1)-subpaths[i].pointCoord(j)(1) ) << " ";
                m_bufStream << "re\n";
                j+=4;
            }
        }
    }
    // Add clipping path operator if necessary.
    std::string clippingPathOp = currentPath.clippingPathOp();
    if( clippingPathOp.length() ) {
        m_bufStream << clippingPathOp << "\n";
    }
    // Painting operator.
    m_bufStream << gOperator.str() << "\n";
    m_streamOut->Append( m_bufStream.str() );
}

void PRStreamLayoutZone::fClippingPath( const PdfeStreamStateOld& streamState,
                                        const PdfePath& currentPath  )
{
}

void PRStreamLayoutZone::fTextObjects( const PdfeStreamStateOld& streamState )
{
    // Simpler references.
    const PdfeGraphicOperator& gOperator = streamState.gOperator;

    // Copy operator.
    m_bufString = gOperator.str();
    m_bufString += "\n";
    m_streamOut->Append( m_bufString );
}

void PRStreamLayoutZone::fTextState( const PdfeStreamStateOld& streamState )
{
    // Simpler references.
    const PdfeGraphicOperator& gOperator = streamState.gOperator;
    const std::vector<std::string>& gOperands = streamState.gOperands;

    if( gOperator.type() == PdfeGOperator::Tf ) {
        // Font: add resource prefix to name.
        m_bufString = "/";
        m_bufString += gOperands[0].substr( 1 );
        m_bufString += this->getSuffixe();
        m_bufString += " ";
        m_bufString += gOperands[1];
        m_bufString += " Tf\n";

        // Add key to out resources.
        this->addResourcesOutKey( PdfeResourcesType::Font,
                                  gOperands[0].substr( 1 ),
                                  streamState.resources );
    }
    else {
        // Copy variables and operator.
        m_bufString.clear();
        this->copyVariables( gOperands, m_bufString );
        m_bufString += gOperator.str();
        m_bufString += "\n";
    }
    m_streamOut->Append( m_bufString );
}

void PRStreamLayoutZone::fTextPositioning( const PdfeStreamStateOld& streamState )
{
    // Simpler references.
    const PdfeGraphicOperator& gOperator = streamState.gOperator;
    const std::vector<std::string>& gOperands = streamState.gOperands;

    // Copy variables and operator.
    m_bufString.clear();
    this->copyVariables( gOperands, m_bufString );
    m_bufString += gOperator.str();
    m_bufString += "\n";
    m_streamOut->Append( m_bufString );
}

PdfeVector PRStreamLayoutZone::fTextShowing( const PdfeStreamStateOld& streamState )
{
    // Simpler references.
    const PdfeGraphicOperator& gOperator = streamState.gOperator;
    const std::vector<std::string>& gOperands = streamState.gOperands;
    const PdfeGraphicsState& gState = streamState.gStates.back();

    // Show text if inside page zone.
    PdfeMatrix tmpMat = gState.textState.transMat * gState.transMat;
    if( tmpMat(2,1) <= m_zone.zoneIn.GetBottom() + m_zone.zoneIn.GetHeight() &&
            tmpMat(2,1) >= m_zone.zoneIn.GetBottom() )
    {
        // Copy variables and operator.
        m_bufString.clear();
        this->copyVariables( gOperands, m_bufString );
        m_bufString += gOperator.str();
        m_bufString += "\n";
        m_streamOut->Append( m_bufString );
    }
    else
    {
        // In the case of operators ' or ", write non-showing code.
        m_bufStream.str( "" );
        if( gOperator.type() == PdfeGOperator::Quote ) {
            m_bufStream << "T*\n";
        }
        else if( gOperator.type() == PdfeGOperator::DoubleQuote ) {
            m_bufStream << gState.textState.wordSpace << "Tw\n"
                        << gState.textState.charSpace << "Tc\n";
        }
        m_streamOut->Append( m_bufStream.str() );
    }
    // TODO: return correct displacement.
    return PdfeVector();
}

void PRStreamLayoutZone::fType3Fonts( const PdfeStreamStateOld& streamState )
{
    // Simpler references.
    const PdfeGraphicOperator& gOperator = streamState.gOperator;
    const std::vector<std::string>& gOperands = streamState.gOperands;

    m_bufString.clear();

    // Copy variables and operator...
    this->copyVariables( gOperands, m_bufString );
    m_bufString += gOperator.str();
    m_bufString += "\n";
    m_streamOut->Append( m_bufString );
}

void PRStreamLayoutZone::fColor( const PdfeStreamStateOld& streamState )
{
    // Simpler references.
    const PdfeGraphicOperator& gOperator = streamState.gOperator;
    const std::vector<std::string>& gOperands = streamState.gOperands;

    m_bufString.clear();
    // Test if the last operand is a PdfName.
    if( gOperands.back()[0] == '/' ) {
        // Copy the sub-vector.
        std::vector<std::string> tmpGOperands( gOperands );
        tmpGOperands.pop_back();
        this->copyVariables( tmpGOperands, m_bufString );

        // Color space resource.
        m_bufString +=  gOperands.back();
        m_bufString += this->getSuffixe();
        m_bufString += " ";
        m_bufString += gOperator.str();
        m_bufString += "\n";

        // Add key to out resources.
        this->addResourcesOutKey( PdfeResourcesType::ColorSpace,
                                  gOperands.back().substr( 1 ),
                                  streamState.resources );
    }
    else {
        // Else, simply copy variables.
        this->copyVariables( gOperands, m_bufString );

        m_bufString += gOperator.str();
        m_bufString += "\n";
    }
    m_streamOut->Append( m_bufString );
}

void PRStreamLayoutZone::fShadingPatterns( const PdfeStreamStateOld& streamState )
{
    // Simpler references.
    const std::vector<std::string>& gOperands = streamState.gOperands;

    // Paint a shading pattern in the current clipping path. TBI: check if inside zone.
    if( true )
    {
        // Command sh: add resource prefix.
        m_bufString = "/";
        m_bufString += gOperands.back().substr( 1 );
        m_bufString += this->getSuffixe();
        m_bufString += " sh\n";
        m_streamOut->Append( m_bufString );

        // Add key to out resources.
        this->addResourcesOutKey( PdfeResourcesType::Shading,
                                  gOperands.back().substr( 1 ),
                                  streamState.resources );
    }
}

void PRStreamLayoutZone::fInlineImages( const PdfeStreamStateOld& streamState )
{
    // Simpler references.
    const PdfeGraphicOperator& gOperator = streamState.gOperator;
    const std::vector<std::string>& gOperands = streamState.gOperands;
    const PdfeGraphicsState& gState = streamState.gStates.back();

    if( gOperator.type() == PdfeGOperator::ID ) {
        // Save variables.
        m_keyValuesII = gOperands;
    }
    else if( gOperator.type() == PdfeGOperator::EI ) {
        // Check if the inline image is inside the zone.
        PdfePath pathII;
        pathII.appendRectangle( PdfeVector(0,0), PdfeVector(1,1) );

        PdfeSubPath subpathII = pathII.subpaths().back();
        subpathII.applyTransMatrix( gState.transMat );

        bool inZone = subpathII.intersectZone( m_zone.zoneIn, m_parameters.inlineImageStrictlyInside );
        if( !inZone ) {
            return;
        }

        // Copy key/values of the inline image.
        m_bufString = "BI ";
        this->copyVariables( m_keyValuesII, m_bufString );

        // Image data.
        m_bufString += "ID ";
        m_bufString += gOperands.back();
        m_bufString += " EI\n";

        m_streamOut->Append( m_bufString );
    }
}

void PRStreamLayoutZone::fXObjects( const PdfeStreamStateOld& streamState )
{
    // Simpler references.
    const std::vector<std::string>& gOperands = streamState.gOperands;
    const PdfeGraphicsState& gState = streamState.gStates.back();

    // Get XObject and subtype.
    std::string xobjName = gOperands.back().substr( 1 );
    PdfObject* xobjPtr = streamState.resources.getIndirectKey( PdfeResourcesType::XObject, xobjName );
    std::string xobjSubtype = xobjPtr->GetIndirectKey( "Subtype" )->GetName().GetName();

    // Distinction between different type of XObjects
    if( !xobjSubtype.compare( "Image" ) ) {
        // Check if the image is inside the zone.
        PdfePath pathImg;
        pathImg.appendRectangle( PdfeVector(0,0), PdfeVector(1,1) );

        PdfeSubPath subpathImg = pathImg.subpaths().back();
        subpathImg.applyTransMatrix( gState.transMat );

        bool inZone = subpathImg.intersectZone( m_zone.zoneIn, m_parameters.imageStrictlyInside );
        if( inZone )
        {
            m_bufString = "/";
            m_bufString += xobjName;
            m_bufString += this->getSuffixe();
            m_bufString += " Do\n";
            m_streamOut->Append( m_bufString );

            // Add key to out resources.
            this->addResourcesOutKey( PdfeResourcesType::XObject,
                                      xobjName,
                                      streamState.resources );
        }
    }
    else if( !xobjSubtype.compare( "Form" ) ) {
        // Nothing to do.
        // See fFormBegin and fFormEnd.
    }
    else if( !xobjSubtype.compare( "PS" ) ) {
        // Depreciated according to Pdf reference.
        // Therefore, don't care about the implementation...
    }
}

void PRStreamLayoutZone::fFormBegin( const PdfeStreamStateOld& streamState,
                                     PoDoFo::PdfXObject* form )
{
    // Push form.
    this->pushForm();

    // Add form to the list.
    m_formObjects.push_back( form->GetObject() );

    // Follows implementation from Pdf Reference: q / cm / re W n.
    m_bufStream.str("");
    m_bufStream << "q\n";

    // If transformation matrix, add it.
    PdfObject* xObjPtr = form->GetObject();
    if( xObjPtr->GetDictionary().HasKey( "Matrix" ) ) {
        PdfArray& mat = xObjPtr->GetIndirectKey( "Matrix" )->GetArray();

        m_bufStream << mat[0].GetReal() << " ";
        m_bufStream << mat[1].GetReal() << " ";
        m_bufStream << mat[2].GetReal() << " ";
        m_bufStream << mat[3].GetReal() << " ";
        m_bufStream << mat[4].GetReal() << " ";
        m_bufStream << mat[5].GetReal() << " ";
        m_bufStream << "cm\n";
    }

    // Form's BBox: add corresponding clipping path.
    PdfRect formBBox = form->GetPageSize();
    m_bufStream << formBBox.GetLeft() << " ";
    m_bufStream << formBBox.GetBottom() << " ";
    m_bufStream << formBBox.GetWidth() << " ";
    m_bufStream << formBBox.GetHeight() << " ";
    m_bufStream << " re W n\n";

    // Append to stream.
    m_streamOut->Append( m_bufStream.str() );
}

void PRStreamLayoutZone::fFormEnd( const PdfeStreamStateOld& streamState,
                                   PoDoFo::PdfXObject* form )
{
    // Pop form.
    this->popForm();

    // Pop graphics stack.
    m_streamOut->Append( "Q\n" );
}

void PRStreamLayoutZone::fMarkedContents( const PdfeStreamStateOld& streamState )
{
    // Vous dîtes ?
}

void PRStreamLayoutZone::fCompatibility( const PdfeStreamStateOld& streamState )
{
    // Simpler references.
    const PdfeGraphicOperator& gOperator = streamState.gOperator;

    // Graphic compatibility mode.
    m_bufString = gOperator.str();
    m_bufString += "\n";
    m_streamOut->Append( m_bufString );
}

void PRStreamLayoutZone::fUnknown( const PdfeStreamStateOld& streamState )
{
    // Euh...
}
void PRStreamLayoutZone::addResourcesOutKey( PdfeResourcesType::Enum resourceType,
                                             const std::string& key,
                                             const PdfeResources& resourcesIn )
{
    // Get and add key.
    PdfObject* objPtr = resourcesIn.getKey( resourceType, key );
    if( objPtr ) {
        m_resourcesOut->addKey( resourceType, key+getSuffixe(), objPtr );
    }
}
void PRStreamLayoutZone::pushForm()
{
    // Add form on the stack.
    m_formsStack.push_back( m_formsNb );
    m_formsNb++;

    // Update form suffixe.
    std::ostringstream suffixe;
    suffixe << "F" << m_formsStack.back();
    m_formSuffixe = suffixe.str();
}
void PRStreamLayoutZone::popForm()
{
    // Pop form on the stack.
    m_formsStack.pop_back();

    // Update form suffixe.
    std::ostringstream suffixe;
    suffixe << "F" << m_formsStack.back();
    m_formSuffixe = suffixe.str();
}


}
