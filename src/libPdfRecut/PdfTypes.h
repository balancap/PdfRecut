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

#include "vmmlib/vmmlib.hpp"
#include <QPointF>
#include <QTransform>
#include <vector>

class PdfVector;
class PdfORect;

/** Transformation matrix used in graphics state.
 */
class PdfMatrix : public vmml::mat3d
{
public:
    /** Default constructor, matrix initialized to unit.
     */
    PdfMatrix() {
        this->init();
    }
    /** Copy constructor for vmml::mat3d.
     */
    PdfMatrix( const vmml::mat3d& source  ) :
        vmml::mat3d( source )
    {
    }

    /** =Operator overload
     */
    const PdfMatrix& operator=( const vmml::mat3d& source ) {
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
     */
    bool inverse( PdfMatrix& invMat ) const {
        return vmml::compute_inverse( *this, invMat );
    }

    /** Convert to a QTransform object.
     */
    QTransform toQTransform() const {
        return QTransform( at(0,0), at(0,1), at(1,0), at(1,1), at(2,0), at(2,1) );
    }

    /** Map a vector in the new coordinate system defined by the matrix.
     */
    PdfVector map( const PdfVector& vect ) const;

    /** Map a rectangle in the new coordinate system defined by the matrix.
     */
    PdfORect map( const PdfORect& rect ) const;
};

/** Position vector 1x3 representing a position in a page,
 * in a given coordinate system.
 */
class PdfVector : public vmml::matrix<1,3,double>
{
public:
    /** Default constructor, vector initialize to (0,0,1)
     */
    PdfVector() {
        this->init();
    }

    PdfVector( double x, double y ) {
        this->init( x, y );
    }

    /** =Operator overloaded
     */
    const PdfVector& operator=( const vmml::matrix<1,3,double>& source ) {
        *( (vmml::matrix<1,3,double>*) this ) = source;
        return *this;
    }
    /** +Operator overloaded
     */
    PdfVector operator*( const PdfVector& vect ) const {
        PdfVector vect2;
        vect2.at(0,0) = vect2.at(0,0) + this->at(0,0);
        vect2.at(0,1) = vect2.at(0,1) + this->at(0,1);
        vect2.at(0,2) = 1.0;

        return vect2;
    }
    /** *Operator overloaded
     */
    PdfVector operator*( const PdfMatrix& mat ) const {
        PdfVector vect;
        vect.at(0,0) = mat(0,0) * this->at(0,0) + mat(1,0) * this->at(0,1) + mat(2,0);
        vect.at(0,1) = mat(0,1) * this->at(0,0) + mat(1,1) * this->at(0,1) + mat(2,1);
        vect.at(0,2) = 1.0;
        // mapVect(0) = mat(0,0) * this->at(0,0) + mat(1,0) * this->at(0,1) + mat(2,0) * this->at(0,2);
        // mapVect(1) = mat(0,1) * this->at(0,0) + mat(1,1) * this->at(0,1) + mat(2,1) * this->at(0,2);
        // mapVect(2) = mat(0,2) * this->at(0,0) + mat(1,2) * this->at(0,1) + mat(2,2) * this->at(0,2);

        return vect;
    }
    /** *Operator overloaded
     */
    PdfVector operator*( double coef ) const {
        PdfVector vect;
        vect.at(0,0) = this->at(0,0) * coef;
        vect.at(0,1) = this->at(0,1) * coef;
        vect.at(0,2) = 1.0;

        return vect;
    }

    /** Rotate a vector of 90°.
     * \return Copy of the vector rotated.
     */
    PdfVector rotate90() const {
        PdfVector vect;
        vect.at(0,0) = -this->at(0,1);
        vect.at(0,1) = this->at(0,0);
        vect.at(0,2) = 1.0;

        return vect;
    }

    /** (idx) Operator overload
     */
    double& operator()( size_t index ) {
        return this->at(0,index);
    }
    /** const (idx) Operator overload
     */
    const double& operator()( size_t index ) const {
        return this->at(0,index);
    }

    /** Initialize vector to (0,0,1)
     */
    void init() {
        this->at(0,0) = this->at(0,1) = 0;
        this->at(0,2) = 1;
    }
    void init( double x, double y ) {
        this->at(0,0) = x;
        this->at(0,1) = y;
        this->at(0,2) = 1;
    }

