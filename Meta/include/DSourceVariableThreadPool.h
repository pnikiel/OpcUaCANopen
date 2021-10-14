/* © Copyright CERN, 2015.  All rights not expressly granted are reserved.
 * DSourceVariableThreadPool.h
 *
 *  Created on: Aug 18, 2015
 * 	Author: Benjamin Farnham <benjamin.farnham@cern.ch>
 *
 *  This file is part of Quasar.
 *
 *  Quasar is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public Licence as published by
 *  the Free Software Foundation, either version 3 of the Licence.
 *
 *  Quasar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public Licence for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Quasar.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __DSourceVariableThreadPool__H__
#define __DSourceVariableThreadPool__H__

#include <vector>

#include <statuscode.h>
#include <uadatetime.h>
#include <session.h>


#include <DRoot.h>
#include <Configuration.hxx>

#include <Base_DSourceVariableThreadPool.h>


namespace Device
{




class
    DSourceVariableThreadPool
    : public Base_DSourceVariableThreadPool
{

public:
    /* sample constructor */
    explicit DSourceVariableThreadPool (const size_t& min, const size_t& max, const size_t& maxJobs);
    /* sample dtr */
    ~DSourceVariableThreadPool ();




    /* delegators for
    cachevariables and sourcevariables */


private:
    /* Delete copy constructor and assignment operator */
    DSourceVariableThreadPool( const DSourceVariableThreadPool & );
    DSourceVariableThreadPool& operator=(const DSourceVariableThreadPool &other);

    // ----------------------------------------------------------------------- *
    // -     CUSTOM CODE STARTS BELOW THIS COMMENT.                            *
    // -     Don't change this comment, otherwise merge tool may be troubled.  *
    // ----------------------------------------------------------------------- *

public:

private:


};





}

#endif // include guard
