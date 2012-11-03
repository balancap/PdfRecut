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

/// Pdf graphic operators categories, as defined in PDF reference 1.7.
enum EPdfeGCategory {
    ePdfGCategory_GeneralGState,
    ePdfGCategory_SpecialGState,
    ePdfGCategory_PathConstruction,
    ePdfGCategory_PathPainting,
    ePdfGCategory_ClippingPath,
    ePdfGCategory_TextObjects,
    ePdfGCategory_TextState,
    ePdfGCategory_TextPositioning,
    ePdfGCategory_TextShowing,
    ePdfGCategory_Type3Fonts,
    ePdfGCategory_Color,
    ePdfGCategory_ShadingPatterns,
    ePdfGCategory_InlineImages,
    ePdfGCategory_XObjects,
    ePdfGCategory_MarkedContents,
    ePdfGCategory_Compatibility,
    ePdfGCategory_Unknown
};

/// Pdf graphic operator, as defined in PDF reference 1.7.
enum EPdfeGOperator {
    ePdfGOperator_w,
    ePdfGOperator_J,
    ePdfGOperator_j,
    ePdfGOperator_M,
    ePdfGOperator_d,
    ePdfGOperator_ri,
    ePdfGOperator_i,
    ePdfGOperator_gs,

    ePdfGOperator_q,
    ePdfGOperator_Q,
    ePdfGOperator_cm,

    ePdfGOperator_m,
    ePdfGOperator_l,
    ePdfGOperator_c,
    ePdfGOperator_v,
    ePdfGOperator_y,
    ePdfGOperator_h,
    ePdfGOperator_re,

    ePdfGOperator_S,
    ePdfGOperator_s,
    ePdfGOperator_f,
    ePdfGOperator_F,
    ePdfGOperator_fstar,
    ePdfGOperator_B,
    ePdfGOperator_Bstar,
    ePdfGOperator_b,
    ePdfGOperator_bstar,
    ePdfGOperator_n,

    ePdfGOperator_W,
    ePdfGOperator_Wstar,

    ePdfGOperator_BT,
    ePdfGOperator_ET,

    ePdfGOperator_Tc,
    ePdfGOperator_Tw,
    ePdfGOperator_Tz,
    ePdfGOperator_TL,
    ePdfGOperator_Tf,
    ePdfGOperator_Tr,
    ePdfGOperator_Ts,

    ePdfGOperator_Td,
    ePdfGOperator_TD,
    ePdfGOperator_Tm,
    ePdfGOperator_Tstar,

    ePdfGOperator_Tj,
    ePdfGOperator_TJ,
    ePdfGOperator_Quote,
    ePdfGOperator_DoubleQuote,

    ePdfGOperator_d0,
    ePdfGOperator_d1,

    ePdfGOperator_CS,
    ePdfGOperator_cs,
    ePdfGOperator_SC,
    ePdfGOperator_SCN,
    ePdfGOperator_sc,
    ePdfGOperator_scn,
    ePdfGOperator_G,
    ePdfGOperator_g,
    ePdfGOperator_RG,
    ePdfGOperator_rg,
    ePdfGOperator_K,
    ePdfGOperator_k,

    ePdfGOperator_sh,

    ePdfGOperator_BI,
    ePdfGOperator_ID,
    ePdfGOperator_EI,

    ePdfGOperator_Do,

    ePdfGOperator_MP,
    ePdfGOperator_DP,
    ePdfGOperator_BMC,
    ePdfGOperator_BDC,
    ePdfGOperator_EMC,

    ePdfGOperator_BX,
    ePdfGOperator_EX,

    ePdfGOperator_Unknown
};

/** Structure gathering information on a graphic operator.
 */
struct PdfeGraphicOperator
{
    /// Pdf name of the operator
    char name[6];
    /// Enum code of the operator
    EPdfeGOperator code;
    /// Enum code of the category it belongs to.
    EPdfeGCategory cat;

