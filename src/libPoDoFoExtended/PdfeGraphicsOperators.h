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

#ifndef PDFEGRAPHICSENUMS_H
#define PDFEGRAPHICSENUMS_H

#include <cstring>

namespace PoDoFoExtended {

namespace PdfeGCategory {
/// PDF graphic operators categories, as defined in PDF reference 1.7.
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
}
namespace PdfeGOperator {
/// PDF graphic operator, as defined in PDF reference 1.7.
enum Enum {
    w = 0,
    J,
    j,
    M,
    d,
    ri,
    i,
    gs,

    q,
    Q,
    cm,

    m,
    l,
    c,
    v,
    y,
    h,
    re,

    S,
    s,
    f,
    F,
    fstar,
    B,
    Bstar,
    b,
    bstar,
    n,

    W,
    Wstar,

    BT,
    ET,

    Tc,
    Tw,
    Tz,
    TL,
    Tf,
    Tr,
    Ts,

    Td,
    TD,
    Tm,
    Tstar,

    Tj,
    TJ,
    Quote,
    DoubleQuote,

    d0,
    d1,

    CS,
    cs,
    SC,
    SCN,
    sc,
    scn,
    G,
    g,
    RG,
    rg,
    K,
    k,

    sh,

    BI,
    ID,
    EI,

    Do,

    MP,
    DP,
    BMC,
    BDC,
    EMC,

    BX,
    EX,

    Unknown
};
}
namespace PdfeFillingRule {
/// Enumeration of PDF Filling rules.
enum Enum {
    Winding = 0,    /// Nonzero Winding Number rule;
    EvenOdd,        /// Even-Odd rule.
    Unknown
};
}

/** Structure gathering information on a graphic operator.
 */
struct PdfeGraphicOperator
{
    /// Pdf name of the operator
    char  name[6];
    /// Enum code of the operator
    PdfeGOperator::Enum  code;
    /// Enum code of the category it belongs to.
    PdfeGCategory::Enum  cat;

    /** Default constructor.
     */
    PdfeGraphicOperator() {
        this->init();
    }
    /** Initialize to the operator Unknown.
     */
    void init() {
        name[0] = 0;
        code = PdfeGOperator::Unknown;
        cat = PdfeGCategory::Unknown;
    }

    /** Constructor from a name string.
     */
    PdfeGraphicOperator( const char* str ) { set(str); }

