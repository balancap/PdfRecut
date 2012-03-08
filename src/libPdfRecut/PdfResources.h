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

#ifndef PDFRESOURCES_H
#define PDFRESOURCES_H

#include <vector>
#include <string>

namespace PoDoFo {
    class PdfObject;
}

namespace PdfRecut {

/** Class used to handle a collection of resources associated to some contents.
 */
class PdfResources
{
public:
    /** Create en empty object.
     */
    PdfResources();
    /** Copy constructor.
     * \param resources Object to copy.
     */
    PdfResources( const PdfResources& resources );
    /** Operator=.
     * \param resources Object to copy.
     */
    PdfResources& operator=( const PdfResources& resources );

    /** Push back a resources dictionary.
     * \param resourcesDict Dictionary to push (not owned by the PdfResources object).
     */
    void pushBack( PoDoFo::PdfObject* resourcesDict );

    /** Get the vector of resources dictionaries.
     * \return Vector of pointers.
     */
    std::vector<PoDoFo::PdfObject*> getResources();

private:
    /** Vector of resources objects, not owned by the class.
     * Sorted by increasing order of importance.
     */
    std::vector<PoDoFo::PdfObject*>  m_resources;

    /** Resources types.
     */
    std::vector< std::string > m_resourcesTypes;
};

}

#endif // PDFRESOURCES_H
