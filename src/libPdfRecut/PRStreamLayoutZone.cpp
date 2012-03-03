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

#include <podofo/podofo.h>
#include "PRStreamLayoutZone.h"

#define BUFFER_SIZE 4096

using namespace PoDoFo;

namespace PdfRecut {

PRStreamLayoutZone::PRStreamLayoutZone( PoDoFo::PdfPage* pageIn,
                                          PoDoFo::PdfStream* streamOut,
                                          const PRPageZone& zone,
                                          const PRLayoutParameters& parameters,
                                          const std::string& resPrefix ) :
    PRStreamAnalysis( pageIn ), m_zone ( zone ),
    m_resPrefix ( resPrefix ), m_parameters( parameters )
{
    // Downcasting to PdfMemStream pointer.resource
    m_streamOut = dynamic_cast<PdfMemStream*>( streamOut );

    m_bufString.reserve( BUFFER_SIZE );
}

void PRStreamLayoutZone::generateStream()
{
    // Zone Out coordinates and transformation.
    PdfMatrix zoneOutTrMatrix;
    zoneOutTrMatrix(2,0) = m_zone.leftZoneOut - m_zone.zoneIn.GetLeft();
    zoneOutTrMatrix(2,1) = m_zone.bottomZoneOut - m_zone.zoneIn.GetBottom();

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
    this->analyse();

    // Close stream.
    m_streamOut->Append("Q\n");
    m_streamOut->EndAppend();


    std::ostringstream bufs;
    PdfOutputDevice out(&bufs);

    m_streamOut->Write( &out );
    //std::cout << bufs.str() << std::endl;
}

void PRStreamLayoutZone::fGeneralGState( const PdfGraphicOperator& gOperator,
                                          const std::vector<std::string>& vecVariables,
                                          const std::vector<PdfGraphicsState>& vecGStates )
{
    m_bufStream.str("");
    m_bufString.clear();

    if( gOperator.code == ePdfGOperator_gs ) {
        // Specific case of command "gs": add resource prefix.
        m_bufString += "/";
        m_bufString += m_resPrefix;
        m_bufString += vecVariables.back().substr( 1 );
        m_bufString += " gs\n";
    }
    else {
        // Copy variables and operator.
        this->copyVariables( vecVariables, m_bufString );
        m_bufString += gOperator.name;
        m_bufString += "\n";
    }
    m_streamOut->Append( m_bufString );
}

void PRStreamLayoutZone::fSpecialGState( const PdfGraphicOperator& gOperator,
                                          const std::vector<std::string>& vecVariables,
                                          const std::vector<PdfGraphicsState>& vecGStates )
{
    m_bufString.clear();

    // Copy variables and operator.
    this->copyVariables( vecVariables, m_bufString );
    m_bufString += gOperator.name;
    m_bufString += "\n";
    m_streamOut->Append( m_bufString );
}

void PRStreamLayoutZone::fPathConstruction( const PdfGraphicOperator& gOperator,
                                             const std::vector<std::string>& vecVariables,
                                             const std::vector<PdfGraphicsState>& vecGStates,
                                             const PdfPath& currentPath )
{
    // Everything is done in painting function !
}

void PRStreamLayoutZone::fPathPainting( const PdfGraphicOperator& gOperator,
                                         const std::vector<std::string>& vecVariables,
                                         const std::vector<PdfGraphicsState>& vecGStates,
                                         const PdfPath& currentPath )
{
    // Subpaths from the current path.
    std::vector<PdfSubPath> subpaths = currentPath.getSubpaths();
    std::vector<PdfSubPath> subpathsBack( subpaths );

    // Temp variable, including inverse transformation matrix.
    bool inZone;
    size_t idx = 0;
    PdfMatrix invTransMat;
    vecGStates.back().transMat.inverse( invTransMat );

    // Keep subpaths which intersect the zone, and reduce them if necessary.
    std::vector<PdfSubPath>::iterator it;
    for( it = subpaths.begin() ; it != subpaths.end() ; )
    {
        // Apply transformation matrix.
        it->applyTransfMatrix( vecGStates.back().transMat );

        // Does the subpath intersect the zone.
        inZone = it->intersectZone( m_zone.zoneIn, m_parameters.pathStrictlyInside );

        if( inZone ) {
            // Reduce to zone if asked, and if not already strictly inside the zone.
            if( m_parameters.pathReduce && !m_parameters.pathStrictlyInside ) {
                it->reduceToZone( m_zone.zoneIn );

                // Apply inverse matrix to get points in current space.
                it->applyTransfMatrix( invTransMat );
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
    for( size_t i = 0 ; i < subpaths.size() ; i++ )
    {
        // Points in the subpath.
        for( size_t j = 0 ; j < subpaths[i].points.size() ; j++ )
        {
            // Distinction between different painting operators.
            if( subpaths[i].opPoints[j] == "m" ) {
                m_bufStream << subpaths[i].points[j](0) << " ";
                m_bufStream << subpaths[i].points[j](1) << " ";
                m_bufStream << "m\n";
            }
            else if( subpaths[i].opPoints[j] == "l" ) {
                m_bufStream << subpaths[i].points[j](0) << " ";
                m_bufStream << subpaths[i].points[j](1) << " ";
                m_bufStream << "l\n";
            }
            else if( subpaths[i].opPoints[j] == "c" ) {
                m_bufStream << subpaths[i].points[j](0) << " ";
                m_bufStream << subpaths[i].points[j](1) << " ";
                m_bufStream << subpaths[i].points[j+1](0) << " ";
                m_bufStream << subpaths[i].points[j+1](1) << " ";
                m_bufStream << subpaths[i].points[j+2](0) << " ";
                m_bufStream << subpaths[i].points[j+2](1) << " ";
                m_bufStream << "c\n";
                j+=2;
            }
            else if( subpaths[i].opPoints[j] == "v" ) {
                m_bufStream << subpaths[i].points[j](0) << " ";
                m_bufStream << subpaths[i].points[j](1) << " ";
                m_bufStream << subpaths[i].points[j+1](0) << " ";
                m_bufStream << subpaths[i].points[j+1](1) << " ";
                m_bufStream << "v\n";
                j+=1;
            }
            else if( subpaths[i].opPoints[j] == "y" ) {
                m_bufStream << subpaths[i].points[j](0) << " ";
                m_bufStream << subpaths[i].points[j](1) << " ";
                m_bufStream << subpaths[i].points[j+1](0) << " ";
                m_bufStream << subpaths[i].points[j+1](1) << " ";
                m_bufStream << "y\n";
                j+=1;
            }
            else if( subpaths[i].opPoints[j] == "h" ) {
                m_bufStream << "h\n";
            }
            else if( subpaths[i].opPoints[j] == "re" ) {
                m_bufStream << subpaths[i].points[j](0) << " ";
                m_bufStream << subpaths[i].points[j](1) << " ";
                m_bufStream << ( subpaths[i].points[j+2](0)-subpaths[i].points[j](0) ) << " ";
                m_bufStream << ( subpaths[i].points[j+2](1)-subpaths[i].points[j](1) ) << " ";
                m_bufStream << "re\n";
                j+=4;
            }
        }
    }
    // Add clipping path operator if necessary.
    std::string clippingPathOp = currentPath.getClippingPathOp();
    if( clippingPathOp.length() ) {
        m_bufStream << clippingPathOp << "\n";
    }
    // Painting operator.
    m_bufStream << gOperator.name << "\n";
    m_streamOut->Append( m_bufStream.str() );
}

void PRStreamLayoutZone::fClippingPath( const PdfGraphicOperator& gOperator,
                                         const std::vector<std::string>& vecVariables,
                                         const std::vector<PdfGraphicsState>& vecGStates,
                                         const PdfPath& currentPath )
{
}

void PRStreamLayoutZone::fTextObjects( const PdfGraphicOperator& gOperator,
                                        const std::vector<std::string>& vecVariables,
                                        const std::vector<PdfGraphicsState>& vecGStates )
{
    // Copy operator.
    m_bufString = gOperator.name;
    m_bufString += "\n";
    m_streamOut->Append( m_bufString );
}

void PRStreamLayoutZone::fTextState( const PdfGraphicOperator& gOperator,
                                      const std::vector<std::string>& vecVariables,
                                      const std::vector<PdfGraphicsState>& vecGStates )
{
    if( gOperator.code == ePdfGOperator_Tf ) {
        // Font: add resource prefix to name.
        m_bufString = "/";
        m_bufString += m_resPrefix;
        m_bufString += vecVariables[0].substr( 1 );
        m_bufString += " ";
        m_bufString += vecVariables[1];
        m_bufString += " Tf\n";
    }
    else {
        // Copy variables and operator.
        m_bufString.clear();
        this->copyVariables( vecVariables, m_bufString );
        m_bufString += gOperator.name;
        m_bufString += "\n";
    }
    m_streamOut->Append( m_bufString );
}

void PRStreamLayoutZone::fTextPositioning( const PdfGraphicOperator& gOperator,
                                            const std::vector<std::string>& vecVariables,
                                            const std::vector<PdfGraphicsState>& vecGStates )
{
    // Copy variables and operator.
    m_bufString.clear();
    this->copyVariables( vecVariables, m_bufString );
    m_bufString += gOperator.name;
    m_bufString += "\n";
    m_streamOut->Append( m_bufString );
}

void PRStreamLayoutZone::fTextShowing( const PdfGraphicOperator& gOperator,
                                        const std::vector<std::string>& vecVariables,
                                        const std::vector<PdfGraphicsState>& vecGStates )
{
    // Show text if inside page zone.
    PdfMatrix tmpMat = vecGStates.back().textState.transMat * vecGStates.back().transMat;
    if( tmpMat(2,1) <= m_zone.zoneIn.GetBottom() + m_zone.zoneIn.GetHeight() &&
        tmpMat(2,1) >= m_zone.zoneIn.GetBottom() )
    {
        // Copy variables and operator.
        m_bufString.clear();
        this->copyVariables( vecVariables, m_bufString );
        m_bufString += gOperator.name;
        m_bufString += "\n";
        m_streamOut->Append( m_bufString );
    }
    else
    {
        // In the case of operators ' or ", write non-showing code.
        m_bufStream.str( "" );
        if( gOperator.code == ePdfGOperator_Quote ) {
            m_bufStream << "T*\n";
        }
        else if( gOperator.code == ePdfGOperator_DoubleQuote ) {
            m_bufStream << vecGStates.back().textState.wordSpace << "Tw\n"
                        << vecGStates.back().textState.charSpace << "Tc\n";
        }
        m_streamOut->Append( m_bufStream.str() );
    }
}

void PRStreamLayoutZone::fType3Fonts( const PdfGraphicOperator& gOperator,
                                       const std::vector<std::string>& vecVariables,
                                       const std::vector<PdfGraphicsState>& vecGStates )
{
    m_bufString.clear();

    // Copy variables and operator...
    this->copyVariables( vecVariables, m_bufString );
    m_bufString += gOperator.name;
    m_bufString += "\n";
    m_streamOut->Append( m_bufString );
}

void PRStreamLayoutZone::fColor( const PdfGraphicOperator& gOperator,
                                  const std::vector<std::string>& vecVariables,
                                  const std::vector<PdfGraphicsState>& vecGStates )
{
    m_bufString.clear();

    // Color space used.
    if( gOperator.code == ePdfGOperator_CS || gOperator.code == ePdfGOperator_cs ) {

        // Modify if not default color space.
        std::string tmpStr = vecVariables.back();
        if( tmpStr.compare( "DeviceGray" ) && tmpStr.compare( "DeviceRGB" ) &&
            tmpStr.compare( "DeviceCMYK" ) && tmpStr.compare( "Pattern" ) ) {

            m_bufString = "/";
            m_bufString += m_resPrefix;
            m_bufString += tmpStr.substr( 1 );
            m_bufString += " ";
        }
    }
    else if( gOperator.code == ePdfGOperator_SCN || gOperator.code == ePdfGOperator_scn ) {
        // Color space is pattern.
        if( vecVariables.back()[0] == '/' ) {
            for( size_t i = 0 ; i < vecVariables.size()-1 ; i++ ) {
                m_bufString += vecVariables[i];
                m_bufString += " ";
            }
            m_bufString += "/";
            m_bufString += m_resPrefix;
            m_bufString += vecVariables.back().substr( 1 );
            m_bufString += " ";
        }
    }
    else {
        // Copy variables.
        this->copyVariables( vecVariables, m_bufString );
    }
    m_bufString += gOperator.name;
    m_bufString += "\n";
    m_streamOut->Append( m_bufString );
}

void PRStreamLayoutZone::fShadingPatterns( const PdfGraphicOperator& gOperator,
                                            const std::vector<std::string>& vecVariables,
                                            const std::vector<PdfGraphicsState>& vecGStates )
{
    // Paint a shading pattern in the current clipping path. TBI: check if inside zone.
    if( true )
    {
        // Command sh: add resource prefix.
        m_bufString = "/";
        m_bufString += m_resPrefix;
        m_bufString += vecVariables.back().substr( 1 );
        m_bufString += " sh\n";
        m_streamOut->Append( m_bufString );
    }
}

void PRStreamLayoutZone::fInlineImages( const PdfGraphicOperator& gOperator,
                                         const std::vector<std::string>& vecVariables,
                                         const std::vector<PdfGraphicsState>& vecGStates )
{
    if( gOperator.code == ePdfGOperator_ID ) {
        // Save variables.
        m_keyValuesII = vecVariables;
    }
    else if( gOperator.code == ePdfGOperator_EI ) {
        // Check if the inline image is inside the zone.
        PdfPath pathII;
        pathII.appendRectangle( PdfVector(0,0), PdfVector(1,1) );

        PdfSubPath& subpathII = pathII.getSubpaths().back();
        subpathII.applyTransfMatrix( vecGStates.back().transMat );

        bool inZone = subpathII.intersectZone( m_zone.zoneIn, m_parameters.inlineImageStrictlyInside );
        if( !inZone ) {
            return;
        }

        // Copy key/values of the inline image.
        m_bufString = "BI ";
        this->copyVariables( m_keyValuesII, m_bufString );

        // Image data.
        m_bufString += "ID ";
        m_bufString += vecVariables.back();
        m_bufString += " EI\n";

        m_streamOut->Append( m_bufString );
    }
}

void PRStreamLayoutZone::fXObjects( const PdfGraphicOperator& gOperator,
                                     const std::vector<std::string>& vecVariables,
                                     const std::vector<PdfGraphicsState>& vecGStates )
{
    // Name of the XObject and dictionary entry.
    std::string xobjName = vecVariables.back().substr( 1 );
    PdfObject* xobjDict = m_page->GetResources()->GetIndirectKey( "XObject" );
    PdfObject* xobjPtr = xobjDict->GetIndirectKey( xobjName );
    std::string xobjSubtype = xobjPtr->GetIndirectKey( "Subtype" )->GetName().GetName();

    // Distinction between different type of XObjects
    if( !xobjSubtype.compare( "Image" ) ) {
        // Check if the image is inside the zone.
        PdfPath pathImg;
        pathImg.appendRectangle( PdfVector(0,0), PdfVector(1,1) );

        PdfSubPath& subpathImg = pathImg.getSubpaths().back();
        subpathImg.applyTransfMatrix( vecGStates.back().transMat );

        bool inZone = subpathImg.intersectZone( m_zone.zoneIn, m_parameters.imageStrictlyInside );
        if( inZone )
        {
            m_bufString = "/";
            m_bufString += m_resPrefix;
            m_bufString += xobjName;
            m_bufString += " Do\n";
            m_streamOut->Append( m_bufString );
        }
    }
    else if( !xobjSubtype.compare( "Form" ) ) {
        // Get parameters in form's dictionary
        PdfMatrix formMat;
        if( xobjPtr->GetDictionary().HasKey( "Matrix" ) ) {
            PdfArray& mat = xobjPtr->GetIndirectKey( "Matrix" )->GetArray();

            formMat(0,0) = mat[0].GetReal();
            formMat(0,1) = mat[1].GetReal();
            formMat(1,0) = mat[2].GetReal();
            formMat(1,1) = mat[3].GetReal();
            formMat(2,0) = mat[4].GetReal();
            formMat(2,1) = mat[5].GetReal();
        }
        formMat = formMat * vecGStates.back().transMat;

        // Position of form's BBox
        PdfArray& bbox = xobjPtr->GetIndirectKey( "BBox" )->GetArray();

        PdfPath pathBBox;
        pathBBox.beginSubpath( PdfVector( bbox[0].GetReal(), bbox[1].GetReal() ) );
        pathBBox.appendLine( PdfVector( bbox[2].GetReal(), bbox[1].GetReal() ) );
        pathBBox.appendLine( PdfVector( bbox[2].GetReal(), bbox[3].GetReal() ) );
        pathBBox.appendLine( PdfVector( bbox[0].GetReal(), bbox[3].GetReal() ) );
        pathBBox.closeSubpath();

        PdfSubPath& subpathBBox= pathBBox.getSubpaths().back();
        subpathBBox.applyTransfMatrix( formMat );

        // Add form if inside zone
        bool inZone = subpathBBox.intersectZone( m_zone.zoneIn, m_parameters.formStrictlyInside );
        if( inZone )
        {
            m_bufString = "/";
            m_bufString += m_resPrefix;
            m_bufString += xobjName;
            m_bufString += " Do\n";
            m_streamOut->Append( m_bufString );
        }
    }
    else if( !xobjSubtype.compare( "PS" ) ) {
        // Depreciated according to Pdf reference.
        // Therefore, don't care about the implementation...
    }
}

void PRStreamLayoutZone::fMarkedContents( const PdfGraphicOperator& gOperator,
                                           const std::vector<std::string>& vecVariables,
                                           const std::vector<PdfGraphicsState>& vecGStates )
{
    // Vous dîtes ?
}

void PRStreamLayoutZone::fCompatibility( const PdfGraphicOperator& gOperator,
                                          const std::vector<std::string>& vecVariables,
                                          const std::vector<PdfGraphicsState>& vecGStates )
{
    // Graphic compatibility mode.
    m_bufString = gOperator.name;
    m_bufString += "\n";
    m_streamOut->Append( m_bufString );
}

}
