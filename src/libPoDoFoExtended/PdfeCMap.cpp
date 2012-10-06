/***************************************************************************
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

#include "PdfeCMap.h"
#include "podofo/podofo.h"

using namespace PoDoFo;

namespace PoDoFoExtended {

//**********************************************************//
//                     PdfCIDSystemInfo                     //
//**********************************************************//
PdfeCIDSystemInfo::PdfeCIDSystemInfo()
{
    this->init();
}
void PdfeCIDSystemInfo::init()
{
    registery.clear();
    ordering.clear();
    supplement = 0;
}
void PdfeCIDSystemInfo::init( PdfObject* cidSysInfoObj )
{
    if( !cidSysInfoObj ) {
        return;
    }
    PdfObject* tmpObj;

    tmpObj = cidSysInfoObj->GetIndirectKey( "Registry" );
    if( tmpObj && tmpObj->IsString() ) {
        registery = tmpObj->GetString().GetString();
    }
    tmpObj = cidSysInfoObj->GetIndirectKey( "Ordering" );
    if( tmpObj && tmpObj->IsString() ) {
        ordering = tmpObj->GetString().GetString();
    }
    tmpObj = cidSysInfoObj->GetIndirectKey( "Supplement" );
    if( tmpObj && tmpObj->IsNumber() ) {
        supplement = tmpObj->GetNumber();
    }
}

//**********************************************************//
//                          PdfCMap                         //
//**********************************************************//
PdfeCMap::PdfeCMap() :
    m_baseCMap( NULL )
{
    this->init();
}
PdfeCMap::PdfeCMap( const PdfName& cmapName ) :
    m_baseCMap( NULL )
{
    this->init( cmapName );
}
PdfeCMap::PdfeCMap( PdfObject* cmapStream ) :
    m_baseCMap( NULL )
{
    this->init( cmapStream );
}
PdfeCMap::~PdfeCMap()
{
    delete m_baseCMap;
}

void PdfeCMap::init()
{
    // Delete existing base CMap if necessary.
    delete m_baseCMap;

    // Default values given in the PdfReference.
    m_wmode = 0;
    m_identity = false;
    m_baseCMap = NULL;

    // Clear vectors.
    m_codeSpaceRanges.clear();
    m_bfRanges.clear();
}
void PdfeCMap::init( const PdfName& cmapName )
{
    // Init to default values.
    this->init();
    m_name = cmapName;
    std::string strCmapName = m_name.GetName();
    m_wmode = 0;
    m_baseCMap = NULL;

    // Identity-H or Identity-V CMaps
    if( strCmapName == "Identity-H" ) {
        m_identity = true;
        return;
    }
    else if( strCmapName == "Identity-V" ) {
        m_identity = true;
        m_wmode = true;
        return;
    }

    // TODO: implement other predefined CMaps that should be loaded from a file.
}
void PdfeCMap::init( PdfObject* pCMapObj )
{
    // Init to default values.
    this->init();

    // Read CMap parameters from the PdfObject dictionary.
    PdfObject* pObj;

    pObj = pCMapObj->GetIndirectKey( "CMapName" );
    if( pObj && pObj->IsName() ) {
        m_name = pObj->GetName();
    }
    pObj = pCMapObj->GetIndirectKey( "WMode" );
    if( pObj && pObj->IsNumber() ) {
        m_wmode = pObj->GetNumber();
    }
    m_CIDSystemInfo.init( pCMapObj->GetIndirectKey( "CIDSystemInfo" ) );

    // CMap used as base for this one: create the corresponding object.
    pObj = pCMapObj->GetIndirectKey( "UseCMap" );
    if( pObj && pObj->IsName() ) {
        m_baseCMap = new PdfeCMap( pObj->GetName() );
    }
    else if( pObj ) {
        m_baseCMap = new PdfeCMap( pObj );
    }
    else {
        m_baseCMap = NULL;
    }

    // Load CMap content from stream.
    if( pCMapObj->HasStream() ) {
        // Copy stream in a buffer.
        char* pBuffer;
        long length;
        pCMapObj->GetStream()->GetFilteredCopy( &pBuffer, &length );

        // TODO: boost::shared_ptr...

        // Load content from buffer.
        this->loadContent( pBuffer, length );

        free( pBuffer );
    }
}

