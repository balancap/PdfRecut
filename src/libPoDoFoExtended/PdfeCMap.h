/***************************************************************************
 *   Copyright (C) 2011 by Paul Balança                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef PDFCMAP_H
#define PDFCMAP_H

#include "PdfeFont.h"
#include "podofo/base/PdfName.h"

namespace PoDoFo {
class PdfArray;
class PdfObject;
class PdfVariant;
class PdfString;
}

namespace PoDoFoExtended {

/** Structure that contains the different informations of CIDSystemInfo.
 */
struct PdfeCIDSystemInfo
{
    /// Issuer of the character collection (i.e. Adobe).
    std::string registery;
    /// Name of the character collection within the specified registry (i.e. Japan1).
    std::string ordering;
    /// Supplement number of the character collection.
    int supplement;

    /** Default constructor.
     */
    PdfeCIDSystemInfo();
    /** Initialize to default values.
     */
    void init();
    /** Initialize structure parameters from a PdfObject.
     * \param cidSysInfoObj PoDoFo object containing CID system info.
     */
    void init( PoDoFo::PdfObject* cidSysInfoObj );
};

/** Class used to represent and handle a CMap as described in the PDF reference.
 */
class PdfeCMap
{
public:
    /** Empty constructor.
     */
    PdfeCMap();
    /** Construction from using predefined CMap.
     */
    PdfeCMap( const PoDoFo::PdfName& cmapName );
    /** Construction from an embedded CMap (represented in a PdfStream).
     */
    PdfeCMap( PoDoFo::PdfObject* cmapStream );

    /** Init members to default values.
     */
    void init();
    /** Initialize from a predefined CMap.
     */
    void init( const PoDoFo::PdfName& cmapName );
    /** Initialize from an embedded CMap (represented in a PdfStream).
     */
    void init( PoDoFo::PdfObject* cmapStream );

    /** Destructor.
     */
    ~PdfeCMap();

    /** Convert character codes in a string into a CID vector.
     * \param ptext Pointer to a string of characters code.
     * \param length Length of the string.
     * \return A vector of corresponding CID.
     */
    std::vector<pdf_cid> getCID( const char* ptext, size_t length ) const;

    /** Convert a simple string to a CID string using the CMap.
     * \param str PoDoFo::PdfString to convert (can contain 0 characters !).
     * \return CID String corresponding.
     */
    PdfeCIDString toCIDString( const PoDoFo::PdfString& str ) const;

protected:
    /// CMap name.
    PoDoFo::PdfName  m_name;
    /// CMap CIDSystemInfo.
    PdfeCIDSystemInfo  m_CIDSystemInfo;

    /// Writting mode: 0 (horizontal), 1 (vertical).
    bool  m_wmode;
    /// Is the CMap simply the identity.
    bool  m_identity;

    /// CMap used in addition to defined this one (owned).
    PdfeCMap*  m_baseCMap;
};

}

#endif // PDFCMAP_H