    /** Default constructor.
     */
    PdfeGraphicOperator() {
        this->init();
    }
    /** Initialize to the operator Unknown.
     */
    void init() {
        name[0] = 0;
        code = ePdfGOperator_Unknown;
        cat = ePdfGCategory_Unknown;
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
            code = ePdfGOperator_Tj;
            cat = ePdfGCategory_TextShowing;
        }
        else if( !strcmp( str, "TJ") ) {
            strcpy( name, str );
            code = ePdfGOperator_TJ;
            cat = ePdfGCategory_TextShowing;
        }
        else if( !strcmp( str, "'") ) {
            strcpy( name, str );
            code = ePdfGOperator_Quote;
            cat = ePdfGCategory_TextShowing;
        }
        else if( !strcmp( str, "\"") ) {
            strcpy( name, str );
            code = ePdfGOperator_DoubleQuote;
            cat = ePdfGCategory_TextShowing;
        }
        else if( !strcmp( str, "Td") ) {
            strcpy( name, str );
            code = ePdfGOperator_Td;
            cat = ePdfGCategory_TextPositioning;
        }
        else if( !strcmp( str, "TD") ) {
            strcpy( name, str );
            code = ePdfGOperator_TD;
            cat = ePdfGCategory_TextPositioning;
        }
        else if( !strcmp( str, "Tm") ) {
            strcpy( name, str );
            code = ePdfGOperator_Tm;
            cat = ePdfGCategory_TextPositioning;
        }
        else if( !strcmp( str, "T*") ) {
            strcpy( name, str );
            code = ePdfGOperator_Tstar;
            cat = ePdfGCategory_TextPositioning;
        }
        else if( !strcmp( str, "BT") ) {
            strcpy( name, str );
            code = ePdfGOperator_BT;
            cat = ePdfGCategory_TextObjects;
        }
        else if( !strcmp( str, "ET") ) {
            strcpy( name, str );
            code = ePdfGOperator_ET;
            cat = ePdfGCategory_TextObjects;
        }
        else if( !strcmp( str, "Tc") ) {
            strcpy( name, str );
            code = ePdfGOperator_Tc;
            cat = ePdfGCategory_TextState;
        }
        else if( !strcmp( str, "Tw") ) {
            strcpy( name, str );
            code = ePdfGOperator_Tw;
            cat = ePdfGCategory_TextState;
        }
        else if( !strcmp( str, "Tz") ) {
            strcpy( name, str );
            code = ePdfGOperator_Tz;
            cat = ePdfGCategory_TextState;
        }
        else if( !strcmp( str, "TL") ) {
            strcpy( name, str );
            code = ePdfGOperator_TL;
            cat = ePdfGCategory_TextState;
        }
        else if( !strcmp( str, "Tf") ) {
            strcpy( name, str );
            code = ePdfGOperator_Tf;
            cat = ePdfGCategory_TextState;
        }
        else if( !strcmp( str, "Tr") ) {
            strcpy( name, str );
            code = ePdfGOperator_Tr;
            cat = ePdfGCategory_TextState;
        }
        else if( !strcmp( str, "Ts") ) {
            strcpy( name, str );
            code = ePdfGOperator_Ts;
            cat = ePdfGCategory_TextState;
        }
        else if( !strcmp( str, "w") ) {
            strcpy( name, str );
            code = ePdfGOperator_w;
            cat = ePdfGCategory_GeneralGState;
        }
        else if( !strcmp( str, "J") ) {
            strcpy( name, str );
            code = ePdfGOperator_J;
            cat = ePdfGCategory_GeneralGState;
        }
        else if( !strcmp( str, "j") ) {
            strcpy( name, str );
            code = ePdfGOperator_j;
            cat = ePdfGCategory_GeneralGState;
        }
        else if( !strcmp( str, "M") ) {
            strcpy( name, str );
            code = ePdfGOperator_M;
            cat = ePdfGCategory_GeneralGState;
        }
        else if( !strcmp( str, "d") ) {
            strcpy( name, str );
            code = ePdfGOperator_d;
            cat = ePdfGCategory_GeneralGState;
        }
        else if( !strcmp( str, "ri") ) {
            strcpy( name, str );
            code = ePdfGOperator_ri;
            cat = ePdfGCategory_GeneralGState;
        }
        else if( !strcmp( str, "i") ) {
            strcpy( name, str );
            code = ePdfGOperator_i;
            cat = ePdfGCategory_GeneralGState;
        }
        else if( !strcmp( str, "gs") ) {
            strcpy( name, str );
            code = ePdfGOperator_gs;
            cat = ePdfGCategory_GeneralGState;
        }
        else if( !strcmp( str, "q") ) {
            strcpy( name, str );
            code = ePdfGOperator_q;
            cat = ePdfGCategory_SpecialGState;
        }
        else if( !strcmp( str, "Q") ) {
            strcpy( name, str );
            code = ePdfGOperator_Q;
            cat = ePdfGCategory_SpecialGState;
        }
        else if( !strcmp( str, "cm") ) {
            strcpy( name, str );
            code = ePdfGOperator_cm;
            cat = ePdfGCategory_SpecialGState;
        }
        else if( !strcmp( str, "m") ) {
            strcpy( name, str );
            code = ePdfGOperator_m;
            cat = ePdfGCategory_PathConstruction;
        }
        else if( !strcmp( str, "l") ) {
            strcpy( name, str );
            code = ePdfGOperator_l;
            cat = ePdfGCategory_PathConstruction;
        }
        else if( !strcmp( str, "c") ) {
            strcpy( name, str );
            code = ePdfGOperator_c;
            cat = ePdfGCategory_PathConstruction;
        }
        else if( !strcmp( str, "v") ) {
            strcpy( name, str );
            code = ePdfGOperator_v;
            cat = ePdfGCategory_PathConstruction;
        }
        else if( !strcmp( str, "y") ) {
            strcpy( name, str );
            code = ePdfGOperator_y;
            cat = ePdfGCategory_PathConstruction;
        }
        else if( !strcmp( str, "h") ) {
            strcpy( name, str );
            code = ePdfGOperator_h;
            cat = ePdfGCategory_PathConstruction;
        }
        else if( !strcmp( str, "re") ) {
            strcpy( name, str );
            code = ePdfGOperator_re;
            cat = ePdfGCategory_PathConstruction;
        }
        else if( !strcmp( str, "S") ) {
            strcpy( name, str );
            code = ePdfGOperator_S;
            cat = ePdfGCategory_PathPainting;
        }
        else if( !strcmp( str, "s") ) {
            strcpy( name, str );
            code = ePdfGOperator_s;
            cat = ePdfGCategory_PathPainting;
        }
        else if( !strcmp( str, "f") ) {
            strcpy( name, str );
            code = ePdfGOperator_f;
            cat = ePdfGCategory_PathPainting;
        }
        else if( !strcmp( str, "F") ) {
            strcpy( name, str );
            code = ePdfGOperator_F;
            cat = ePdfGCategory_PathPainting;
        }
        else if( !strcmp( str, "f*") ) {
            strcpy( name, str );
            code = ePdfGOperator_fstar;
            cat = ePdfGCategory_PathPainting;
        }
        else if( !strcmp( str, "B") ) {
            strcpy( name, str );
            code = ePdfGOperator_B;
            cat = ePdfGCategory_PathPainting;
        }
        else if( !strcmp( str, "B*") ) {
            strcpy( name, str );
            code = ePdfGOperator_Bstar;
            cat = ePdfGCategory_PathPainting;
        }
        else if( !strcmp( str, "b") ) {
            strcpy( name, str );
            code = ePdfGOperator_b;
            cat = ePdfGCategory_PathPainting;
        }
        else if( !strcmp( str, "b*") ) {
            strcpy( name, str );
            code = ePdfGOperator_bstar;
            cat = ePdfGCategory_PathPainting;
        }
        else if( !strcmp( str, "n") ) {
            strcpy( name, str );
            code = ePdfGOperator_n;
            cat = ePdfGCategory_PathPainting;
        }
        else if( !strcmp( str, "W") ) {
            strcpy( name, str );
            code = ePdfGOperator_W;
            cat = ePdfGCategory_ClippingPath;
        }
        else if( !strcmp( str, "W*") ) {
            strcpy( name, str );
            code = ePdfGOperator_Wstar;
            cat = ePdfGCategory_ClippingPath;
        }
        else if( !strcmp( str, "d0") ) {
            strcpy( name, str );
            code = ePdfGOperator_d0;
            cat = ePdfGCategory_Type3Fonts;
        }
        else if( !strcmp( str, "d1") ) {
            strcpy( name, str );
            code = ePdfGOperator_d1;
            cat = ePdfGCategory_Type3Fonts;
        }
        else if( !strcmp( str, "CS") ) {
            strcpy( name, str );
            code = ePdfGOperator_CS;
            cat = ePdfGCategory_Color;
        }
        else if( !strcmp( str, "cs") ) {
            strcpy( name, str );
            code = ePdfGOperator_cs;
            cat = ePdfGCategory_Color;
        }
        else if( !strcmp( str, "SC") ) {
            strcpy( name, str );
            code = ePdfGOperator_SC;
            cat = ePdfGCategory_Color;
        }
        else if( !strcmp( str, "SCN") ) {
            strcpy( name, str );
            code = ePdfGOperator_SCN;
            cat = ePdfGCategory_Color;
        }
        else if( !strcmp( str, "sc") ) {
            strcpy( name, str );
            code = ePdfGOperator_sc;
            cat = ePdfGCategory_Color;
        }
        else if( !strcmp( str, "scn") ) {
            strcpy( name, str );
            code = ePdfGOperator_scn;
            cat = ePdfGCategory_Color;
        }
        else if( !strcmp( str, "G") ) {
            strcpy( name, str );
            code = ePdfGOperator_G;
            cat = ePdfGCategory_Color;
        }
        else if( !strcmp( str, "g") ) {
            strcpy( name, str );
            code = ePdfGOperator_g;
            cat = ePdfGCategory_Color;
        }
        else if( !strcmp( str, "RG") ) {
            strcpy( name, str );
            code = ePdfGOperator_RG;
            cat = ePdfGCategory_Color;
        }
        else if( !strcmp( str, "rg") ) {
            strcpy( name, str );
            code = ePdfGOperator_rg;
            cat = ePdfGCategory_Color;
        }
        else if( !strcmp( str, "K") ) {
            strcpy( name, str );
            code = ePdfGOperator_K;
            cat = ePdfGCategory_Color;
        }
        else if( !strcmp( str, "k") ) {
            strcpy( name, str );
            code = ePdfGOperator_k;
            cat = ePdfGCategory_Color;
        }
        else if( !strcmp( str, "sh") ) {
            strcpy( name, str );
            code = ePdfGOperator_sh;
            cat = ePdfGCategory_ShadingPatterns;
        }
        else if( !strcmp( str, "BI") ) {
            strcpy( name, str );
            code = ePdfGOperator_BI;
            cat = ePdfGCategory_InlineImages;
        }
        else if( !strcmp( str, "ID") ) {
            strcpy( name, str );
            code = ePdfGOperator_ID;
            cat = ePdfGCategory_InlineImages;
        }
        else if( !strcmp( str, "EI") ) {
            strcpy( name, str );
            code = ePdfGOperator_EI;
            cat = ePdfGCategory_InlineImages;
        }
        else if( !strcmp( str, "Do") ) {
            strcpy( name, str );
            code = ePdfGOperator_Do;
            cat = ePdfGCategory_XObjects;
        }
        else if( !strcmp( str, "MP") ) {
            strcpy( name, str );
            code = ePdfGOperator_MP;
            cat = ePdfGCategory_MarkedContents;
        }
        else if( !strcmp( str, "DP") ) {
            strcpy( name, str );
            code = ePdfGOperator_DP;
            cat = ePdfGCategory_MarkedContents;
        }
        else if( !strcmp( str, "BMC") ) {
            strcpy( name, str );
            code = ePdfGOperator_BMC;
            cat = ePdfGCategory_MarkedContents;
        }
        else if( !strcmp( str, "BDC") ) {
            strcpy( name, str );
            code = ePdfGOperator_BDC;
            cat = ePdfGCategory_MarkedContents;
        }
        else if( !strcmp( str, "EMC") ) {
            strcpy( name, str );
            code = ePdfGOperator_EMC;
            cat = ePdfGCategory_MarkedContents;
        }
        else if( !strcmp( str, "BX") ) {
            strcpy( name, str );
            code = ePdfGOperator_BX;
            cat = ePdfGCategory_Compatibility;
        }
        else if( !strcmp( str, "EX") ) {
            strcpy( name, str );
            code = ePdfGOperator_EX;
            cat = ePdfGCategory_Compatibility;
        }
        else {
            // Copy name if short enough.
            if( strlen( str ) < 6 ) {
                strcpy( name, str );
            }
            else {
                name[0] = 0;
            }
            code = ePdfGOperator_Unknown;
            cat = ePdfGCategory_Unknown;
        }
    }

    /** Is the operator a painting operator which closes the path.
     * \return True if it closes the path.
     */
    bool isClosePainting() const
    {
        return (cat == ePdfGCategory_PathPainting) &&
                ( code == ePdfGOperator_s ||
                  code == ePdfGOperator_f ||
                  code == ePdfGOperator_F ||
                  code == ePdfGOperator_fstar ||
                  code == ePdfGOperator_B ||
                  code == ePdfGOperator_Bstar ||
                  code == ePdfGOperator_b ||
                  code == ePdfGOperator_bstar );
    }
    /** Is the painting operator using the even-odd rule (otherwise, non zero winding rule).
     * \return True if even-odd rule.
     */
    bool isEvenOddRule() const
    {
        return  code == ePdfGOperator_fstar ||
                code == ePdfGOperator_Bstar ||
                code == ePdfGOperator_bstar ||
                code == ePdfGOperator_Wstar;
    }
};

}

#endif // PDFEGRAPHICSENUMS_H