void PdfeCMap::loadContent( const char *pBuffer, long length )
{
    // Content tokenizer.
    PdfContentsTokenizer tokenizer( pBuffer, length );

    // Returned variables.
    EPdfContentsType eType;
    const char* pKeyword;
    PdfVariant variant;

    // Buffered variant.
    PdfVariant bufVariant;

    // Read CMap content.
    while( tokenizer.ReadNext( eType, pKeyword, variant ) ) {
        if( eType == ePdfContentsType_Keyword ) {
            // Read a CMap keyword.
            if( !strcmp( pKeyword, "begincodespacerange" ) ) {
                this->loadCodeSpaceRange( tokenizer, bufVariant.GetNumber() );
            }
            else if( !strcmp( pKeyword, "beginbfchar" ) ) {
                this->loadBFChars( tokenizer, bufVariant.GetNumber() );
            }
            else if( !strcmp( pKeyword, "beginbfrange" ) ) {
                this->loadBFRanges( tokenizer, bufVariant.GetNumber() );
            }
            else if( !strcmp( pKeyword, "begincidchar" ) ) {

            }
            else if( !strcmp( pKeyword, "begincidrange" ) ) {

            }
            else if( !strcmp( pKeyword, "beginnotdefchar" ) ) {

            }
            else if( !strcmp( pKeyword, "beginnotdefrange" ) ) {

            }
//            std::cout << eType << " : " << pKeyword << std::endl;
        }
        else if( eType == ePdfContentsType_Variant ) {
            // Kee variant buffered.
            bufVariant = variant;

//            std::string strData;
//            variant.ToString( strData );
//            std::cout << eType << " : " << strData << std::endl;
        }

    }
}
void PdfeCMap::loadCodeSpaceRange( PdfContentsTokenizer& tokenizer, long nbRanges )
{
    EPdfContentsType eType;
    const char* pKeyword;
    PdfVariant lVariant, uVariant;

    // Read ranges from CMap data.
    long idx = 0;
    while( tokenizer.ReadNext( eType, pKeyword, lVariant ) && idx < nbRanges ) {
        // Should be a variant (lower bound)...
        if( eType == ePdfContentsType_Variant ) {
            // Read upper bound.
            tokenizer.ReadNext( eType, pKeyword, uVariant );

            // Lower and upper bounds.
            PdfeCMap::CodeValue lBound( lVariant );
            PdfeCMap::CodeValue uBound( uVariant );

            // Add range to the vector.
            m_codeSpaceRanges.push_back( PdfeCMap::CodeSpaceRange( lBound, uBound ) );
            ++idx;
        }
        else if( eType == ePdfContentsType_Keyword ) {
            break;
        }
    }
    // Sort range vector.
    std::sort( m_codeSpaceRanges.begin(), m_codeSpaceRanges.end(), PdfeCMap::CodeSpaceRange::compare );
}
void PdfeCMap::loadBFChars( PdfContentsTokenizer& tokenizer, long nbRanges )
{
    EPdfContentsType eType;
    const char* pKeyword;
    PdfVariant codeVariant, valVariant;

    // Read ranges from CMap data.
    long idx = 0;
    while( tokenizer.ReadNext( eType, pKeyword, codeVariant ) && idx < nbRanges ) {
        // Should be a variant
        if( eType == ePdfContentsType_Variant ) {
            // Read value variant.
            tokenizer.ReadNext( eType, pKeyword, valVariant );

            // Add bfrange to the vector.
            m_bfChars.push_back( PdfeCMap::BFChar( codeVariant, valVariant ) );
            ++idx;
        }
        else if( eType == ePdfContentsType_Keyword ) {
            break;
        }
    }
}
void PdfeCMap::loadBFRanges( PdfContentsTokenizer& tokenizer, long nbRanges )
{
    EPdfContentsType eType;
    const char* pKeyword;
    PdfVariant lVariant, uVariant, valVariant;

    // Read ranges from CMap data.
    long idx = 0;
    while( tokenizer.ReadNext( eType, pKeyword, lVariant ) && idx < nbRanges ) {
        // Should be a variant
        if( eType == ePdfContentsType_Variant ) {
            // Read upper bound and values variant.
            tokenizer.ReadNext( eType, pKeyword, uVariant );
            tokenizer.ReadNext( eType, pKeyword, valVariant );

            // Add bfrange to the vector.
            m_bfRanges.push_back( PdfeCMap::BFRange( lVariant, uVariant, valVariant ) );
            ++idx;
        }
        else if( eType == ePdfContentsType_Keyword ) {
            break;
        }
    }
}

