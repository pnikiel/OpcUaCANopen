
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


#ifndef __DTpdo__H__
#define __DTpdo__H__

#include <mutex>

#include <CanMessage.h>

#include <Base_DTpdo.h>
#include <Enumerator.hpp>

namespace Device
{

class
    DTpdo
    : public Base_DTpdo
{

public:
    /* sample constructor */
    explicit DTpdo (
        const Configuration::Tpdo& config,
        Parent_DTpdo* parent
    ) ;
    /* sample dtr */
    ~DTpdo ();

    /* delegators for
    cachevariables and sourcevariables */

    /* ASYNCHRONOUS !! */
    UaStatus readInvokeRtr (
        OpcUa_UInt32& value,
        UaDateTime& sourceTime
    );
    /* ASYNCHRONOUS !! */
    UaStatus writeInvokeRtr (
        OpcUa_UInt32& value
    );

    /* delegators for methods */

private:
    /* Delete copy constructor and assignment operator */
    DTpdo( const DTpdo& other );
    DTpdo& operator=(const DTpdo& other);

    // ----------------------------------------------------------------------- *
    // -     CUSTOM CODE STARTS BELOW THIS COMMENT.                            *
    // -     Don't change this comment, otherwise merge tool may be troubled.  *
    // ----------------------------------------------------------------------- *

public:
    void initialize ();

    void onReplyReceived(const CanMessage& msg);

    //! To be called just before the server emits sync
    void notifySync ();

    //! To be called when the bus goes down
    void propagateNull ();

    void tick(); // periodic trigger, here mostly to trigger the periodic RTR feature.

private:
    // sends RTR corresponding to this TPDO
    void sendRtr();
    bool m_onSync;
    bool m_firstIteration;
    Enumerator::Tpdo::TransportMechanism m_transportMechanism;

    unsigned int m_receivedCtrSinceLastSync;

    std::mutex m_accessLock;

    unsigned int m_numRtrsInvokedAddressSpace;

    std::chrono::steady_clock::time_point m_lastPeriodicRtr;

};

}

#endif // __DTpdo__H__