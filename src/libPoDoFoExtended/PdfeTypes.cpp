/***************************************************************************
 *   Copyright (C) 2011 by Paul Balança                                    *
 *   paul.balanca@gmail.com                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "PdfeTypes.h"

//namespace PoDoFoExtended {

//**********************************************************//
//                       CID/GID/UTF16                      //
//**********************************************************//
std::vector<pdfe_utf16> UTF16BEStrToUTF16Vec( const char* pstr, size_t length )
{
    std::vector<pdfe_utf16>  utf16vec;
    utf16vec.reserve( length / 2 );

    const pdfe_utf16* pchar = reinterpret_cast<const pdfe_utf16*>( pstr );
    for( size_t i = 0 ; i < length; i+=2 ) {
        utf16vec.push_back( PDFE_UTF16BE_TO_HBO( *pchar ) );
        ++pchar;
    }
    return utf16vec;
}
QString UTF16VecToQString( const std::vector<pdfe_utf16>& utf16vec )
{
    const ushort* putf16 = reinterpret_cast<const ushort*>( &utf16vec[0] );
    return QString::fromUtf16( putf16, utf16vec.size() );
}

//**********************************************************//
//                         PdfeVector                       //
//**********************************************************//

//**********************************************************//
//                         PdfeMatrix                       //
//**********************************************************//
PdfeVector PdfeMatrix::map( const PdfeVector& vect ) const
{
    PdfeVector mapVect;
    mapVect(0) = this->at(0,0) * vect(0) + this->at(1,0) * vect(1) + this->at(2,0);
    mapVect(1) = this->at(0,1) * vect(0) + this->at(1,1) * vect(1) + this->at(2,1);
    mapVect(2) = 1.0;
    return mapVect;
}
PdfeORect PdfeMatrix::map( const PdfeORect& rect ) const
{
    PdfeORect mapRect;
    PdfeVector tmpVect1, tmpVect2;
    double tmpVal;

    // Map left bottom point.
    mapRect.setLeftBottom( this->map( rect.leftBottom() ) );

    // Set direction and width: only apply the 2x2 transformation matrix.
    tmpVect2 = rect.direction();
    tmpVect1(0) = this->at(0,0) * tmpVect2(0) + this->at(1,0) * tmpVect2(1);
    tmpVect1(1) = this->at(0,1) * tmpVect2(0) + this->at(1,1) * tmpVect2(1);
    tmpVect1(2) = 1.0;
    tmpVal = tmpVect1.norm2();

    mapRect.setDirection( tmpVect1 );
    mapRect.setWidth( rect.width() * tmpVal );

    // Set height (slight approximation...).
    tmpVect1(0) = this->at(0,0) * -tmpVect2(1) + this->at(1,0) * tmpVect2(0);
    tmpVect1(1) = this->at(0,1) * -tmpVect2(1) + this->at(1,1) * tmpVect2(0);
    tmpVect1(2) = 1.0;
    tmpVect2 = mapRect.direction();
    tmpVal = tmpVect1(0) * -tmpVect2(1) + tmpVect1(1) * tmpVect2(0);

    double height = rect.height() * tmpVal;
    if( height < 0.0 ) {
        // Negative height: modify left bottom point so that the height becomes positive.
        mapRect.setLeftBottom( mapRect.leftBottom() + mapRect.direction().rotate90() * height );
        mapRect.setHeight( -height );
    }
    else {
        mapRect.setHeight( height );
    }

    return mapRect;
}
//**********************************************************//
//                         PdfeORect                        //
//**********************************************************//
PdfeMatrix PdfeORect::localTransMatrix()
{
    PdfeMatrix transMat;
    transMat(0,0) = m_direction(0);      transMat(0,1) = -m_direction(1);
    transMat(1,0) = m_direction(1);      transMat(1,1) = m_direction(0);
    transMat(2,0) = -m_leftBottom(0) * m_direction(0) - m_leftBottom(1) * m_direction(1);
    transMat(2,1) =  m_leftBottom(0) * m_direction(1) - m_leftBottom(1) * m_direction(0);
    return transMat;
}

