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

#ifndef PDFCMAP_H
#define PDFCMAP_H

#include "PdfeTypes.h"

#include "podofo/base/PdfName.h"
#include "podofo/base/PdfContentsTokenizer.h"

#include <QString>

namespace PoDoFo {
class PdfArray;
class PdfObject;
class PdfVariant;
class PdfString;
}

namespace PoDoFoExtended {

//**********************************************************//
//                      PdfeCIDSystemInfo                   //
//**********************************************************//
/** Class that contains the different informations of CIDSystemInfo.
 */
struct PdfeCIDSystemInfo
{
public:
    /** Default constructor.
     */
    PdfeCIDSystemInfo();
    /** Initialize to default values.
     */
    void init();
    /** Initialize structure parameters from a PdfObject.
     * \param cidSysInfoObj PoDoFo object containing CID system info.
     */
    void init( PoDoFo::PdfObject* cidSysInfoObj );

public:
    // Getters.
    const std::string& registery() const    {   return m_registery; }
    const std::string& ordering() const     {   return m_ordering; }
    int supplement() const                  {   return m_supplement; }

    // Setters.
    void setRegistery( const std::string& rhs )     {   m_registery = rhs;  }
    void setOrdering( const std::string& rhs )      {   m_ordering = rhs;  }
    void setSupplement( int rhs )                   {   m_supplement = rhs;  }

private:
    /// Issuer of the character collection (i.e. Adobe).
    std::string  m_registery;
    /// Name of the character collection within the specified registry (i.e. Japan1).
    std::string  m_ordering;
    /// Supplement number of the character collection.
    int  m_supplement;
};

//**********************************************************//
//                          PdfCMap                         //
//**********************************************************//
/** Class used to represent and handle a CMap as described in the PDF reference.
 */
class PdfeCMap
{
public:
    /** Empty constructor.
     */
    PdfeCMap();
    /** Construction from using predefined CMap.
     * \param cmapName Name of the predefined CMap.
     */
    PdfeCMap( const PoDoFo::PdfName& cmapName );
    /** Construction from an embedded CMap (represented in a PdfStream).
     * \param pCMapObj Pointer to the PdfObject which contains the data stream.
     */
    PdfeCMap( PoDoFo::PdfObject* pCMapObj );

    /** Init members to default values.
     */
    void init();
    /** Initialize from a predefined CMap.
     * \param cmapName Name of the predefined CMap.
     */
    void init( const PoDoFo::PdfName& cmapName );
    /** Initialize from an embedded CMap (represented in a PdfStream).
     * \param pCMapObj Pointer to the PdfObject which contains the data stream.
     */
    void init( PoDoFo::PdfObject* pCMapObj );

    /** Destructor.
     */
    ~PdfeCMap();

private:
    /** Load content from buffer which contains the CMap data.
     * \param pBuffer Buffer containing the data.
     * \param length Length of the buffer.
     */
    void loadContent( const char* pBuffer, long length );

    /** Load code space range from CMap data.
     * \param tokenizer Reference to the tokenizer that read data.
     * \param nbRanges Number of ranges to read.
     */
    void loadCodeSpaceRange( PoDoFo::PdfContentsTokenizer& tokenizer,
                             long nbRanges );

    /** Load bfchars from CMap data.
     * \param tokenizer Reference to the tokenizer that read data.
     * \param nbRanges Number of bfchar to read.
     */
    void loadBFChars( PoDoFo::PdfContentsTokenizer& tokenizer,
                      long nbRanges );

