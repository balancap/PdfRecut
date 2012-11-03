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

#ifndef PREXCEPTION_H
#define PREXCEPTION_H

#include <QtCore/QString>

#include <string>
#include <exception>

#include <QsLog/QsLog.h>

namespace PoDoFo {
class PdfError;
}

namespace PdfRecut {

namespace PRExceptionCode {
/** PRException types that can be raised.
 */
enum Enum {
    ErrorOK = 0,    /// The default value indicating no error.
    PoDoFo,         /// Error from library PoDoFo.
    FreeType,       /// FreeType error.
    Abort           /// Current operation aborted.
};
}

/** Class that handle errors during threatment on PDF document.
 * Error code and description can be provided to an exception.
 */
class PRException : public std::exception
{
public:
    /** Default constructor of a PdfRecut exception.
     * \param code Error code.
     * \param description String describing the error.
     * \param logException Log the exception (error level)? False by default.
     */
    PRException( PRExceptionCode::Enum code = PRExceptionCode::ErrorOK,
                 const QString& description = "",
                 bool logException = false ) throw();

    /** Constructor based on a PoDoFo error.
     * \param error PoDoFo exception error.
     */
    PRException( const PoDoFo::PdfError& error ) throw();
    /** Copy constructor.
     * \param exception Exception object to copy.
     */
    PRException( const PRException& exception ) throw();
    /** Operator=.
     * \param exception Exception object to copy.
     */
    PRException& operator=( const PRException& exception ) throw();

    /** Destructor.
     */
    virtual ~PRException() throw();

    /** Log the exception.
     * \param level Level used in the log (default: error).
     */
    void log( QsLogging::Level level = QsLogging::ErrorLevel ) const throw();

public:
    // Getters...
    /** Obtain exception code.
     * \return Exception code.
     */
    PRExceptionCode::Enum code() const throw();
    /** Obtain exception description.
     * \return Exception description.
     */
    QString description() const throw();
    /** Overload what member function, inheritated from std::exception.
     * \return Description of the exception.
     */
    virtual const char* what() const throw();

public:
    /** Obtain the description of an exception error.
     * \param code Exception code.
     * \return Code description (QString).
     */
    static QString codeDescription( PRExceptionCode::Enum code );

protected:
    /// Exception code.
    PRExceptionCode::Enum  m_code;
    /// Exception description.
    QString  m_description;
};

}

#endif // PREXCEPTION_H