PdfeCIDString PdfeCMap::toCIDString( const PoDoFo::PdfString& str ) const
{
    PdfeCIDString cidstr;

    // PDF String data.
    const char* pstr = str.GetString();
    long length = str.GetLength();

    // Identity CMap
    if( m_identity ) {
        pdfe_cid c;
        const pdfe_cid* pValue = reinterpret_cast<const pdfe_cid*>( pstr );

        cidstr.reserve( length / 2 );
        for( long i = 0 ; i < length-1 ; i += 2 ) {

            // Read character (Litte endian: need to invert bytes).
            c = *pValue;
#ifdef PODOFO_IS_LITTLE_ENDIAN
            c = ( (c & 0xff) << 8 ) | ( (c & 0xff00) >> 8 );
#endif
            cidstr.push_back( c );

            ++pValue;
        }
        return cidstr;
    }

    // Otherwise: TODO !
    return cidstr;
}

QString PdfeCMap::toUnicode( const PdfString& str ) const
{
    // No code space ranges: can not do much...
    if( !m_codeSpaceRanges.size() ) {
        return QString();
    }

    // Maximum size for a code.
    size_t maxCodeSize = m_codeSpaceRanges.back().codeSize();

    size_t strLen = str.GetLength();
    const char* pstr = str.GetString();
    size_t index = 0;

    // Unicode string.
    QString ustr;

    // Loop on str characters.
    while( index < strLen ) {
        // Find a code that could correspond to first chars in the string.
        PdfeCMap::CodeValue code;
        size_t codeLen;
        bool inside = false;

        for( codeLen = 1 ; codeLen <= std::min( maxCodeSize, strLen-index) ; ++codeLen ) {
            code.init( pstr, codeLen );

            // Find a code space range for this code.
            for( size_t i = 0 ; i < m_codeSpaceRanges.size() ; ++i ) {
                inside = m_codeSpaceRanges[i].inside( code );
                if( inside ) {
                    break;
                }
            }
            // Found a space range!
            if( inside ) {
                break;
            }
        }

        if( inside ) {
            // Unicode QString for the code.
            ustr += this->toUnicode( code );

            pstr += codeLen;
            index += codeLen;
        }
        else {
            // No space range found: simply increment pstr and index.
            ++pstr;
            ++index;
        }
    }
    return ustr;
}
QString PdfeCMap::toUnicode( const PdfeCMap::CodeValue& code ) const
{
    // Look inside bfchars.
    for( long i = long(m_bfChars.size())-1 ; i >= 0 ; ++i ) {
        if( m_bfChars[i].equal( code ) ) {
            return UTF16VecToQString( m_bfChars[i].toUTF16( code ) );
        }
    }
    // Look inside bfranges.
    for( long i = long(m_bfRanges.size())-1 ; i >= 0 ; ++i ) {
        if( m_bfRanges[i].inside( code ) ) {
            return UTF16VecToQString( m_bfRanges[i].toUTF16( code ) );
        }
    }

    // Default: empty QString.
    return QString();
}

//**********************************************************//
//                    PdfCMap::CodeValue                    //
//**********************************************************//
PdfeCMap::CodeValue::CodeValue() :
    std::vector<pdf_uint8>()
{
}
PdfeCMap::CodeValue::CodeValue( const PdfVariant& variant )
{
    // Variant must be a string.
    if( variant.IsString() || variant.IsHexString() ) {
        const char* pstr = variant.GetString().GetString();
        size_t length = variant.GetString().GetLength();

        this->init( pstr, length );
    }
    else {
        this->clear();
    }
}
PdfeCMap::CodeValue::CodeValue( const char* pstr, size_t length )
{
    this->init( pstr, length );
}