    /** Load bfranges from CMap data.
     * \param tokenizer Reference to the tokenizer that read data.
     * \param nbRanges Number of ranges to read.
     */
    void loadBFRanges( PoDoFo::PdfContentsTokenizer& tokenizer,
                       long nbRanges );
public:
    class CharCode;
    class CodeSpaceRange;
    class BFRange;
    class BFChar;

public:
    /** Convert a simple string to a CID string using the CMap.
     * \param str PoDoFo::PdfString to convert (can contain 0 characters !).
     * \return CID String corresponding.
     */
    PdfeCIDString toCIDString( const PoDoFo::PdfString& str ) const;
    /** Retrieve the characters codes which are mapped to a given CID by the CMap.
     * \param c CID to match.
     * \return Vector of corresponding char codes.
     */
    std::vector<CharCode> toCharCode( pdfe_cid c ) const;
    /** Convert a code representing a character to a unicode QString using the CMap.
     * \param code Code for a character.
     * \return  Unicode QString corresponding.
     */
    QString toUnicode( const CharCode& code ) const;
    /** Convert a simple string to a unicode QString using the CMap.
     * \param str PoDoFo::PdfString to convert (can contain 0 characters !).
     * \return  Unicode QString corresponding.
     */
    QString toUnicode( const PoDoFo::PdfString& str ) const;
    /** Does the CMap has an empty code space range?
     * \return Answer!
     */
    bool emptyCodeSpaceRange() const {
        return ( m_codeSpaceRanges.size() == 0 );
    }

private:
    /// CMap name.
    PoDoFo::PdfName  m_name;
    /// CMap CIDSystemInfo.
    PdfeCIDSystemInfo  m_CIDSystemInfo;

    /// Writting mode: 0 (horizontal), 1 (vertical).
    bool  m_wmode;
    /// Is the CMap simply the identity.
    bool  m_identity;

    /// Code space ranges.
    std::vector<CodeSpaceRange>  m_codeSpaceRanges;
    /// BFChars.
    std::vector<BFChar>  m_bfChars;
    /// BFRanges.
    std::vector<BFRange>  m_bfRanges;

    /// CMap used in addition to defined this one (owned).
    PdfeCMap*  m_baseCMap;

public:
    //**********************************************************//
    //                   Public nested classes                  //
    //**********************************************************//
    /** Nested class: represent a character code as used in a CMap.
     */
    class CharCode : public std::vector<PoDoFo::pdf_uint8>
    {
    public:
        /** Empty constructor.
         */
        CharCode();
        /** Create a code from a PoDoFo::PdfVariant
         * \param variant Must be a PdfString representing the code.
         */
        explicit CharCode( const PoDoFo::PdfVariant& variant );
        /** Create a code from a one byte integer.
         * \param val Input value (pdf_uint8).
         */
        explicit CharCode( PoDoFo::pdf_uint8 val );
        /** Create a code from a two bytes integer.
         * \param val Input value (pdf_uint16).
         */
        explicit CharCode( PoDoFo::pdf_uint16 val );
        /** Create a code from an array of char.
         * \param pstr Pointer to a char string.
         * \param length Number of characters to consider.
         */
        explicit CharCode( const char* pstr, size_t length );

        /** Copy constructor.
         * \param rhs Object to copy.
         */
        CharCode( const CharCode& rhs );
        /** Assignment operator.
         * \param rhs Object to copy.
         */
        CharCode& operator=( const CharCode& rhs );

        /** Initialize a code from a one byte integer.
         * \param val Input value (pdf_uint8).
         */
        void init( PoDoFo::pdf_uint8 val );
        /** Initialize a code from a two bytes integer.
         * \param val Input value (pdf_uint16).
         */
        void init( PoDoFo::pdf_uint16 val );
        /** Initialize a code from an array of char.
         * \param pstr Pointer to a char string.
         * \param length Number of characters to consider.
         */
        void init( const char* pstr, size_t length );

        /** Is (this) strictly greater than another code value.
         * \param rhs Code to compare to.
         * \return this >= rhs ?
         */
        bool greater( const CharCode& rhs ) const;

        /** Is (this) strictly smaller than another code value.
         * \param rhs Code to compare to.
         * \return this <= rhs ?
         */
        bool smaller( const CharCode& rhs ) const;

        /** Compare two code value using the weak order.
         * \param lhs First code.
         * \param rhs Second code.
         * \return lhs < rhs.
         */
        static bool compare( const CharCode& lhs, const CharCode& rhs );
    };

