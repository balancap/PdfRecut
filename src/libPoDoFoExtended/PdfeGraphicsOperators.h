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

#ifndef PDFEGRAPHICSOPERATORS_H
#define PDFEGRAPHICSOPERATORS_H

#include <cstring>

namespace PoDoFoExtended {

namespace PdfeGCategory {
/// PDF graphics operators categories, as defined in PDF reference 1.7.
enum Enum {
    GeneralGState = 0,
    SpecialGState,
    PathConstruction,
    PathPainting,
    ClippingPath,
    TextObjects,
    TextState,
    TextPositioning,
    TextShowing,
    Type3Fonts,
    Color,
    ShadingPatterns,
    InlineImages,
    XObjects,
    MarkedContents,
    Compatibility,
    Unknown
};
/// Number of graphics categories.
inline size_t size() {
    return 17;
}
}

namespace PdfeGOperator {
/// PDF graphics operator, as defined in PDF reference 1.7.
enum Enum {
    w = 0,J,j,M,d,ri,i,gs,
    q,Q,cm,
    m,l,c,v,y,h,re,
    S,s,f,F,fstar,B,Bstar,b,bstar,n,
    W,Wstar,
    BT,ET,
    Tc,Tw,Tz,TL,Tf,Tr,Ts,
    Td,TD,Tm,Tstar,
    Tj,TJ,Quote,DoubleQuote,
    d0,d1,
    CS,cs,SC,SCN,sc,scn,G,g,RG,rg,K,k,
    sh,
    BI,ID,EI,
    Do,
    MP,DP,BMC,BDC,EMC,
    BX,EX,
    Unknown
};
/// Number of graphics operators.
inline size_t size() {
    static size_t size = 74;
    return size;
}
/// Category of a given operator.
inline PdfeGCategory::Enum category( PdfeGOperator::Enum type ) {
    static int categories[74] = {
        0,0,0,0,0,0,0,0,
        1,1,1,
        2,2,2,2,2,2,2,
        3,3,3,3,3,3,3,3,3,3,
        4,4,
        5,5,
        6,6,6,6,6,6,6,
        7,7,7,7,
        8,8,8,8,
        9,9,
        10,10,10,10,10,10,10,10,10,10,10,10,
        11,
        12,12,12,
        13,
        14,14,14,14,14,
        15,15,
        16
    };
    if( type >= 0 && type <= 73 ) {
        return PdfeGCategory::Enum( categories[ type ] );
    }
    return PdfeGCategory::Unknown;
}
/// String representation of a given operator.
inline const char* str( PdfeGOperator::Enum type ) {
    static const char* names[74] = {
        "w","J","j","M","d","ri","i","gs",
        "q","Q","cm",
        "m","l","c","v","y","h","re",
        "S","s","f","F","f*","B","B*","b","b*","n",
        "W","W*",
        "BT","ET",
        "Tc","Tw","Tz","TL","Tf","Tr","Ts",
        "Td","TD","Tm","T*",
        "Tj","TJ","'","\"",
        "d0","d1",
        "CS","cs","SC","SCN","sc","scn","G","g","RG","rg","K","k",
        "sh",
        "BI","ID","EI",
        "Do",
        "MP","DP","BMC","BDC","EMC",
        "BX","EX",
        "Unknown"
    };
    if( type >= 0 && type <= 73 ) {
        return names[ type ];
    }
    return names[ PdfeGOperator::Unknown ];
}
// TODO: number of arguments for each operator.
}
namespace PdfeFillingRule {
/// Enumeration of PDF Filling rules.
enum Enum {
    Winding = 0,    /// Nonzero Winding Number rule;
    EvenOdd,        /// Even-Odd rule.
    Unknown
};
}
namespace PdfeXObjectType {
/// Enumeration of XObject types.
enum Enum {
    Image = 0,      /// Image content.
    Form,           /// Form XObject.
    PS,             /// PostScript object.
    Unknown
};
}

/** Class gathering information on a graphics operator.
 */
class PdfeGraphicOperator
{
public:
    /** Default constructor.
     */
    PdfeGraphicOperator() {
        this->init();
    }
    /** Constructor from an operator type.
     */
    PdfeGraphicOperator( PdfeGOperator::Enum type ) {
        this->set( type );
    }
    /** Constructor from a name string.
     */
    PdfeGraphicOperator( const char* str ) {
        this->set( str );
    }
    /** Initialize to the operator Unknown.
     */
    void init() {
        this->set( PdfeGOperator::Unknown );
    }

public:
    // Getters...
    /// Operator type.
    PdfeGOperator::Enum type() const {
        return m_type;
    }
    /// Category of the operator.
    PdfeGCategory::Enum category() const {
        return PdfeGOperator::category( m_type );
    }
    /// String representation of the operator.
    const char* str() const {
        return PdfeGOperator::str( m_type );
    }

    // Setters...
    /// Set the operator from its type.
    void set( PdfeGOperator::Enum type ) {
        m_type = type;
    }
    /// Set the operator from its string representation.
    void set( const char* str ) {
        for( unsigned int i = 0 ; i < PdfeGOperator::size() ; ++i ) {
            if( !strcmp( str,  PdfeGOperator::str( PdfeGOperator::Enum( i ) ) ) ) {
                m_type = PdfeGOperator::Enum( i );
                return;
            }
        }
        this->set( PdfeGOperator::Unknown );
    }

public:
    /** Is the operator a painting operator which closes the path.
     * \return True if it closes the path.
     */
    bool isClosePainting() const {

        return  ( category() == PdfeGCategory::PathPainting ) &&
                ( m_type == PdfeGOperator::s ||
                  m_type == PdfeGOperator::f ||
                  m_type == PdfeGOperator::F ||
                  m_type == PdfeGOperator::fstar ||
                  m_type == PdfeGOperator::B ||
                  m_type == PdfeGOperator::Bstar ||
                  m_type == PdfeGOperator::b ||
                  m_type == PdfeGOperator::bstar );
    }
    /** Is the painting operator using the even-odd rule (otherwise, non zero winding rule).
     * \return True if even-odd rule.
     */
    bool isEvenOddRule() const {
        return  m_type == PdfeGOperator::fstar ||
                m_type == PdfeGOperator::Bstar ||
                m_type == PdfeGOperator::bstar ||
                m_type == PdfeGOperator::Wstar;
    }
    /** Get the filling rule of the painting operator.
     * \return Filling rule. Unknown if does not apply to the current operator.
     */
    PdfeFillingRule::Enum fillingRule() const {
        if( m_type == PdfeGOperator::fstar ||
                m_type == PdfeGOperator::Bstar ||
                m_type == PdfeGOperator::bstar ||
                m_type == PdfeGOperator::Wstar ) {
            return PdfeFillingRule::Winding;
        }
        else if( m_type == PdfeGOperator::f ||
                 m_type == PdfeGOperator::F ||
                 m_type == PdfeGOperator::B ||
                 m_type == PdfeGOperator::b ||
                 m_type == PdfeGOperator::W ) {
            return PdfeFillingRule::EvenOdd;
        }
        return PdfeFillingRule::Unknown;
    }
private:
    /// Operator type.
    PdfeGOperator::Enum  m_type;
};

}

#endif // PDFEGRAPHICSOPERATORS_H
