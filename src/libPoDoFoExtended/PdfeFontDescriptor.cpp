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

#include "PdfeFontDescriptor.h"
#include "podofo/podofo.h"

#include <QDebug>

using namespace PoDoFo;

namespace PoDoFoExtended {

//**********************************************************//
//                     PdfeFontEmbedded                     //
//**********************************************************//
PoDoFo::PdfObject* PdfeFontEmbedded::fontFile( size_t idx ) const
{
    // Valid index.
    if( idx >= 1 && idx <= 3 ) {
        if( idx == 1 ) {
            return m_fontFile;
        }
        else if( idx == 2 ) {
            return m_fontFile2;
        }
        else {
            return m_fontFile3;
        }
    }

    // First pointer in the list not null.
    if( m_fontFile ) {
        return m_fontFile;
    }
    else if( m_fontFile2 ) {
        return m_fontFile3;
    }
    else {
        return m_fontFile3;
    }
}
void PdfeFontEmbedded::setFontFiles( PoDoFo::PdfObject* fontFile,
                                     PoDoFo::PdfObject* fontFile2,
                                     PoDoFo::PdfObject* fontFile3 )
{
    m_fontFile = fontFile;
    m_fontFile2 = fontFile2;
    m_fontFile3 = fontFile3;
}
void PdfeFontEmbedded::fontProgram( char** pBuffer, long* pLength ) const
{
    // Get font file.
    PoDoFo::PdfObject* fontFile = this->fontFile();

    // Uncompress and copy into a buffer.
    if( fontFile ) {
        PdfStream* stream = fontFile->GetStream();
        stream->GetFilteredCopy( pBuffer, pLength );
    }
    else {
        *pBuffer = NULL;
        *pLength = -1;
    }
}

//**********************************************************//
//                    PdfeFontDescriptor                    //
//**********************************************************//
PdfeFontDescriptor::PdfeFontDescriptor()
{
    this->init();
}
PdfeFontDescriptor::PdfeFontDescriptor( PdfObject* pFontDesc )
{
    // Read values in the PdfObject.
    this->init( pFontDesc );
}
PdfeFontDescriptor::PdfeFontDescriptor( const PdfeFontDescriptor& fontDesc )
{
    this->init();

    m_fontName = fontDesc.m_fontName;
    m_fontFamily = fontDesc.m_fontFamily;
    m_fontStretch = fontDesc.m_fontStretch;
    m_fontBBox = fontDesc.m_fontBBox;

    m_fontWeight = fontDesc.m_fontWeight;
    m_flags = fontDesc.m_flags;
    m_italicAngle = fontDesc.m_italicAngle;

    m_ascent = fontDesc.m_ascent;
    m_descent = fontDesc.m_descent;
    m_leading = fontDesc.m_leading;
    m_capHeight = fontDesc.m_capHeight;
    m_xHeight = fontDesc.m_xHeight;
    m_stemV = fontDesc.m_stemV;
    m_stemH = fontDesc.m_stemH;
    m_avgWidth = fontDesc.m_avgWidth;
    m_maxWidth = fontDesc.m_maxWidth;
    m_missingWidth = fontDesc.m_missingWidth;

    m_fontEmbedded = fontDesc.m_fontEmbedded;

    // TODO: CharSet and CID Keys.
    m_charSet = PdfString();
}

void PdfeFontDescriptor::init()
{
    // Default values.
    m_fontName = PdfName();
    m_fontFamily = PdfString();
    m_fontStretch = PdfName();

    m_fontBBox.resize( 4, 0.0 );
    m_fontBBox[2] = 1.0;
    m_fontBBox[3] = 1.0;

    m_fontWeight = 400;
    m_flags = 0;
    m_italicAngle = 0;

    m_ascent = 0.0;
    m_descent = 0.0;
    m_leading = 0.0;
    m_capHeight = 0.0;
    m_xHeight = 0.0;
    m_stemV = 0.0;
    m_stemH = 0.0;
    m_avgWidth = 0.0;
    m_maxWidth = 0.0;
    m_missingWidth = 0.0;

    m_fontEmbedded.init();

    m_charSet = PdfString();
}
void PdfeFontDescriptor::init( PdfObject* pFontDesc )
{
    // Check if the PdfObject is a font descriptor dictionary.
    if( pFontDesc && pFontDesc->IsDictionary() && pFontDesc->GetDictionary().HasKey( PdfName::KeyType ) ) {
        const PdfName& rType = pFontDesc->GetDictionary().GetKey( PdfName::KeyType )->GetName();
        if( rType != PdfName( "FontDescriptor" ) ) {
            PODOFO_RAISE_ERROR( ePdfError_InvalidDataType );
        }
    }
    else {
        qWarning() << "No key \'type\' in the PDF font descriptor:" << pFontDesc << ".";
    }

    // Default values.
    this->init();

    // Read keys in the dictionary.
    PdfObject* pObj;
    m_fontName = pFontDesc->GetIndirectKey( "FontName" )->GetName();

    pObj = pFontDesc->GetIndirectKey( "FontFamily" );
    if( pObj ) {
        m_fontFamily = pObj->GetString();
    }
    pObj = pFontDesc->GetIndirectKey( "FontStretch" );
    if( pObj ) {
        m_fontStretch = pObj->GetName();
    }
    pObj = pFontDesc->GetIndirectKey( "FontBBox" );
    if( pObj ) {
        m_fontBBox = pObj->GetArray();
    }

    m_fontWeight = static_cast<pdf_uint32>( pFontDesc->GetDictionary().GetKeyAsLong( "FontWeight", 400L ) );
    m_flags = static_cast<pdf_uint32>( pFontDesc->GetDictionary().GetKeyAsLong( "Flags", 0 ) );
    m_italicAngle = static_cast<pdf_int32>( pFontDesc->GetDictionary().GetKeyAsLong( "ItalicAngle", 0 ) );

    m_ascent = pFontDesc->GetDictionary().GetKeyAsReal( "Ascent", 0.0 );
    m_descent = pFontDesc->GetDictionary().GetKeyAsReal( "Descent", 0.0 );
    m_leading = pFontDesc->GetDictionary().GetKeyAsReal( "Leading", 0.0 );
    m_capHeight = pFontDesc->GetDictionary().GetKeyAsReal( "CapHeight", 0.0 );
    m_xHeight = pFontDesc->GetDictionary().GetKeyAsReal( "XHeight", 0.0 );
    m_stemV = pFontDesc->GetDictionary().GetKeyAsReal( "StemV", 0.0 );
    m_stemH = pFontDesc->GetDictionary().GetKeyAsReal( "StemH", 0.0 );
    m_avgWidth = pFontDesc->GetDictionary().GetKeyAsReal( "AvgWidth", 0.0 );
    m_maxWidth = pFontDesc->GetDictionary().GetKeyAsReal( "MaxWidth", 0.0 );
    m_missingWidth = pFontDesc->GetDictionary().GetKeyAsReal( "MissingWidth", 0.0 );

    // Read FontFiles
    m_fontEmbedded.setFontFiles( pFontDesc->GetIndirectKey( "FontFile" ),
                                 pFontDesc->GetIndirectKey( "FontFile2" ),
                                 pFontDesc->GetIndirectKey( "FontFile3" ) );

    // TODO: FontFiles, CharSet and CID Keys.
}
void PdfeFontDescriptor::resetKey( PdfeFontDescriptor::Key key )
{
    // Reset to default value
    switch( key ) {
    case FontName:
        m_fontName = PdfName();         break;
    case FontFamily:
        m_fontFamily = PdfString();     break;
    case FontStretch:
        m_fontStretch = PdfName();      break;
    case FontBBox:
        m_fontBBox.resize( 4, 0.0 );
        m_fontBBox[2] = 1.0;
        m_fontBBox[3] = 1.0;
        break;
    case FontWeight:
        m_fontWeight = 400; break;
    case Flags:
        m_flags = 0;        break;
    case ItalicAngle:
        m_italicAngle = 0;  break;
    case Ascent:
        m_ascent = 0;       break;
    case Descent:
        m_descent = 0;      break;
    case Leading:
        m_leading = 0;      break;
    case CapHeight:
        m_capHeight = 0;    break;
    case XHeight:
        m_xHeight = 0;      break;
    case StemV:
        m_stemV = 0;        break;
    case StemH:
        m_stemH = 0;        break;
    case AvgWidth:
        m_avgWidth = 0;     break;
    case MaxWidth:
        m_maxWidth = 0;     break;
    case MissingWidth:
        m_missingWidth = 0; break;
    }
    // TODO: FontFiles, CharSet and CID Keys.
}


}