    /** Nested class: represent a code space range used in a CMap.
     */
    class CodeSpaceRange
    {
    public:
        /** Empty constructor.
         */
        CodeSpaceRange();
        /** Construct a range using lower and upper bounds values.
         * \param lBound Lower bound value.
         * \param uBound Upper bound value.
         */
        CodeSpaceRange( const CharCode& lBound, const CharCode& uBound );

        /** Is a code value inside the range.
         * \param code Code to consider.
         * \return Code inside the range?
         */
        bool inside( const CharCode& code ) const;

        /** Index of a code in the code space range.
         * \param code Code to consider.
         * \return Index of the code inside the range. -1 if not inside.
         */
        long index( const CharCode& code ) const;

        /** Next code in the range. This function goes through
         * the range to return every successive values inside.
         * \param reset Reset to the lower bound.
         * \return Next value in the range, beginning at the lower bound.
         */
        CharCode nextCode( bool reset ) const;

        /** Get the lower bound of the range.
         * \return Lower bound.
         */
        CharCode lowerBound() const {
            return m_lowerBound;
        }
        /** Get the upper bound of the range.
         * \return Upper bound.
         */
        CharCode upperBound() const {
            return m_upperBound;
        }
        /** Get the size of code used for this range.
         * \return Code size.
         */
        size_t codeSize() const {
            return m_lowerBound.size();
        }

        /** Compare two code space ranges using the weak order.
         * \param lhs First code range.
         * \param rhs Second code range.
         * \return lhs < rhs.
         */
        static bool compare( const CodeSpaceRange& lhs, const CodeSpaceRange& rhs );

    private:
        /// Lower bound.
        CharCode  m_lowerBound;
        /// Upper bound.
        CharCode  m_upperBound;

        /// Cache code.
        mutable CharCode  m_cacheCode;
    };

    /** Nested class: represent a bfrange used in a CMap.
     */
    class BFRange
    {
    public:
        /** Empty constructor.
         */
        BFRange();
        /** Construct a range using lower, upper bounds and corresponding values.
         * \param lBound Lower bound variant.
         * \param uBound Upper bound variant.
         * \param values Variant containing values.
         */
        BFRange( const PoDoFo::PdfVariant& lBound,
                 const PoDoFo::PdfVariant& rBound,
                 const PoDoFo::PdfVariant& values );

        /** Get the size of code used for the bfrange.
         * \return Code size.
         */
        size_t codeSize() const {
            return m_codeSpaceRange.codeSize();
        }

        /** Is a code value inside the bfrange.
         * \param code Code to consider.
         * \return Code inside the bfrange?
         */
        bool inside( const CharCode& code ) const;

        /** Convert a code to a unicode QString.
         * The code must belong to the bfrange.
         * \param code Code to convert.
         * \return Unicode QString.
         */
        QString toUnicode( const CharCode& code ) const;

    private:
        /// Space range of characters codes.
        CodeSpaceRange  m_codeSpaceRange;
        /// UTF16 values.
        std::vector< std::vector<pdfe_utf16> >  m_utf16Values;
    };

    /** Nested class: represent a bfchar used in a CMap.
     */
    class BFChar
    {
    public:
        /** Empty constructor.
         */
        BFChar();
        /** Construct a range using char code and corresponding value.
         * \param code Code of a character (PdfVariant).
         * \param value Variant containing value.
         */
        BFChar( const PoDoFo::PdfVariant& code,
                const PoDoFo::PdfVariant& value );

        /** Get the size of code used for the bfchar.
         * \return Code size.
         */
        size_t codeSize() const {
            return m_codeChar.size();
        }

        /** Is a code value corresponds to the bfchar.
         * \param code Code to consider.
         * \return Code ==  bfchar.
         */
        bool equal( const CharCode& code ) const;

        /** Convert a code to a unicode QString.
         * The code must be equal to the bfchar.
         * \param code Code to convert.
         * \return Unicode QString.
         */
        QString toUnicode( const CharCode& code ) const;

    private:
        /// Code of the character.
        CharCode  m_codeChar;
        /// UTF16 value.
        std::vector<pdfe_utf16>  m_utf16Value;
    };
};

}

#endif // PDFCMAP_H
