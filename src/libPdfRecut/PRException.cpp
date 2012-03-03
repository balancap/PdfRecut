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

#include "PRException.h"

#include <QtCore>

namespace PdfRecut {

PRException::PRException( const EPdfDocException& code,
                                  const QString& description ) throw() :
    std::exception(), m_code( code ), m_description( description )
{
}
PRException::PRException( const PoDoFo::PdfError& error ) throw() :
    std::exception(), m_code( ePdfDocE_PoDoFo )
{
    m_description = QCoreApplication::translate( "PoDoFoError",
                    "Error from library PoDoFo (code %1): %2" )
                    .arg( PoDoFo::PdfError::ErrorName( error.GetError() ) )
                    .arg( PoDoFo::PdfError::ErrorMessage( error.GetError() ) );
}

PRException::PRException( const PRException& exception ) throw() :
    std::exception( exception )
{
    this->operator =( exception );
}
PRException& PRException::operator=( const PRException& exception ) throw()
{
    this->std::exception::operator =( exception );
    this->m_code = exception.m_code;
    this->m_description = exception.m_description;

    return *this;
}

PRException::~PRException() throw()
{
}

EPdfDocException PRException::getCode() const throw()
{
    return m_code;
}
QString PRException::getDescription() const throw()
{
    return m_description;
}
const char* PRException::what() const throw()
{
    return m_description.toLocal8Bit().data();
}

}
