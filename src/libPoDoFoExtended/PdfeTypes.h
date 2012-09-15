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

#ifndef PDFTYPES_H
#define PDFTYPES_H

//#ifndef NDEBUG
//#define NDEBUG
//#include "vmmlib/vmmlib.hpp"
//#undef NDEBUG
//#else
#include "vmmlib/vmmlib.hpp"
//#endif

#include "podofo/base/PdfRect.h"

#include <QPointF>
#include <QTransform>

#include <vector>
#include <ostream>

//namespace PoDoFoExtended {

class PdfeVector;
class PdfeORect;

/** Transformation matrix used in graphics state.
 */
class PdfeMatrix : public vmml::mat3d
{
public:
    /** Default constructor, matrix initialized to unit.
     */
    PdfeMatrix() {
        this->init();
    }
    /** Copy constructor for vmml::mat3d.
     */
    PdfeMatrix( const vmml::mat3d& source  ) :
        vmml::mat3d( source )
    {
    }

    /** =Operator overload
     */
    const PdfeMatrix& operator=( const vmml::mat3d& source ) {
        *( (vmml::mat3d*) this ) = source;
        return *this;
    }
    /** Initialize matrix to unit
     */
    void init() {
        this->fill( 0 );
        this->at(0,0) = this->at(1,1) = this->at(2,2) = 1;
    }

    /** Compute inverse matrix
     * \param invMat Reference where is stored the inverse matrix.
     * \return Is the matrix inversible?
     */
    bool inverse( PdfeMatrix& invMat ) const {
        return vmml::compute_inverse( *this, invMat );
    }
    /** Compute inverse matrix
     * \return Inverse matrix. Null matrix if not inversible.
     */
    PdfeMatrix inverse() const {
        PdfeMatrix invMat;
        bool exist = vmml::compute_inverse( *this, invMat );
        if( !exist ) {
            invMat.fill( 0 );
        }
        return invMat;
    }

    /** Convert to a QTransform object.
     */
    QTransform toQTransform() const {
        return QTransform( at(0,0), at(0,1), at(1,0), at(1,1), at(2,0), at(2,1) );
    }

    /** Map a vector in the new coordinate system defined by the matrix.
     */
    PdfeVector map( const PdfeVector& vect ) const;
    /** Map a rectangle in the new coordinate system defined by the matrix.
     */
    PdfeORect map( const PdfeORect& rect ) const;
};

/** Position vector 1x3 representing a position in a page,
 * in a given coordinate system.
 */
class PdfeVector : public vmml::matrix<1,3,double>
{
public:
    /** Default constructor, vector initialize to (0,0,1)
     */
    PdfeVector() {
        this->init();
    }
    /** Construction of a vector (x,y).
     * \param x X coordinate.
     * \param y Y coordinate.
     */
    PdfeVector( double x, double y ) {
        this->init( x, y );
    }

    /** Operator = overloaded
     */
    const PdfeVector& operator=( const vmml::matrix<1,3,double>& source ) {
        *( (vmml::matrix<1,3,double>*) this ) = source;
        return *this;
    }
    /** Operator + overloaded
     */
    PdfeVector operator+( const PdfeVector& vect ) const {
        PdfeVector rvect;
        rvect.at(0,0) = vect.at(0,0) + this->at(0,0);
        rvect.at(0,1) = vect.at(0,1) + this->at(0,1);
        rvect.at(0,2) = 1.0;
        return rvect;
    }
    /** *Operator overloaded
     */
    PdfeVector operator*( const PdfeMatrix& mat ) const {
        PdfeVector rvect;
        rvect.at(0,0) = mat(0,0) * this->at(0,0) + mat(1,0) * this->at(0,1) + mat(2,0);
        rvect.at(0,1) = mat(0,1) * this->at(0,0) + mat(1,1) * this->at(0,1) + mat(2,1);
        rvect.at(0,2) = 1.0;
        return rvect;
    }
    /** Operator * overloaded
     */
    PdfeVector operator*( double coef ) const {
        PdfeVector rvect;
        rvect.at(0,0) = this->at(0,0) * coef;
        rvect.at(0,1) = this->at(0,1) * coef;
        rvect.at(0,2) = 1.0;

        return rvect;
    }
    /** Operator (idx) overloaded.
     */
    double& operator()( size_t index ) {
        return this->at(0,index);
    }
    /** const Operator (idx) overloaded.
     */
    const double& operator()( size_t index ) const {
        return this->at(0,index);
    }
    /** Operator << overloaded.
     */
    friend std::ostream& operator<< ( std::ostream& out, PdfeVector& vect )
    {
        out << "(" << vect(0) << "," << vect(1) << ")";
        return out;
    }