PoDoFo::PdfRect PdfeORect::toPdfRect( bool bbox ) const
{
    if( bbox ) {
        // Compute bounding box coordinates.
        double left = std::min( leftBottomX(), std::min( leftTopX(), std::min( rightBottomX(), rightTopX() ) ) );
        double right = std::max( leftBottomX(), std::max( leftTopX(), std::max( rightBottomX(), rightTopX() ) ) );
        double bottom = std::min( leftBottomY(), std::min( leftTopY(), std::min( rightBottomY(), rightTopY() ) ) );
        double top = std::max( leftBottomY(), std::max( leftTopY(), std::max( rightBottomY(), rightTopY() ) ) );

        return PoDoFo::PdfRect( left, bottom, right-left, top-bottom );
    }
    else {
        // Very deep approximation !
        return PoDoFo::PdfRect( m_leftBottom(0), m_leftBottom(1), m_width, m_height );
    }
}

double PdfeORect::minDistance( const PdfeORect& rect1, const PdfeORect& rect2 )
{
    PdfeVector point;
    double dist = std::numeric_limits<double>::max();

    point = rect1.leftBottom() - rect2.leftBottom();
    dist = std::min( dist, point.norm2() );
    point = rect1.leftBottom() - rect2.rightBottom();
    dist = std::min( dist, point.norm2() );
    point = rect1.leftBottom() - rect2.leftTop();
    dist = std::min( dist, point.norm2() );
    point = rect1.leftBottom() - rect2.rightTop();
    dist = std::min( dist, point.norm2() );

    point = rect1.rightBottom() - rect2.leftBottom();
    dist = std::min( dist, point.norm2() );
    point = rect1.rightBottom() - rect2.rightBottom();
    dist = std::min( dist, point.norm2() );
    point = rect1.rightBottom() - rect2.leftTop();
    dist = std::min( dist, point.norm2() );
    point = rect1.rightBottom() - rect2.rightTop();
    dist = std::min( dist, point.norm2() );

    point = rect1.leftTop() - rect2.leftBottom();
    dist = std::min( dist, point.norm2() );
    point = rect1.leftTop() - rect2.rightBottom();
    dist = std::min( dist, point.norm2() );
    point = rect1.leftTop() - rect2.leftTop();
    dist = std::min( dist, point.norm2() );
    point = rect1.leftTop() - rect2.rightTop();
    dist = std::min( dist, point.norm2() );

    point = rect1.rightTop() - rect2.leftBottom();
    dist = std::min( dist, point.norm2() );
    point = rect1.rightTop() - rect2.rightBottom();
    dist = std::min( dist, point.norm2() );
    point = rect1.rightTop() - rect2.leftTop();
    dist = std::min( dist, point.norm2() );
    point = rect1.rightTop() - rect2.rightTop();
    dist = std::min( dist, point.norm2() );

    return dist;
}
double PdfeORect::minDistance( const PdfeORect& rect, const PdfeVector& point )
{
    PdfeVector point0;
    double dist = std::numeric_limits<double>::max();

    point0 = point - rect.leftBottom();
    dist = std::min( dist, point0.norm2() );
    point0 = point - rect.rightBottom();
    dist = std::min( dist, point0.norm2() );
    point0 = point - rect.leftTop();
    dist = std::min( dist, point0.norm2() );
    point0 = point - rect.rightTop();
    dist = std::min( dist, point0.norm2() );

    return dist;
}

