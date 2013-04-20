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

#include "PdfeTextElement.h"
#include "PdfeFont.h"

#include <podofo/podofo.h>

using namespace PoDoFo;

namespace PoDoFoExtended {

//**********************************************************//
//                        PdfeTextWord                      //
//**********************************************************//
PdfeTextWord::PdfeTextWord()
{
    this->init();
}
PdfeTextWord::PdfeTextWord( const PdfeCIDString& cidword,
                            PdfeTextWordType::Enum type,
                            PdfeTextElement* pTextElement )
{
    this->init( cidword, type, pTextElement );
}
PdfeTextWord::PdfeTextWord( double transWidth,
                            PdfeTextWordType::Enum type,
                            PdfeTextElement* pTextElement )
{
    this->init( transWidth, type, pTextElement );
}
PdfeTextWord::PdfeTextWord( const PdfeTextWord& rhs ) :
    m_pTextElement( rhs.m_pTextElement ),
    m_nodeSubID( rhs.m_nodeSubID ),
    m_type( rhs.m_type ),
    // m_pdfWord
    m_cidWord( rhs.m_cidWord ),
    m_advance( rhs.m_advance ),
    m_bbox( rhs.m_bbox )
{
}
PdfeTextWord& PdfeTextWord::operator=( const PdfeTextWord& rhs )
{
    m_pTextElement = rhs.m_pTextElement;
    m_nodeSubID = rhs.m_nodeSubID;
    m_type = rhs.m_type;
    // m_pdfWord
    m_cidWord = rhs.m_cidWord;
    m_advance = rhs.m_advance;
    m_bbox = rhs.m_bbox;
    return *this;
}

void PdfeTextWord::init()
{
    m_pTextElement = NULL;
    m_nodeSubID = NodeSubIDUndefined();
    m_type = PdfeTextWordType::Unknown;
    //m_pdfWord = PdfString();
    m_cidWord.clear();
    m_advance.init();
    m_bbox = PdfRect( 0,0,0,0 );
}
void PdfeTextWord::init( const PdfeCIDString& cidword,
                         PdfeTextWordType::Enum type,
                         PdfeTextElement* pTextElement )
{
    // Check input arguments...
    if( !pTextElement || ( type != PdfeTextWordType::Classic && type != PdfeTextWordType::Space ) ) {
        PODOFO_RAISE_ERROR_INFO( ePdfError_InternalLogic,
                                 "<PdfeTextWord> Invalid arguments for construction/initialization of word." );
    }
    m_pTextElement = pTextElement;
    m_nodeSubID = NodeSubIDUndefined();
    m_type = type;
    // m_pdfWord.
    m_cidWord = cidword;

    // Compute advance vector and bbox.
    PdfeFont* pfont = m_pTextElement->font();
    m_advance = pfont->advance( m_cidWord );
    m_bbox = pfont->bbox( m_cidWord );

    // Custom bbox for space words.
    if( m_type == PdfeTextWordType::Space ) {
        m_bbox.SetBottom( 0.0 );
        m_bbox.SetHeight( pfont->spaceHeight() );
        m_bbox.SetLeft( 0.0 );
        m_bbox.SetWidth( m_advance( 0 ) );
    }
    else {
        // Check minimal height for classic words.
        double minHeight = pfont->spaceHeight() * MinHeightScale;
        if( m_bbox.GetHeight() < minHeight ) {
            // Custom weighted increase.
            m_bbox.SetBottom( m_bbox.GetBottom() - minHeight / 4 );
            m_bbox.SetHeight( m_bbox.GetHeight() + minHeight );
        }
    }
}
void PdfeTextWord::init( double transWidth,
                         PdfeTextWordType::Enum type,
                         PdfeTextElement* pTextElement )
{
    // Check input arguments...
    if( !pTextElement || ( type != PdfeTextWordType::PDFTranslation && type != PdfeTextWordType::PDFTranslationCS ) ) {
        PODOFO_RAISE_ERROR_INFO( ePdfError_InternalLogic,
                                 "<PdfeTextWord> Invalid arguments for construction/initialization of PDF translation word." );
    }
    m_pTextElement = pTextElement;
    m_nodeSubID = NodeSubIDUndefined();
    m_type = type;
    // m_pdfWord.
    m_cidWord.clear();

    // Advance vector and bbox.
    m_advance = PdfeVector( transWidth, 0.0 );
    m_bbox = PdfRect( 0.0, 0.0, transWidth, m_pTextElement->font()->spaceHeight() );
}

void PdfeTextWord::setTextElement( PdfeTextElement* pTextElement )
{
    if( !pTextElement ) {
        PODOFO_RAISE_ERROR_INFO( ePdfError_InternalLogic,
                                 "<PdfeTextWord> Invalid parent text element (nullptr)." );
    }
    m_pTextElement = pTextElement;
}

PdfRect PdfeTextWord::bbox( bool useBottomCoord ) const
{
    if( useBottomCoord ) {
        return m_bbox;
    }
    // Set bottom to zero.
    PoDoFo::PdfRect bbox = m_bbox;
    bbox.SetHeight( bbox.GetHeight() + bbox.GetBottom() );
    bbox.SetBottom( 0.0 );
    return bbox;
}
QString PdfeTextWord::toUnicode() const
{
    QString ustr;
    if( m_type == PdfeTextWordType::Classic ) {
        ustr = m_pTextElement->font()->toUnicode( m_cidWord );
    }
    else if( m_type == PdfeTextWordType::Space ||
             m_type == PdfeTextWordType::PDFTranslation ||
             m_type == PdfeTextWordType::PDFTranslationCS ) {
        ustr = QString( " " );
    }
    // Unknown character...
    //qDebug() << QChar(0xFFFD);
    return ustr;
}

//**********************************************************//
//                       PdfeTextElement                    //
//**********************************************************//
PdfeTextElement::PdfeTextElement( const PdfeGraphicsState& gstate, PdfeFont* pfont ) :
    PdfeGElement()
{
    this->init( gstate, pfont );
}
PdfeTextElement::~PdfeTextElement()
{
    // Guess what, I am so perfect that I don't need to be destroy!
}
void PdfeTextElement::init( const PdfeGraphicsState& gstate, PdfeFont* pfont )
{
    this->PdfeGElement::init();
    if( !pfont ) {
        PODOFO_RAISE_ERROR_INFO( ePdfError_InternalLogic,
                                 "<PdfeTextElement> Invalid font pointer passed as argument." );
    }
    this->setGState( gstate );
    m_pFont = pfont;
}

PdfeContentsStream::Node* PdfeTextElement::load( PdfeContentsStream::Node* pnode )
{
    return pnode;
}
PdfeContentsStream::Node* PdfeTextElement::save( PdfeContentsStream::Node* pnode,
                                                 PdfeGElementSave::Enum savePolicy ) const
{
    return pnode;
}

}
