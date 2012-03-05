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

#ifndef PRDOCUMENTLAYOUT_H
#define PRDOCUMENTLAYOUT_H

#include <QtCore/QObject>

#include <podofo/base/PdfRect.h>

#include "PdfTypes.h"
#include "PdfSemaphore.h"
#include "PRDocument.h"
#include "PdfMisc.h"

namespace PoDoFo {
    class PdfObject;
    class PdfDocument;
    class PdfMemDocument;
    class PdfPage;
    class PdfStream;
    class PdfOutlineItem;
    class PdfReference;
    class PdfRect;
}

namespace PdfRecut {

class PRPageLayout;

/** Pdf zone in the input document to be inserted in an output page.
 */
struct PRPageZone
{
    /** Index of the page in which the zone is located.
     */
    int indexIn;

    /** Zone in the page to import.
     */
    PoDoFo::PdfRect zoneIn;

    /** Location of the zone in the new page.
     */
    double leftZoneOut;
    double bottomZoneOut;

    /** PdfPageLayout parent pointer.
     */
    PRPageLayout* parent;

    /** Default constructor.
     */
    PRPageZone() : indexIn(0), zoneIn(),
        leftZoneOut(0), bottomZoneOut(0), parent(NULL) {}
};

/** Layout of an output page in the redesigned document.
 */
struct PRPageLayout
{
    /** Index of the new page.
     */
    int indexOut;

    /** Pdf mediaBox of the page.
     */
    PoDoFo::PdfRect mediaBox;

    /** Pdf cropBox of the page.
     */
    PoDoFo::PdfRect cropBox;

    /** Vector of page zones to incorporated in the new page.
     */
    std::vector<PRPageZone> zonesIn;

    /** Default constructor.
     */
    PRPageLayout() : indexOut(-1), mediaBox(), cropBox(), zonesIn() {}

    /** Copy constructor (change parent value in zones).
     */
    PRPageLayout(const PRPageLayout& layout)
    {
        this->indexOut = layout.indexOut;
        this->mediaBox = layout.mediaBox;
        this->cropBox = layout.cropBox;
        this->zonesIn = layout.zonesIn;

        // Reset parent pointer
        for(size_t i = 0 ; i < zonesIn.size() ; i++)
            zonesIn[i].parent = this;
    }
};

/** Structure containing the different parameters of a document layout
 * used when a Pdf is generated.
 */
struct PRLayoutParameters
{
    /** Rotate pages in the output document (0, 90, 180 or 270).
     */
    int pagesRotation;

    /** Add overlay corresponding to layout zones
     */
    bool overlayLayoutZones;

    /** Add a clipping path for every zone and which corresponds
     * to its dimensions.
     */
    bool zoneClippingPath;

    /** Keep paths strictly inside a zone if =true. Otherwise, keep
     * path that intersect the zone.
     */
    bool pathStrictlyInside;
    /** Reduce path to zone (modify path points so that they are stricly included
     * in the zone) if =true. Otherwise, keep original points. Also apply to
     * potential clipping paths in a zone.
     */
    bool pathReduce;

    /** Keep text strictly inside the zone. Not implemented yet.
     */
    bool textStrictlyInside;
    /** Reduce text to only keep the part which is inside a zone.
     * Not implemented yet.
     */
    bool textReduce;

    /** Keep images strictly inside a zone.
     */
    bool imageStrictlyInside;

    /** Keep inline image strictly inside a zone.
     */
    bool inlineImageStrictlyInside;

    /** Keep form XObjects strictly included in a zone.
     */
    bool formStrictlyInside;

    /** Default constructor, initializing values to false.
     */
    PRLayoutParameters()
    {
        pagesRotation = 0;
        zoneClippingPath = false;
        pathStrictlyInside = pathReduce = false;
        textStrictlyInside = textReduce = false;
        imageStrictlyInside = false;
        inlineImageStrictlyInside = false;
        formStrictlyInside = false;
    }
};

/** A class which describes a new layout for a PdfMemDocument.
 * This class is thread-safe ( Hope so ! ).
 */
class PRDocumentLayout : public QObject
{
    Q_OBJECT

public:
    /** Default constructor.
     */
    PRDocumentLayout( QObject* parent = 0 );

    /** Initialize layout to a default empty template
     */
    void init();

    /** Add pages to the document layout.
     * Need write access on the class.
     * \param nbPages Number of pages to add.
     */
    void addPages( int nbPages );

    /** Add a page zone to the document layout.
     * Need write access on the class.
     * \param pageIndexOut Index of the page into the zone is inserted.
     * \param zoneIn Zone to insert in the new document.
     */
    void addPageZone( int pageIndexOut, const PRPageZone& zoneIn );

    /** Set page mediaBox and cropBox.
     * Need write access on the class.
     * \param pageIndexOut Index of the page to modify.
     * \param mediaBox Media box of the page.
     * \param cropBox Crop box of the page.
     */
    void setPageBoxes( int pageIndexOut,
                       const PoDoFo::PdfRect& mediaBox,
                       const PoDoFo::PdfRect& cropBox );

