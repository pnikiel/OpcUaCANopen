
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


#include <Configuration.hxx> // TODO; should go away, is already in Base class for ages

#include <DBus.h>
#include <ASBus.h>
#include <DRoot.h>
#include <DGlobalSettings.h>

#include <CanLibLoader.h>

#include <Utils.h>
#include <SockCanScan.h>
#include <DNode.h>
#include <DWarnings.h>

#include <Logging.hpp>
#include <FrameFactory.hpp>
#include <PiotrsUtils.h>

using namespace Logging;
using namespace std::placeholders;

namespace Device
{
// 1111111111111111111111111111111111111111111111111111111111111111111111111
// 1     GENERATED CODE STARTS HERE AND FINISHES AT SECTION 2              1
// 1     Users don't modify this code!!!!                                  1
// 1     If you modify this code you may start a fire or a flood somewhere,1
// 1     and some human being may possible cease to exist. You don't want  1
// 1     to be charged with that!                                          1
// 1111111111111111111111111111111111111111111111111111111111111111111111111






// 2222222222222222222222222222222222222222222222222222222222222222222222222
// 2     SEMI CUSTOM CODE STARTS HERE AND FINISHES AT SECTION 3            2
// 2     (code for which only stubs were generated automatically)          2
// 2     You should add the implementation but dont alter the headers      2
// 2     (apart from constructor, in which you should complete initializati2
// 2     on list)                                                          2
// 2222222222222222222222222222222222222222222222222222222222222222222222222

/* sample ctr */
DBus::DBus (
    const Configuration::Bus& config,
    Parent_DBus* parent
):
    Base_DBus( config, parent)