    /** Rotate a vector of 90°.
     * \return Copy of the vector rotated.
     */
    PdfeVector rotate90() const {
        PdfeVector rvect;
        rvect.at(0,0) = -this->at(0,1);
        rvect.at(0,1) = this->at(0,0);
        rvect.at(0,2) = 1.0;
        return rvect;
    }
    /** Initialize vector to (0,0).
     */
    void init() {
        this->at(0,0) = this->at(0,1) = 0;
        this->at(0,2) = 1;
    }
    /** Initialize vector to (x,y).
     */
    void init( double x, double y ) {
        this->at(0,0) = x;
        this->at(0,1) = y;
        this->at(0,2) = 1;
    }

    /** Convert to a QPointF object.
     * \return Corresponding QPointF.
     */
    QPointF toQPoint() const {
        return QPointF( this->at(0,0), this->at(0,1) );
    }

    /** L^2 norm of the vector.
     */
    double norm2() const {
        return sqrt( this->at(0,0)*this->at(0,0) + this->at(0,1)*this->at(0,1) );
    }
    /** Dot product between two vectors.
     */
    static double dotProduct( const PdfeVector& vect1, const PdfeVector& vect2 ) {
        return vect1.at(0,0) * vect2.at(0,0) + vect1.at(0,1) * vect2.at(0,1);
    }
    /** Angle made between two vectors (inside the interval [0,pi]).
     */
    static double angle( const PdfeVector& vect1, const PdfeVector& vect2 ) {
        return acos( PdfeVector::dotProduct( vect1, vect2 ) / vect1.norm2() / vect2.norm2() );
    }
};

/** Oriented rectangle: characterize by bottom-left coordinates, direction, width and height.
 */
class PdfeORect
{
public:
    /** Construct an oriented rectangle with given width (w) and height (h) and direction (1,0).
     * \param w Width of the rectangle.
     * \param h Height of the rectangle.
     */
    PdfeORect( double width = 1.0, double height = 1.0 ) {
        this->init( width, height );
    }
    /** Construct  an oriented rectangle from a PoDoFo::PdfRect and with direction (1,0).
     */
    PdfeORect( const PoDoFo::PdfRect& rect ) {
        this->init( rect.GetWidth(), rect.GetHeight() );
        m_leftBottom(0) = rect.GetLeft();
        m_leftBottom(1) = rect.GetBottom();
    }
    /** Initialize to oriented rectangle (0,0,w,h).
     * \param w Width of the rectangle.
     * \param h Height of the rectangle.
     */
    void init( double width = 1.0, double height = 1.0 ) {
        m_leftBottom.init();
        m_direction.init();
        m_direction(0) = 1.0;
        m_width = width;
        m_height = height;
    }

    /** Local transformation matrix: map a vector into the local coordinate system
     * defined by the direction of the PdfORect.
     * \return PdfeMatrix object representing the transformation.
     */
    PdfeMatrix localTransMatrix();

    // Getters and setters.
    /// Get Left Bottom point.
    PdfeVector leftBottom() const {
        return m_leftBottom;
    }
    /// Get Right Bottom point.
    PdfeVector rightBottom() const {
        return m_leftBottom + m_direction * m_width;
    }
    /// Get Left Top point.
    PdfeVector leftTop() const {
        return m_leftBottom + m_direction.rotate90() * m_height;
    }
    /// Get Right Top point.
    PdfeVector rightTop() const {
        return m_leftBottom + m_direction * m_width + m_direction.rotate90() * m_height;
    }