    /** Get page media box.
     * Need read access on the class.
     * \param pageIndexOut Index of the page.
     * \return Media box.
     */
    PoDoFo::PdfRect getPageMediaBox( int pageIndexOut ) const;

    /** Get page crop box.
     * Need read access on the class.
     * \param pageIndexOut Index of the page.
     * \return Crop box.
     */
    PoDoFo::PdfRect getPageCropBox( int pageIndexOut ) const;

    /** Set layout parameters used when a document is reorganized
     * Need write access on the class.
     * \param params Layout parameters.
     */
    void setLayoutParameters( const PRLayoutParameters& params );

    /** transformDocumenters used when a document is reorganized
     * Need read access on the class.
     * \return Layout parameters.
     */
    PRLayoutParameters getLayoutParameters() const;

signals:
    /** Progress signal sent by methods.
     * \param title Title of the current operation.
     * \param progress Progress, between 0 and 1.
     */
    void methodProgress( const QString& title, double progress ) const;

    /** Abort signal.
     */
    void methodAbort() const;

    /** Error signal.
     * \param title Title of the error.
     * \param description Description of the error.
     */
    void methodError( const QString& title, const QString& description ) const;

public slots:

    /** Write a Pdf document with the layout defined in the class.
     * Need read access on the class.
     * Need the PoDoFo mutex on the PdfDocumentHandle object.
     * \param documentHandle Document on which to apply the layout.
     * \param filename File where to save the resulting Pdf.
     */
    void writeLayoutToPdf( PRDocument* documentHandle,
                           const QString& filename );

    /** Write a Pdf document with an overlay of the layout corresponding to the
     * input document.
     * Need read access on the class.
     * Need the PoDoFo mutex on the PdfDocumentHandle object.
     * \param documentHandle Input document on which to print the layout.
     * \param filename File where to save the resulting Pdf.
     */
    void writeOverlayInToPdf( PRDocument* documentHandle,
                              const QString& filename );

    /** Write a Pdf document with an overlay of the layout corresponding to the
     * output document.
     * Need read access on the class.
     * Need the PoDoFo mutex on the PdfDocumentHandle object.
     * \param documentHandle Output document on which to print the layout.
     * \param filename File where to save the resulting Pdf.
     */
    void writeOverlayOutToPdf( PRDocument* documentHandle,
                               const QString& filename );
public:
    /** Abort current operation. Used in a context of multithread program where
     * slots are executed on another thread.
     * \param abort True to abort current method.
     */
    void setAbortOperation( bool abort = true );

protected:
    // Note that mutex/semaphore are not locked in protected functions as
    // we assume it has been done in public callers.

    /** Reorganize a PdfDocument according to the layout defined in this class.
     * Need read access on the class.
     * Need the PoDoFo mutex on the PdfDocumentHandle object.
     * \param documentHandle Object containing the PoDoFo document to modify.
     */
    void transformDocument( PRDocument* documentHandle ) const;

    /** Print the layout on the input document.
     * Need read access on the class.
     * Need the PoDoFo mutex on the PdfDocumentHandle object.
     * \param documentHandle Document on which to print the layout.
     */
    void printLayoutIn( PRDocument* documentHandle ) const;

    /** Print the layout of the output document.
     * Need read access on the class.
     * Need the PoDoFo mutex on the PdfDocumentHandle object.
     * \param documentHandle Document on which to print the layout.
     */
    void printLayoutOut( PRDocument* documentHandle ) const;

    /** Copy ressources from a page and add a prefix based on zone index.
     * \param pageOut Output page where resources are copied.
     * \param pageIn Input page where resources are read.
     * \param zoneIdx Zone index used to add a prefix to Pdf names.
     */
    void copyPageRessources( PoDoFo::PdfPage* pageOut,
                             PoDoFo::PdfPage* pageIn,
                             int zoneIdx ) const;

    /** Copy stream which corresponds to a zone.
     * \param streamOut Output stream where operators are written.
     * \param pageIn Input page where information is read.
     * \param zone Zone to consider in the input page.
     * \param zoneIdx Zone index in the output page.
     */
    void copyZoneStream( PoDoFo::PdfStream* streamOut,
                         PoDoFo::PdfPage* pageIn,
                         const PRPageZone& zone,
                         int zoneIdx ) const;

    /** Remove pages and PdfObjects corresponding to contents.
     * \param document Document where to remove pages.
     * \param firstPage Index of the first page.
     * \param nbPages Number of pages to delete.
     */
    void deletePagesAndContents( PoDoFo::PdfMemDocument* document,
                                 int firstPage, int nbPages ) const;

    /** Copy and modify outlines in a document to fit the new layout.
     * \param document Document to modify.
     * \param mapPageInZones Pages / Zones map.
     * \param origPagesNb Number of pages in the original document.
     */
    void copyOutlines( PoDoFo::PdfMemDocument* document,
                       std::map< PoDoFo::PdfReference, std::vector<const PRPageZone*> >& mapPageInZones,
                       int origPagesNb ) const;

