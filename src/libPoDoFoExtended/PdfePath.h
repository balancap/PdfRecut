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
#include "PdfeContentsStream.h"

#include "podofo/base/PdfRect.h"

namespace PoDoFoExtended {

//**********************************************************//
//                         PdfeSubPath                      //
//**********************************************************//
/** Class representing a PDF subpath. It is used as the basic
 * brick for a the construction of PDF paths. It should be
 * sufficiently smart to check the integrity of the subpath
 * construction process, but also allow some afterwards
 * modifications.
 */
class PdfeSubPath
{
public:
    /** Default constructor: create empty subpath with a
     * starting point.
     * \param startCoords Coordinates of the starting point,
     * corresponding to the operator "x y m". (0,0) by default.
     */
    PdfeSubPath( const PdfeVector& startCoords = PdfeVector() );
    /** Create a subpath from a rectangle.
     * \param rect Coordinates of the rectangle, corresponding
     * to the operator "x y w h re".
     */
    PdfeSubPath( const PoDoFo::PdfRect& rect );
    /** Initialize to an empty subpath with a given starting point.
     * \param startCoords Coordinates of the starting point,
     * corresponding to the operator "x y m". (0,0) by default.
     */
    void init( const PdfeVector& startCoords = PdfeVector() );
    /** Initialize the subpath with a rectangle.
     * \param rect Coordinates of the rectangle, corresponding
     * to the operator "x y w h re".
     */
    void init( const PoDoFo::PdfRect& rect );

public:
    /** Basic structure which represents a point inside a subpath,
     * i.e. its coordinates and the corresponding graphics operator.
     */
    struct Point
    {
        /// Coordinates of the point.
        PdfeVector  coordinates;
        /// Graphics operator.
        PdfeGraphicOperator  goperator;
        /// Simple constructor.
        Point( const PdfeVector& coords = PdfeVector(),
               const PdfeGraphicOperator& gop = PdfeGraphicOperator() );
    };

public:
    // Subpath construction...
    /** Move the starting point: erase completely the
     * subpath and set the starting coordinates.
     * Corresponds to operator "x y m".
     * \param coords Coordinates of the starting point.
     * \return Reference to the modified subpath.
     */
    PdfeSubPath& moveTo( const PdfeVector& coords );
    /** Append a straight line to the subpath, begin at the
     * current point, if the subpath is not already closed.
     * Corresponds to operator "x y l".
     * \param coords End coordinates of the straight line.
     * \return Reference to the modified subpath.
     */
    PdfeSubPath& appendLine( const PdfeVector& coords );
    /** Append a Bézier curve, with 2 controlling points.
     * Corresponds to operator "x1 y1 x2 y2 x3 y3 c".
     * \param coords1 Control point 1.
     * \param coords2 Control point 2.
     * \param coords3 End point of the Bézier curve.
     * \return Reference to the modified subpath.
     */
    PdfeSubPath& appendBezierC( const PdfeVector& coords1,
                                const PdfeVector& coords2,
                                const PdfeVector& coords3 );
    /** Append a Bézier curve, with 1 controlling point.
     * Corresponds to operator "x2 y2 x3 y3 v".
     * \param coords2 Control point 2.
     * \param coords3 End point of the Bézier curve.
     * \return Reference to the modified subpath.
     */
    PdfeSubPath& appendBezierV( const PdfeVector& coords2,
                                const PdfeVector& coords3 );
    /** Append a Bézier curve, with 1 controlling point.
     * Corresponds to operator "x1 y1 x3 y3 y".
     * \param coords1 Control point 1.
     * \param coords3 End point of the Bézier curve.
     * \return Reference to the modified subpath.
     */
    PdfeSubPath& appendBezierY( const PdfeVector& coords1,
                                const PdfeVector& coords3 );
    /** Close the subpath, with operator "h".
     * \return Reference to the modified subpath.
     */
    PdfeSubPath& close();

    /** Modify the coordinates of a point in the subpath.
     * Use it at yout own risk!
     * \param idx Index of the point.
     * \param coords New coordinates.
     */
    void setCoordinates( size_t idx, const PdfeVector& coords );

public:
    // Subpath getters...
    /// Is the subpath closed ?
    bool isClosed() const;
    /// Is subpath empty (i.e. with only a starting point)?
    bool isEmpty() const        {   return ( m_points.size() <= 1 );    }

    /// Number of points in the subpath.
    size_t nbPoints() const             {   return m_points.size();     }
    /// Current point in the subpath.
    const Point& currentPoint() const   {   return m_points.back();     }
    /// Get a subpath's point.
    const Point& point( size_t idx ) const                  {   return m_points.at( idx );      }
    /// Get a point coordinates.
    const PdfeVector& coordinates( size_t idx ) const       {   return m_points.at( idx ).coordinates;  }
    /// Get a point graphics operator.
    const PdfeGraphicOperator& goperator( size_t idx ) const{   return m_points.at( idx ).goperator;    }

public:
    // Transformation and observation on the subpath.
    /** Evaluate if the subpath path intersects a given zone.
     * \param zone Rectangle representing the zone.
     * \param strictInclusion Evaluate if the subpath is stricly inside the zone.
     */
    bool intersects( const PoDoFo::PdfRect& zone,
                     bool strictInclusion = false ) const;
    /** Modify the subpath such that it corresponds to the intersection
     * with a given zone.
     * \param zone Rectangle representing the zone to consider.
     * \return Reference to the modified subpath.
     */
    PdfeSubPath& intersection( const PoDoFo::PdfRect& zone );
    /** Map a transformation matrix to points of the subpath.
     * \param transMat Transformation matrix.
     * \return Reference to the modified subpath.
     */
    PdfeSubPath& map( const PdfeMatrix& transMat );

