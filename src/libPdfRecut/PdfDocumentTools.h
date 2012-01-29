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

#ifndef PDFDOCUMENTTOOLS_H
#define PDFDOCUMENTTOOLS_H

#include "PdfDocumentHandle.h"

namespace PdfeBooker {

/** A class which contains static functions operating on PdfDocument.
 */
class PdfDocumentTools
{
public:
    /** Add a graphic state stack on every page in order to
     * have the initial graphic state when operators are appended.
     * Need the PoDoFo mutex on the PdfDocumentHandle object.
     * \param document Document to modify.
     */
    static void addGraphicStateStack( PdfDocumentHandle* documentHandle );

    /** Uncompress streams in a PdfMemDocument.
     * Need the PoDoFo mutex on the PdfDocumentHandle object.
     * \param document Document to uncompress.
     */
    static void uncompressStreams( PdfDocumentHandle* documentHandle );

private:
    /** Default constructor, private.
     */
    PdfDocumentTools() { }
};

}

#endif // PDFDOCUMENTTOOLS_H
