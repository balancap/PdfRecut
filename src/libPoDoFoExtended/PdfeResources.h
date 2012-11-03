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
    /** Copy constructor.
     * \param resources Object to copy.
     */
    PdfeResources(const PdfeResources& rhs );
    /** Operator=.
     * \param resources Object to copy.
     */
    PdfeResources& operator=( const PdfeResources& rhs );

    /** Push back a resources dictionary.
     * \param resourcesDict Dictionary to push (not owned by the PdfResources object).
     */
    void pushBack( PoDoFo::PdfObject* resourcesObj );

    /** Get the vector of resources dictionaries.
     * \return Vector of pointers.
     */
    const std::vector<PoDoFo::PdfObject*>& resources() const;

    /** Add a key in resources.
     * \param resource Resource type where to add the key.
     * \param key Key to add.
     * \param PdfObject Value corresponding to the key.
     */
    void addKey( PdfeResourcesType::Enum resource, const PoDoFo::PdfName& key, const PoDoFo::PdfObject* object );

    /** Get a key in resources (try each resource object by order of importance).
     * \param resource Resource type where to search the key.
     * \param key Key to find.
     * \return PdfObject corresponding to the key. Null, if not found.
     */
    PoDoFo::PdfObject* getKey( PdfeResourcesType::Enum resource, const PoDoFo::PdfName& key ) const;

    /** Get an indirect key in resources (try each resource object by order of importance).
     * \param resource Resource type where to search the key.
     * \param key Key to find.
     * \return PdfObject corresponding to the key. Null, if not found.
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