double PdfeORect::maxDistance( const PdfeORect& rect1, const PdfeORect& rect2 )
{
    PdfeVector point;
    double dist = 0;

    point = rect1.leftBottom() - rect2.leftBottom();
    dist = std::max( dist, point.norm2() );
    point = rect1.leftBottom() - rect2.rightBottom();
    dist = std::max( dist, point.norm2() );
    point = rect1.leftBottom() - rect2.leftTop();
    dist = std::max( dist, point.norm2() );
    point = rect1.leftBottom() - rect2.rightTop();
    dist = std::max( dist, point.norm2() );

    point = rect1.rightBottom() - rect2.leftBottom();
    dist = std::max( dist, point.norm2() );
    point = rect1.rightBottom() - rect2.rightBottom();
    dist = std::max( dist, point.norm2() );
    point = rect1.rightBottom() - rect2.leftTop();
    dist = std::max( dist, point.norm2() );
    point = rect1.rightBottom() - rect2.rightTop();
    dist = std::max( dist, point.norm2() );

    point = rect1.leftTop() - rect2.leftBottom();
    dist = std::max( dist, point.norm2() );
    point = rect1.leftTop() - rect2.rightBottom();
    dist = std::max( dist, point.norm2() );
    point = rect1.leftTop() - rect2.leftTop();
    dist = std::max( dist, point.norm2() );
    point = rect1.leftTop() - rect2.rightTop();
    dist = std::max( dist, point.norm2() );

    point = rect1.rightTop() - rect2.leftBottom();
    dist = std::max( dist, point.norm2() );
    point = rect1.rightTop() - rect2.rightBottom();
    dist = std::max( dist, point.norm2() );
    point = rect1.rightTop() - rect2.leftTop();
    dist = std::max( dist, point.norm2() );
    point = rect1.rightTop() - rect2.rightTop();
    dist = std::max( dist, point.norm2() );

    return dist;
}
double PdfeORect::maxDistance( const PdfeORect& rect, const PdfeVector& point )
{
    PdfeVector point0;
    double dist = 0;

    point0 = point - rect.leftBottom();
    dist = std::max( dist, point0.norm2() );
    point0 = point - rect.rightBottom();
    dist = std::max( dist, point0.norm2() );
    point0 = point - rect.leftTop();
    dist = std::max( dist, point0.norm2() );
    point0 = point - rect.rightTop();
    dist = std::max( dist, point0.norm2() );

    return dist;
}

PoDoFo::PdfRect PdfeORect::intersection(const PoDoFo::PdfRect& lhs,
                                               const PoDoFo::PdfRect& rhs )
{
    double left = std::max( lhs.GetLeft(), rhs.GetLeft() );
    double bottom = std::max( lhs.GetBottom(), rhs.GetBottom() );
    double right = std::min( lhs.GetLeft() + lhs.GetWidth(),
                             rhs.GetLeft() + rhs.GetWidth() );
    double top = std::min( lhs.GetBottom() + lhs.GetHeight(),
                           rhs.GetBottom() + rhs.GetHeight() );

    return PoDoFo::PdfRect( left, bottom,
                            std::max( 0.0, right - left ),
                            std::max( 0.0, top - bottom ) );
}
PoDoFo::PdfRect PdfeORect::reunion(const PoDoFo::PdfRect &lhs, const PoDoFo::PdfRect &rhs)
{
    double left = std::min( lhs.GetLeft(), rhs.GetLeft() );
    double bottom = std::min( lhs.GetBottom(), rhs.GetBottom() );
    double right = std::max( lhs.GetLeft() + lhs.GetWidth(),
                             rhs.GetLeft() + rhs.GetWidth() );
    double top = std::max( lhs.GetBottom() + lhs.GetHeight(),
                           rhs.GetBottom() + rhs.GetHeight() );

    return PoDoFo::PdfRect( left, bottom,
                            right - left,
                            top - bottom );
}
bool PdfeORect::inside( const PoDoFo::PdfRect& rect1, const PoDoFo::PdfRect& rect2 )
{
    return ( rect2.GetLeft() >= rect1.GetLeft() &&
             rect2.GetBottom() >= rect1.GetBottom() &&
             rect2.GetLeft() + rect2.GetWidth() <= rect1.GetLeft() + rect1.GetWidth() &&
             rect2.GetBottom() + rect2.GetHeight() <= rect1.GetBottom() + rect1.GetHeight() );
}


//}