    /** Set the operator from a string.
     * \param str String containing the operator name.
     */
    void set( const char* str )
    {
        if( !strcmp( str, "Tj") ) {
            strcpy( name, str );
            code = PdfeGOperator::Tj;
            cat = PdfeGCategory::TextShowing;
        }
        else if( !strcmp( str, "TJ") ) {
            strcpy( name, str );
            code = PdfeGOperator::TJ;
            cat = PdfeGCategory::TextShowing;
        }
        else if( !strcmp( str, "'") ) {
            strcpy( name, str );
            code = PdfeGOperator::Quote;
            cat = PdfeGCategory::TextShowing;
        }
        else if( !strcmp( str, "\"") ) {
            strcpy( name, str );
            code = PdfeGOperator::DoubleQuote;
            cat = PdfeGCategory::TextShowing;
        }
        else if( !strcmp( str, "Td") ) {
            strcpy( name, str );
            code = PdfeGOperator::Td;
            cat = PdfeGCategory::TextPositioning;
        }
        else if( !strcmp( str, "TD") ) {
            strcpy( name, str );
            code = PdfeGOperator::TD;
            cat = PdfeGCategory::TextPositioning;
        }
        else if( !strcmp( str, "Tm") ) {
            strcpy( name, str );
            code = PdfeGOperator::Tm;
            cat = PdfeGCategory::TextPositioning;
        }
        else if( !strcmp( str, "T*") ) {
            strcpy( name, str );
            code = PdfeGOperator::Tstar;
            cat = PdfeGCategory::TextPositioning;
        }
        else if( !strcmp( str, "BT") ) {
            strcpy( name, str );
            code = PdfeGOperator::BT;
            cat = PdfeGCategory::TextObjects;
        }
        else if( !strcmp( str, "ET") ) {
            strcpy( name, str );
            code = PdfeGOperator::ET;
            cat = PdfeGCategory::TextObjects;
        }
        else if( !strcmp( str, "Tc") ) {
            strcpy( name, str );
            code = PdfeGOperator::Tc;
            cat = PdfeGCategory::TextState;
        }
        else if( !strcmp( str, "Tw") ) {
            strcpy( name, str );
            code = PdfeGOperator::Tw;
            cat = PdfeGCategory::TextState;
        }
        else if( !strcmp( str, "Tz") ) {
            strcpy( name, str );
            code = PdfeGOperator::Tz;
            cat = PdfeGCategory::TextState;
        }
        else if( !strcmp( str, "TL") ) {
            strcpy( name, str );
            code = PdfeGOperator::TL;
            cat = PdfeGCategory::TextState;
        }
        else if( !strcmp( str, "Tf") ) {
            strcpy( name, str );
            code = PdfeGOperator::Tf;
            cat = PdfeGCategory::TextState;
        }
        else if( !strcmp( str, "Tr") ) {
            strcpy( name, str );
            code = PdfeGOperator::Tr;
            cat = PdfeGCategory::TextState;
        }
        else if( !strcmp( str, "Ts") ) {
            strcpy( name, str );
            code = PdfeGOperator::Ts;
            cat = PdfeGCategory::TextState;
        }
        else if( !strcmp( str, "w") ) {
            strcpy( name, str );
            code = PdfeGOperator::w;
            cat = PdfeGCategory::GeneralGState;
        }
        else if( !strcmp( str, "J") ) {
            strcpy( name, str );
            code = PdfeGOperator::J;
            cat = PdfeGCategory::GeneralGState;
        }
        else if( !strcmp( str, "j") ) {
            strcpy( name, str );
            code = PdfeGOperator::j;
            cat = PdfeGCategory::GeneralGState;
        }
        else if( !strcmp( str, "M") ) {
            strcpy( name, str );
            code = PdfeGOperator::M;
            cat = PdfeGCategory::GeneralGState;
        }
        else if( !strcmp( str, "d") ) {
            strcpy( name, str );
            code = PdfeGOperator::d;
            cat = PdfeGCategory::GeneralGState;
        }
        else if( !strcmp( str, "ri") ) {
            strcpy( name, str );
            code = PdfeGOperator::ri;
            cat = PdfeGCategory::GeneralGState;
        }
        else if( !strcmp( str, "i") ) {
            strcpy( name, str );
            code = PdfeGOperator::i;
            cat = PdfeGCategory::GeneralGState;
        }
        else if( !strcmp( str, "gs") ) {
            strcpy( name, str );
            code = PdfeGOperator::gs;
            cat = PdfeGCategory::GeneralGState;
        }
        else if( !strcmp( str, "q") ) {
            strcpy( name, str );
            code = PdfeGOperator::q;
            cat = PdfeGCategory::SpecialGState;
        }
        else if( !strcmp( str, "Q") ) {
            strcpy( name, str );
            code = PdfeGOperator::Q;
            cat = PdfeGCategory::SpecialGState;
        }
        else if( !strcmp( str, "cm") ) {
            strcpy( name, str );
            code = PdfeGOperator::cm;
            cat = PdfeGCategory::SpecialGState;
        }
        else if( !strcmp( str, "m") ) {
            strcpy( name, str );
            code = PdfeGOperator::m;
            cat = PdfeGCategory::PathConstruction;
        }
        else if( !strcmp( str, "l") ) {
            strcpy( name, str );
            code = PdfeGOperator::l;
            cat = PdfeGCategory::PathConstruction;
        }
        else if( !strcmp( str, "c") ) {
            strcpy( name, str );
            code = PdfeGOperator::c;
            cat = PdfeGCategory::PathConstruction;
        }
        else if( !strcmp( str, "v") ) {
            strcpy( name, str );
            code = PdfeGOperator::v;
            cat = PdfeGCategory::PathConstruction;
        }
        else if( !strcmp( str, "y") ) {
            strcpy( name, str );
            code = PdfeGOperator::y;
            cat = PdfeGCategory::PathConstruction;
        }
        else if( !strcmp( str, "h") ) {
            strcpy( name, str );
            code = PdfeGOperator::h;
            cat = PdfeGCategory::PathConstruction;
        }
        else if( !strcmp( str, "re") ) {
            strcpy( name, str );
            code = PdfeGOperator::re;
            cat = PdfeGCategory::PathConstruction;
        }
        else if( !strcmp( str, "S") ) {
            strcpy( name, str );
            code = PdfeGOperator::S;
            cat = PdfeGCategory::PathPainting;
        }
        else if( !strcmp( str, "s") ) {
            strcpy( name, str );
            code = PdfeGOperator::s;
            cat = PdfeGCategory::PathPainting;
        }
        else if( !strcmp( str, "f") ) {
            strcpy( name, str );
            code = PdfeGOperator::f;
            cat = PdfeGCategory::PathPainting;
        }
        else if( !strcmp( str, "F") ) {
            strcpy( name, str );
            code = PdfeGOperator::F;
            cat = PdfeGCategory::PathPainting;
        }
        else if( !strcmp( str, "f*") ) {
            strcpy( name, str );
            code = PdfeGOperator::fstar;
            cat = PdfeGCategory::PathPainting;
        }
        else if( !strcmp( str, "B") ) {
            strcpy( name, str );
            code = PdfeGOperator::B;
            cat = PdfeGCategory::PathPainting;
        }
        else if( !strcmp( str, "B*") ) {
            strcpy( name, str );
            code = PdfeGOperator::Bstar;
            cat = PdfeGCategory::PathPainting;
        }
        else if( !strcmp( str, "b") ) {
            strcpy( name, str );
            code = PdfeGOperator::b;
            cat = PdfeGCategory::PathPainting;
        }
        else if( !strcmp( str, "b*") ) {
            strcpy( name, str );
            code = PdfeGOperator::bstar;
            cat = PdfeGCategory::PathPainting;
        }
        else if( !strcmp( str, "n") ) {
            strcpy( name, str );
            code = PdfeGOperator::n;
            cat = PdfeGCategory::PathPainting;
        }
        else if( !strcmp( str, "W") ) {
            strcpy( name, str );
            code = PdfeGOperator::W;
            cat = PdfeGCategory::ClippingPath;
        }
        else if( !strcmp( str, "W*") ) {
            strcpy( name, str );
            code = PdfeGOperator::Wstar;
            cat = PdfeGCategory::ClippingPath;
        }
        else if( !strcmp( str, "d0") ) {
            strcpy( name, str );
            code = PdfeGOperator::d0;
            cat = PdfeGCategory::Type3Fonts;
        }
        else if( !strcmp( str, "d1") ) {
            strcpy( name, str );
            code = PdfeGOperator::d1;
            cat = PdfeGCategory::Type3Fonts;
        }
        else if( !strcmp( str, "CS") ) {
            strcpy( name, str );
            code = PdfeGOperator::CS;
            cat = PdfeGCategory::Color;
        }
        else if( !strcmp( str, "cs") ) {
            strcpy( name, str );
            code = PdfeGOperator::cs;
            cat = PdfeGCategory::Color;
        }
        else if( !strcmp( str, "SC") ) {
            strcpy( name, str );
            code = PdfeGOperator::SC;
            cat = PdfeGCategory::Color;
        }
        else if( !strcmp( str, "SCN") ) {
            strcpy( name, str );
            code = PdfeGOperator::SCN;
            cat = PdfeGCategory::Color;
        }
        else if( !strcmp( str, "sc") ) {
            strcpy( name, str );
            code = PdfeGOperator::sc;
            cat = PdfeGCategory::Color;
        }
        else if( !strcmp( str, "scn") ) {
            strcpy( name, str );
            code = PdfeGOperator::scn;
            cat = PdfeGCategory::Color;
        }
        else if( !strcmp( str, "G") ) {
            strcpy( name, str );
            code = PdfeGOperator::G;
            cat = PdfeGCategory::Color;
        }
        else if( !strcmp( str, "g") ) {
            strcpy( name, str );
            code = PdfeGOperator::g;
            cat = PdfeGCategory::Color;
        }
        else if( !strcmp( str, "RG") ) {
            strcpy( name, str );
            code = PdfeGOperator::RG;
            cat = PdfeGCategory::Color;
        }
        else if( !strcmp( str, "rg") ) {
            strcpy( name, str );
            code = PdfeGOperator::rg;
            cat = PdfeGCategory::Color;
        }
        else if( !strcmp( str, "K") ) {
            strcpy( name, str );
            code = PdfeGOperator::K;
            cat = PdfeGCategory::Color;
        }
        else if( !strcmp( str, "k") ) {
            strcpy( name, str );
            code = PdfeGOperator::k;
            cat = PdfeGCategory::Color;
        }
        else if( !strcmp( str, "sh") ) {
            strcpy( name, str );
            code = PdfeGOperator::sh;
            cat = PdfeGCategory::ShadingPatterns;
        }
        else if( !strcmp( str, "BI") ) {
            strcpy( name, str );
            code = PdfeGOperator::BI;
            cat = PdfeGCategory::InlineImages;
        }
        else if( !strcmp( str, "ID") ) {
            strcpy( name, str );
            code = PdfeGOperator::ID;
            cat = PdfeGCategory::InlineImages;
        }
        else if( !strcmp( str, "EI") ) {
            strcpy( name, str );
            code = PdfeGOperator::EI;
            cat = PdfeGCategory::InlineImages;
        }
        else if( !strcmp( str, "Do") ) {
            strcpy( name, str );
            code = PdfeGOperator::Do;
            cat = PdfeGCategory::XObjects;
        }
        else if( !strcmp( str, "MP") ) {
            strcpy( name, str );
            code = PdfeGOperator::MP;
            cat = PdfeGCategory::MarkedContents;
        }
        else if( !strcmp( str, "DP") ) {
            strcpy( name, str );
            code = PdfeGOperator::DP;
            cat = PdfeGCategory::MarkedContents;
        }
        else if( !strcmp( str, "BMC") ) {
            strcpy( name, str );
            code = PdfeGOperator::BMC;
            cat = PdfeGCategory::MarkedContents;
        }
        else if( !strcmp( str, "BDC") ) {
            strcpy( name, str );
            code = PdfeGOperator::BDC;
            cat = PdfeGCategory::MarkedContents;
        }
        else if( !strcmp( str, "EMC") ) {
            strcpy( name, str );
            code = PdfeGOperator::EMC;
            cat = PdfeGCategory::MarkedContents;
        }
        else if( !strcmp( str, "BX") ) {
            strcpy( name, str );
            code = PdfeGOperator::BX;
            cat = PdfeGCategory::Compatibility;
        }
        else if( !strcmp( str, "EX") ) {
            strcpy( name, str );
            code = PdfeGOperator::EX;
            cat = PdfeGCategory::Compatibility;
        }
        else {
            // Copy name if short enough.
            if( strlen( str ) < 6 ) {
                strcpy( name, str );
            }
            else {
                name[0] = 0;
            }
            code = PdfeGOperator::Unknown;
            cat = PdfeGCategory::Unknown;
        }
    }