PdfeCMap::CodeValue::CodeValue( const PdfeCMap::CodeValue& rhs ) :
    std::vector<pdf_uint8>( rhs )
{
}
PdfeCMap::CodeValue& PdfeCMap::CodeValue::operator=( const PdfeCMap::CodeValue& rhs )
{
    static_cast< std::vector<pdf_uint8>* >(this)->operator =( rhs );
    return *this;
}

void PdfeCMap::CodeValue::init(const char *pstr, size_t length)
{
    this->resize( length );
    const pdf_uint8* pval = reinterpret_cast<const pdf_uint8*>( pstr );
    for( size_t i = 0 ; i < length ; ++i ) {
        this->at( i ) = *pval;
        ++pval;
    }
}

bool PdfeCMap::CodeValue::greater( const PdfeCMap::CodeValue& rhs ) const
{
    // First check size.
    if( this->size() != rhs.size() ) {
        return ( this->size() > rhs.size() );
    }
    // Vector values.
    for( size_t i = 0 ; i < this->size() ; ++i ) {
        if( (*this)[i] < rhs[i] ) {
            return false;
        }
    }
    return true;
}
bool PdfeCMap::CodeValue::smaller( const PdfeCMap::CodeValue& rhs ) const
{
    // First check size.
    if( this->size() != rhs.size() ) {
        return ( this->size() < rhs.size() );
    }
    // Vector values.
    for( size_t i = 0 ; i < this->size() ; ++i ) {
        if( (*this)[i] > rhs[i] ) {
            return false;
        }
    }
    return true;
}

bool PdfeCMap::CodeValue::compare( const PdfeCMap::CodeValue& lhs,
                                   const PdfeCMap::CodeValue& rhs )
{
    // First check size.
    if( lhs.size() != rhs.size() ) {
        return ( lhs.size() < rhs.size() );
    }
    // Vector values.
    for( size_t i = 0 ; i < lhs.size() ; ++i ) {
        // Different values/
        if( lhs[i] != rhs[i] ) {
            return ( lhs[i] < rhs[i] );
        }
    }
    return false;
}

//**********************************************************//
//                  PdfCMap::CodeSpaceRange                 //
//**********************************************************//
PdfeCMap::CodeSpaceRange::CodeSpaceRange() :
    m_lowerBound(), m_upperBound()
{
}
PdfeCMap::CodeSpaceRange::CodeSpaceRange( const PdfeCMap::CodeValue& lBound,
                                          const PdfeCMap::CodeValue& uBound )
{
    // Perform some check on inputs...
    m_lowerBound.resize( std::min( lBound.size(), uBound.size() ) );
    m_upperBound.resize( m_lowerBound.size() );
    for( size_t i = 0 ; i < m_lowerBound.size() ; ++i ) {
        m_lowerBound[i] = std::min( lBound[i], uBound[i] );
        m_upperBound[i] = std::max( lBound[i], uBound[i] );
    }
}

bool PdfeCMap::CodeSpaceRange::inside( const PdfeCMap::CodeValue& code ) const
{
    // Should have the same size.
    if( code.size() != m_lowerBound.size() ) {
        return false;
    }
    return ( code.smaller( m_upperBound ) && code.greater( m_lowerBound ) );
}
long PdfeCMap::CodeSpaceRange::index( const PdfeCMap::CodeValue& code ) const
{
    bool inside = this->inside( code );
    if( !inside ) {
        return -1;
    }

    // Compute the index.
    long index = 0;
    long base = 1;
    for( long i =  m_lowerBound.size()-1 ; i >= 0 ; --i ) {
        index += base * ( code[i] - m_lowerBound[i] );
        base = base * ( m_upperBound[i] - m_lowerBound[i] + 1 );
    }
    return index;
}
PdfeCMap::CodeValue PdfeCMap::CodeSpaceRange::nextCode( bool reset ) const
{
    // Reset the mutable cache value.
    if( reset || m_cacheCode.size() == 0 || m_cacheCode.size() != m_lowerBound.size() ) {
        m_cacheCode = m_lowerBound;
        return m_cacheCode;
    }

    // Increment the cache code.
    CodeValue::iterator it( m_cacheCode.end() );
    long i = m_cacheCode.size()-1;
    while( i >= 0  ) {
        // Can we increment the current byte ?
        if( m_cacheCode[i] < m_upperBound[i] ) {
            ++m_cacheCode[i];

            // Reset previous bytes to the lower bound value.
            std::copy( m_lowerBound.begin()+(i+1), m_lowerBound.end(),
                       m_cacheCode.begin()+(i+1) );
        }
        --i;
    }
    // Reset to lower bound.
    m_cacheCode = m_lowerBound;
    return m_cacheCode;
}

