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

#include "PdfePath.h"
#include "podofo/podofo.h"

using namespace PoDoFo;

namespace PoDoFoExtended {

//**********************************************************//
//                        PdfeSubPath                       //
//**********************************************************//
PdfeSubPath::Point::Point( const PdfeVector& coords, const PdfeGraphicOperator& gop ) :
    coordinates( coords ),
    goperator( gop )
{
}

PdfeSubPath::PdfeSubPath( const PdfeVector& startCoords ) :
    m_nodeSubID( NodeSubIDUndefined() )
{
    this->init( startCoords );
}
PdfeSubPath::PdfeSubPath( const PdfRect& rect ) :
    m_nodeSubID( NodeSubIDUndefined() )
{
    this->setRectangle( rect );
}
void PdfeSubPath::init( const PdfeVector& startCoords )
{
    // Clear subpath and append starting point.
    m_nodeSubID = NodeSubIDUndefined();
    m_points.clear();
    m_points.push_back( Point( startCoords, PdfeGOperator::m ) );
}


/*PdfeContentsStream::Node* PdfeSubPath::load( PdfeContentsStream::Node* pnode,
                                             const PdfeVector& defStartCoords )
{
    // Initialize subpath with default starting coordinates.
    this->init( defStartCoords );

    // Read subpath, until an ending node is reached (m/h/re).
    PdfeContentsStream::Node* pNodePrev = pnode;
    while( pnode && pnode->category() == PdfeGCategory::PathConstruction ) {
        if( pnode->type() == PdfeGOperator::m ) {
            // Start point of the subpath.
            if( this->isEmpty() ) {
                PdfeVector point( pnode->operand<double>( 0 ),
                                  pnode->operand<double>( 1 ) );
                this->moveTo( point );
            }
            // Beginning of another subpath.
            else {
                return pNodePrev;
            }
        }
        else if( pnode->type() == PdfeGOperator::l ) {
            // Append straight line.
            PdfeVector point( pnode->operand<double>( 0 ),
                              pnode->operand<double>( 1 ) );
            this->appendLine( point );
        }
        else if( pnode->type() == PdfeGOperator::c ) {
            // Append Bézier curve (c).
            PdfeVector point1( pnode->operand<double>( 0 ),
                               pnode->operand<double>( 1 ) );
            PdfeVector point2( pnode->operand<double>( 2 ),
                               pnode->operand<double>( 3 ) );
            PdfeVector point3( pnode->operand<double>( 4 ),
                               pnode->operand<double>( 5 ) );
            this->appendBezierC( point1, point2, point3 );
        }
        else if( pnode->type() == PdfeGOperator::v ) {
            // Append Bézier curve (v).
            PdfeVector point2( pnode->operand<double>( 0 ),
                               pnode->operand<double>( 1 ) );
            PdfeVector point3( pnode->operand<double>( 2 ),
                               pnode->operand<double>( 3 ) );
            this->appendBezierV( point2, point3 );
        }
        else if( pnode->type() == PdfeGOperator::y ) {
            // Append Bézier curve (y).
            PdfeVector point1( pnode->operand<double>( 0 ),
                               pnode->operand<double>( 1 ) );
            PdfeVector point3( pnode->operand<double>( 2 ),
                               pnode->operand<double>( 3 ) );
            this->appendBezierY( point1, point3 );
        }
        else if( pnode->type() == PdfeGOperator::h ) {
            // Close the current subpath by appending a straight line.
            this->close();
            // Check there is no other "h" node after.
            while( pnode->next() && pnode->next()->type() == PdfeGOperator::h ) {
                pnode = pnode->next();
            }
            return pnode;
        }
        else if( pnode->type() == PdfeGOperator::re ) {
            // Append a rectangle to the current path as a complete subpath.
            PdfRect rect( pnode->operand<double>( 0 ),
                          pnode->operand<double>( 1 ),
                          pnode->operand<double>( 2 ),
                          pnode->operand<double>( 3 ) );
            this->init( rect );
            return pnode;
        }
        pNodePrev = pnode;
        pnode = pnode->next();
    }
    // End of path construction nodes.
    return pNodePrev;
}
PdfeContentsStream::Node* PdfeSubPath::save( PdfeContentsStream::Node* pnode,
                                             bool eraseExisting ) const
{
    return pnode;
}*/

PdfeSubPath& PdfeSubPath::moveTo( const PdfeVector& coords )
{
    // Clear subpath and append starting point.
    m_points.clear();
    m_points.push_back( Point( coords, PdfeGOperator::m ) );
    return *this;
}
PdfeSubPath& PdfeSubPath::appendLine( const PdfeVector& coords )
{
    if( !this->isClosed() ) {
        m_points.push_back( Point( coords, PdfeGOperator::l ) );
    }
    return *this;
}
PdfeSubPath& PdfeSubPath::appendBezierC( const PdfeVector& coords1,
                                         const PdfeVector& coords2,
                                         const PdfeVector& coords3 )
{
    if( !this->isClosed() ) {
        m_points.push_back( Point( coords1, PdfeGOperator::c ) );
        m_points.push_back( Point( coords2, PdfeGOperator::c ) );
        m_points.push_back( Point( coords3, PdfeGOperator::c ) );
    }
    return *this;
}
PdfeSubPath& PdfeSubPath::appendBezierV( const PdfeVector& coords2,
                                         const PdfeVector& coords3 )
{
    if( !this->isClosed() ) {
        m_points.push_back( Point( coords2, PdfeGOperator::v ) );
        m_points.push_back( Point( coords3, PdfeGOperator::v ) );
    }
    return *this;
}
PdfeSubPath& PdfeSubPath::appendBezierY( const PdfeVector& coords1,
                                         const PdfeVector& coords3 )
{
    if( !this->isClosed() ) {
        m_points.push_back( Point( coords1, PdfeGOperator::y ) );
        m_points.push_back( Point( coords3, PdfeGOperator::y ) );
    }
    return *this;
}
PdfeSubPath& PdfeSubPath::close()
{
    if( !this->isClosed() ) {
        m_points.push_back( Point( m_points.front().coordinates,
                                   PdfeGOperator::h ) );
    }
    return *this;
}
PdfeSubPath& PdfeSubPath::setRectangle( const PdfRect& rect )
{
    /* Known to be equivalent to
      x y m
      ( x + width ) y l
      ( x + width ) ( y + height ) l
      x ( y + height ) l
      h
    */
    m_points.clear();
    m_points.push_back( Point( PdfeVector( rect.GetLeft(), rect.GetBottom() ),
                               PdfeGOperator::re ) );
    m_points.push_back( Point( PdfeVector( rect.GetLeft()+rect.GetWidth(), rect.GetBottom() ),
                               PdfeGOperator::re ) );
    m_points.push_back( Point( PdfeVector( rect.GetLeft()+rect.GetWidth(), rect.GetBottom()+rect.GetHeight() ),
                               PdfeGOperator::re ) );
    m_points.push_back( Point( PdfeVector( rect.GetLeft(), rect.GetBottom()+rect.GetHeight() ),
                               PdfeGOperator::re ) );
    m_points.push_back( Point( PdfeVector( rect.GetLeft(), rect.GetBottom() ),
                               PdfeGOperator::re ) );
    return *this;
}

PdfRect PdfeSubPath::bbox() const
{
    if( this->isEmpty() ) {
        return PdfeRect::infinite();
    }
    double left( std::numeric_limits<double>::max() );
    double right( -std::numeric_limits<double>::max() );
    double bottom( std::numeric_limits<double>::max() );
    double top( -std::numeric_limits<double>::max() );
    for( size_t i = 0 ; i < m_points.size() ; ++i ) {
        left = std::min( left, m_points[i].coordinates(0) );
        right = std::max( right, m_points[i].coordinates(0) );
        bottom = std::min( bottom, m_points[i].coordinates(1) );
        top = std::max( top, m_points[i].coordinates(1) );
    }
    return PdfRect( left, bottom,
                    std::max( right-left, 0.0 ), std::max( top-bottom, 0.0 ) );
}

void PdfeSubPath::setCoordinates( size_t idx, const PdfeVector& coords )
{
    m_points.at( idx ).coordinates = coords;
}

bool PdfeSubPath::isClosed() const
{
    return  ( m_points.back().goperator.type() == PdfeGOperator::h ) ||
            ( m_points.back().goperator.type() == PdfeGOperator::re );
}

bool PdfeSubPath::intersects( const PoDoFo::PdfRect& zone,
                              bool strictInclusion ) const
{
    bool leftTop(false), left(false), leftBottom(false);
    bool centerTop(false), center(false), centerBottom(false);
    bool rightTop(false), right(false), rightBottom(false);
    bool strictCenter(true);

    // Loop on the points of the subpath.
    for( size_t i = 0 ; i < m_points.size() ; i++ ) {
        const PdfeVector& coords = m_points[i].coordinates;
        // Only check center with strict inclusion
        if( strictInclusion ) {
            strictCenter = strictCenter &&
                    coords(0) >= zone.GetLeft() &&
                    coords(0) <= zone.GetLeft()+zone.GetWidth() &&
                    coords(1) >= zone.GetBottom() &&
                    coords(1) <= zone.GetBottom()+zone.GetHeight();
        }
        else {
            // Where is the current point / zone.
            if( coords(0) <= zone.GetLeft() &&
                    coords(1) <= zone.GetBottom() )
                leftBottom = true;
            else if( coords(0) <= zone.GetLeft() &&
                     coords(1) <= zone.GetBottom()+zone.GetHeight() &&
                     coords(1) >= zone.GetBottom() )
                left = true;
            else if( coords(0) <= zone.GetLeft() &&
                     coords(1) >= zone.GetBottom()+zone.GetHeight() )
                leftTop = true;
            else if( coords(0) >= zone.GetLeft() &&
                     coords(0) <= zone.GetLeft()+zone.GetWidth() &&
                     coords(1) <= zone.GetBottom() )
                centerBottom = true;
            else if( coords(0) >= zone.GetLeft() &&
                     coords(0) <= zone.GetLeft()+zone.GetWidth() &&
                     coords(1) >= zone.GetBottom()+zone.GetHeight() )
                centerTop = true;
            else if( coords(0) >= zone.GetLeft()+zone.GetWidth() &&
                     coords(1) <= zone.GetBottom() )
                rightBottom = true;
            else if( coords(0) >= zone.GetLeft()+zone.GetWidth() &&
                     coords(1) <= zone.GetBottom()+zone.GetHeight() &&
                     coords(1) >= zone.GetBottom() )
                right = true;
            else if( coords(0) >= zone.GetLeft()+zone.GetWidth() &&
                     coords(1) >= zone.GetBottom()+zone.GetHeight() )
                rightTop = true;
            else
                center = true;
        }
    }
    if( strictInclusion ) {
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
PdfeSubPath& PdfeSubPath::intersection( const PoDoFo::PdfRect& zone )
{
    // Modify points in the subpath.
    for( size_t j = 0 ; j < m_points.size() ; j++ ) {
        PdfeGOperator::Enum goperator = m_points[j].goperator.type();
        // Distinction between different painting operators.
        if( goperator == PdfeGOperator::m ) {
            m_points[j].coordinates.intersection( zone );
        }
        else if( goperator == PdfeGOperator::l ) {
            m_points[j].coordinates.intersection( zone );
        }
        else if( goperator == PdfeGOperator::c ) {
            m_points[j].coordinates.intersection( zone );
            m_points[j+1].coordinates.intersection( zone );
            m_points[j+2].coordinates.intersection( zone );
            j+=2;
        }
        else if( goperator == PdfeGOperator::v ) {
            m_points[j].coordinates.intersection( zone );
            m_points[j+1].coordinates.intersection( zone );
            j+=1;
        }
        else if( goperator == PdfeGOperator::y ) {
            m_points[j].coordinates.intersection( zone );
            m_points[j+1].coordinates.intersection( zone );
            j+=1;
        }
        else if( goperator == PdfeGOperator::h ) {
            m_points[j+1].coordinates.intersection( zone );
        }
        else if( goperator == PdfeGOperator::re ) {
            // Rectangle: modify points and replace "re" command by "m l l l h"
            // in case it is not a rectangle anymore.
            m_points[j].coordinates.intersection( zone );
            m_points[j+1].coordinates.intersection( zone );
            m_points[j+2].coordinates.intersection( zone );
            m_points[j+3].coordinates.intersection( zone );
            m_points[j+4].coordinates.intersection( zone );

            m_points[j].goperator = PdfeGOperator::m;
            m_points[j+1].goperator = PdfeGOperator::l;
            m_points[j+2].goperator = PdfeGOperator::l;
            m_points[j+3].goperator = PdfeGOperator::l;
            m_points[j+4].goperator = PdfeGOperator::h;

            j+=4;
        }
    }
    return *this;
}
PdfeSubPath& PdfeSubPath::map( const PdfeMatrix& transMat )
{
    // Modify points' coordinates in the subpath.
    for( size_t j = 0 ; j < m_points.size() ; j++ ) {
        PdfeGOperator::Enum goperator = m_points[j].goperator.type();
        if( goperator == PdfeGOperator::re ) {
            // Case of the rectangle modify points and replace "re" command by "m l l l h".
            m_points[j].coordinates = m_points[j].coordinates * transMat;
            m_points[j+1].coordinates = m_points[j+1].coordinates * transMat;
            m_points[j+2].coordinates = m_points[j+2].coordinates * transMat;
            m_points[j+3].coordinates = m_points[j+3].coordinates * transMat;
            m_points[j+4].coordinates = m_points[j+4].coordinates * transMat;

            m_points[j].goperator = PdfeGOperator::m;
            m_points[j+1].goperator = PdfeGOperator::l;
            m_points[j+2].goperator = PdfeGOperator::l;
            m_points[j+3].goperator = PdfeGOperator::l;
            m_points[j+4].goperator = PdfeGOperator::h;

            j+=4;
        }
        else {
            m_points[j].coordinates = m_points[j].coordinates * transMat;
        }
    }
    return *this;
}
QPainterPath PdfeSubPath::toQPainterPath( bool closeSubpath, PdfeFillingRule::Enum fillingRule ) const
{
    if( this->isEmpty() ) {
        return QPainterPath();
    }
    // Qt painter path to create from PDF subpath.
    QPainterPath qPath;
    // Add points from the subpath.
    for( size_t j = 0 ; j < m_points.size() ; ++j ) {
        PdfeGOperator::Enum goperator = m_points[j].goperator.type();

        // Distinction between different painting operators.
        if( goperator == PdfeGOperator::m ) {
            qPath.moveTo( m_points[j].coordinates.toQPoint() );
        }
        else if( goperator == PdfeGOperator::l ) {
            qPath.lineTo( m_points[j].coordinates.toQPoint() );
        }
        else if( goperator == PdfeGOperator::c ) {
            qPath.cubicTo( m_points[j].coordinates.toQPoint(),
                           m_points[j+1].coordinates.toQPoint(),
                           m_points[j+2].coordinates.toQPoint() );
            j+=2;
        }
        else if( goperator == PdfeGOperator::v ) {
            qPath.cubicTo( qPath.currentPosition(),
                           m_points[j].coordinates.toQPoint(),
                           m_points[j+1].coordinates.toQPoint() );
            j+=1;
        }
        else if( goperator == PdfeGOperator::y ) {
            qPath.cubicTo( m_points[j].coordinates.toQPoint(),
                           m_points[j+1].coordinates.toQPoint(),
                           m_points[j+1].coordinates.toQPoint() );
            j+=1;
        }
        else if( goperator == PdfeGOperator::h ) {
            qPath.closeSubpath();
        }
        else if( goperator == PdfeGOperator::re ) {
            // Rectangle:  "re" corresponds to "m l l l h"
            const PdfeVector& coordsLB = m_points[j].coordinates;
            const PdfeVector& coordsUR = m_points[j+2].coordinates;
            qPath.addRect( coordsLB(0), coordsLB(1), coordsUR(0)-coordsLB(0), coordsUR(1)-coordsLB(1) );
            j+=4;
        }
    }
    // Force the subpaths to be closed according, based on the painting operator.
    if( closeSubpath ) {
        qPath.closeSubpath();
    }
    // Filling rule.
    if( fillingRule == PdfeFillingRule::EvenOdd ) {
        qPath.setFillRule( Qt::OddEvenFill );
    }
    else {
        qPath.setFillRule( Qt::WindingFill );
    }
    return qPath;
}

//**********************************************************//
//                          PdfePath                        //
//**********************************************************//
PdfePath::PdfePath() :
    PdfeGElement(),
    m_clippingPathOp( PdfeGOperator::Unknown ),
    m_paintingOp( PdfeGOperator::n )
{
}
void PdfePath::init( bool initGElement )
{
    if( initGElement ) {
        this->PdfeGElement::init();
    }
    // Clear subpaths and add an empty one.
    m_subpaths.clear();
    m_subpaths.push_back( PdfeSubPath() );
    m_clippingPathOp.set( PdfeGOperator::Unknown );
    m_paintingOp.set( PdfeGOperator::n );
    this->updateSubpathsID();
}

void PdfePath::setGState( const PdfeGraphicsState& gstate )
{
    this->PdfeGElement::setGState( gstate );
    this->clearTextGState();
}

PdfeContentsStream::Node* PdfePath::load( PdfeContentsStream::Node* pnode ,
                                          const PdfeGraphicsState& gstate )
{
    if( pnode->category() != PdfeGCategory::PathConstruction ) {
        PODOFO_RAISE_ERROR_INFO( ePdfError_InvalidDataType,
                                 "<PdfePath> Can not load path from node. Invalid type (not a path construction node)." );
    }
    this->init( false );
    // Set node ID and graphics state.
    this->setNodeID( pnode->id() );
    this->setGState( gstate );

    PdfeContentsStream::Node* pNodePrev = pnode;
    // Path construction nodes.
    while( pnode->category() == PdfeGCategory::PathConstruction ) {
        if( pnode->type() == PdfeGOperator::m ) {
            // Begin a new subpath.
            PdfeVector point( pnode->operand<double>( 0 ),
                              pnode->operand<double>( 1 ) );
            this->moveTo( point );
        }
        else if( pnode->type() == PdfeGOperator::l ) {
            // Append straight line.
            PdfeVector point( pnode->operand<double>( 0 ),
                              pnode->operand<double>( 1 ) );
            this->appendLine( point );
        }
        else if( pnode->type() == PdfeGOperator::c ) {
            // Append Bézier curve (c).
            PdfeVector point1( pnode->operand<double>( 0 ),
                               pnode->operand<double>( 1 ) );
            PdfeVector point2( pnode->operand<double>( 2 ),
                               pnode->operand<double>( 3 ) );
            PdfeVector point3( pnode->operand<double>( 4 ),
                               pnode->operand<double>( 5 ) );
            this->appendBezierC( point1, point2, point3 );
        }
        else if( pnode->type() == PdfeGOperator::v ) {
            // Append Bézier curve (c).
            PdfeVector point2( pnode->operand<double>( 0 ),
                               pnode->operand<double>( 1 ) );
            PdfeVector point3( pnode->operand<double>( 2 ),
                               pnode->operand<double>( 3 ) );
            this->appendBezierV( point2, point3 );
        }
        else if( pnode->type() == PdfeGOperator::y ) {
            // Append Bézier curve (c).
            PdfeVector point1( pnode->operand<double>( 0 ),
                               pnode->operand<double>( 1 ) );
            PdfeVector point3( pnode->operand<double>( 2 ),
                               pnode->operand<double>( 3 ) );
            this->appendBezierY( point1, point3 );
        }
        else if( pnode->type() == PdfeGOperator::h ) {
            // Close the current subpath by appending a straight line.
            this->closeSubpath();
        }
        else if( pnode->type() == PdfeGOperator::re ) {
            // Append a rectangle to the current path as a complete subpath.
            PdfRect rect( pnode->operand<double>( 0 ),
                          pnode->operand<double>( 1 ),
                          pnode->operand<double>( 2 ),
                          pnode->operand<double>( 3 ) );
            this->appendRectangle( rect );
        }
        pNodePrev = pnode;
        pnode = pnode->next();
    }
    return pNodePrev;
}
PdfeContentsStream::Node* PdfePath::save( PdfeContentsStream::Node* pnode,
                                          PdfeGElementSave::Enum savePolicy ) const
{
    // TODO: implement...

    PODOFO_RAISE_ERROR( ePdfError_NotImplemented )
    return pnode;
}

PdfePath& PdfePath::appendPath( const PdfePath& path )
{
    // Append subpaths.
    m_subpaths.insert( m_subpaths.end(), path.m_subpaths.begin(), path.m_subpaths.end() );
    this->updateSubpathsID();
    return *this;
}
PdfePath& PdfePath::appendSubpath( const PdfeSubPath& subpath )
{
    // Append subpath.
    m_subpaths.push_back( subpath );
    this->updateSubpathsID();
    return *this;
}
PdfePath& PdfePath::eraseSubpath( size_t idx )
{
    m_subpaths.erase( m_subpaths.begin() + idx );
    this->updateSubpathsID();
    return *this;
}
PdfePath& PdfePath::clearSubpaths()
{
    m_subpaths.clear();
    return *this;
}

PdfePath& PdfePath::moveTo( const PdfeVector& coords )
{
    // Add new subpath if necessary and move to.
    PdfeSubPath& subpath = this->createSubpath();
    subpath.moveTo( coords );
    return *this;
}
PdfePath& PdfePath::appendLine( const PdfeVector& coords )
{
    // Append line to current subpath.
    PdfeSubPath& subpath = this->currentSubpath();
    subpath.appendLine( coords );
    return *this;
}
PdfePath& PdfePath::appendBezierC( const PdfeVector& coords1,
                                   const PdfeVector& coords2,
                                   const PdfeVector& coords3 )
{
    // Append Bezier curve to current subpath.
    PdfeSubPath& subpath = this->currentSubpath();
    subpath.appendBezierC( coords1, coords2, coords3 );
    return *this;
}
PdfePath& PdfePath::appendBezierV( const PdfeVector& coords2,
                                   const PdfeVector& coords3 )
{
    // Append Bezier curve to current subpath.
    PdfeSubPath& subpath = this->currentSubpath();
    subpath.appendBezierV( coords2, coords3 );
    return *this;
}
PdfePath& PdfePath::appendBezierY( const PdfeVector& coords1,
                                   const PdfeVector& coords3 )
{
    // Append Bezier curve to current subpath.
    PdfeSubPath& subpath = this->currentSubpath();
    subpath.appendBezierY( coords1, coords3 );
    return *this;
}
PdfePath& PdfePath::appendRectangle( const PdfRect& rect )
{
    // Add new subpath if necessary and set rectangle.
    PdfeSubPath& subpath = this->createSubpath();
    subpath.setRectangle( rect );
    return *this;
}
PdfePath& PdfePath::closeSubpath()
{
    // Close last subpath in the list.
    if( m_subpaths.size() ) {
        m_subpaths.back().close();
    }
    return *this;
}

QPainterPath PdfePath::toQPainterPath( bool closeSubpaths, PdfeFillingRule::Enum fillingRule ) const
{
    // Qt painter path to create from PDF path.
    QPainterPath qPath;
    // Append subpaths.
    for( size_t i = 0 ; i < m_subpaths.size() ; ++i ) {
        qPath.addPath( m_subpaths[i].toQPainterPath( closeSubpaths, fillingRule ) );
    }
    // Filling rule.
    if( fillingRule == PdfeFillingRule::EvenOdd ) {
        qPath.setFillRule( Qt::OddEvenFill );
    }
    else {
        qPath.setFillRule( Qt::WindingFill );
    }
    return qPath;
}
PdfRect PdfePath::bbox() const
{
    PdfRect bbox( PdfeRect::infinite() );
    for( size_t i = 0 ; i < m_subpaths.size() ; ++i ) {
        bbox = PdfeRect::intersection( bbox, m_subpaths[i].bbox() );
    }
    return bbox;
}

bool PdfePath::isClippingPath() const
{
    return ( m_clippingPathOp.category() == PdfeGCategory::ClippingPath );
}
const PdfeGraphicOperator& PdfePath::clippingPathOp() const
{
    return m_clippingPathOp;
}
void PdfePath::setClippingPathOp( const PdfeGraphicOperator& gop )
{
    if( gop.category() == PdfeGCategory::ClippingPath ) {
        m_clippingPathOp = gop;
    }
    else {
        m_clippingPathOp.set( PdfeGOperator::Unknown );
    }
}

const PdfeGraphicOperator& PdfePath::paintingOp() const
{
    return m_paintingOp;
}
void PdfePath::setPaintingOp( const PdfeGraphicOperator& gop )
{
    if( gop.category() == PdfeGCategory::PathPainting ) {
        m_paintingOp = gop;
    }
    /*else {
        m_paintingOp.set( PdfeGOperator::n );
    }*/
}

PdfeSubPath& PdfePath::createSubpath()
{
    if( !m_subpaths.size() || !m_subpaths.back().isEmpty() ) {
        m_subpaths.push_back( PdfeSubPath() );
        this->updateSubpathsID();
    }
    return m_subpaths.back();
}
PdfeSubPath& PdfePath::currentSubpath()
{
    // Vector of subpaths empty: create an empty one and returned it.
    if( m_subpaths.empty() ) {
        m_subpaths.push_back( PdfeSubPath() );
        this->updateSubpathsID();
        return m_subpaths.back();
    }
    // Last subpath is closed: created an empty one and returned it.
    if( m_subpaths.back().isClosed() ) {
        // Guess the starting point from the previous subpath.
        PdfeVector coords;
        if( m_subpaths.size() ) {
            coords = m_subpaths.back().currentPoint().coordinates;
        }
        m_subpaths.push_back( PdfeSubPath( coords ) );
        this->updateSubpathsID();
        return m_subpaths.back();
    }
    // Default behaviour; return last one in the list.
    return m_subpaths.back();
}
void PdfePath::updateSubpathsID()
{
    pdfe_nodesubid subpathid( 0 );
    for( size_t i = 0 ; i < m_subpaths.size() ; ++i ) {
        m_subpaths[i].setNodeSubID( subpathid );
        ++subpathid;
    }
}


}
