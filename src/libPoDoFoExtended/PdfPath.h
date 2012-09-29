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

#ifndef PDFPATH_H
#define PDFPATH_H

#include "PdfeTypes.h"
#include "PdfeGraphicsOperators.h"

#include "podofo/base/PdfRect.h"

namespace PdfRecut {

struct PdfSubPath
{
    /** Points which composed the path.
     */
    std::vector<PdfeVector> points;

    /** Vector which identifies the constructor operator
     * used for each point in the path.
     */
    std::vector<std::string> opPoints;

    /** Is the subpath closed.
     */
    bool closed;

    /** Default constructor: create empty subpath.
     */
    PdfSubPath() {
        this->init();
    }
    /** Initialize to an empty subpath.
     */
    void init() {
        points.clear();
        opPoints.clear();
        closed = false;
    }

    /** Append a point to the path.
     * \param point Point to append.
     * \param op Construction operator corresponding.
     */
    void appendPoint( const PdfeVector& point, const std::string& op ) {
        points.push_back( point );
        opPoints.push_back( op );
    }
    /** Is subpath empty ?
     */
    bool isEmpty() {
        return ( points.size() == 0 );
    }

    /** Evaluate if the subpath path intersects a given zone.
     * \param zone Rectangle representing the zone.
     * \param strictInclusion Evaluate if the subpath is stricly inside the zone.
     */
    bool intersectZone( const PoDoFo::PdfRect& zone,
                        bool strictInclusion = false );

    /** Evaluate if a rectangle path intersects a zone.
     * \params path Rectangle path.
     * \param zone Rectangle representing the zone.
     * \param strictInclusion Evaluate if the path is stricly inside the zone.
     */
    static bool intersectZone( const PoDoFo::PdfRect& path,
                               const PoDoFo::PdfRect& zone,
                               bool strictInclusion = false );

    /** Modify a point in order to be inside a given zone.
     * \param point Point which is modified.
     * \param zone Rectangle representing the zone to consider.
     */
    static void reduceToZone( PdfeVector& point,
                              const PoDoFo::PdfRect& zone ) ;

    /** Reduce the subpath in order to be inside a given zone.
     * \param zone Rectangle representing the zone to consider.
     */
    void reduceToZone( const PoDoFo::PdfRect& zone );

    /** Apply a transformation matrix to points of the subpath.
     * \param transMat Transformation matrix.
     */
    void applyTransfMatrix( const PdfeMatrix& transMat );
};

class PdfPath
{
public:
    /** Constructor: create an empty path.
     */
    PdfPath();

    /** Initialize to an empty path.
     */
    void init();

    /** Get subPaths vector.
     */
    std::vector<PdfSubPath>& getSubpaths() {
        return m_subpaths;
    }
    const std::vector<PdfSubPath>& getSubpaths() const {
        return m_subpaths;
    }

    //**** Path Construction ****//
    /** Append an entire path.
     * \param path Path to append.
     */
    void appendPath( const PdfPath& path );

    /** Begin a new subpath, by moving current point.
     * Corresponds to operator "x y m".
     * \param point Point where the current point.
     */
    void beginSubpath( const PdfeVector& point );

    /** Append a straight line to the current subpath, begin at the current point.
     * Corresponds to operator "x y l".
     * \param point End point of the straight line.
     */
    void appendLine( const PdfeVector& point );

    /** Append a Bézier curve, with 2 controlling points.
     * Corresponds to operator "x1 y1 x2 y2 x3 y3 c".
     * \param point1 Control point 1.
     * \param point2 Control point 2.
     * \param point3 End point of the Bézier curve.
     */
    void appendBezierC( const PdfeVector& point1,
                        const PdfeVector& point2,
                        const PdfeVector& point3 );

    /** Append a Bézier curve, with 1 controlling point.
     * Corresponds to operator "x2 y2 x3 y3 v".
     * \param point2 Control point 2.
     * \param point3 End point of the Bézier curve.
     */
    void appendBezierV( const PdfeVector& point2,
                        const PdfeVector& point3 );

    /** Append a Bézier curve, with 1 controlling point.
     * Corresponds to operator "x1 y1 x3 y3 y".
     * \param point1 Control point 1.
     * \param point3 End point of the Bézier curve.
     */
    void appendBezierY( const PdfeVector& point1,
                        const PdfeVector& point3 );

    /** Close current subpath with a straight line.
     * Corresponds to operator h.
     */
    void closeSubpath();

    /** Append a rectangle to the path as a complete subpath.
     * Corresponds to operator "x y width height re".
     * \param llPoint Left-low point of the rectangle.
     * \param size Width and height of the rectangle.
     */
    void appendRectangle( const PdfeVector& llPoint,
                          const PdfeVector& size );

    /** Get clipping path operator.
     * \return Clipping path operator. Empty if it is not a clipping path.
     */
    std::string getClippingPathOp() const;

    /** Set clipping path operator of the path.
     * \param op Clipping path operator. Empty if it is not a clipping path.
     */
    void setClippingPathOp( const std::string& op );

    /** Convert the path to a Qt Painter Path object.
     * \param closeSubpaths Force (or not) to close subpaths.
     * \param evenOddRule Use the even-odd rule for painting.
     * \return QPainterPath object.
     */
    QPainterPath toQPainterPath( bool closeSubpaths = true,
                                 bool evenOddRule = false ) const;

protected:
    /** Get current subpath. Append a new one if the last in the list
     * is closed.
     * \return Reference to the current subpath in the path.
     */
    PdfSubPath& getCurrentSubPath();

protected:
    /** Subpaths vector.
     */
    std::vector<PdfSubPath> m_subpaths;

    /** Current point in the path.
     */
    PdfeVector m_currentPoint;

    /** Is a clipping path ?
     */
    std::string m_clippingPathOp;
};

//**********************************************************//
//                      Inline methods                      //
//**********************************************************//
inline void PdfSubPath::reduceToZone( PdfeVector& point, const PoDoFo::PdfRect& zone )
{
    point(0) = std::min( std::max( point(0), zone.GetLeft() ),
                             zone.GetLeft() + zone.GetWidth() );
    point(1) = std::min( std::max( point(1), zone.GetBottom() ),
                             zone.GetBottom() + zone.GetHeight() );
}
inline void PdfSubPath::applyTransfMatrix( const PdfeMatrix& transMat )
{
    // Modify points in the subpath.
    for( size_t j = 0 ; j < points.size() ; j++ ) {
        points[j] = points[j] * transMat;
    }
}

inline std::string PdfPath::getClippingPathOp() const
{
    return m_clippingPathOp;
}
inline void PdfPath::setClippingPathOp( const std::string& op )
{
    m_clippingPathOp = op;
}

}

#endif // PDFPATH_H
