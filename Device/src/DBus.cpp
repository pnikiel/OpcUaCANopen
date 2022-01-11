
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

#include <Logging.hpp>
#include <FrameFactory.hpp>

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
    sockCanAccess->canMessageCame.connect(std::bind(&DBus::onMessageReceived, this, std::placeholders::_1));
    sockCanAccess->canMessageError.connect(std::bind(&DBus::onError, this, placeholders::_1, placeholders::_2, placeholders::_3));

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



/* delegators for methods */

// 3333333333333333333333333333333333333333333333333333333333333333333333333
// 3     FULLY CUSTOM CODE STARTS HERE                                     3
// 3     Below you put bodies for custom methods defined for this class.   3
// 3     You can do whatever you want, but please be decent.               3
// 3333333333333333333333333333333333333333333333333333333333333333333333333

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
    unsigned int nodeId = msg.c_id & 0x7f;
    // TODO shall everything be passed to specific node
    DNode* node = getNodeById(nodeId);
    if (node)
        node->onMessageReceived(msg);
    else
        SPOOKY(getFullName()) << "received msg suggesting unknown node " << wrapValue(std::to_string(nodeId)) << ", fix your hardware or your configuration." << SPOOKY_;

}

void DBus::sendMessage(const CanMessage& msg)
{
    // TODO handle disable functions ...
    if (this->isInSpyMode())
    {
        LOG(Log::TRC) << wrapId(getFullName()) << " message not sent, bus is in spy mode";
    }
    else
    {
        CanMessage copy (msg);
        m_canAccess->sendMessage(&copy); // TODO: suggest to Michael that sendMessage should be const-ref
    }
}


void DBus::tickSync()
{
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    unsigned int millisecondsSinceLastSync = std::chrono::duration_cast<std::chrono::milliseconds> (now - m_lastSyncTimePoint).count();
    if (millisecondsSinceLastSync >= getAddressSpaceLink()->getSyncIntervalMs())
    {
        sendMessage(CANopen::makeSyncRequest());
        m_lastSyncTimePoint = now;
    }
}

void DBus::onError (const int errorCode, const char* errorDescription, timeval&)
{
    getAddressSpaceLink()->setPortError(errorCode, OpcUa_Good);
    getAddressSpaceLink()->setPortErrorDescription(errorDescription, OpcUa_Good);
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
