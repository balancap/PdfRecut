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

#ifndef PRGTEXTSTATISTICS_H
#define PRGTEXTSTATISTICS_H

#include <cstdlib>
#include <vector>

namespace PdfRecut {

class PRGTextGroupWords;

namespace PRGTextVariables
{
/** Enumeration of text variables that are considered
 * in a PDF document.
 */
enum Enum {
    CharAllWidth = 0,   /// All characters width.
    CharAllHeight,      /// All characters height.
    CharLNWidth,        /// Letters or numbers width.
    CharLNHeight,       /// Letters or numbers height.
    LineHWidth,
    LineVWidth,
    LineInnerSpace
};
/** Get the number of text variables.
 */
inline size_t number() {
    static size_t nb = 7;
    return nb;
}
}

//**********************************************************//
//                     PRGTextStatistics                    //
//**********************************************************//
/** Class whose aim is to gather different statistics on page's text:
 * words, lines,...  It should be use on a group of page, i.e. a PRGSubDocument
 * in order to obtain reliable estimations.
 */
class PRGTextStatistics
{
public:
    /** Construct an empty object.
     */
    PRGTextStatistics();
    /** Destructor.
     */
    ~PRGTextStatistics();
    /** Initialize to an empty object.
     */
    void init();
    /** Clear the object.
     */
    void clear();

public:
    /** Class that represents a variable studied.
     */
    class Variable;
public:
    /** Add measures corresponding to a group of words.
     * \param group Reference to the group to consider.
     */
    void addGroupWords( const PRGTextGroupWords& group );
    /** Get a text variable object.
     * \param vartype Text variable to return.
     * \return Constant reference to the variable object.
     */
    const Variable& variable( PRGTextVariables::Enum vartype ) const   {   return *m_pVariables[vartype];   }

private:
    // No copy constructor and assignement operator.
    PRGTextStatistics( const PRGTextStatistics& );
    PRGTextStatistics& operator=( const PRGTextStatistics& );

private:
    /// Vector of text variables.
    std::vector<Variable*>  m_pVariables;
};

//**********************************************************//
//                PRGTextStatistics::Variable               //
//**********************************************************//
class PRGTextStatistics::Variable
{
public:
    /** Default constructor.
     */
    Variable();
    /** Initialize to an empty variable.
     */
    void init();
public:
    /** Estimate the mean.
     * \return Estimation of the mean.
     */
    double mean() const;
    /** Estimate the variance.
     * \return Unbiased estimation of the variance.
     */
    double variance();
    /** Add a measured value for the variable.
     * \param val Measure to add.
     */
    void addValue( double val );

private:
    /// Number of measures.
    size_t  m_nbMeasures;
    /// Sum of values (not renormalized).
    double  m_sumValues;
    /// Sum of squared values (not renormalized).
    double  m_sumSquaredValues;
};

}

#endif // PRGTEXTSTATISTICS_H
