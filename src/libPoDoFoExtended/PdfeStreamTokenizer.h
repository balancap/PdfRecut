/***************************************************************************
 *   Copyright (C) 2007 by Dominik Seichter                                *
 *   domseichter@web.de                                                    *
 *   Copyright (C) 2011 by Paul Balança                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef PDFESTREAMTOKENIZER_H
#define PDFESTREAMTOKENIZER_H

#include "PdfeGraphicsOperators.h"
#include "podofo/base/PdfContentsTokenizer.h"

namespace PdfRecut {

/** This class is a parser for content streams in PDF documents.
 * Reimplementation of the class PdfContentsTokenizer with slight modifications
 * in the ReadNext function.
 */
class PdfeStreamTokenizer : public PoDoFo::PdfTokenizer {
public:

    /** Construct a PdfeStreamTokenizer from an existing buffer. Usually a stream from a PdfPage.
     *  \param pBuffer pointer to a buffer.
     *  \param lLen length of the buffer.
     */
    PdfeStreamTokenizer( const char* pBuffer, long lLen )
        : PoDoFo::PdfTokenizer( pBuffer, lLen ), m_readingInlineImgData(false)
    {
    }

    /** Construct a PdfeStreamTokenizer from a PdfCanvas (i.e. PdfPage or a PdfXObject).
     *  \param pCanvas an object that hold a PDF contents stream
     */
    PdfeStreamTokenizer( PoDoFo::PdfCanvas* pCanvas );

    virtual ~PdfeStreamTokenizer() { }

    /** Reads the next token from the current file position ignoring all comments.
     *  \param pszToken On true return, set to a pointer to the read token (a NULL-terminated C string).
     *                  The pointer is to memory owned by PdfTokenizer and must NOT be freed.
     *  \param peType On true return, if not NULL the type of the read token will be stored into this parameter.
     *                Undefined on false return.
     *  \returns True if a token was read, false if there are no more tokens to read.
     */
    bool GetNextToken( const char *& pszToken, PoDoFo::EPdfTokenType* peType = NULL);

    /** Read the next keyword or variant, returning true and setting reType if something was read.
     *  Either op or variant, but never both, have defined and usable values on
     *  true return, with which being controlled by the value of type.
     *
     *  If EOF is encountered, returns false and leaves type, op and variant undefined.
     *  As a special case, type may be set to ePdfContentsType_ImageData. In
     *  this case op is undefined, and variant contains the byte sequence between the ID
     *  and BI keywords sans the one byte of leading- and trailing- white space. No filter
     *  decoding is performed.
     *
     *  \param type will be set to either keyword or variant if true is returned. Undefined if false is returned.
     *  \param op if type is set to ePdfContentsType_Keyword this will point to the keyword.
     *  \param variant if type is haveset to ePdfContentsType_Variant or ePdfContentsType_ImageData
     *              this will be set to the read variant, otherwise the value is undefined.
     */
    bool ReadNext( PoDoFo::EPdfContentsType& type, PdfeGraphicOperator& op, std::string& variant );

    /** Read a dictionary from the input device and store it into a std string.
     *  \param variant store the dictionary into this variable.
     */
    void ReadDictionary( std::string& variant );

    /** Read an array from the input device and store it into a std string.
     *  \param variant store the array into this variable.
     */
    void ReadArray( std::string& variant );

    /** Read a string from the input device and store it into a std string.
     *  \param variant store the string into this variable.
     */
    void ReadString( std::string& variant );

    /** Read an hex string from the input device and store it into a std string.
     *  \param variant store the hex string into this variable.
     */
    void ReadHexString( std::string& variant );

    /** Read a name from the input device and store it into a std string.
     *  \param variant store the name into this variable.
     */
    void ReadName(std::string& variant );

 private:
    /** Set another objects stream as the current stream for parsing.
     *  \param pObject use the stream of this object for parsing.
     */
    void SetCurrentContentsStream( PoDoFo::PdfObject* pObject );

    /** Read inline image in the current stream.
     */
    bool ReadInlineImgData( PoDoFo::EPdfContentsType& type, std::string& variant );

 private:
    /// A list containing pointers to all contents objects.
    std::list<PoDoFo::PdfObject*>  m_lstContents;

    /// At stage of reading inline image data?
    bool m_readingInlineImgData;
};

}

#endif // PDFESTREAMTOKENIZER_H