bool PdfeCMap::CodeSpaceRange::compare( const PdfeCMap::CodeSpaceRange& lhs,
                                        const PdfeCMap::CodeSpaceRange& rhs )
{
    return ( PdfeCMap::CodeValue::compare( lhs.m_lowerBound,
                                           rhs.m_lowerBound ) );
}

//**********************************************************//
//                     PdfCMap::BFRange                     //
//**********************************************************//
PdfeCMap::BFRange::BFRange()
{
}
PdfeCMap::BFRange::BFRange( const PdfVariant& lBound,
                            const PdfVariant& rBound,
                            const PdfVariant& values ) :
    m_codeSpaceRange( lBound, rBound )
{
    // Values: can be an array or a single value.
    if( values.IsString() || values.IsHexString() ) {
        const PdfString& strVal = values.GetString();

        // Assign corresponding utf16 vector.
        m_utf16Values.push_back( UTF16BEStrToUTF16Vec( strVal.GetString(),
                                                       strVal.GetLength() ) );
    }
    else if( values.IsArray() ) {
        const PdfArray& arrayVal = values.GetArray();

        for( size_t i = 0 ; i < arrayVal.size() ; ++i ) {
            const PdfString& strVal = arrayVal[i].GetString();

            // Push corresponding value.
            m_utf16Values.push_back( UTF16BEStrToUTF16Vec( strVal.GetString(),
                                                           strVal.GetLength() ) );
        }
    }
}
bool PdfeCMap::BFRange::inside( const PdfeCMap::CodeValue& code ) const
{
    return m_codeSpaceRange.inside( code );
}
std::vector<pdfe_utf16> PdfeCMap::BFRange::toUTF16( const PdfeCMap::CodeValue& code ) const
{
    // Find the index of the code inside the range.
    long index = m_codeSpaceRange.index( code );
    long utf16size = static_cast<long>( m_utf16Values.size() );

    std::vector<pdfe_utf16> utf16vec;

    if( index >= 0 && utf16size ) {
        // Copy corresponding value.
        if( index < utf16size ) {
            utf16vec = m_utf16Values[index];
        }
        else {
            // Use the first element and increment the last utf16 character.
            utf16vec = m_utf16Values[0];
            utf16vec.back() += index;
        }
    }
    return utf16vec;
}

//**********************************************************//
//                      PdfCMap::BFChar                     //
//**********************************************************//
PdfeCMap::BFChar::BFChar()
{
}
PdfeCMap::BFChar::BFChar( const PdfVariant& code, const PdfVariant& value ) :
    m_codeChar( code )
{
    // Values: should a PdfString
    if( value.IsString() || value.IsHexString() ) {
        const PdfString& strVal = value.GetString();

        // Assign corresponding utf16 vector.
        m_utf16Value = UTF16BEStrToUTF16Vec( strVal.GetString(), strVal.GetLength() );
    }
}
bool PdfeCMap::BFChar::equal( const PdfeCMap::CodeValue& code ) const
{
    return ( m_codeChar == code );
}

std::vector<pdfe_utf16> PdfeCMap::BFChar::toUTF16( const PdfeCMap::CodeValue& code ) const
{
    if( this->equal( code ) ) {
        // Utf16 vector.
        return m_utf16Value;
    }
    else {
        // Empty component
        return std::vector<pdfe_utf16>();
    }
}

}
