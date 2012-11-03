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

#include "PdfeFontDescriptor.h"
#include "podofo/podofo.h"

#include <QsLog/QsLog.h>

#include <QRegExp>
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
        return m_fontFile2;
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
void PdfeFontEmbedded::copyFontProgram( char** pBuffer, long* pLength ) const
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
    // Descriptor default values.
    this->init();
}
PdfeFontDescriptor::PdfeFontDescriptor( PdfObject* pFontDesc )
{
    // Read values in the PdfObject.
    this->init( pFontDesc );
}
PdfeFontDescriptor::PdfeFontDescriptor( const PdfeFontDescriptor& rhs )
{
    this->init();

    m_fontName = rhs.m_fontName;
    m_fontNameShort = rhs.m_fontNameShort;
    m_subsetFont = rhs.m_subsetFont;

    m_fontFamily = rhs.m_fontFamily;
    m_fontStretch = rhs.m_fontStretch;
    m_fontBBox = rhs.m_fontBBox;

    m_fontWeight = rhs.m_fontWeight;
    m_flags = rhs.m_flags;
    m_italicAngle = rhs.m_italicAngle;

    m_ascent = rhs.m_ascent;
    m_descent = rhs.m_descent;
    m_leading = rhs.m_leading;
    m_capHeight = rhs.m_capHeight;
    m_xHeight = rhs.m_xHeight;
    m_stemV = rhs.m_stemV;
    m_stemH = rhs.m_stemH;
    m_avgWidth = rhs.m_avgWidth;
    m_maxWidth = rhs.m_maxWidth;
    m_missingWidth = rhs.m_missingWidth;

    m_fontEmbedded = rhs.m_fontEmbedded;

    // TODO: CharSet and CID Keys.
    m_charSet = PdfString();
}

void PdfeFontDescriptor::init()
{
    // Default values.
    this->setFontName( PdfName() );

    m_fontFamily = PdfString();
    m_fontStretch = PdfName();
    m_fontBBox = PdfRect( 0.0, 0.0, 0.0, 0.0 );

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
        QLOG_WARN() << "No key \'type\' in the PDF font descriptor:" << pFontDesc;
    }

    // Default values.
    this->init();

    // Read keys in the dictionary.
    this->setFontName( pFontDesc->GetIndirectKey( "FontName" )->GetName() );

    PdfObject* pObj = pFontDesc->GetIndirectKey( "FontFamily" );
    if( pObj ) {
        m_fontFamily = pObj->GetString();
    }
    pObj = pFontDesc->GetIndirectKey( "FontStretch" );
    if( pObj ) {
        m_fontStretch = pObj->GetName();
    }
    pObj = pFontDesc->GetIndirectKey( "FontBBox" );
    if( pObj ) {
        // Initialize PdfRect from PdfArray.
        m_fontBBox = PdfRect( pObj->GetArray() );
    }

    const PdfDictionary& pFontDescDict = pFontDesc->GetDictionary();

    m_fontWeight = static_cast<pdf_uint32>( pFontDescDict.GetKeyAsLong( "FontWeight", 400L ) );
    m_flags = static_cast<pdf_uint32>( pFontDescDict.GetKeyAsLong( "Flags", 0 ) );
    m_italicAngle = static_cast<pdf_int32>( pFontDescDict.GetKeyAsLong( "ItalicAngle", 0 ) );

    m_ascent = pFontDescDict.GetKeyAsReal( "Ascent", 0.0 );
    m_descent = pFontDescDict.GetKeyAsReal( "Descent", 0.0 );
    m_leading = pFontDescDict.GetKeyAsReal( "Leading", 0.0 );
    m_capHeight = pFontDescDict.GetKeyAsReal( "CapHeight", 0.0 );
    m_xHeight = pFontDescDict.GetKeyAsReal( "XHeight", 0.0 );
    m_stemV = pFontDescDict.GetKeyAsReal( "StemV", 0.0 );
    m_stemH = pFontDescDict.GetKeyAsReal( "StemH", 0.0 );
    m_avgWidth = pFontDescDict.GetKeyAsReal( "AvgWidth", 0.0 );
    m_maxWidth = pFontDescDict.GetKeyAsReal( "MaxWidth", 0.0 );
    m_missingWidth = pFontDescDict.GetKeyAsReal( "MissingWidth", 0.0 );

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
        this->setFontName( PdfName() );
        break;
    case FontFamily:
        m_fontFamily = PdfString();
        break;
    case FontStretch:
        m_fontStretch = PdfName();      break;
    case FontBBox:
        m_fontBBox = PdfRect( 0.0, 0.0, 0.0, 0.0 );
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

PdfName PdfeFontDescriptor::fontName( bool subsetPrefix ) const
{
    if( subsetPrefix ) {
        return m_fontName;
    }
    else {
        return m_fontNameShort;
    }
}
void PdfeFontDescriptor::setFontName( const PdfName& fontName )
{
    std::string sname = fontName.GetName();

    // Name correspond to a subset font?
    QRegExp rx( "^[A-Z]{6}\\+.*$" );
    m_subsetFont = rx.exactMatch( sname.c_str() );

    // Remove prefix for short name.
    m_fontName = fontName;
    if( m_subsetFont ) {
        m_fontNameShort = sname.substr( 7 );
    }
    else {
        m_fontNameShort = fontName;
    }
}

}
