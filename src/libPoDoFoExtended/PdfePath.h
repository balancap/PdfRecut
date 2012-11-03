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

#ifndef PDFEPATH_H
#define PDFEPATH_H

#include "PdfeTypes.h"
#include "PdfeGraphicsOperators.h"

#include "podofo/base/PdfRect.h"

namespace PoDoFoExtended {

namespace PdfePathOperators
{
/// Enum of operators used to define a PDF path.
enum Enum
{
    m = 0,  // move
    l,      // line
    c,      // Bézier c
    v,      // Bézier v
    y,      // Bézier y
    h,      // close
    re      // rectangle
};
}

/** Class representing a PDF subpath.
 */
class PdfeSubPath
{
public:
    /** Default constructor: create empty subpath.
     */
    PdfeSubPath();

    /** Initialize to an empty subpath.
     */
    void init();

    /** Append a point to the subpath.
     * \param coord Coordinates of the point to append.
     * \param op Construction operator corresponding.
     */
    void appendPoint( const PdfeVector& coord, PdfePathOperators::Enum op );

    /** Modify a point of the subpath.
     * \param coord Coordinates of the point to append.
     * \param op Construction operator corresponding.
     * \param idx Index of the point to modify.
     */
    void setPoint( size_t idx, const PdfeVector& coord, PdfePathOperators::Enum op );

public:
    // Transformation on subpath.
    /** Evaluate if the subpath path intersects a given zone.
     * \param zone Rectangle representing the zone.
     * \param strictInclusion Evaluate if the subpath is stricly inside the zone.
     */
    bool intersectZone( const PoDoFo::PdfRect& zone,
                        bool strictInclusion = false ) const;

    /** Evaluate if a rectangle path intersects a zone.
     * \param path Rectangle path.
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
    void applyTransMatrix( const PdfeMatrix& transMat );

public:
    // Getters.
    /// Is the subpath closed ?
    bool isClosed() const   {   return m_closed; }
    /// Is subpath empty ?
    bool isEmpty() const    {   return ( m_coordPoints.size() == 0 );  }

    /// Number of points in the subpath.
    size_t nbPoints() const {   return m_coordPoints.size();    }
    /// Get a point coordinates.
    const PdfeVector& pointCoord( size_t idx ) const    {   return m_coordPoints.at( idx );  }
    /// Get a point operator.
    PdfePathOperators::Enum pointOp( size_t idx ) const {   return m_opPoints.at( idx );  }

    /// Close a subpath.
    void close()    {   m_closed = true;    }

private:
    /// Coordinates of points which composed the subpath.
    std::vector<PdfeVector>  m_coordPoints;
    /// Vector which identifies the constructor operator used for each point in the path.
    std::vector<PdfePathOperators::Enum>  m_opPoints;

    /// Is the subpath closed.
    bool  m_closed;
};


/** Class representing a PDF path.
 */
class PdfePath
{
public:
    /** Constructor: create an empty path.
     */
    PdfePath();

    /** Initialize to an empty path.
     */
    void init();

public:
    //**********************************************************//
    //                      Path construction                   //
    //**********************************************************//
    /** Append an entire path.
     * \param path Path to append.
     */
    void appendPath( const PdfePath& path );

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

    /** Convert the path to a Qt Painter Path object.
     * \param closeSubpaths Force (or not) to close subpaths.
     * \param evenOddRule Use the even-odd rule for painting.
     * \return QPainterPath object.
     */
    QPainterPath toQPainterPath( bool closeSubpaths = true,
                                 bool evenOddRule = false ) const;


public:
    //**********************************************************//
    //                       Getters/Setters                    //
    //**********************************************************//
    /** Get a constant reference to subpaths that compose the PdfePath.
     * \return Constant reference to an std::vector of PdfeSubPath.
     */
    const std::vector<PdfeSubPath>& subpaths() const {
        return m_subpaths;
    }
    /** Set a subpaths of the PdfePath.
     * \param idx Index of the subpath to modify.
     * \param subpath New subpath.
     */
    void setSubPath( size_t idx, const PdfeSubPath& subpath ) {
        m_subpaths.at( idx ) = subpath;
    }

    /** Get clipping path operator.
     * \return Clipping path operator. Empty if it is not a clipping path.
     */
    std::string clippingPathOp() const;
    /** Is it a clipping path.
     * \return Boolean.
     */
    bool isClippingPath() const;
    /** Set clipping path operator of the path.
     * \param op Clipping path operator. Empty if it is not a clipping path.
     */
    void setClippingPathOp( const std::string& op );

public:
    /** Get the operator name.
     * \param op PDF path operator.
     * \return Name of the operator (string).
     */
    static const char* OperatorName( PdfePathOperators::Enum op );

private:
    /** Get the current subpath. Append a new one if the last in the list
     * is closed.
     * \return Reference to the current subpath in the path.
     */
    PdfeSubPath& currentSubPath();

private:
    /// Subpaths vector.
    std::vector<PdfeSubPath>  m_subpaths;

    /// Current point in the path.
    PdfeVector  m_currentPoint;

    /// Clipping path operator (empty if not a clipping path).
    std::string  m_clippingPathOp;
};

//**********************************************************//
//                      Inline methods                      //
//**********************************************************//
inline void PdfeSubPath::reduceToZone( PdfeVector& point, const PoDoFo::PdfRect& zone )
{
    point(0) = std::min( std::max( point(0), zone.GetLeft() ),
                         zone.GetLeft() + zone.GetWidth() );
    point(1) = std::min( std::max( point(1), zone.GetBottom() ),
                         zone.GetBottom() + zone.GetHeight() );
}
inline void PdfeSubPath::applyTransMatrix( const PdfeMatrix& transMat )
{
    // Modify points in the subpath.
    for( size_t j = 0 ; j < m_coordPoints.size() ; j++ ) {
        m_coordPoints[j] = m_coordPoints[j] * transMat;
    }
}

inline std::string PdfePath::clippingPathOp() const
{
    return m_clippingPathOp;
}
inline bool PdfePath::isClippingPath() const
{
    return m_clippingPathOp.length();
}
inline void PdfePath::setClippingPathOp( const std::string& op )
{
    m_clippingPathOp = op;
}

}

#endif // PDFEPATH_H
