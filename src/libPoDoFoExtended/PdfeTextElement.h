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

class PdfeGraphicsState;
class PdfeFont;

//**********************************************************//
//                        PdfeTextWord                      //
//**********************************************************//
namespace PdfeTextWordType {
/// Different types of word allowed.
enum Enum {
    Classic = 0,        /// Classic word.
    Space,              /// Space character (c.f. PdfeFont space characters).
    PDFTranslation,     /// PDF translation (c.f. TJ operator).
    PDFTranslationCS,   /// PDF translation replacing some char space.
    Unknown             /// Unknown...
};
}

class PdfeTextElement;

/** Class whose aims is to represent a basic text element used in
 * a contents stream. It can be a usual word or a space (different
 * types possible...)
 * It should always belong to a PdfeTextElement object, and therefore
 * has a corresponding sub-node ID.
 */
class PdfeTextWord
{
    // PdfeTextElement friend class, i.e. abled to modify parent element.
    friend class PdfeTextElement;

public:
    /** Empty word constructor, with unknown type and no parent text element.
     */
    PdfeTextWord();
    /** Construct a word (classic or space) from a CID string.
     * TODO: replace with PDF string.
     * \param cidword Word (CID representation).
     * \param type Type of the word (should be classic or space).
     * \param pTextElement Parent text element (should not be nullptr).
     */
    PdfeTextWord( const PdfeCIDString& cidword,
                  PdfeTextWordType::Enum type,
                  PdfeTextElement* pTextElement );
    /** Create a translation word, with a given width.
     * \param transWidth Translation width.
     * \param type Type of translation (common or CS).
     * \param pTextElement Parent text element (should not be nullptr).
     */
    PdfeTextWord( double transWidth,
                  PdfeTextWordType::Enum type,
                  PdfeTextElement* pTextElement );

    /** Copy constructor.
     */
    PdfeTextWord( const PdfeTextWord& rhs );
    /** Assignment operator.
     */
    PdfeTextWord& operator=( const PdfeTextWord& rhs );

public:
    /** Initialize to an empty word, with unknown type and
     * no parent text element.
     */
    void init();

private:
    // Keep them private for now, since it should only be used
    // by PdfeTextElement.
    /** Initialize a word (classic or space) from a CID string.
     * The node sub-ID is set to undefined.
     * TODO: replace with PDF string.
     * \param cidword Word (CID representation).
     * \param type Type of the word (should be classic or space).
     * \param pTextElement Parent text element (should not be nullptr).
     */
    void init( const PdfeCIDString& cidword,
               PdfeTextWordType::Enum type,
               PdfeTextElement* pTextElement );
    /** Initialize to a translation word, with a given width.
     * The node sub-ID is set to undefined.
     * \param transWidth Translation width.
     * \param type Type of translation (common or CS).
     * \param pTextElement Parent text element (should not be nullptr).
     */
    void init( double transWidth,
               PdfeTextWordType::Enum type,
               PdfeTextElement* pTextElement );

public:
    // Simple getters...
    /// Parent text element (can be null for empty word).
    PdfeTextElement* textElement() const    {   return m_pTextElement;  }
    /// Text word node sub ID.
    pdfe_nodesubid nodeSubID() const        {   return m_nodeSubID;     }
    /// Word type.
    PdfeTextWordType::Enum type() const     {   return m_type;          }
    //const PoDoFo::PdfString& pdfString() const  {   return m_pdfString; }
    /// CID representation of the word.
    const PdfeCIDString& cidString() const  {   return m_cidWord;       }
    /// Lenght of the word (zero for translation words).
    size_t length() const       {   return m_cidWord.length();      }


private:
    // Keep them private for now.
    /// Set the parent text element.
    void setTextElement( PdfeTextElement* pTextElement );
    /// Set text word node sub ID.
    void setNodeSubID( pdfe_nodesubid id )  {   m_nodeSubID = id;   }

public:
    /** Get word's advance/displacement vector.
     * \return Advance vector of the word.
     */
    PdfeVector advance() const  {   return m_advance;   }
    /** Get word's bounding box.
     * \param useBottomCoord Use the bottom coordinate of the bbox?
     * \return Rectangle representing the bounding box.
     */
    PoDoFo::PdfRect bbox( bool useBottomCoord = true ) const;
    /** Get the unicode string corresponding to the word. By default,
     * a space character is used to represent a translation word.
     * \return QString containing the unicode word. Empty if can not
     * translate into unicode.
     */
    QString toUnicode() const;

private:
    /** Minimal height scale factor for a word. Use default space
     * height as reference.
     */
    static const double MinHeightScale = 0.3;

private:
    /// Parent text element.
    PdfeTextElement*  m_pTextElement;
    /// Subpath node sub ID inside the path.
    pdfe_nodesubid  m_nodeSubID;
    /// Word type.
    PdfeTextWordType::Enum  m_type;
    /// PDF string containing the classic word: TODO.
    //PoDoFo::PdfString  m_pdfWord;
    /// CID expression of the classic word.
    PdfeCIDString  m_cidWord;

    /// Advance vector of the word.
    PdfeVector  m_advance;
    /// Bounding box in local coordinates.
    PoDoFo::PdfRect  m_bbox;
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
    /** Construct the PDF element given a graphics state
     * and PDF font object (the pair should be consistent!).
     * @param gstate Graphics state.
     * @param pfont Font object consistent with the
     * text state entry (should not be NULL!).
     */
    PdfeTextElement( const PdfeGraphicsState& gstate, PdfeFont* pfont );
    /** Destructor.
     */
    ~PdfeTextElement();
    /** (Deep) copy constructor.
     */
    PdfeTextElement( const PdfeTextElement& rhs );
    /** Assignment operator.
     */
    PdfeTextElement operator=( const PdfeTextElement& rhs );

    /** Initialize the PDF element with a graphics state
     * and PDF font object (the pair should be consistent!).
     * @param gstate Graphics state.
     * @param pfont Font object consistent with the
     * text state entry (should not be NULL!).
     */
    void init( const PdfeGraphicsState& gstate, PdfeFont* pfont);

public:
    // Text element loading and saving from stream.
    /** Load a text element from a node in a contents stream. The node ID is
     * saved back, but the graphics state and the font are set unchanged.
     * @param pnode Pointer to the text showing node.
     * @return pnode value (a text element is one node long).
     */
    PdfeContentsStream::Node* load( PdfeContentsStream::Node* pnode );
    /** Save back the path into a contents stream.
     * @param pnode Pointer of the node where to insert the text element.
     * @param savePolicy Save policy of the element (replace/push back/push front).
     * @return pnode value (a text element is one node long).
     */
    PdfeContentsStream::Node* save( PdfeContentsStream::Node* pnode,
                                    PdfeGElementSave::Enum savePolicy = PdfeGElementSave::PushBack ) const;


public:
    // Simple getters...
    /// Get the font object attached to the text element.
    PoDoFoExtended::PdfeFont* font() const      {   return m_pFont;     }



private:
    /// Font object used by the text element.
    PoDoFoExtended::PdfeFont*  m_pFont;
    /// Words which form the text element.
    std::vector<PdfeTextWord>  m_words;


};

}

#endif // PDFETEXTELEMENT_H
