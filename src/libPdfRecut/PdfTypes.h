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
#include <QTransform>
#include <vector>

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

    QTransform toQTransform() const {
        return QTransform( at(0,0), at(0,1), at(1,0), at(1,1), at(2,0), at(2,1) );
    }
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
};

#endif // PDFTYPES_H

