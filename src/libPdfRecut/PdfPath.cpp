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

#include "PdfPath.h"
#include "podofo/podofo.h"

using namespace PoDoFo;

namespace PdfRecut {

//**********************************************************//
//                   PdfSubPath functions                   //
//**********************************************************//
bool PdfSubPath::intersectZone( const PoDoFo::PdfRect& zone,
                                bool strictInclusion )
{
    bool leftTop(false), left(false), leftBottom(false);
    bool centerTop(false), center(false), centerBottom(false);
    bool rightTop(false), right(false), rightBottom(false);
    bool strictCenter(true);

    // Loop on the points of the subpath.
    for( size_t i = 0 ; i < points.size() ; i++ )
    {
        // Only check center with strict inclusion
        if( strictInclusion )
        {
            strictCenter = strictCenter &&
                           points[i](0) >= zone.GetLeft() &&
                           points[i](0) <= zone.GetLeft()+zone.GetWidth() &&
                           points[i](1) >= zone.GetBottom() &&
                           points[i](1) <= zone.GetBottom()+zone.GetHeight();
        }
        else
        {
            // Where is the current point / zone.
            if( points[i](0) <= zone.GetLeft() &&
                    points[i](1) <= zone.GetBottom() )
                leftBottom = true;
            else if( points[i](0) <= zone.GetLeft() &&
                     points[i](1) <= zone.GetBottom()+zone.GetHeight() &&
                     points[i](1) >= zone.GetBottom() )
                left = true;
            else if( points[i](0) <= zone.GetLeft() &&
                     points[i](1) >= zone.GetBottom()+zone.GetHeight() )
                leftTop = true;
            else if( points[i](0) >= zone.GetLeft() &&
                     points[i](0) <= zone.GetLeft()+zone.GetWidth() &&
                     points[i](1) <= zone.GetBottom() )
                centerBottom = true;
            else if( points[i](0) >= zone.GetLeft() &&
                     points[i](0) <= zone.GetLeft()+zone.GetWidth() &&
                     points[i](1) >= zone.GetBottom()+zone.GetHeight() )
                centerTop = true;
            else if( points[i](0) >= zone.GetLeft()+zone.GetWidth() &&
                     points[i](1) <= zone.GetBottom() )
                rightBottom = true;
            else if( points[i](0) >= zone.GetLeft()+zone.GetWidth() &&
                     points[i](1) <= zone.GetBottom()+zone.GetHeight() &&
                     points[i](1) >= zone.GetBottom() )
                right = true;
            else if( points[i](0) >= zone.GetLeft()+zone.GetWidth() &&
                     points[i](1) >= zone.GetBottom()+zone.GetHeight() )
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
bool PdfSubPath::intersectZone( const PoDoFo::PdfRect& path,
                                const PoDoFo::PdfRect& zone,
                                bool strictInclusion )
{
    if( strictInclusion )
    {
        // Strict inclusion: all points in the zone.
        strictInclusion = ( path.GetLeft() >= zone.GetLeft() ) &&
                ( path.GetBottom() >= zone.GetBottom() ) &&
                ( path.GetLeft()+path.GetWidth() <= zone.GetLeft()+zone.GetWidth() ) &&
                ( path.GetBottom()+path.GetHeight() <= zone.GetBottom()+zone.GetHeight() );
        return strictInclusion;
    }
    else
    {
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

void PdfSubPath::reduceToZone( const PoDoFo::PdfRect& zone )
{
    // Modify points in the subpath.
    for( size_t j = 0 ; j < points.size() ; j++ )
    {
        // Distinction between different painting operators.
        if( opPoints[j] == "m" ) {
            PdfSubPath::reduceToZone( points[j], zone );
        }
        else if( opPoints[j] == "l" ) {
            PdfSubPath::reduceToZone( points[j], zone );
        }
        else if( opPoints[j] == "c" ) {
            PdfSubPath::reduceToZone( points[j], zone );
            PdfSubPath::reduceToZone( points[j+1], zone );
            PdfSubPath::reduceToZone( points[j+2], zone );
            j+=2;
        }
        else if( opPoints[j] == "v" ) {
            PdfSubPath::reduceToZone( points[j], zone );
            PdfSubPath::reduceToZone( points[j+1], zone );
            j+=1;
        }
        else if( opPoints[j] == "y" ) {
            PdfSubPath::reduceToZone( points[j], zone );
            PdfSubPath::reduceToZone( points[j+1], zone );
            j+=1;
        }
        else if( opPoints[j] == "h" ) {
            PdfSubPath::reduceToZone( points[j], zone );
        }
        else if( opPoints[j] == "re" ) {
            // Rectangle: modify points and transform "re" command by "m l l l h"
            // in case it is not a rectangle anymore.
            PdfSubPath::reduceToZone( points[j], zone );
            PdfSubPath::reduceToZone( points[j+1], zone );
            PdfSubPath::reduceToZone( points[j+2], zone );
            PdfSubPath::reduceToZone( points[j+3], zone );
            PdfSubPath::reduceToZone( points[j+4], zone );

            opPoints[j] = "m";
            opPoints[j+1] = "l";
            opPoints[j+2] = "l";
            opPoints[j+3] = "l";
            opPoints[j+4] = "h";

            j+=4;
        }
    }
}

//**********************************************************//
//                    PdfPath functions                     //
//**********************************************************//
PdfPath::PdfPath()
{
    this->init();
}
void PdfPath::init()
{
    m_subpaths.clear();
    m_currentPoint.init();
    m_clippingPathOp.clear();
}

void PdfPath::appendPath( const PdfPath& path )
{
    // Append subpaths and set current point.
    m_subpaths.insert( m_subpaths.end(), path.m_subpaths.begin(), path.m_subpaths.end() );
    m_currentPoint = path.m_currentPoint;
}

void PdfPath::beginSubpath( const PdfVector& point )
{
    // Get current subpath.
    PdfSubPath& curSubpath = this->getCurrentSubPath();

    if( curSubpath.points.size() <= 1 ) {
        // If is empty or with only one point: erase this subpath.
        curSubpath.init();
        curSubpath.appendPoint( point, "m" );
    }
    else {
        // Default behaviour: create a new subpath.
        m_subpaths.push_back( PdfSubPath() );
        m_subpaths.back().appendPoint( point, "m" );
    }
    m_currentPoint = point;
}

void PdfPath::appendLine( const PdfVector& point )
{
    // Get current subpath.
    PdfSubPath& curSubpath = this->getCurrentSubPath();

    // If is empty, add a first point which corresponds to current point.
    if( curSubpath.points.empty() ) {
        curSubpath.appendPoint( m_currentPoint, "m" );
    }

    // Add line end point and set current point.
    curSubpath.appendPoint( point, "l" );
    m_currentPoint = point;
}

void PdfPath::appendBezierC( const PdfVector& point1,
                             const PdfVector& point2,
                             const PdfVector& point3 )
{
    // Get current subpath.
    PdfSubPath& curSubpath = this->getCurrentSubPath();

    // If is empty, add a first point which corresponds to current point.
    if( curSubpath.points.empty() ) {
        curSubpath.appendPoint( m_currentPoint, "m" );
    }

    // Add  Bézier curve points and set current point.
    curSubpath.appendPoint( point1, "c" );
    curSubpath.appendPoint( point2, "c" );
    curSubpath.appendPoint( point3, "c" );
    m_currentPoint = point3;
}
void PdfPath::appendBezierV( const PdfVector& point2,
                             const PdfVector& point3 )
{
    // Get current subpath.
    PdfSubPath& curSubpath = this->getCurrentSubPath();

    // If is empty, add a first point which corresponds to current point.
    if( curSubpath.points.empty() ) {
        curSubpath.appendPoint( m_currentPoint, "m" );
    }

    // Add  Bézier curve points and set current point.
    curSubpath.appendPoint( point2, "v" );
    curSubpath.appendPoint( point3, "v" );
    m_currentPoint = point3;
}
void PdfPath::appendBezierY( const PdfVector& point1,
                             const PdfVector& point3 )
{
    // Get current subpath.
    PdfSubPath& curSubpath = this->getCurrentSubPath();

    // If is empty, add a first point which corresponds to current point.
    if( curSubpath.points.empty() ) {
        curSubpath.appendPoint( m_currentPoint, "m" );
    }

    // Add  Bézier curve points and set current point.
    curSubpath.appendPoint( point1, "y" );
    curSubpath.appendPoint( point3, "y" );
    m_currentPoint = point3;
}

void PdfPath::closeSubpath()
{
    // Get current subpath.
    PdfSubPath& curSubpath = this->getCurrentSubPath();

    // Close if subpath not empty.
    if( !curSubpath.points.empty() ) {
        curSubpath.appendPoint( curSubpath.points.front(), "h" );
        curSubpath.closed = true;
        m_currentPoint = curSubpath.points.back();
    }
}

void PdfPath::appendRectangle( const PdfVector& llPoint,
                               const PdfVector& size )
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

    PdfSubPath& subpath = m_subpaths.back();
    subpath.opPoints[0] = "re";
    subpath.appendPoint( PdfVector( llPoint(0)+size(0), llPoint(1) ), "re" );
    subpath.appendPoint( PdfVector( llPoint(0)+size(0), llPoint(1)+size(1) ), "re" );
    subpath.appendPoint( PdfVector( llPoint(0), llPoint(1)+size(1) ), "re" );
    subpath.appendPoint( PdfVector( llPoint(0), llPoint(1) ), "re" );

    // Close rectangle subpath and set current point.
    subpath.closed = true;
    m_currentPoint = llPoint;
}

PdfSubPath& PdfPath::getCurrentSubPath()
{
    // Vector of subpaths empty: create an empty one and returned it.
    if( m_subpaths.empty() )
    {
        m_subpaths.push_back( PdfSubPath() );
        return m_subpaths.back();
    }
    // Last subpath is closed: created an empty one and returned it.
    if( m_subpaths.back().closed ) {
        m_subpaths.push_back( PdfSubPath() );
        return m_subpaths.back();
    }

    // Default behaviour; return last one in the list.
    return m_subpaths.back();
}

QPainterPath PdfPath::toQPainterPath( bool closeSubpaths ) const
{
    // Qt painter path to create from Pdf path.
    QPainterPath qPath;

    // Add every subpath to the qt painter path.
    for( size_t i = 0 ; i < m_subpaths.size() ; ++i )
    {
        // Points from the subpath.
        for( size_t j = 0 ; j < m_subpaths[i].points.size() ; ++j )
        {
            const PdfVector& point = m_subpaths[i].points[j];
            const std::string& opPoint = m_subpaths[i].opPoints[j];

            // Distinction between different painting operators.
            if( opPoint == "m" ) {
                qPath.moveTo( point(0), point(1) );
            }
            else if( opPoint == "l" ) {
                qPath.lineTo( point(0), point(1) );
            }
            else if( opPoint == "c" ) {
                QPointF c1Pt( m_subpaths[i].points[j](0), m_subpaths[i].points[j](1) );
                QPointF c2Pt( m_subpaths[i].points[j+1](0), m_subpaths[i].points[j+1](1) );
                QPointF endPt( m_subpaths[i].points[j+2](0), m_subpaths[i].points[j+2](1) );

                qPath.cubicTo( c1Pt, c2Pt, endPt );
                j+=2;
            }
            else if( opPoint == "v" ) {
                QPointF c1Pt( qPath.currentPosition() );
                QPointF c2Pt( m_subpaths[i].points[j](0), m_subpaths[i].points[j](1) );
                QPointF endPt( m_subpaths[i].points[j+1](0), m_subpaths[i].points[j+1](1) );

                qPath.cubicTo( c1Pt, c2Pt, endPt );
                j+=1;
            }
            else if( opPoint == "y" ) {
                QPointF c1Pt( m_subpaths[i].points[j](0), m_subpaths[i].points[j](1) );
                QPointF c2Pt( m_subpaths[i].points[j+1](0), m_subpaths[i].points[j+1](1) );
                QPointF endPt( c2Pt );

                qPath.cubicTo( c1Pt, c2Pt, endPt );
                j+=1;
            }
            else if( opPoint == "h" ) {
                qPath.closeSubpath();
            }
            else if( opPoint == "re" ) {
                // Rectangle:  "re" corresponds to "m l l l h"
                const PdfVector& pointUR = m_subpaths[i].points[j+2];

                qPath.addRect( point(0), point(1), pointUR(0)-point(0), pointUR(1)-point(1) );
                j+=4;
            }
        }
        // Force the subpaths to be closed according, based on the painting operator.
        if( closeSubpaths ) {
            qPath.closeSubpath();
        }
    }
    return qPath;
}

}
