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

#ifndef PDFSEMAPHORE_H
#define PDFSEMAPHORE_H

#include <QSemaphore>

/** A semaphore class used in different Pdf class to protect members
 * in a multithread program and make the class thread-safe.
 */
class PdfClassSemaphore : public QSemaphore
{
public:
    /** Numbers of resources used for the semaphore
     */
    static const int NResources = 50;

public:
    /** Constructor initialize semaphore with NResources
     */
    PdfClassSemaphore() : QSemaphore( NResources ) { }
};

/** Similar to QMutexLocker: necessary to have a read-only access to class
 * members. Need to acquire one resource from the class semaphore. Allow parallel
 * access (NRessources as a maximum).
 */
class PdfSemaphoreReadLocker
{
public:
    /** Constructor: block until can obtain one resource.
     */
    PdfSemaphoreReadLocker( PdfClassSemaphore* semaphore )
    {
        m_semaphore = semaphore;
        m_semaphore->acquire( 1 );
    }
    /** Destructor: release the resource blocked.
     */
    ~PdfSemaphoreReadLocker()
    {
        m_semaphore->release( 1 );
    }

private:
    /** Pointer to the semaphore used.
     */
    PdfClassSemaphore* m_semaphore;
};

/** Similar to QMutexLocker: necessary to have a write access to class
 * members. Need to acquire all resources from the class semaphore, and thus block
 * read and write on the class.
 */
class PdfSemaphoreWriteLocker
{
public:
    /** Constructor: block until can obtain all resources of the semaphore.
     */
    PdfSemaphoreWriteLocker( PdfClassSemaphore* semaphore )
    {
        m_semaphore = semaphore;
        m_semaphore->acquire( PdfClassSemaphore::NResources );
    }
    /** Destructor: release resources blocked.
     */
    ~PdfSemaphoreWriteLocker()
    {
        m_semaphore->release( PdfClassSemaphore::NResources );
    }

private:
    /** Pointer to the semaphore used.
     */
    PdfClassSemaphore* m_semaphore;
};

#endif // PDFSEMAPHORE_H
