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

#ifndef PDFESEMAPHORE_H
#define PDFESEMAPHORE_H

#include <QSemaphore>

namespace PdfRecut {

/** A semaphore class used in different Pdf class to protect members
 * in a multithread program and make the class thread-safe.
 */
class PdfeClassSemaphore : public QSemaphore
{
public:
    /// Numbers of resources used for the semaphore
    static const int NResources = 50;

public:
    /** Constructor initialize semaphore with NResources
     */
    PdfeClassSemaphore() : QSemaphore( NResources ) { }
};

/** Similar to QMutexLocker: necessary to have a read-only access to class
 * members. Need to acquire one resource from the class semaphore. Allow parallel
 * access (NRessources as a maximum).
 */
class PdfeSemaphoreReadLocker
{
public:
    /** Constructor: block until can obtain one resource.
     */
    PdfeSemaphoreReadLocker( PdfeClassSemaphore* semaphore ) {
        m_semaphore = semaphore;
        m_semaphore->acquire( 1 );
    }
    /** Destructor: release the resource blocked.
     */
    ~PdfeSemaphoreReadLocker() {
        m_semaphore->release( 1 );
    }

private:
    /// Pointer to the semaphore used.
    PdfeClassSemaphore*  m_semaphore;
};

/** Similar to QMutexLocker: necessary to have a write access to class
 * members. Need to acquire all resources from the class semaphore, and thus block
 * read and write on the class.
 */
class PdfeSemaphoreWriteLocker
{
public:
    /** Constructor: block until can obtain all resources of the semaphore.
     */
    PdfeSemaphoreWriteLocker( PdfeClassSemaphore* semaphore ) {
        m_semaphore = semaphore;
        m_semaphore->acquire( PdfeClassSemaphore::NResources );
    }
    /** Destructor: release resources blocked.
     */
    ~PdfeSemaphoreWriteLocker() {
        m_semaphore->release( PdfeClassSemaphore::NResources );
    }

private:
    /// Pointer to the semaphore used.
    PdfeClassSemaphore*  m_semaphore;
};

}

#endif // PDFESEMAPHORE_H
