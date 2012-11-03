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

#ifndef PDFEENCODING_H
#define PDFEENCODING_H

#include <string>

namespace PoDoFoExtended {

namespace PdfeEncodingType {
/** Enumeration of the different types of encodings described in the PDF Reference.
 */
enum Enum {
    Standard = 0,
    WinAnsi,
    MacRoman,
    MacExpert,
    Unknown
};
}

/** Class used to handle PDF encodings, in addition to PoDoFo::PdfEncoding.
 */
class PdfeEncoding
{
public:
    PdfeEncoding();

public:
    /** Convert from character code to character name.
     * \param code Code of the character (should be between 0 and 256).
     * \param enctype Encoding to consider.
     * \return Name of the character (empty if no name assigned to this code).
     */
    static std::string FromCodeToName( int code, PdfeEncodingType::Enum enctype );

    /** Convert from character code to character name.
     * \param name Name of the character.
     * \param enctype Encoding to consider.
     * \return Code of the character (-1 if no corresponding character for the name).
     */
    static int FromNameToCode( const std::string& name, PdfeEncodingType::Enum enctype );

private:
    /// Doc encoding.
    static const unsigned short DocEncoding[256];
    /// Standard encoding.
    static const char* const StandardEncoding[256];
    /// Mac Roman encoding.
    static const char* const  MacRomanEncoding[256];
    /// Mac Expert encoding.
    static const char* const  MacExpertEncoding[256];
    /// Win Ansi encoding.
    static const char* const  WinAnsiEncoding[256];
};

}

#endif // PDFEENCODING_H
