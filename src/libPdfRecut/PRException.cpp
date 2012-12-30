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

#include <podofo/podofo.h>
#include <QtCore>

namespace PdfRecut {

PRException::PRException(PRExceptionCode::Enum code,
                          const QString& description , bool logException) throw() :
    std::exception(), m_code( code ), m_description( description )
{
    if( logException ) {
        this->log( QsLogging::ErrorLevel );
    }
}
PRException::PRException( const PoDoFo::PdfError& error ) throw() :
    std::exception(), m_code( PRExceptionCode::PoDoFo )
{
    m_description = QCoreApplication::translate( "PoDoFoError",
                                                 "Error raised by PoDoFo library (code %1): \"%2\"" )
            .arg( PoDoFo::PdfError::ErrorName( error.GetError() ) )
            .arg( PoDoFo::PdfError::ErrorMessage( error.GetError() ) );
}
PRException::PRException( const PRException& exception ) throw() :
    std::exception( exception ),
    m_code( exception.m_code ),
    m_description( exception.m_description )
{
}
PRException& PRException::operator=( const PRException& exception ) throw()
{
    std::exception::operator=( exception );
    m_code = exception.m_code;
    m_description = exception.m_description;
    return *this;
}
PRException::~PRException() throw()
{
}

void PRException::log( QsLogging::Level level ) const throw()
{
    QString logMessage = QString( "<PRException> (Code %1): %2" ).arg( codeName( m_code ) ).arg( m_description );

    switch( level ) {
    case QsLogging::TraceLevel:
        QLOG_TRACE() << logMessage.toAscii().constData();
        break;
    case QsLogging::DebugLevel:
        QLOG_DEBUG() << logMessage.toAscii().constData();
        break;
    case QsLogging::InfoLevel:
        QLOG_INFO() << logMessage.toAscii().constData();
        break;
    case QsLogging::WarnLevel:
        QLOG_WARN() << logMessage.toAscii().constData();
        break;
    case QsLogging::ErrorLevel:
        QLOG_ERROR() << logMessage.toAscii().constData();
        break;
    case QsLogging::FatalLevel:
        QLOG_FATAL() << logMessage.toAscii().constData();
        break;
    }
}

PRExceptionCode::Enum PRException::code() const throw()
{
    return m_code;
}
QString PRException::description() const throw()
{
    return m_description;
}
const char* PRException::what() const throw()
{
    return m_description.toLocal8Bit().data();
}

QString PRException::codeName( PRExceptionCode::Enum code )
{
    // List of codes description. To be sync with PRExceptionCode enum.
    static QStringList strCodes;
    strCodes << "ErrorOK"
             << "PoDoFo"
             << "FreeType"
             << "PRInvalidHandle"
             << "PRCache"
             << "PRAbort"
             << "PRUnknown";
    return strCodes.at( code );
}

}