    /** Convert to a QPointF object.
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
    static double dotProduct( const PdfVector& vect1, const PdfVector& vect2 ) {
        return vect1.at(0,0) * vect2.at(0,0) + vect1.at(0,1) * vect2.at(0,1);
    }
    /** Angle made between two vectors (inside the interval [0,pi]).
     */
    static double angle( const PdfVector& vect1, const PdfVector& vect2 ) {
        return acos( PdfVector::dotProduct( vect1, vect2 ) / vect1.norm2() / vect2.norm2() );
    }
};

/** Orienlengthted rectangle: characterize by bottom-left coordinates, direction, width and height.
 */
class PdfORect
{
public:
    /** Default constructor of the oriented rectangle: initialize to (0,0,w,h).
     */
    PdfORect( double width = 1.0, double height = 1.0 ) {
        this->init( width, height );
    }
    /** Initialize to oriented rectangle (0,0,w,h).
     */
    void init( double width = 1.0, double height = 1.0 ) {
        m_leftBottom.init();
        m_direction.init();
        m_direction(0) = 1.0;
        m_width = width;
        m_height = height;
    }

    /** Get Left Bottom point.
     */
    PdfVector getLeftBottom() const {
        return m_leftBottom;
    }
    /** Set Left Bottom point.
     */
    void setLeftBottom( const PdfVector& leftBottom ) {
        m_leftBottom = leftBottom;
    }
    /** Get direction (unit vector).
     */
    PdfVector getDirection() const {
        return m_direction;
    }
    /** Set direction vector.
     */
    void setDirection( const PdfVector& direction ) {
        // Transform into unit vector.
        double norm = direction.norm2();
        m_direction(0) = direction(0) / norm;
        m_direction(1) = direction(1) / norm;
    }

    /** Get rectangle width.
     */
    double getWidth() const {
        return m_width;
    }
    /** Set rectangle width.
     */
    void setWidth( double width ) {
        m_width = width;
    }
    /** Get rectangle height.
     */
    double getHeight() const {
        return m_height;
    }
    /** Set rectangle height.
     */
    void setHeight( double height ) {
        m_height = height;
    }

private:
    /** Left bottom point.
     */
    PdfVector m_leftBottom;
    /** Direction (unit vector).
     */
    PdfVector m_direction;

    /** Width of the rectangle.
     */
    double m_width;
    /** Height of the rectangle.
     */
    double m_height;
};

//**********************************************************//
//                         PdfVector                        //
//**********************************************************//

//**********************************************************//
//                         PdfMatrix                        //
//**********************************************************//
inline PdfVector PdfMatrix::map( const PdfVector& vect ) const
{
    PdfVector mapVect;
//    mapVect(0) = this->at(0,0) * vect(0) + this->at(1,0) * vect(1) + this->at(2,0) * vect(2);
//    mapVect(1) = this->at(0,1) * vect(0) + this->at(1,1) * vect(1) + this->at(2,1) * vect(2);
//    mapVect(2) = this->at(0,2) * vect(0) + this->at(1,2) * vect(1) + this->at(2,2) * vect(2);
    mapVect(0) = this->at(0,0) * vect(0) + this->at(1,0) * vect(1) + this->at(2,0);
    mapVect(1) = this->at(0,1) * vect(0) + this->at(1,1) * vect(1) + this->at(2,1);
    mapVect(2) = 1.0;
    return mapVect;
}
inline PdfORect PdfMatrix::map( const PdfORect& rect ) const
{
    PdfORect mapRect;
    PdfVector tmpVect1, tmpVect2;
    double tmpVal;

    // Set left bottom point.
    mapRect.setLeftBottom( this->map( rect.getLeftBottom() ) );

    // Set direction and width.
    tmpVect2 = rect.getDirection();
    tmpVect1(0) = this->at(0,0) * tmpVect2(0) + this->at(1,0) * tmpVect2(1);
    tmpVect1(1) = this->at(0,1) * tmpVect2(0) + this->at(1,1) * tmpVect2(1);
    tmpVect1(2) = 1.0;
    tmpVal = tmpVect1.norm2();

    mapRect.setDirection( tmpVect1 );
    mapRect.setWidth( rect.getWidth() * tmpVal );

    // Set height (slight approximation...).
    tmpVect1(0) = this->at(0,0) * -tmpVect2(1) + this->at(1,0) * tmpVect2(0);
    tmpVect1(1) = this->at(0,1) * -tmpVect2(1) + this->at(1,1) * tmpVect2(0);
    tmpVect1(2) = 1.0;
    tmpVect2 = mapRect.getDirection();
    tmpVal = tmpVect1(0) * -tmpVect2(1) + tmpVect1(1) * tmpVect2(0);

    mapRect.setHeight( rect.getHeight() * tmpVal );

    return mapRect;
}
//**********************************************************//
//                         PdfORect                         //
//**********************************************************//

#endif // PDFTYPES_H

