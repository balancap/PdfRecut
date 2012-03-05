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
