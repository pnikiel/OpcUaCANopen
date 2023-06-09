
/*  © Copyright CERN, 2015. All rights not expressly granted are reserved.

    The stub of this file was generated by quasar (https://github.com/quasar-team/quasar/)

    Quasar is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public Licence as published by
    the Free Software Foundation, either version 3 of the Licence.
    Quasar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public Licence for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Quasar.  If not, see <http://www.gnu.org/licenses/>.


 */


#ifndef __DMultiplexedChannel__H__
#define __DMultiplexedChannel__H__

#include <Base_DMultiplexedChannel.h>

#include <DTpdoMultiplex.h>

#include <CanMessage.h>

namespace Device
{

class
    DMultiplexedChannel
    : public Base_DMultiplexedChannel
{

public:
    /* sample constructor */
    explicit DMultiplexedChannel (
        const Configuration::MultiplexedChannel& config,
        Parent_DMultiplexedChannel* parent
    ) ;
    /* sample dtr */
    ~DMultiplexedChannel ();

    /* delegators for
    cachevariables and sourcevariables */


    /* delegators for methods */

private:
    /* Delete copy constructor and assignment operator */
    DMultiplexedChannel( const DMultiplexedChannel& other );
    DMultiplexedChannel& operator=(const DMultiplexedChannel& other);

    // ----------------------------------------------------------------------- *
    // -     CUSTOM CODE STARTS BELOW THIS COMMENT.                            *
    // -     Don't change this comment, otherwise merge tool may be troubled.  *
    // ----------------------------------------------------------------------- *

public:
    void initialize();

    void onReplyReceived(const CanMessage& msg);

    //! To be called just before the server emits sync
    bool notifySyncAndCheckMismatch ();

    void setParentMultiplex(DTpdoMultiplex* parent) { m_parentMultiplex = parent; }

    //! To be called when the bus goes down
    void notifyBusError ();

private:
    DTpdoMultiplex* m_parentMultiplex;
    bool m_firstIteration;
    unsigned int m_receivedCtrSinceLastSync;


};

}

#endif // __DMultiplexedChannel__H__