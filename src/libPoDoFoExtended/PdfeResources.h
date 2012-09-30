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

#ifndef PDFERESOURCES_H
#define PDFERESOURCES_H

#include <vector>
#include <string>

namespace PoDoFo {
    class PdfObject;
    class PdfName;
}

namespace PdfRecut {

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
