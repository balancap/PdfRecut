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

#ifndef PDFFONTMETRICSCACHE_H
#define PDFFONTMETRICSCACHE_H

#include <map>

namespace PoDoFo {
    class PdfMemDocument;
    class PdfReference;
    class PdfFontMetrics;
}

namespace PdfRecut {

/** Simple structure which represents a pointer to
 * a PdfFontMetrics object.
 */
struct PdfFontMetricsPointer {
    /** Pointer to the font metrics object.
     */
    PoDoFo::PdfFontMetrics* ptr;

    /** Do we own the font metrics object ?
     * Used when memory is freed.
     */
    bool owned;

    /** Initialize values (NULL,true) in constructor.
     */
    PdfFontMetricsPointer() {
        ptr = NULL;
        owned = true;
    }

};

class PdfFontMetricsCache
{
public:
    /** Default constructor.
     * \param document PdfMemDocument used to define and get fonts.
     */
    PdfFontMetricsCache( PoDoFo::PdfMemDocument* document );

    /** Destructor, free memory owned by the cache.
     */
    ~PdfFontMetricsCache();

    /** Get font metrics object of a font.
     * \param fontRef Reference to the Pdf font object.
     * \return PdfFontMetrics pointer, owned by the PdfFontMetricsCache object.
     */
    PoDoFo::PdfFontMetrics* getFontMetrics( const PoDoFo::PdfReference& fontRef );

private:
    /** Add a font metrics object in the cache.
     * \param fontRef Reference to the Pdf font object.
     * \return PdfFontMetrics pointer, owned by the PdfFontMetricsCache object.
     */
    PoDoFo::PdfFontMetrics* addFontMetrics( const PoDoFo::PdfReference& fontRef );

private:
    /** Pdf document.
     */
    PoDoFo::PdfMemDocument* m_document;

    /** Map containing font metrics cache.
     * The key corresponds to the reference of the font object.
     */
    std::map< PoDoFo::PdfReference, PdfFontMetricsPointer > m_fontMetricsCache;
};

}

#endif // PDFFONTMETRICSCACHE_H
