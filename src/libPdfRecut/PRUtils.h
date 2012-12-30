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

#ifndef PRUTILS_H
#define PRUTILS_H

#include <QString>

namespace PdfRecut {

// File that gathers misc. simple classes/functions that can be useful.

/** Is a QString uniquely composed of letters and numbers?
 * \param ustr Unicode string.
 * \return Answer! False if the string is empty.
 */
bool QStringIsLettersNumbers( const QString& ustr );

}

#endif // PRUTILS_H
