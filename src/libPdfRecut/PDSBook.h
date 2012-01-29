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

#ifndef PDSBOOK_H
#define PDSBOOK_H

#include "PdfDocumentStructure.h"

namespace PdfeBooker {


struct PDSBookPageData
{
    PoDoFo::PdfRect mediaBox;

    PoDoFo::PdfRect cropBox;

    PoDoFo::PdfRect dataBox;

    std::vector<unsigned long> horizontalData;

    std::vector<unsigned long> verticalData;

};

struct PDSBookPageStructure
{
    bool specialPage;

    bool breakpage;

    PoDoFo::PdfRect headerBox;

    PoDoFo::PdfRect contentBox;

    PoDoFo::PdfRect footerBox;
};

struct PDSBookStatistics
{

};


class PDSBook : public PdfDocumentStructure
{
public:
    PDSBook( QObject* parent = 0 );

public slots:
    /** Analyse structure of a Pdf document.
     * \param documentHandle Document to analyse.
     */
    virtual void analyseDocument( PdfDocumentHandle* documentHandle );

    /** Generate a document layout based on the document structure.
     * \param layout Layout to overwrite.
     */
    virtual void generateLayout( PdfDocumentLayout* layout );
};

}

#endif // PDSBOOK_H
