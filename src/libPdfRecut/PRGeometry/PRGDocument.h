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

#ifndef PRGDOCUMENT_H
#define PRGDOCUMENT_H

#include <vector>
#include <QObject>

namespace PdfRecut {

class PRDocument;
class PRGSubDocument;
class PRGPage;

//************************************************************//
//                         PRGDocument                        //
//************************************************************//
/** Class that describes the basic geometry of PDF document.
 */
class PRGDocument : public QObject
{
    Q_OBJECT

public:
    /** Default constructor: initialize using a PRDocument parent
     * object.
     */
    explicit PRGDocument( PRDocument* parent = 0 );
    /** Destructor: delete PoDoFo document object, if loaded.
     */
    virtual ~PRGDocument();
    /** Reimplement QObject parent function.
     * \return Pointer to the parent PRDocument.
     */
    PRDocument* parent() const;

public:
    /// Structure which describes geometry analysis parameters.
    struct GParameters;

    // Contents related member functions.
    /** Analyse the geomtry of the PDF file.
     * Create an internal structure (based on PRGSubDocument, PRGPage, ...) that
     * describes the geometry of the PDF content.
     * \param params Parameters used for the analysis.
     */
    void analyse( const PRGDocument::GParameters& params );
    /** Clear the internal structure which describes the PDF geometry.
     */
    void clear();

private:
    /** Detect sub-documents inside the PDF.
     * \param tolerance Tolerance used for the size.
     */
    void detectSubDocuments( double tolerance );

public:
    // Getters...
    /// Number of sub-documents detected.
    size_t nbSubDocuments() const {     return m_subDocuments.size();   }
    /// Get a sub-document object.
    PRGSubDocument* subDocument( size_t idx )                {  return m_subDocuments.at( idx );    }
    const PRGSubDocument* subDocument( size_t idx ) const    {  return m_subDocuments.at( idx );    }
    /// Number of pages in the document.
    size_t nbPages() const;
    /// Get a geometry page object.
    PRGPage* page( size_t idx );
    const PRGPage* page( size_t idx ) const;

private:
    // No copy constructor and operator= allowed.
    Q_DISABLE_COPY(PRGDocument)

private:
    // PDF geometrical content.
    /// Vector of sub-documents.
    std::vector<PRGSubDocument*>  m_subDocuments;
};

//************************************************************//
//                   PRGDocument::GParameters                 //
//************************************************************//
/** Structure that describes PRGDocument geometry analysis parameters.
 */
struct PRGDocument::GParameters
{
    /// Sub-document tolerance (relative) for the page size.
    double  subDocumentTolerance;
    /// Perform the text line detection.
    bool  textLineDetection;

    /// Default constructor.
    GParameters();
    /// Initialize to default values.
    void init();
};


}

#endif // PRGDOCUMENT_H
