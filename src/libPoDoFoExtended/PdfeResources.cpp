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
#include "PdfeUtils.h"

#include <podofo/podofo.h>
#include <QsLog/QsLog.h>

using namespace PoDoFo;

namespace PoDoFoExtended {

PdfeResources::PdfeResources(const PdfVecObjects *pOwner ) :
    m_pOwner( pOwner )
{
    m_resourcesDict.resize( PdfeResourcesType::size() );
    m_resourcesProcSet.Clear();
}
PdfeResources::PdfeResources( PdfObject* pResourcesObj )
{
    m_resourcesDict.resize( PdfeResourcesType::size() );
    m_resourcesProcSet.Clear();
    if( pResourcesObj ) {
        m_pOwner = pResourcesObj->GetOwner();
        this->load( pResourcesObj );
    }
}
void PdfeResources::init()
{
    m_resourcesDict.assign( PdfeResourcesType::size(), PdfDictionary() );
    m_resourcesProcSet.Clear();
    m_pOwner = NULL;
}
PdfeResources::PdfeResources( const PdfeResources& rhs ) :
    m_resourcesDict( rhs.m_resourcesDict ),
    m_resourcesProcSet( rhs.m_resourcesProcSet ),
    m_pOwner( rhs.m_pOwner )
{
}
PdfeResources& PdfeResources::operator=( const PdfeResources& rhs )
{
    m_resourcesDict = rhs.m_resourcesDict;
    m_resourcesProcSet = rhs.m_resourcesProcSet;
    m_pOwner = rhs.m_pOwner;
    return *this;
}

void PdfeResources::load( PdfObject* pResourcesObj )
{
    this->init();
    // Check input object.
    if( !pResourcesObj || !pResourcesObj->IsDictionary() ) {
        QLOG_WARN() << QString( "<PdfeResources> Try to load a resources object which is not a dictionary." )
                       .toAscii().constData();
        // TODO: raise exception?
    }
    m_pOwner = pResourcesObj->GetOwner();

    // Copy resources dictionaries.
    PdfeResourcesType::Enum rtype;
    PdfObject* pResSubDict;
    for( size_t i = 0 ; i < PdfeResourcesType::size() ; ++i ) {
        rtype = PdfeResourcesType::Enum( i );
        if( rtype != PdfeResourcesType::ProcSet ) {
            pResSubDict = pResourcesObj->GetIndirectKey( PdfeResourcesType::str( rtype ) );
            if( pResSubDict && pResSubDict->IsDictionary() ) {
                m_resourcesDict[i] = pResSubDict->GetDictionary();
            }
        }
    }
    // Copy ProcSet.
    rtype = PdfeResourcesType::ProcSet;
    pResSubDict = pResourcesObj->GetIndirectKey( PdfeResourcesType::str( rtype ) );
    if( pResSubDict && pResSubDict->IsArray() ) {
        m_resourcesProcSet = pResSubDict->GetArray();
    }
}
void PdfeResources::save( PdfObject* pResourcesObj )
{
    // Check output object.
    if( !pResourcesObj || !pResourcesObj->IsDictionary() ) {
        QLOG_WARN() << QString( "<PdfeResources> Try to save resources into an object which is not a dictionary." )
                       .toAscii().constData();
        // TODO: raise exception?
    }
    // Copy resources dictionaries.
    PdfeResourcesType::Enum rtype;
    PdfObject* pResSubDict;
    for( size_t i = 0 ; i < PdfeResourcesType::size() ; ++i ) {
        rtype = PdfeResourcesType::Enum( i );
        if( rtype != PdfeResourcesType::ProcSet ) {
            pResSubDict = pResourcesObj->GetIndirectKey( PdfeResourcesType::str( rtype ) );
            // Create dictionary if needed.
            if( !pResSubDict ) {
                pResourcesObj->GetDictionary().AddKey( PdfeResourcesType::str( rtype ),
                                                       m_resourcesDict[i] );
            }
            else {
                pResSubDict->PdfVariant::operator=( m_resourcesDict[i] );
            }
        }
    }
    // Copy ProcSet.
    rtype = PdfeResourcesType::ProcSet;
    pResSubDict = pResourcesObj->GetIndirectKey( PdfeResourcesType::str( rtype ) );
    if( !pResSubDict ) {
        pResourcesObj->GetDictionary().AddKey( PdfeResourcesType::str( rtype ),
                                               m_resourcesProcSet );
    }
    else {
        (*pResSubDict) = m_resourcesProcSet;
    }
}

void PdfeResources::append( const PdfeResources& rhs )
{
    for( size_t i = 0 ; i < PdfeResourcesType::size() ; ++i ) {
        TKeyMap::const_iterator it;
        for( it = rhs.m_resourcesDict[i].GetKeys().begin() ; it != rhs.m_resourcesDict[i].GetKeys().end() ; ++it ) {
            m_resourcesDict[i].AddKey( it->first, it->second );
        }
    }
    // TODO: remove duplicate entries.
    m_resourcesProcSet.insert( m_resourcesProcSet.end(),
                               rhs.m_resourcesProcSet.begin(),
                               rhs.m_resourcesProcSet.end() );
    // Owner?
    if( !m_pOwner && rhs.m_pOwner ) {
        m_pOwner = rhs.m_pOwner;
    }
}
void PdfeResources::addSuffix( const std::string& suffix )
{
    TKeyMap bufferMap;
    TKeyMap::iterator it;
    PdfeResourcesType::Enum rtype;
    for( size_t i = 0 ; i < PdfeResourcesType::size() ; ++i ) {
        rtype = PdfeResourcesType::Enum( i );
        if( rtype != PdfeResourcesType::ProcSet ) {
            // Copy modified resources into buffer.
            TKeyMap& resourcesMap = m_resourcesDict[i].GetKeys();
            bufferMap.clear();
            for( it = resourcesMap.begin() ; it != resourcesMap.end() ; ++it ) {
                bufferMap[ it->first.GetName() + suffix ] = it->second;
            }
            resourcesMap = bufferMap;
            m_resourcesDict[i].SetDirty( true );
        }
    }
}

void PdfeResources::addKey( PdfeResourcesType::Enum resource, const PoDoFo::PdfName& key, const PoDoFo::PdfObject* pobject )
{
    // Specific case of ProcSet.
    if( resource != PdfeResourcesType::ProcSet ) {
        m_resourcesDict[ resource ].AddKey( key, pobject );
    }
    else {
        if( !this->insideProcSet( key) ) {
            m_resourcesProcSet.push_back( key );
        }
    }
}
PdfObject* PdfeResources::getKey( PdfeResourcesType::Enum resource, const PoDoFo::PdfName& key ) const
{
    // Specific case of ProcSet.
    if( resource != PdfeResourcesType::ProcSet ) {
        return const_cast<PdfObject*>( m_resourcesDict[ resource ].GetKey( key ) );
    }
    else {
        for( size_t i = 0 ; i < m_resourcesProcSet.size() ; ++i ) {
            const PdfObject& resObj = m_resourcesProcSet[i];
            if( resObj.IsName() && resObj.GetName() == key ) {
                return const_cast<PdfObject*>( &resObj );
            }
        }
    }
    return NULL;
}
PdfObject* PdfeResources::getIndirectKey( PdfeResourcesType::Enum resource, const PdfName& key) const
{
    PdfObject* pObj = this->getKey( resource, key );
    if( pObj && pObj->IsReference() && m_pOwner ) {
        return m_pOwner->GetObject( pObj->GetReference() );
        //return PdfeIndirectObject( pObj, m_pOwner );
    }
    return pObj;
}
bool PdfeResources::insideProcSet( const PdfName& name ) const
{
    for( size_t i = 0 ; i < m_resourcesProcSet.size() ; ++i ) {
        const PdfObject& resObj = m_resourcesProcSet[i];
        if( resObj.IsName() && resObj.GetName() == name ) {
            return true;
        }
    }
    return false;
}

}
