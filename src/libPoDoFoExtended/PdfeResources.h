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

namespace PoDoFo {
    class PdfObject;
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
}

/** Class used to handle a collection of resources associated to some contents.
 */
class PdfeResources
{
public:
    /** Create en empty object.
     */
    PdfeResources();
    /** Initialize to an empty collection of resources.
     */
    void init();
    /** Copy constructor.
     * \param resources Object to copy.
     */
    PdfeResources( const PdfeResources& rhs );
    /** Operator=.
     * \param resources Object to copy.
     */
    PdfeResources& operator=( const PdfeResources& rhs );

public:
    // Resources PDF objects.
    /** Push back a resources dictionary inside the collection
     * \param pResourcesObj Dictionary to add (not owned by the PdfResources object).
     */
    void push_back( PoDoFo::PdfObject* pResourcesObj );
    /** Remove a resources object from the collection.
     * \param pResourcesObj Resources dictionary to remove.
     * \return True if found and successively removed.
     */
    bool remove( PoDoFo::PdfObject* pResourcesObj );
    /** Is a resources object inside the collection?
     * \param pResourcesObj Object to find in the collection.
     * \return Found?
     */
    bool inside( const PoDoFo::PdfObject* pResourcesObj ) const;
    /** Get the vector of resources dictionaries.
     * \return Vector of pointers.
     */
    const std::vector<PoDoFo::PdfObject*>& resources() const;

public:
    // Resources contents.
    /** Add a pair key/value to resources. The PdfObject is copied
     * \param resource Resource type where to add the key.
     * \param key Key of the object to add.
     * \param PdfObject Value corresponding to the key. Copied.
     */
    void addKey( PdfeResourcesType::Enum resource, const PoDoFo::PdfName& key, const PoDoFo::PdfObject* object );
    /** Get a key in resources (try each resource object by order of importance).
     * \param resource Resource type where to search the key.
     * \param key Key to find.
     * \return PdfObject corresponding to the key. NULL, if not found.
     */
    PoDoFo::PdfObject* getKey( PdfeResourcesType::Enum resource, const PoDoFo::PdfName& key ) const;
    /** Get an indirect key in resources (try each resource object by order of importance).
     * \param resource Resource type where to search the key.
     * \param key Key to find.
     * \return PdfObject corresponding to the key. NULL, if not found.
     */
    PoDoFo::PdfObject* getIndirectKey( PdfeResourcesType::Enum resource, const PoDoFo::PdfName& key ) const;

public:
    /// Vector of C string corresponding to PdfResources types.
    static const char* CTypes[];

private:
    /// Vector of resources objects, not owned by the class. Sorted by increasing order of importance.
    std::vector<PoDoFo::PdfObject*>  m_resources;
};

}

#endif // PDFERESOURCES_H
