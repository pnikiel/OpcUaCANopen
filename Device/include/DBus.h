
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


#ifndef __DBus__H__
#define __DBus__H__

#include <chrono>

#include <GhostPointer.h>
#include <CCanAccess.h>

#include <NodeGuarding.hpp>
#include <CobidCoordinator.hpp>

#include <Base_DBus.h>

namespace Device
{

typedef std::function<void()> BusErrorNotification;

class
    DBus
    : public Base_DBus
{

public:
    /* sample constructor */
    explicit DBus (
        const Configuration::Bus& config,
        Parent_DBus* parent
    ) ;
    /* sample dtr */
    ~DBus ();

    /* delegators for
    cachevariables and sourcevariables */
    /* Note: never directly call this function. */
    UaStatus writeSyncIntervalMs ( const OpcUa_UInt32& v);
    /* Note: never directly call this function. */
    UaStatus writeNodeGuardIntervalMs ( const OpcUa_UInt32& v);


    /* delegators for methods */

private:
    /* Delete copy constructor and assignment operator */
    DBus( const DBus& other );
    DBus& operator=(const DBus& other);

    // ----------------------------------------------------------------------- *
    // -     CUSTOM CODE STARTS BELOW THIS COMMENT.                            *
    // -     Don't change this comment, otherwise merge tool may be troubled.  *
    // ----------------------------------------------------------------------- *

public:
    //! Call it once before server enters the actual mainloop.
    void initialize();

    //! Call it 1/sec or so to run internal state machines and other timed actions.
    void tick();

    //! This is to be used by device logic of this as well as child classes for sending msgs to the CAN interface.
    void sendMessage(const CanMessage& msg);

    // to be called when server is shutting down; will stop processing traffic, etc.
    void shutDown();

    bool isInSpyMode () const;

    void addBusErrorNotification(BusErrorNotification notification) { m_busErrorNotifications.push_back(notification); }

    void getStatistics(CanModule::CanStatistics& statistics) const;

    CANopen::CobidCoordinator& cobidCoordinator() { return m_cobidCoordinator; } // TODO probably move it somehow.

private:
    GhostPointer<CanModule::CCanAccess> m_canAccess;

    std::chrono::steady_clock::time_point m_lastNodeGuardingTimePoint; // probably remove - went to the DNode
    std::chrono::steady_clock::time_point m_lastSyncTimePoint;
    std::chrono::steady_clock::time_point m_lastStatsCalc;

    void invokeNodeGuarding();
    void tickSync();
    void tickPublishingStatistics ();

    //! Every message received (by CanModule) goes via this function.
    void onMessageReceived (const CanMessage& msg);

    void onError (const int errorCode, const char* errorDescription, timeval&);

    std::vector<BusErrorNotification> m_busErrorNotifications;

    std::string translateBusSettingsToCanModuleFormat (const std::string& busSettingsInServerFormat);

    int m_lastErrorCode; // to compare in onError against the current value to handle transitions

    CANopen::CobidCoordinator m_cobidCoordinator;



};

}

#endif // __DBus__H__