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

#include "PdfeCMap.h"
#include "podofo/podofo.h"

using namespace PoDoFo;

namespace PoDoFoExtended {

//**********************************************************//
//                     PdfCIDSystemInfo                     //
//**********************************************************//
PdfeCIDSystemInfo::PdfeCIDSystemInfo()
{
    this->init();
}
void PdfeCIDSystemInfo::init()
{
    registery.clear();
    ordering.clear();
    supplement = 0;
}
void PdfeCIDSystemInfo::init( PdfObject* cidSysInfoObj )
{
    if( !cidSysInfoObj ) {
        return;
    }
    PdfObject* tmpObj;

    tmpObj = cidSysInfoObj->GetIndirectKey( "Registry" );
    if( tmpObj && tmpObj->IsString() ) {
        registery = tmpObj->GetString().GetString();
    }
    tmpObj = cidSysInfoObj->GetIndirectKey( "Ordering" );
    if( tmpObj && tmpObj->IsString() ) {
        ordering = tmpObj->GetString().GetString();
    }
    tmpObj = cidSysInfoObj->GetIndirectKey( "Supplement" );
    if( tmpObj && tmpObj->IsNumber() ) {
        supplement = tmpObj->GetNumber();
    }
}

//**********************************************************//
//                          PdfCMap                         //
//**********************************************************//
PdfeCMap::PdfeCMap()
    : m_baseCMap( NULL )
{
    this->init();
}
PdfeCMap::PdfeCMap( const PdfName& cmapName )
    : m_baseCMap( NULL )
{
    this->init( cmapName );
}
PdfeCMap::PdfeCMap( PdfObject* cmapStream )
    : m_baseCMap( NULL )
{
    this->init( cmapStream );
}
PdfeCMap::~PdfeCMap()
{
    delete m_baseCMap;
}

void PdfeCMap::init()
{
    // Delete existing base CMap if necessary.
    if( m_baseCMap ) {
        delete m_baseCMap;
    }

    // Default values given in the PdfReference.
    m_wmode = 0;
    m_identity = 0;
    m_baseCMap = NULL;
}
void PdfeCMap::init( const PdfName& cmapName )
{
    // Init to default values.
    this->init();
    m_name = cmapName;
    std::string strCmapName = m_name.GetName();

    // Identity-H or Identity-V CMaps
    if( strCmapName == "Identity-H" ) {
        m_identity = true;
        return;
    }
    else if( strCmapName == "Identity-V" ) {
        m_identity = true;
        m_wmode = true;
        return;
    }

    // TODO: implement other predefined CMaps that should be loaded from a file.
}
void PdfeCMap::init( PdfObject* cmapStream )
{
    // Init to default values.
    this->init();

    // Read CMap parameters from the PdfObject.
    PdfObject* tmpObj;

    tmpObj = cmapStream->GetIndirectKey( "CMapName" );
    if( tmpObj && tmpObj->IsName() ) {
        m_name = tmpObj->GetName();
    }
    tmpObj = cmapStream->GetIndirectKey( "WMode" );
    if( tmpObj && tmpObj->IsNumber() ) {
        m_wmode = tmpObj->GetNumber();
    }
    m_CIDSystemInfo.init( cmapStream->GetIndirectKey( "CIDSystemInfo" ) );

    // CMap used as base for this one.
    tmpObj = cmapStream->GetIndirectKey( "UseCMap" );
    if( tmpObj && tmpObj->IsName() ) {
        m_baseCMap = new PdfeCMap( tmpObj->GetName() );
    }
    else if( tmpObj ) {
        m_baseCMap = new PdfeCMap( tmpObj );
    }
}

std::vector<pdf_cid> PdfeCMap::getCID( const char *ptext, size_t length ) const
{
    std::vector<pdf_cid> cidVect;

    // Identity CMap
    if( m_identity ) {
        const pdf_uint16* pvalue = (const pdf_uint16*) ptext;
        cidVect.reserve( length / 2 );

        for( size_t i = 0 ; i < length-1 ; i+=2 ) {
            cidVect.push_back( *pvalue );

            // Litte endian: need to invert bytes.
#ifdef PODOFO_IS_LITTLE_ENDIAN
            cidVect.back() = ((cidVect.back() & 0xff) << 8) | ((cidVect.back() & 0xff00) >> 8);
#endif
            ++pvalue;
        }
        return cidVect;
    }

    // Otherwise: TODO !
    return cidVect;
}

PdfeCIDString PdfeCMap::toCIDString( const PoDoFo::PdfString& str ) const
{
    PdfeCIDString cidstr;

    // PDF String data.
    const char* pstr = str.GetString();
    size_t length = str.GetLength();

    // Identity CMap
    if( m_identity ) {
        pdf_cid c;
        const pdf_cid* pValue = reinterpret_cast<const pdf_cid*>( pstr );

        cidstr.reserve( length / 2 );
        for( size_t i = 0 ; i < length-1 ; i += 2 ) {

            // Read character (Litte endian: need to invert bytes).
            c = *pValue;
#ifdef PODOFO_IS_LITTLE_ENDIAN
            c = ( (c & 0xff) << 8 ) | ( (c & 0xff00) >> 8 );
#endif
            cidstr.push_back( c );

            ++pValue;
        }
        return cidstr;
    }

    // Otherwise: TODO !
    return cidstr;
}

}
