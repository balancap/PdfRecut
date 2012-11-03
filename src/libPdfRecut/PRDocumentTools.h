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

#ifndef PRDOCUMENTTOOLS_H
#define PRDOCUMENTTOOLS_H

#include "PRDocument.h"

namespace PdfRecut {

/** A class which contains static functions operating on PdfDocument.
 */
class PRDocumentTools
{
public:
    /** Add a graphic state stack on every page in order to
     * have the initial graphic state when operators are appended.
     * Need the PoDoFo mutex on the PdfDocumentHandle object.
     * \param document Document to modify.
     */
    static void addGraphicStateStack( PRDocument* documentHandle );

    /** Uncompress streams in a PdfMemDocument.
     * Need the PoDoFo mutex on the PdfDocumentHandle object.
     * \param document Document to uncompress.
     */
    static void uncompressStreams( PRDocument* documentHandle );

private:
    /** Default constructor, private.
     */
    PRDocumentTools() { }
};

}

#endif // PRDOCUMENTTOOLS_H