    /** Set Left Bottom point.
     */
    void setLeftBottom( const PdfeVector& leftBottom ) {
        m_leftBottom = leftBottom;
    }
    /** Get direction (unit vector).
     */
    PdfeVector direction() const {
        return m_direction;
    }
    /** Set direction vector.
     */
    void setDirection( const PdfeVector& direction ) {
        // Transform into unit vector.
        double norm = direction.norm2();
        m_direction(0) = direction(0) / norm;
        m_direction(1) = direction(1) / norm;
    }

    /** Get rectangle width.
     */
    double width() const {
        return m_width;
    }
    /** Set rectangle width.
     */
    void setWidth( double width ) {
        m_width = width;
    }
    /** Get rectangle height.
     */
    double height() const {
        return m_height;
    }
    /** Set rectangle height.
     */
    void setHeight( double height ) {
        m_height = height;
    }

    /** Compute the minimal distance between two PdfORect (roughly the Hausdorff distance).
     * \param rect1 First rectangle to consider.
     * \param rect2 Second rectangle.
     *  \return Estimate of the minimal distance.
     */
    static double minDistance( const PdfeORect& rect1, const PdfeORect& rect2 );

    /** Compute the minimal distance between a PdfORect and a point (represented by a PdfeVector).
     * \param rect Rectangle to consider.
     * \param point Point to consider.
     *  \return Estimate of the minimal distance.
     */
    static double minDistance( const PdfeORect& rect, const PdfeVector& point );

    /** Compute the maximal distance between two PdfORect.
     * \param rect1 First rectangle to consider.
     * \param rect2 Second rectangle.
     *  \return Estimate of the maximal distance.
     */
    static double maxDistance( const PdfeORect& rect1, const PdfeORect& rect2 );

    /** Compute the maximal distance between a PdfORect and a point (represented by a PdfeVector).
     * \param rect Rectangle to consider.
     * \param point Point to consider.
     *  \return Estimate of the maximal distance.
     */
    static double maxDistance( const PdfeORect& rect, const PdfeVector& point );

private:
    /// Left bottom point.
    PdfeVector  m_leftBottom;
    /// Direction (unit vector).
    PdfeVector  m_direction;
    /// Width of the rectangle.
    double  m_width;
    /// Height of the rectangle.
    double m_height;
};

//**********************************************************//
//                         PdfeVector                       //
//**********************************************************//

//**********************************************************//
//                         PdfeMatrix                       //
//**********************************************************//
inline PdfeVector PdfeMatrix::map( const PdfeVector& vect ) const
{
    PdfeVector mapVect;
    mapVect(0) = this->at(0,0) * vect(0) + this->at(1,0) * vect(1) + this->at(2,0);
    mapVect(1) = this->at(0,1) * vect(0) + this->at(1,1) * vect(1) + this->at(2,1);
    mapVect(2) = 1.0;
    return mapVect;
}
inline PdfeORect PdfeMatrix::map( const PdfeORect& rect ) const
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

    mapRect.setHeight( rect.height() * tmpVal );

    return mapRect;
}
//**********************************************************//
//                         PdfeORect                        //
//**********************************************************//
inline PdfeMatrix PdfeORect::localTransMatrix()
{
    PdfeMatrix transMat;
    transMat(0,0) = m_direction(0);      transMat(0,1) = -m_direction(1);
    transMat(1,0) = m_direction(1);      transMat(1,1) = m_direction(0);
    transMat(2,0) = -m_leftBottom(0) * m_direction(0) - m_leftBottom(1) * m_direction(1);
    transMat(2,1) =  m_leftBottom(0) * m_direction(1) - m_leftBottom(1) * m_direction(0);
    return transMat;
}

inline double PdfeORect::minDistance( const PdfeORect& rect1, const PdfeORect& rect2 )
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
inline double PdfeORect::minDistance( const PdfeORect& rect, const PdfeVector& point )
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

inline double PdfeORect::maxDistance( const PdfeORect& rect1, const PdfeORect& rect2 )
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
inline double PdfeORect::maxDistance( const PdfeORect& rect, const PdfeVector& point )
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



//}

#endif // PDFTYPES_H

