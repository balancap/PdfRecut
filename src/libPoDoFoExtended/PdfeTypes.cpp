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

#include "PdfeTypes.h"

#include <QString>

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
        utf16vec.push_back( PDFE_UTF16BE_HBO( *pchar ) );
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
PdfeVector& PdfeVector::intersection( const PoDoFo::PdfRect& zone )
{
    this->at(0,0) = std::min( std::max( this->at(0,0), zone.GetLeft() ),
                              zone.GetLeft() + zone.GetWidth() );
    this->at(0,1) = std::min( std::max( this->at(0,1), zone.GetBottom() ),
                              zone.GetBottom() + zone.GetHeight() );
    return *this;
}

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

//**********************************************************//
//                      PdfRect methods                     //
//**********************************************************//
namespace PdfeRect
{

PoDoFo::PdfRect intersection( const PoDoFo::PdfRect& rect1, const PoDoFo::PdfRect& rect2 )
{
    double left = std::max( rect1.GetLeft(), rect2.GetLeft() );
    double bottom = std::max( rect1.GetBottom(), rect2.GetBottom() );
    double right = std::min( rect1.GetLeft() + rect1.GetWidth(),
                             rect2.GetLeft() + rect2.GetWidth() );
    double top = std::min( rect1.GetBottom() + rect1.GetHeight(),
                           rect2.GetBottom() + rect2.GetHeight() );
    return PoDoFo::PdfRect( left, bottom,
                            std::max( 0.0, right - left ),
                            std::max( 0.0, top - bottom ) );
}
PoDoFo::PdfRect reunion( const PoDoFo::PdfRect& rect1, const PoDoFo::PdfRect& rect2 )
{
    double left = std::min( rect1.GetLeft(), rect2.GetLeft() );
    double bottom = std::min( rect1.GetBottom(), rect2.GetBottom() );
    double right = std::max( rect1.GetLeft() + rect1.GetWidth(),
                             rect2.GetLeft() + rect2.GetWidth() );
    double top = std::max( rect1.GetBottom() + rect1.GetHeight(),
                           rect2.GetBottom() + rect2.GetHeight() );
    return PoDoFo::PdfRect( left, bottom,
                            right - left,
                            top - bottom );
}
PoDoFo::PdfRect rescale( const PoDoFo::PdfRect& rect, double coef )
{
    return PoDoFo::PdfRect( rect.GetLeft() * coef,
                            rect.GetBottom() * coef,
                            rect.GetWidth() * coef,
                            rect.GetHeight() * coef );
}

bool contains( const PoDoFo::PdfRect& zone, const PoDoFo::PdfRect& rect )
{
    return ( rect.GetLeft() >= zone.GetLeft() &&
             rect.GetBottom() >= zone.GetBottom() &&
             rect.GetLeft() + rect.GetWidth() <= zone.GetLeft() + zone.GetWidth() &&
             rect.GetBottom() + rect.GetHeight() <= zone.GetBottom() + zone.GetHeight() );
}
bool intersects( const PoDoFo::PdfRect& zone, const PoDoFo::PdfRect& rect, bool strictInclusion )
{
    if( strictInclusion ) {
        return PdfeRect::contains( zone, rect );
    }
    else {
        bool left(false), right(false), bottom(false), top(false), center(false);

        // Evaluate the intersection with the different part of the zone.
        center = ( rect.GetLeft() >= zone.GetLeft() ) &&
                ( rect.GetBottom() >= zone.GetBottom() ) &&
                ( rect.GetLeft()+rect.GetWidth() <= zone.GetLeft()+zone.GetWidth() ) &&
                ( rect.GetBottom()+rect.GetHeight() <= zone.GetBottom()+zone.GetHeight() );

        left = !( rect.GetBottom()+rect.GetHeight() <= zone.GetBottom() ||
                  rect.GetBottom() >= zone.GetBottom()+zone.GetHeight() ) &&
                rect.GetLeft() <= zone.GetLeft();

        right = !( rect.GetBottom()+rect.GetHeight() <= zone.GetBottom() ||
                   rect.GetBottom() >= zone.GetBottom()+zone.GetHeight() ) &&
                rect.GetLeft()+rect.GetWidth() >= zone.GetLeft()+zone.GetWidth();

        bottom = !( rect.GetLeft()+rect.GetWidth() <= zone.GetLeft() ||
                    rect.GetLeft() >= zone.GetWidth()+zone.GetLeft() ) &&
                rect.GetBottom() <= zone.GetBottom();

        top = !( rect.GetLeft()+rect.GetWidth() <= zone.GetLeft() ||
                 rect.GetLeft() >= zone.GetWidth()+zone.GetLeft() ) &&
                rect.GetBottom()+rect.GetHeight() <= zone.GetBottom()+zone.GetHeight();

        bool intersect = center ||
                ( left && top ) ||
                ( left && bottom ) ||
                ( right && top ) ||
                ( right && bottom ) ||
                ( top && bottom ) ||
                ( left && right );
        return intersect;
    }
}


std::string toString( const PoDoFo::PdfRect& rect )
{
    size_t MaxLength = 5;
    QString desc = QString( "[ %1 %2 %3 %4 ]" )
            .arg( rect.GetLeft(), -MaxLength )
            .arg( rect.GetBottom(), -MaxLength )
            .arg( rect.GetWidth(), -MaxLength )
            .arg( rect.GetHeight(), -MaxLength );
    return std::string( desc.toLocal8Bit().constData() );
}
QRectF toQRectF( const PoDoFo::PdfRect& rect )
{
    return QRectF( rect.GetLeft(), rect.GetBottom(), rect.GetWidth(), rect.GetHeight() );
}

}

//}

