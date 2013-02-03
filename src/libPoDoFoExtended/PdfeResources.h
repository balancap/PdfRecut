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

#ifndef PDFERESOURCES_H
#define PDFERESOURCES_H

#include <vector>
#include <string>

#include <podofo/base/PdfArray.h>
#include <podofo/base/PdfDictionary.h>

namespace PoDoFo {
    class PdfObject;
    class PdfVecObjects;
    class PdfName;
}

namespace PoDoFoExtended {

namespace PdfeResourcesType {
/** Enumeration of the different types of resources allowed in the PDF Reference.
 */
enum Enum {
    ExtGState = 0,
    ColorSpace,
    Pattern,
    Shading,
    XObject,
    Font,
    ProcSet,
    Properties,
    Unknown
};
/// Number of resources types.
inline size_t size() {
    static size_t size = 8;
    return size;
}
/// String representation of a given resource.
inline const char* str( PdfeResourcesType::Enum type ) {
    static const char* names[9] = {
        "ExtGState",
        "ColorSpace",
        "Pattern",
        "Shading",
        "XObject",
        "Font",
        "ProcSet",
        "Properties",
        "Unknown"
    };
    if( type >= 0 && type <= 8 ) {
        return names[ type ];
    }
    return names[ PdfeResourcesType::Unknown ];
}
}

/** Class used to handle a collection of resources associated to some contents.
 * Resources dictionaries are stored independently of PDF document, and can be
 * retrieved or written down using load/save routines.
 */
class PdfeResources
{
public:
    /** Create an empty resources object.
     * Owner of the collection of objects (to resolve references...).
     */
    PdfeResources( const PoDoFo::PdfVecObjects* pOwner = NULL );
    /** Create a resources object from an existing one.
     * \param pResourcesObj Resources object from a PDF.
     */
    PdfeResources( PoDoFo::PdfObject* pResourcesObj );
    /** Initialize to an empty collection of resources.
     */
    void init();
    /** (Deep) copy constructor.
     * \param resources Object to copy.
     */
    PdfeResources( const PdfeResources& rhs );
    /** Operator=.
     * \param resources Object to copy.
     */
    PdfeResources& operator=( const PdfeResources& rhs );

public:
    /** Load resources from an existing object. Previous content
     * are erased.
     * \param pResourcesObj Resources object from a PDF document.
     */
    void load( PoDoFo::PdfObject* pResourcesObj );
    /** Save resources into a PoDoFo object. Previous content of
     * the object is replaced completely.
     * \param pResourcesObj Resources object from a PDF document.
     */
    void save( PoDoFo::PdfObject* pResourcesObj );

public:
    /** Append another resources object. Existing keys are not
     * erased.
     * \param rhs Resources to append.
     */
    void append( const PdfeResources& rhs );
    /** Add a suffix to every entry in the resources collection.
     * ProcSet category is not concerned.
     * \param suffix Suffix to append.
     */
    void addSuffix( const std::string& suffix );

public:
    // Resources contents.
    /** Add a pair key/value to resources. The PdfObject value is copied.
     * Only the key name is used when added to ProcSet.
     * \param resource Resource type where to add the key.
     * \param key Key of the object to add.
     * \param pobject Value corresponding to the key. Copied.
     */
    void addKey( PdfeResourcesType::Enum resource, const PoDoFo::PdfName& key, const PoDoFo::PdfObject* pobject );
    /** Get a key in resources.
     * \param resource Resource type where to search the key.
     * \param key Key to find.
     * \return PdfObject corresponding to the key. NULL, if not found.
     */
    PoDoFo::PdfObject* getKey( PdfeResourcesType::Enum resource, const PoDoFo::PdfName& key ) const;
    /** Get an indirect key in resources. Resolve it if it is a reference.
     * \param resource Resource type where to search the key.
     * \param key Key to find.
     * \return PdfObject corresponding to the key. NULL, if not found.
     */
    PoDoFo::PdfObject* getIndirectKey( PdfeResourcesType::Enum resource, const PoDoFo::PdfName& key ) const;
    /** Is a procedure name inside the ProcSet array?
     * \param name Name of the procedure.
     * \return Inside?
     */
    bool insideProcSet( const PoDoFo::PdfName& name ) const;

public:
    // Simple getters/setters...
    /// Get the owner object.
    const PoDoFo::PdfVecObjects* owner() const              {   return m_pOwner;    }
    /// Set the owner object.
    void setOwner( const PoDoFo::PdfVecObjects* pOwner )    {   m_pOwner = pOwner;  }

private:
    /// Dictionaries containing different type of resources (except ProcSet).
    std::vector<PoDoFo::PdfDictionary>  m_resourcesDict;
    /// ProcSet array containing procedure set names.
    PoDoFo::PdfArray  m_resourcesProcSet;
    /// Owner of the collection of objects (to resolve references...).
    const PoDoFo::PdfVecObjects*  m_pOwner;
};

}

#endif // PDFERESOURCES_H
