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
