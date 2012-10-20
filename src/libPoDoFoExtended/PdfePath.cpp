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

#include "PdfePath.h"
#include "podofo/podofo.h"

using namespace PoDoFo;

namespace PoDoFoExtended {

//**********************************************************//
//                        PdfeSubPath                       //
//**********************************************************//
PdfeSubPath::PdfeSubPath()
{
    this->init();
}
void PdfeSubPath::init()
{
    m_coordPoints.clear();
    m_opPoints.clear();
    m_closed = false;
}

void PdfeSubPath::appendPoint( const PdfeVector& coord, PdfePathOperators::Enum op )
{
    m_coordPoints.push_back( coord );
    m_opPoints.push_back( op );
}

void PdfeSubPath::setPoint( size_t idx , const PdfeVector& coord, PdfePathOperators::Enum op )
{
    m_coordPoints.at( idx ) = coord;
    m_opPoints.at( idx ) = op;
}

bool PdfeSubPath::intersectZone( const PoDoFo::PdfRect& zone,
                                 bool strictInclusion ) const
{
    bool leftTop(false), left(false), leftBottom(false);
    bool centerTop(false), center(false), centerBottom(false);
    bool rightTop(false), right(false), rightBottom(false);
    bool strictCenter(true);

    // Loop on the points of the subpath.
    for( size_t i = 0 ; i < m_coordPoints.size() ; i++ )
    {
        // Only check center with strict inclusion
        if( strictInclusion ) {
            strictCenter = strictCenter &&
                           m_coordPoints[i](0) >= zone.GetLeft() &&
                           m_coordPoints[i](0) <= zone.GetLeft()+zone.GetWidth() &&
                           m_coordPoints[i](1) >= zone.GetBottom() &&
                           m_coordPoints[i](1) <= zone.GetBottom()+zone.GetHeight();
        }
        else {
            // Where is the current point / zone.
            if( m_coordPoints[i](0) <= zone.GetLeft() &&
                    m_coordPoints[i](1) <= zone.GetBottom() )
                leftBottom = true;
            else if( m_coordPoints[i](0) <= zone.GetLeft() &&
                     m_coordPoints[i](1) <= zone.GetBottom()+zone.GetHeight() &&
                     m_coordPoints[i](1) >= zone.GetBottom() )
                left = true;
            else if( m_coordPoints[i](0) <= zone.GetLeft() &&
                     m_coordPoints[i](1) >= zone.GetBottom()+zone.GetHeight() )
                leftTop = true;
            else if( m_coordPoints[i](0) >= zone.GetLeft() &&
                     m_coordPoints[i](0) <= zone.GetLeft()+zone.GetWidth() &&
                     m_coordPoints[i](1) <= zone.GetBottom() )
                centerBottom = true;
            else if( m_coordPoints[i](0) >= zone.GetLeft() &&
                     m_coordPoints[i](0) <= zone.GetLeft()+zone.GetWidth() &&
                     m_coordPoints[i](1) >= zone.GetBottom()+zone.GetHeight() )
                centerTop = true;
            else if( m_coordPoints[i](0) >= zone.GetLeft()+zone.GetWidth() &&
                     m_coordPoints[i](1) <= zone.GetBottom() )
                rightBottom = true;
            else if( m_coordPoints[i](0) >= zone.GetLeft()+zone.GetWidth() &&
                     m_coordPoints[i](1) <= zone.GetBottom()+zone.GetHeight() &&
                     m_coordPoints[i](1) >= zone.GetBottom() )
                right = true;
            else if( m_coordPoints[i](0) >= zone.GetLeft()+zone.GetWidth() &&
                     m_coordPoints[i](1) >= zone.GetBottom()+zone.GetHeight() )
                rightTop = true;
            else
                center = true;
        }
    }
    if(strictInclusion) {
        return strictCenter;
    }

    // Try to estimate if the path intersects the zone.
    bool intersect = center ||
            ( centerTop && ( leftBottom || centerBottom || rightBottom ) ) ||
            ( centerBottom && ( leftTop || centerTop || rightTop ) ) ||
            ( left && ( rightBottom || right || rightTop ) ) ||
            ( right && ( leftBottom || left || leftTop ) ) ||
            ( leftBottom && rightTop ) ||
            ( leftTop && rightBottom );

    return intersect;
}
bool PdfeSubPath::intersectZone( const PoDoFo::PdfRect& path,
                                 const PoDoFo::PdfRect& zone,
                                 bool strictInclusion )
{
    if( strictInclusion ) {
        // Strict inclusion: all points in the zone.
        strictInclusion = ( path.GetLeft() >= zone.GetLeft() ) &&
                ( path.GetBottom() >= zone.GetBottom() ) &&
                ( path.GetLeft()+path.GetWidth() <= zone.GetLeft()+zone.GetWidth() ) &&
                ( path.GetBottom()+path.GetHeight() <= zone.GetBottom()+zone.GetHeight() );
        return strictInclusion;
    }
    else {
        bool left(false), right(false), bottom(false), top(false), center(false);

        // Evaluate the intersection with the different part of the zone.
        center = ( path.GetLeft() >= zone.GetLeft() ) &&
                ( path.GetBottom() >= zone.GetBottom() ) &&
                ( path.GetLeft()+path.GetWidth() <= zone.GetLeft()+zone.GetWidth() ) &&
                ( path.GetBottom()+path.GetHeight() <= zone.GetBottom()+zone.GetHeight() );

        left = !( path.GetBottom()+path.GetHeight() <= zone.GetBottom() ||
                  path.GetBottom() >= zone.GetBottom()+zone.GetHeight() ) &&
                path.GetLeft() <= zone.GetLeft();

        right = !( path.GetBottom()+path.GetHeight() <= zone.GetBottom() ||
                   path.GetBottom() >= zone.GetBottom()+zone.GetHeight() ) &&
                path.GetLeft()+path.GetWidth() >= zone.GetLeft()+zone.GetWidth();

        bottom = !( path.GetLeft()+path.GetWidth() <= zone.GetLeft() ||
                  path.GetLeft() >= zone.GetWidth()+zone.GetLeft() ) &&
                path.GetBottom() <= zone.GetBottom();

        top = !( path.GetLeft()+path.GetWidth() <= zone.GetLeft() ||
                 path.GetLeft() >= zone.GetWidth()+zone.GetLeft() ) &&
                path.GetBottom()+path.GetHeight() <= zone.GetBottom()+zone.GetHeight();

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

void PdfeSubPath::reduceToZone( const PoDoFo::PdfRect& zone )
{
    // Modify points in the subpath.
    for( size_t j = 0 ; j < m_coordPoints.size() ; j++ )
    {
        // Distinction between different painting operators.
        if( m_opPoints[j] == PdfePathOperators::m ) {
            PdfeSubPath::reduceToZone( m_coordPoints[j], zone );
        }
        else if( m_opPoints[j] == PdfePathOperators::l ) {
            PdfeSubPath::reduceToZone( m_coordPoints[j], zone );
        }
        else if( m_opPoints[j] == PdfePathOperators::c ) {
            PdfeSubPath::reduceToZone( m_coordPoints[j], zone );
            PdfeSubPath::reduceToZone( m_coordPoints[j+1], zone );
            PdfeSubPath::reduceToZone( m_coordPoints[j+2], zone );
            j+=2;
        }
        else if( m_opPoints[j] == PdfePathOperators::v ) {
            PdfeSubPath::reduceToZone( m_coordPoints[j], zone );
            PdfeSubPath::reduceToZone( m_coordPoints[j+1], zone );
            j+=1;
        }
        else if( m_opPoints[j] == PdfePathOperators::y ) {
            PdfeSubPath::reduceToZone( m_coordPoints[j], zone );
            PdfeSubPath::reduceToZone( m_coordPoints[j+1], zone );
            j+=1;
        }
        else if( m_opPoints[j] == PdfePathOperators::h ) {
            PdfeSubPath::reduceToZone( m_coordPoints[j], zone );
        }
        else if( m_opPoints[j] == PdfePathOperators::re ) {
            // Rectangle: modify points and transform "re" command by "m l l l h"
            // in case it is not a rectangle anymore.
            PdfeSubPath::reduceToZone( m_coordPoints[j], zone );
            PdfeSubPath::reduceToZone( m_coordPoints[j+1], zone );
            PdfeSubPath::reduceToZone( m_coordPoints[j+2], zone );
            PdfeSubPath::reduceToZone( m_coordPoints[j+3], zone );
            PdfeSubPath::reduceToZone( m_coordPoints[j+4], zone );

            m_opPoints[j] = PdfePathOperators::m;
            m_opPoints[j+1] = PdfePathOperators::l;
            m_opPoints[j+2] = PdfePathOperators::l;
            m_opPoints[j+3] = PdfePathOperators::l;
            m_opPoints[j+4] = PdfePathOperators::h;

            j+=4;
        }
    }
}

//**********************************************************//
//                          PdfePath                        //
//**********************************************************//
PdfePath::PdfePath()
{
    this->init();
}
void PdfePath::init()
{
    m_subpaths.clear();
    m_currentPoint.init();
    m_clippingPathOp.clear();
}

void PdfePath::appendPath( const PdfePath& path )
{
    // Append subpaths and set current point.
    m_subpaths.insert( m_subpaths.end(), path.m_subpaths.begin(), path.m_subpaths.end() );
    m_currentPoint = path.m_currentPoint;
}

void PdfePath::beginSubpath( const PdfeVector& point )
{
    // Get current subpath.
    PdfeSubPath& curSubpath = this->currentSubPath();

    if( curSubpath.nbPoints() <= 1 ) {
        // If is empty or with only one point: erase this subpath.
        curSubpath.init();
        curSubpath.appendPoint( point, PdfePathOperators::m );
    }
    else {
        // Default behaviour: create a new subpath.
        m_subpaths.push_back( PdfeSubPath() );
        m_subpaths.back().appendPoint( point, PdfePathOperators::m );
    }
    m_currentPoint = point;
}

void PdfePath::appendLine( const PdfeVector& point )
{
    // Get current subpath.
    PdfeSubPath& curSubpath = this->currentSubPath();

    // If is empty, add a first point which corresponds to current point.
    if( curSubpath.isEmpty() ) {
        curSubpath.appendPoint( m_currentPoint, PdfePathOperators::m );
    }

    // Add line end point and set current point.
    curSubpath.appendPoint( point, PdfePathOperators::l );
    m_currentPoint = point;
}

void PdfePath::appendBezierC( const PdfeVector& point1,
                             const PdfeVector& point2,
                             const PdfeVector& point3 )
{
    // Get current subpath.
    PdfeSubPath& curSubpath = this->currentSubPath();

    // If is empty, add a first point which corresponds to current point.
    if( curSubpath.isEmpty() ) {
        curSubpath.appendPoint( m_currentPoint, PdfePathOperators::m );
    }

    // Add  Bézier curve points and set current point.
    curSubpath.appendPoint( point1, PdfePathOperators::c );
    curSubpath.appendPoint( point2, PdfePathOperators::c );
    curSubpath.appendPoint( point3, PdfePathOperators::c );
    m_currentPoint = point3;
}
void PdfePath::appendBezierV( const PdfeVector& point2,
                             const PdfeVector& point3 )
{
    // Get current subpath.
    PdfeSubPath& curSubpath = this->currentSubPath();

    // If is empty, add a first point which corresponds to current point.
    if( curSubpath.isEmpty() ) {
        curSubpath.appendPoint( m_currentPoint, PdfePathOperators::m );
    }

    // Add  Bézier curve points and set current point.
    curSubpath.appendPoint( point2, PdfePathOperators::v );
    curSubpath.appendPoint( point3, PdfePathOperators::v );
    m_currentPoint = point3;
}
void PdfePath::appendBezierY( const PdfeVector& point1,
                             const PdfeVector& point3 )
{
    // Get current subpath.
    PdfeSubPath& curSubpath = this->currentSubPath();

    // If is empty, add a first point which corresponds to current point.
    if( curSubpath.isEmpty() ) {
        curSubpath.appendPoint( m_currentPoint, PdfePathOperators::m );
    }

    // Add  Bézier curve points and set current point.
    curSubpath.appendPoint( point1, PdfePathOperators::y );
    curSubpath.appendPoint( point3, PdfePathOperators::y );
    m_currentPoint = point3;
}

void PdfePath::closeSubpath()
{
    // Get current subpath.
    PdfeSubPath& curSubpath = this->currentSubPath();

    // Close if subpath not empty.
    if( !curSubpath.isEmpty() ) {
        curSubpath.appendPoint( curSubpath.pointCoord( 0 ), PdfePathOperators::h );
        curSubpath.close();
        m_currentPoint = curSubpath.pointCoord( curSubpath.nbPoints()-1 );
    }
}

void PdfePath::appendRectangle( const PdfeVector& llPoint,
                               const PdfeVector& size )
{
    /* Known to be equivalent to
      x y m
      ( x + width ) y l
      ( x + width ) ( y + height ) l
      x ( y + height ) l
      h
    */

    // Begin subpath and add points.
    this->beginSubpath( llPoint );

    PdfeSubPath& subpath = m_subpaths.back();
    subpath.setPoint( 0, subpath.pointCoord( 0 ), PdfePathOperators::re );
    subpath.appendPoint( PdfeVector( llPoint(0)+size(0), llPoint(1) ), PdfePathOperators::re );
    subpath.appendPoint( PdfeVector( llPoint(0)+size(0), llPoint(1)+size(1) ), PdfePathOperators::re );
    subpath.appendPoint( PdfeVector( llPoint(0), llPoint(1)+size(1) ), PdfePathOperators::re );
    subpath.appendPoint( PdfeVector( llPoint(0), llPoint(1) ), PdfePathOperators::re );

    // Close rectangle subpath and set current point.
    subpath.close();
    m_currentPoint = llPoint;
}

PdfeSubPath& PdfePath::currentSubPath()
{
    // Vector of subpaths empty: create an empty one and returned it.
    if( m_subpaths.empty() )
    {
        m_subpaths.push_back( PdfeSubPath() );
        return m_subpaths.back();
    }
    // Last subpath is closed: created an empty one and returned it.
    if( m_subpaths.back().isClosed() ) {
        m_subpaths.push_back( PdfeSubPath() );
        return m_subpaths.back();
    }

    // Default behaviour; return last one in the list.
    return m_subpaths.back();
}

QPainterPath PdfePath::toQPainterPath( bool closeSubpaths, bool evenOddRule ) const
{
    // Qt painter path to create from Pdf path.
    QPainterPath qPath;

    // Add every subpath to the qt painter path.
    for( size_t i = 0 ; i < m_subpaths.size() ; ++i ) {
        // Points from the subpath.
        for( size_t j = 0 ; j < m_subpaths[i].nbPoints() ; ++j ) {
            const PdfeVector& point = m_subpaths[i].pointCoord( j );
            PdfePathOperators::Enum opPoint = m_subpaths[i].pointOp( j );

            // Distinction between different painting operators.
            if( opPoint == PdfePathOperators::m ) {
                qPath.moveTo( point(0), point(1) );
            }
            else if( opPoint == PdfePathOperators::l ) {
                qPath.lineTo( point(0), point(1) );
            }
            else if( opPoint == PdfePathOperators::c ) {
                QPointF c1Pt( m_subpaths[i].pointCoord(j)(0), m_subpaths[i].pointCoord(j)(1) );
                QPointF c2Pt( m_subpaths[i].pointCoord(j+1)(0), m_subpaths[i].pointCoord(j+1)(1) );
                QPointF endPt( m_subpaths[i].pointCoord(j+2)(0), m_subpaths[i].pointCoord(j+2)(1) );

                qPath.cubicTo( c1Pt, c2Pt, endPt );
                j+=2;
            }
            else if( opPoint == PdfePathOperators::v ) {
                QPointF c1Pt( qPath.currentPosition() );
                QPointF c2Pt( m_subpaths[i].pointCoord(j)(0), m_subpaths[i].pointCoord(j)(1) );
                QPointF endPt( m_subpaths[i].pointCoord(j+1)(0), m_subpaths[i].pointCoord(j+1)(1) );

                qPath.cubicTo( c1Pt, c2Pt, endPt );
                j+=1;
            }
            else if( opPoint == PdfePathOperators::y ) {
                QPointF c1Pt( m_subpaths[i].pointCoord(j)(0), m_subpaths[i].pointCoord(j)(1) );
                QPointF c2Pt( m_subpaths[i].pointCoord(j+1)(0), m_subpaths[i].pointCoord(j+1)(1) );
                QPointF endPt( c2Pt );

                qPath.cubicTo( c1Pt, c2Pt, endPt );
                j+=1;
            }
            else if( opPoint == PdfePathOperators::h ) {
                qPath.closeSubpath();
            }
            else if( opPoint == PdfePathOperators::re ) {
                // Rectangle:  "re" corresponds to "m l l l h"
                const PdfeVector& pointUR = m_subpaths[i].pointCoord(j+2);

                qPath.addRect( point(0), point(1), pointUR(0)-point(0), pointUR(1)-point(1) );
                j+=4;
            }
        }
        // Force the subpaths to be closed according, based on the painting operator.
        if( closeSubpaths ) {
            qPath.closeSubpath();
        }
    }
    // Filling rule.
    if( evenOddRule ) {
        qPath.setFillRule( Qt::OddEvenFill );
    }
    else {
        qPath.setFillRule( Qt::WindingFill );
    }
    return qPath;
}

const char *PdfePath::OperatorName( PdfePathOperators::Enum op )
{
    static const char* const operatorsName[7] = {
        "m", "l", "c", "v", "y", "h", "re"
    };
    return operatorsName[ op ];
}

}
