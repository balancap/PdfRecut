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

#include "PdfeResources.h"

#include <podofo/podofo.h>
#include <QsLog/QsLog.h>

using namespace PoDoFo;

namespace PoDoFoExtended {

const char* PdfeResources::CTypes[] = { "ExtGState",
                                        "ColorSpace",
                                        "Pattern",
                                        "Shading",
                                        "XObject",
                                        "Font",
                                        "ProcSet",
                                        "Properties" };

PdfeResources::PdfeResources()
{
    // Nothing to do!
}
void PdfeResources::init()
{
    m_resources.clear();
}
PdfeResources::PdfeResources( const PdfeResources& rhs ) :
    m_resources( rhs.m_resources )
{
}
PdfeResources& PdfeResources::operator=( const PdfeResources& rhs )
{
    m_resources = rhs.m_resources;
    return *this;
}

void PdfeResources::push_back( PdfObject* pResourcesObj )
{
    if( !pResourcesObj ) {
        return;
    }
    // Check it is a dictionary and add it.
    if( pResourcesObj->IsDictionary() ) {
        m_resources.push_back( pResourcesObj );
    }
    else {
        QLOG_WARN() << QString( "<PdfeResources> Try to add a resources object which is not a dictionary." )
                       .toAscii().constData();
        // TODO: raise exception?
    }
}
bool PdfeResources::remove( PdfObject* pResourcesObj )
{
    std::vector<PdfObject*>::iterator it;
    it = std::find( m_resources.begin(), m_resources.end(), pResourcesObj );
    if( it !=  m_resources.end() ) {
        m_resources.erase( it );
        return true;
    }
    return false;
}
bool PdfeResources::inside( const PdfObject* pResourcesObj ) const
{
    std::vector<PdfObject*>::const_iterator it;
    it = std::find( m_resources.begin(), m_resources.end(), pResourcesObj );
    return ( it !=  m_resources.end() );
}
const std::vector<PdfObject*>& PdfeResources::resources() const
{
    return m_resources;
}

void PdfeResources::addKey( PdfeResourcesType::Enum resource, const PoDoFo::PdfName& key, const PoDoFo::PdfObject* object )
{
    // Empty resources object...
    if( m_resources.empty() ) {
        return;
    }
    // Get resources dictionary. Create it, if necessary. Then add key.
    if( !m_resources.back()->GetDictionary().HasKey( CTypes[resource] ) ) {
        m_resources.back()->GetDictionary().AddKey( CTypes[resource], PdfDictionary() );
    }
    PdfObject* resDict = m_resources.back()->GetIndirectKey( CTypes[resource] );
    resDict->GetDictionary().AddKey( key, object );
}
PoDoFo::PdfObject* PdfeResources::getKey( PdfeResourcesType::Enum resource, const PoDoFo::PdfName& key ) const
{
    PdfObject* keyObj = NULL;
    // Look at every resources object.
    for( int i = m_resources.size()-1 ; i >= 0 ; --i ) {
        // Get resources dict.
        PdfObject* resDict = m_resources[i]->GetIndirectKey( CTypes[resource] );
        if( resDict ) {
            // Find the value of the key (can be a PdfReference).
            keyObj = resDict->GetDictionary().GetKey( key );
            if( keyObj ) {
                return keyObj;
            }
        }
    }
    return NULL;
}
PoDoFo::PdfObject* PdfeResources::getIndirectKey( PdfeResourcesType::Enum resource, const PoDoFo::PdfName& key ) const
{
    PdfObject* keyObj = NULL;
    // Look in every resources object.
    for( int i = m_resources.size()-1 ; i >= 0 ; --i ) {
        // Get resources dict.
        PdfObject* resDict = m_resources[i]->GetIndirectKey( CTypes[resource] );
        if( resDict ) {
            // Find the (indirect) value of the key.
            keyObj = resDict->GetIndirectKey( key );
            if( keyObj ) {
                return keyObj;
            }
        }
    }
    return NULL;
}

}