    /** Is the operator a painting operator which closes the path.
     * \return True if it closes the path.
     */
    bool isClosePainting() const {
        return (cat == PdfeGCategory::PathPainting) &&
                ( code == PdfeGOperator::s ||
                  code == PdfeGOperator::f ||
                  code == PdfeGOperator::F ||
                  code == PdfeGOperator::fstar ||
                  code == PdfeGOperator::B ||
                  code == PdfeGOperator::Bstar ||
                  code == PdfeGOperator::b ||
                  code == PdfeGOperator::bstar );
    }
    /** Is the painting operator using the even-odd rule (otherwise, non zero winding rule).
     * \return True if even-odd rule.
     */
    bool isEvenOddRule() const {
        return  code == PdfeGOperator::fstar ||
                code == PdfeGOperator::Bstar ||
                code == PdfeGOperator::bstar ||
                code == PdfeGOperator::Wstar;
    }
    /** Get the filling rule of the painting operator.
     * \return Filling rule. Unknown if does not apply to the current operator.
     */
    PdfeFillingRule::Enum fillingRule() const {
        if( code == PdfeGOperator::fstar ||
                code == PdfeGOperator::Bstar ||
                code == PdfeGOperator::bstar ||
                code == PdfeGOperator::Wstar ) {
            return PdfeFillingRule::Winding;
        }
        else if( code == PdfeGOperator::f ||
                 code == PdfeGOperator::F ||
                 code == PdfeGOperator::B ||
                 code == PdfeGOperator::b ||
                 code == PdfeGOperator::W ) {
            return PdfeFillingRule::EvenOdd;
        }
        return PdfeFillingRule::Unknown;
    }
};

}

#endif // PDFEGRAPHICSENUMS_H
