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

#ifndef PDFETEXTELEMENT_H
#define PDFETEXTELEMENT_H

#include "PdfeGElement.h"

namespace PoDoFoExtended {

class PdfeFont;

//**********************************************************//
//                        PdfeTextWord                      //
//**********************************************************//
namespace PRGTextWordType {
/// Different types of word allowed.
enum Enum {
    Classic = 0,        /// Classic word.
    Space,              /// Space characters (c.f. PdfeFont space characters).
    PDFTranslation,     /// PDF translation (c.f. TJ operator).
    PDFTranslationCS,   /// PDF translation replacing char space.
    Unknown             /// Unknown...
};
}

/** Class whose aims is to represent a basic text element used in
 * a contents stream. It can be a usual word or a space (different
 * types possible...)
 * It should always belong to a PdfeTextElement, and therefore
 * has a corresponding sub-node ID, and has a font specified.
 */
class PdfeTextWord
{
public:
    PdfeTextWord();

private:
    /// Pointer to the font object.
    PoDoFoExtended::PdfeFont*  m_pFont;
    /// PDF string corresponding to the classic word.
    PoDoFo::PdfString  m_pdfWord;
    /// CID string corresponding to the classic word.
    PdfeCIDString  m_cidWord;
    /// Word type.
    PRGTextWordType::Enum  m_type;

    /// Advance vector of the word.
    PdfeVector  m_advance;
    /// Bounding box in local coordinates.
    PoDoFo::PdfRect  m_bbox;

    /// Char space parameter used for the word.
    double  m_charSpace;
};

//**********************************************************//
//                       PdfeTextElement                    //
//**********************************************************//
/** Class representing a text element in PDF contents stream.
 * It is made of a collection of PdfeTextWord(s). It should be
 * the main interface for manipulating text elements from PDF
 * contents streams (it thereby inherits from PdfeGElement).
 */
class PdfeTextElement : PdfeGElement
{
public:
    PdfeTextElement();
};

}

#endif // PDFETEXTELEMENT_H