    /* fill up constructor initialization list here */
{
    /* fill up constructor body here */
    //CanModule::CanLibLoader* loader = CanModule::CanLibLoader::createInstance("sock");
    //CanModule::CCanAccess* canAccess = loader->openCanBus("sock:vcan0", "Unspecified");
    CSockCanScan* sockCanAccess = new CSockCanScan;
    sockCanAccess->initialiseLogging(LogItInstance::getInstance());
    sockCanAccess->createBus("sock:"+port(), "Unspecified");

    CanModule::CCanAccess* canAccess = sockCanAccess;
    

    if (!canAccess)
        throw std::runtime_error("port is not available");
    m_canAccess.assign(canAccess);
    m_canAccess.enableAccess();
}

/* sample dtr */
DBus::~DBus ()
{
}

/* delegates for cachevariables */

/* Note: never directly call this function. */

UaStatus DBus::writeNodeGuardIntervalMs ( const OpcUa_UInt32& v)
{
    return OpcUa_BadNotImplemented; // TODO
}


/* delegators for methods */

// 3333333333333333333333333333333333333333333333333333333333333333333333333
// 3     FULLY CUSTOM CODE STARTS HERE                                     3
// 3     Below you put bodies for custom methods defined for this class.   3
// 3     You can do whatever you want, but please be decent.               3
// 3333333333333333333333333333333333333333333333333333333333333333333333333

void DBus::initialize()
{
    m_canAccess->canMessageCame.connect(std::bind(&DBus::onMessageReceived, this, std::placeholders::_1));
    m_canAccess->canMessageError.connect(std::bind(&DBus::onError, this, placeholders::_1, placeholders::_2, placeholders::_3));
    m_canAccess->updateInitialError();
    //getAddressSpaceLink()->setPortError()

    for (Device::DNode* node : nodes())
        node->initialize();
}

void DBus::tick()
{
	LOG(Log::TRC) << "Tick called";
    for (DNode* node : nodes())
        node->tick();
    tickSync();
    tickPublishingStatistics();
}

void DBus::onMessageReceived (const CanMessage& msg)
{
    // TOD?O: option question, how do we deal with long msgs ?
    //LOG(Log::INF) << "msg came, " << msg.c_id; // some separate levels ... ?
    // what is the message
    //unsigned int functionCode = msg.c_id >> 7;

    // TODO Msg=0x0 is the NMT control (e.g.  [id=0x0 dlc=2 data=[01 06 ]])
    // This would be legal in the spy mode! --> Spy Mode INFORMATION

    if (msg.c_id == 0x00) // NMT Control
    {
        if (this->isInSpyMode())
        {
            // Feature clause FC4.1: Spy mode
            LOG(Log::TRC, "Spy") << wrapId(getFullName()) << " seeing NMT Control from another host, CS is " << wrapId(Utils::toHexString(msg.c_data[0])) << 
                " and the ID is " << wrapId(std::to_string((int)msg.c_data[1]));
        }
        else
        {
            if (DRoot::getInstance()->warningss()[0]->unexpectedNmt())
            {
                SPOOKY(getFullName()) << "received unexpected NMT. " << SPOOKY_ << " [WunexpectedNmt] " <<
                    "This is only expected for the server in the spy mode. Someone is messing badly with your CAN bus!";
            }         
        }

        return;

    }

    if (msg.c_id == 0x80)
    { /* definitely a SYNC */
        if (this->isInSpyMode())
        {
            // Feature clause FC4.1: Spy mode
            LOG(Log::TRC, "Spy") << wrapId(getFullName()) << " seeing SYNC from another host.";
            for (DNode* node : nodes())
                node->notifySync();
        }
        else
        {
            if (DRoot::getInstance()->warningss()[0]->unexpectedSync())
            {
                SPOOKY(getFullName()) << "received unexpected SYNC. " << SPOOKY_ << " [WunexpectedSync] " <<
                    "This is only expected for the server in the spy mode. Someone is messing badly with your CAN bus!";
            }         
        }
        return;
    }

    unsigned int nodeId = msg.c_id & 0x7f;
    // TODO shall everything be passed to specific node
    DNode* node = getNodeById(nodeId);
    if (node)
        node->onMessageReceived(msg);
    else
    {
        if (DRoot::getInstance()->warningss()[0]->rxFromUnknownNode()) // TODO: the model to study warnings must be invincible to the situation when the object structure is not entirely ready
        {
            SPOOKY(getFullName()) << "received msg suggesting unknown node " << wrapValue(std::to_string(nodeId)) 
                << ", fix your hardware or your configuration." << SPOOKY_ << " [WrxFromUnknownNode]" <<
                " (the frame was: " << wrapValue(Common::CanMessageToString(msg)) << ")";
        }
    }
}

void DBus::sendMessage(const CanMessage& msg)
{
    if (this->isInSpyMode())
    {
        // Feature clause FC4.1: Spy mode
        LOG(Log::TRC, "Spy") << wrapId(getFullName()) << " message not sent, bus is in spy mode";
    }
    else
    {
        CanMessage copy (msg);
        m_canAccess->sendMessage(&copy); // TODO: suggest to Michael that sendMessage should be const-ref
    }
}


void DBus::tickSync()
{
    if (isInSpyMode())
        return; // we're not sending SYNC in the spy mode.

    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    unsigned int millisecondsSinceLastSync = std::chrono::duration_cast<std::chrono::milliseconds> (now - m_lastSyncTimePoint).count();
    // Feature clause FP1.2: Support of SYNC messages
    if (millisecondsSinceLastSync >= getAddressSpaceLink()->getSyncIntervalMs())
    {
        // notify that new sync cycle starts -- this is relevant for book-keeping of TPDOs, MTPDOs and other stuff.
        for (DNode* node : nodes())
            node->notifySync();
        sendMessage(CANopen::makeSyncRequest());
        m_lastSyncTimePoint = now;
    }
}

void DBus::onError (const int errorCode, const char* errorDescription, timeval&)
{
    // Feature clause FC1.3: Monitor CAN port state
    getAddressSpaceLink()->setPortError(errorCode, OpcUa_Good);
    getAddressSpaceLink()->setPortErrorDescription(errorDescription, OpcUa_Good);

    // Feature clause FC2.1: CAN port statistics
    // TODO: we should count when there is the transition from Good to Bad, but that is to be revisited with the CAN module.
    getAddressSpaceLink()->setStatsTransitionsIntoErrorCounter(getAddressSpaceLink()->getStatsTransitionsIntoErrorCounter() + 1, OpcUa_Good);

    if (errorCode != 0)
    {
        // Feature clause FC3.1: Propagation of CAN port state into affected variables
        LOG(Log::TRC) << "Propagating bus error notification";
        for (BusErrorNotification& notification : m_busErrorNotifications)
            notification();
    }
}

void DBus::tickPublishingStatistics ()
{

    CanStatistics stats;
    m_canAccess->getStatistics(stats);
    getAddressSpaceLink()->setStatsTotalReceived(stats.totalReceived(), OpcUa_Good);
    getAddressSpaceLink()->setStatsTotalTransmitted(stats.totalTransmitted(), OpcUa_Good);
    getAddressSpaceLink()->setStatsTxRate(stats.txRate(), OpcUa_Good);
    getAddressSpaceLink()->setStatsRxRate(stats.rxRate(), OpcUa_Good);
    
}

bool DBus::isInSpyMode () const
{
    // Feature clause FC4.1: Spy mode
    Device::DGlobalSettings* globalSettings = DRoot::getInstance()->globalsettings();
    // TODO implement
    if (globalSettings->spyMode() == "enforceTrue")
        return true;
    return false;
}

void DBus::shutDown()
{
    m_canAccess->stopBus();
}

}