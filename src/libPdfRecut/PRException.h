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

#include <podofo/podofo.h>
#include <QtCore/QString>

#include <string>
#include <exception>

namespace PdfRecut {

/** Error Code defines which are used in PdfDocException to describe
 * the exception type.
 */
enum EPdfDocException {
    ePdfDocE_ErrOK = 0,           /**< The default value indicating no error. */
    ePdfDocE_PoDoFo,              /**< Error from library PoDoFo.             */
    ePdfDocE_Poppler,             /**< Error from library Poppler.            */
    ePdfDocE_Abort                /**< Current operation aborted.             */
};

/** Class that handle errors during threatment on Pdf document.
 * Error code and description can be provided to an exception.
 */
class PRException : public std::exception
{
public:
    /** Default constructor
     * \param code Error code.
     * \param description String describing the error.
     */
    PRException( const EPdfDocException& code = ePdfDocE_ErrOK,
                     const QString& description = "" ) throw();

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

    /** Obtain exception code.
     * \return Error code.
     */
    EPdfDocException getCode() const throw();

    /** Obtain exception description.
     * \return Error description.
     */
    QString getDescription() const throw();

    /** Classic what function, inheritated from std::exception.
     * \return Description of the exception.
     */
    virtual const char* what() const throw();

protected:
    /** Exception code.
     */
    EPdfDocException m_code;

    /** Exception description.
     */
    QString m_description;
};

}

#endif // PREXCEPTION_H