    /** Copy and modify and outline item in a document to fit the new layout.
     * \param document Document to modify.
     * \param item Item to modify.
     * \param mapPageInZones Pages / Zones map.
     * \param origPagesNb Number of pages in the original document.
     */
    void copyOutlineItem( PoDoFo::PdfMemDocument* document,
                          PoDoFo::PdfOutlineItem* item,
                          std::map< PoDoFo::PdfReference, std::vector<const PRPageZone*> >& mapPageInZones,
                          int origPagesNb ) const;

    /** Copy and modify an action in a document to fit the new layout.
     * \param document Document to modify.
     * \param action Action to modify.
     * \param mapPageInZones Pages / Zones map.
     * \param origPagesNb Number of pages in the original document.
     */
    void copyAction( PoDoFo::PdfMemDocument* document,
                     PoDoFo::PdfObject* action,
                     std::map< PoDoFo::PdfReference, std::vector<const PRPageZone*> >& mapPageInZones,
                     int origPagesNb ) const;

    /** Copy and modify an action in a document to fit the new layout.
     * \param document Document to modify.
     * \param destination Destination to modify.
     * \param mapPageInZones Pages / Zones map.
     * \param origPagesNb Number of pages in the original document.
     */
    void copyDestination( PoDoFo::PdfMemDocument* document,
                          PoDoFo::PdfObject* destination,
                          std::map< PoDoFo::PdfReference, std::vector<const PRPageZone*> >& mapPageInZones,
                          int origPagesNb ) const;

    /** Copy and modify annotations in a document to fit the new layout.
     * \param document Document to modify.
     * \param mapPageInZones Pages / Zones map.
     * \param origPagesNb Number of pages in the original document.
     */
    void copyAnnotations( PoDoFo::PdfMemDocument* document,
                          std::map< PoDoFo::PdfReference, std::vector<const PRPageZone*> >& mapPageInZones,
                          int origPagesNb ) const;

    /** Copy and modify annotations in a page to fit the new layout.
     * \param document Document to modify.
     * \param idxPageIn Page where to consider annotations.
     * \param mapPageInZones Pages / Zones map.
     * \param origPagesNb Number of pages in the original document.
     */
    void copyPageAnnotations( PoDoFo::PdfMemDocument* document,
                              int idxPageIn,
                              std::map< PoDoFo::PdfReference, std::vector<const PRPageZone*> >& mapPageInZones,
                              int origPagesNb ) const;

    /** Copy and modify page labels in a document to fit the new layout.
     * \param document Document to modify.
     * \param vecPageInZones Pages / Zones correspondance.
     */
    void copyPageLabels( PoDoFo::PdfMemDocument* document,
                         std::vector< std::vector<const PRPageZone*> >& vecPageInZones ) const;

    /** Copy and modify a page label node in a document to fit the new layout.
     * \param document Document to modify.
     * \param node Node in the number tree to consider.
     * \param vecPageInZones Pages / Zones correspondance.
     * \return Min/Max values in this node and its children.
     */
    std::vector<int> copyPageLabelsNode( PoDoFo::PdfMemDocument* document,
                                         PoDoFo::PdfObject* node,
                                         std::vector< std::vector<const PRPageZone*> >& vecPageInZones ) const;

protected:
    /** Evaluate if a path intersects a zone.
     * \params points Points which constitute the path.
     * \param zone Rectangle representing the zone.
     * \param strictInclusion Evaluate if the path is stricly inside the zone.
     */
    static bool intersectZone( const std::vector<PdfVector>& points,
                               const PoDoFo::PdfRect& zone,
                               bool strictInclusion = false );

    /** Evaluate if a rectangle path intersects a zone.
     * \params path Rectangle path.
     * \param zone Rectangle representing the zone.
     * \param strictInclusion Evaluate if the path is stricly inside the zone.
     */
    static bool intersectZone( const PoDoFo::PdfRect& path,
                               const PoDoFo::PdfRect& zone,
                               bool strictInclusion = false );

    /** Reduce a point in order to be inside a given zone.
     * \param point Point which is modified.
     * \param zone Rectangle representing the zone to consider.
     */
    static void reduceToZone( PdfVector& point,
                              const PoDoFo::PdfRect& zone );

    /** Reduce the size of rectangle path in order to be inside a given zone.
     * \param points Rectangle path which is modified.
     * \param zone Rectangle representing the zone to consider.
     */
    static void reduceToZone( PoDoFo::PdfRect& path,
                              const PoDoFo::PdfRect& zone );

protected:
    /** Boolean used to indicate if a current operation should be aborted.
     */
    bool m_abortOperation;

    /** Internal semaphore used to protect data and multithread access.
     */
    PdfClassSemaphore m_semaphore;

    /** Vector of page layouts for the redesigned document.
     */
    std::vector<PRPageLayout> m_pageLayouts;

    /** Parameters used when a document is reorganized.
     */
    PRLayoutParameters m_parameters;

    /** Prefix used when import zones.
     */
    static std::string zonePrefix;
};

//**********************************************************//
//                      Inline methods                      //
//**********************************************************//
inline void PRDocumentLayout::setAbortOperation( bool abort )
{
    this->m_abortOperation = abort;
}

}

#endif // PRDOCUMENTLAYOUT_H
