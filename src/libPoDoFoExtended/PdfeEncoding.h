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
