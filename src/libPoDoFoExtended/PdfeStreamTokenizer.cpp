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

#include "PdfeStreamTokenizer.h"

#include "podofo/base/PdfCanvas.h"
#include "podofo/base/PdfInputDevice.h"
#include "podofo/base/PdfOutputStream.h"
#include "podofo/base/PdfStream.h"
#include "podofo/base/PdfVecObjects.h"
#include "podofo/base/PdfData.h"

#include <stdio.h>

#define PDF_BUFFER 4096

#define DICT_SEP_LENGTH 2
#define NULL_LENGTH     4
#define TRUE_LENGTH     4
#define FALSE_LENGTH    5

using namespace PoDoFo;

namespace PoDoFoExtended {

PdfeStreamTokenizer::PdfeStreamTokenizer( PdfCanvas* pCanvas )
    : PdfTokenizer(), m_readingInlineImgData( false )
{
    if( !pCanvas )
    {
        PODOFO_RAISE_ERROR( ePdfError_InvalidHandle );
    }

    PdfObject* pContents = pCanvas->GetContents();
    if( pContents && pContents->IsArray()  )
    {
        PdfArray& a = pContents->GetArray();
        for ( PdfArray::iterator it = a.begin(); it != a.end() ; ++it )
        {
            if ( !(*it).IsReference() )
            {
                PODOFO_RAISE_ERROR_INFO( ePdfError_InvalidDataType, "/Contents array contained non-references" );
            }

            m_lstContents.push_back( pContents->GetOwner()->GetObject( (*it).GetReference() ) );
        }
    }
    else if ( pContents && pContents->HasStream() )
    {
        m_lstContents.push_back( pContents );
    }
    else if ( pContents && pContents->IsDictionary() )
    {
        m_lstContents.push_back( pContents );
        PdfError::LogMessage(eLogSeverity_Information,
                  "PdfContentsTokenizer: found canvas-dictionary without stream => empty page");
        // OC 18.09.2010 BugFix: Found an empty page in a PDF document:
        //    103 0 obj
        //    <<
        //    /Type /Page
        //    /MediaBox [ 0 0 595 842 ]
        //    /Parent 3 0 R
        //    /Resources <<
        //    /ProcSet [ /PDF ]
        //    >>
        //    /Rotate 0
        //    >>
        //    endobj
    }
    else
    {
        PODOFO_RAISE_ERROR_INFO( ePdfError_InvalidDataType, "Page /Contents not stream or array of streams" );
    }

    if( m_lstContents.size() )
    {
        SetCurrentContentsStream( m_lstContents.front() );
        m_lstContents.pop_front();
    }
}

void PdfeStreamTokenizer::SetCurrentContentsStream( PdfObject* pObject )
{
    PODOFO_RAISE_LOGIC_IF( pObject == NULL, "Content stream object == NULL!" );

    PdfStream* pStream = pObject->GetStream();

    PdfRefCountedBuffer buffer;
    PdfBufferOutputStream stream( &buffer );
    pStream->GetFilteredCopy( &stream );

    m_device = PdfRefCountedInputDevice( buffer.GetBuffer(), buffer.GetSize() );
}

bool PdfeStreamTokenizer::GetNextToken( const char*& pszToken , EPdfTokenType* peType )
{
    bool result = PdfTokenizer::GetNextToken(pszToken, peType);
    while (!result) {
        if( !m_lstContents.size() )
            return false;

        SetCurrentContentsStream( m_lstContents.front() );
        m_lstContents.pop_front();
        result = PdfTokenizer::GetNextToken(pszToken, peType);
    }
    return result;
}

bool PdfeStreamTokenizer::ReadNext( PoDoFo::EPdfContentsType& type, PdfeGraphicOperator& op, std::string& variant )
{
    // Reading inline image.
    if (m_readingInlineImgData)
    {
        op.init();
        return ReadInlineImgData( type, variant );
    }

    // Read token.
    EPdfTokenType eTokenType;
    const char*   pszToken;
    bool gotToken = this->GetNextToken( pszToken, &eTokenType );

    // Necessary to switch to next stream ?
    if ( !gotToken )
    {
        if ( m_lstContents.size() ) {
            // We ran out of tokens in this stream. Switch to the next stream and try again.
            SetCurrentContentsStream( m_lstContents.front() );
            m_lstContents.pop_front();
            return ReadNext( type, op, variant );
        }
        else {
            // No more content stream tokens to read.
            return false;
        }
    }

    if( eTokenType == ePdfTokenType_Token )
    {
        // Set operator from token read.
        op.set( pszToken );

        // Assume it is variant when operator is set to Unknown.
        if( op.type() == PdfeGOperator::Unknown )
        {
            type = ePdfContentsType_Variant;
            variant = pszToken;
        }
        else
        {
            type = ePdfContentsType_Keyword;
            variant.clear();

            // Token == ID: read inline image at the next step.
            if( op.type() == PdfeGOperator::ID ) {
                m_readingInlineImgData = true;
            }
        }
    }
    else if( eTokenType == ePdfTokenType_Delimiter )
    {
        type = ePdfContentsType_Variant;
        op.init();

        // Check the kind of delimiter it corresponds (dictionnary, array, string).
        if( strncmp( "<<", pszToken, DICT_SEP_LENGTH ) == 0 )
            this->ReadDictionary( variant );
        else if( pszToken[0] == '[' )
            this->ReadArray( variant );
        else if( pszToken[0] == '(' )
            this->ReadString( variant );
        else if( pszToken[0] == '<' )
            this->ReadHexString( variant );
        else if( pszToken[0] == '/' )
            this->ReadName( variant );

        return true;
    }
    else
    {
        op.init();
        variant.clear();
        return false;
    }
    return true;
}

void PdfeStreamTokenizer::ReadDictionary( std::string& variant )
{
    // Basic obtention. To be optimized...
    PdfVariant val;
    this->PdfTokenizer::ReadDictionary( val, NULL );
    val.ToString( variant, ePdfWriteMode_Compact );
}
void PdfeStreamTokenizer::ReadArray( std::string& variant )
{
    // Basic obtention. To be optimized...
    PdfVariant val;
    this->PdfTokenizer::ReadArray( val, NULL );
    val.ToString( variant, ePdfWriteMode_Compact );
}
void PdfeStreamTokenizer::ReadString( std::string& variant )
{
    // Basic obtention. To be optimized...
    PdfVariant val;
    this->PdfTokenizer::ReadString( val, NULL );
    val.ToString( variant, ePdfWriteMode_Compact );
}
void PdfeStreamTokenizer::ReadHexString( std::string& variant )
{
    // Basic obtention. To be optimized...
    //PdfVariant val;
    //this->PdfTokenizer::ReadHexString( val, NULL );
    //val.ToString( variant, ePdfWriteMode_Clean );

    // Read hex string.
    variant = '<';
    int c;
    while( (c = m_device.Device()->GetChar()) != EOF )
    {
        // End of hex string.
        if( c == '>' ) {
            break;
        }

        // Add hexa characters.
        if( isdigit( c ) || ( c >= 'A' && c <= 'F') || ( c >= 'a' && c <= 'f') ) {
            variant.push_back( c );
        }
    }
    variant.push_back( '>' );
}

void PdfeStreamTokenizer::ReadName(std::string& variant )
{
    // Basic obtention. To be optimized...
    PdfVariant val;
    this->PdfTokenizer::ReadName( val );
    val.ToString( variant, ePdfWriteMode_Compact );
}

bool PdfeStreamTokenizer::ReadInlineImgData( PoDoFo::EPdfContentsType& type, std::string& variant )
{
    int  c;
    long long  counter  = 0;
    if( !m_device.Device() ) {
        PODOFO_RAISE_ERROR( ePdfError_InvalidHandle );
    }

    // Consume the only whitespace between ID and data
    c = m_device.Device()->Look();
    if( PdfTokenizer::IsWhitespace( c ) ) {
        c = m_device.Device()->GetChar();
    }

    while((c = m_device.Device()->Look()) != EOF)
    {
        c = m_device.Device()->GetChar();
        if (c=='E' &&  m_device.Device()->Look()=='I')
        {
            // Consume character
            m_device.Device()->GetChar();
            char w = m_device.Device()->Look();
            if (w==EOF || PdfTokenizer::IsWhitespace(w))
            {
                // EI is followed by whitespace => stop
                m_device.Device()->Seek(-2, std::ios::cur); // put back "EI"
                m_buffer.GetBuffer()[counter] = '\0';

                variant = m_buffer.GetBuffer();
                type = ePdfContentsType_ImageData;
                m_readingInlineImgData = false;
                return true;
            }
            else
            {
                // no whitespace after EI => do not stop
                m_device.Device()->Seek(-1, std::ios::cur); // put back "I"
                m_buffer.GetBuffer()[counter] = c;
                ++counter;
            }
        }
        else
        {
            m_buffer.GetBuffer()[counter] = c;
            ++counter;
        }
        if (counter ==  static_cast<long long>(m_buffer.GetSize()))
        {
            // image is larger than buffer => resize buffer
            m_buffer.Resize(m_buffer.GetSize()*2);
        }
    }
    variant.clear();
    return false;
}

}