    /** Convert the subpath to a Qt Painter Path object.
     * \param closeSubpath Force (or not) to close the subpath.
     * \param fillingRule Filling rule (winding by default).
     * \return QPainterPath object.
     */
    QPainterPath toQPainterPath( bool closeSubpath = true,
                                 PdfeFillingRule::Enum fillingRule = PdfeFillingRule::Winding ) const;

private:
    /// Points which composed the subpath.
    std::vector<Point>  m_points;
};


//**********************************************************//
//                          PdfePath                        //
//**********************************************************//
/** Class representing a PDF path. A path is made of a
 * collection of independent subpaths (PdfeSubPath).
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
    // Path loading and saving from stream.
    /** Load a path from a node in a contents stream.
     * \param pnode Pointer to the node at which begins the path.
     * \return Pointer to the last node of path's definition
     */
    PdfeContentsStream::Node* load( PdfeContentsStream::Node* pnode );
    /** Save back the path into a contents stream.
     * \param pnode Pointer of the node where to insert the path definition.
     * \param eraseExisting Erase existing contents at this node.
     * \return Pointer to the last node of path's definition
     */
    PdfeContentsStream::Node* save( PdfeContentsStream::Node* pnode,
                                    bool eraseExisting ) const;

public:
    // Path construction...
    /** Append an entire path.
     * \param path Path to append.
     */
    PdfePath& appendPath( const PdfePath& path );
    /** Append a subpath.
     * \param subpath Subpath to append.
     */
    PdfePath& appendSubpath( const PdfeSubPath& subpath );

    /** Begin a new subpath by moving current point.
     * Corresponds to operator "x y m".
     * \param coords Coordinates of the starting point.
     */
    PdfePath& moveTo( const PdfeVector& coords );
    /** Append a straight line to the current subpath, begin at the
     * current point. Corresponds to operator "x y l".
     * \param coords Coordinates of the ending point of the straight line.
     */
    PdfePath& appendLine( const PdfeVector& coords );
    /** Append a Bézier curve, with 2 controlling points.
     * Corresponds to operator "x1 y1 x2 y2 x3 y3 c".
     * \param coords1 Control point 1.
     * \param coords2 Control point 2.
     * \param coords3 Ending point of the Bézier curve.
     */
    PdfePath& appendBezierC( const PdfeVector& coords1,
                             const PdfeVector& coords2,
                             const PdfeVector& coords3 );
    /** Append a Bézier curve, with 1 controlling point.
     * Corresponds to operator "x2 y2 x3 y3 v".
     * \param coords2 Control point 2.
     * \param coords3 End point of the Bézier curve.
     */
    PdfePath& appendBezierV( const PdfeVector& coords2,
                             const PdfeVector& coords3 );
    /** Append a Bézier curve, with 1 controlling point.
     * Corresponds to operator "x1 y1 x3 y3 y".
     * \param coords1 Control point 1.
     * \param coords3 End point of the Bézier curve.
     */
    PdfePath& appendBezierY( const PdfeVector& coords1,
                             const PdfeVector& coords3 );
    /** Append a rectangle to the path as a complete subpath.
     * Corresponds to operator "x y width height re".
     * \param rect Coordinates of the rectangle.
     */
    PdfePath& appendRectangle( const PoDoFo::PdfRect& rect );
    /** Close current subpath with a straight line.
     * Corresponds to operator "h".
     */
    PdfePath& closeSubpath();

public:
    /** Convert the path to a Qt Painter Path object.
     * \param closeSubpaths Force (or not) to close subpaths.
     * \param fillingRule Filling rule (winding by default).
     * \return QPainterPath object.
     */
    QPainterPath toQPainterPath( bool closeSubpaths = true,
                                 PdfeFillingRule::Enum fillingRule = PdfeFillingRule::Winding ) const;

public:
    // Clipping path operator.
    /// Is the path a clipping path.
    bool isClippingPath() const;
    /// Get clipping path operator (unknown if not a clipping path).
    const PdfeGraphicOperator& clippingPathOp() const;
    /// Set clipping path operator of the path.
    void setClippingPathOp( const PdfeGraphicOperator& gop );

public:
    // Getters and setters...
    /// Numbers of subpaths.
    size_t nbSubpaths() const   {   return m_subpaths.size();   }
    /// Reference to a given subpath.
    PdfeSubPath& subpath( size_t idx )              {   return m_subpaths.at( idx );    }
    const PdfeSubPath& subpath( size_t idx ) const  {   return m_subpaths.at( idx );    }

public:
    // DEPRECIATED...
    const std::vector<PdfeSubPath>& subpaths() const    {   return m_subpaths;  }

private:
    /** Get the current subpath. Append a new one if the last in the list
     * is closed.
     * \return Reference to the current subpath in the path.
     */
    PdfeSubPath& currentSubpath();

private:
    /// Subpaths vector.
    std::vector<PdfeSubPath>  m_subpaths;
    /// Clipping path operator (unknown if not a clipping path).
    PdfeGraphicOperator  m_clippingPathOp;
};

}

#endif // PDFEPATH_H
