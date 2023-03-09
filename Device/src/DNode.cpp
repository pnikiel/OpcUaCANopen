
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


#include <chrono>

#include <Configuration.hxx> // TODO; should go away, is already in Base class for ages

#include <DNode.h>
#include <ASNode.h>

#include <DBus.h>
#include <ASBus.h>

#include <DTpdoMultiplex.h>
#include <DTpdo.h>
#include <DRpdo.h>
#include <DEmergencyParser.h>
#include <DSdoSameIndexGroup.h>
#include <DSdoVariable.h>
#include <DRoot.h>
#include <DGlobalSettings.h>
#include <DWarnings.h>
#include <DOnlineConfigValidator.h>

#include <FrameFactory.hpp>

#include <Logging.hpp>
#include <MyLogComponents.h>

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
DNode::DNode (
    const Configuration::Node& config,
    Parent_DNode* parent
):
    Base_DNode( config, parent),
        m_previousState(CANopen::NodeState::UNKNOWN),
        m_nodeGuardingOperationsState(CANopen::NodeGuardingOperationsState::IDLE),
        m_stateInfoModel(CANopen::stateInfoModelFromText(config.stateInfoSource())),
        m_nodeStateEngine(
            config.id(),
            m_stateInfoModel,
            CANopen::textToStateEnum(config.requestedState()), /* initial requested state */
            getFullName(), /* ID for logging */
            std::bind(&DBus::sendMessage, getParent(), std::placeholders::_1)),
        m_emergencyMappingModel(Enumerator::Node::emergencyMappingModelFromString(config.emergencyMappingModel()))

    /* fill up constructor initialization list here */
{
    /* fill up constructor body here */
    m_nodeStateEngine.addNodeStateNotification([this](uint8_t rawState, CANopen::NodeState state){this->publishState(rawState, state);});
    m_nodeStateEngine.setOnBootupNotification([this](){this->onBootupReceived();});
    m_nodeStateEngine.setStateToggleViolationNotification([this](uint8_t rawState)
    {
        if (DRoot::getInstance()->warningss()[0]->stateToggleViolation())
        {
            SPOOKY(getFullName()) << "NoToggle detected!" << SPOOKY_ << " (state byte is 0x" << wrapValue(Utils::toHexString(rawState)) << ") [-WstateToggleViolation]";
        }

        getAddressSpaceLink()->setStateToggleViolationCounter(
            getAddressSpaceLink()->getStateToggleViolationCounter() + 1, // FN1.1.1: Toggle bit support: Counting part.
            OpcUa_Good); 
    });

    getParent()->addBusErrorNotification([this]()
    {
        // Feature clause: FC3.1: Propagation of CAN port state into affected variables
        this->propagateNullToTpdos();
    
    });

    // FN5.1: Propagation of state transitions into affected variables
    addNodeStateChangeCallBack(
    [this](CANopen::NodeState previous, CANopen::NodeState current)
    {   
        if ((previous == CANopen::NodeState::OPERATIONAL || previous == CANopen::NodeState::STOPPED) && (current == CANopen::NodeState::DISCONNECTED || current == CANopen::NodeState::UNKNOWN))
        {
            this->propagateNullToTpdos();
        }
    });

    // Feature clause FS0.2: SDO engine deactivated for given node
    if (config.hasSdoSupport())
        m_sdoEnginePtr.reset(
            new CANopen::SdoEngine(
                this->id(),
                std::bind(&DBus::sendMessage, getParent(), std::placeholders::_1),
                getParent()->cobidCoordinator()));

}

/* sample dtr */
DNode::~DNode ()
{
}

/* delegates for cachevariables */

/* Note: never directly call this function. */

UaStatus DNode::writeRequestedState ( const UaString& v)
{
    try
    {
        CANopen::NodeState requestedStateEnum = CANopen::textToStateEnum(v.toUtf8());
        m_nodeStateEngine.setRequestedState(requestedStateEnum);
        return OpcUa_Good;
    }
    catch(const std::exception& e)
    {
        LOG(Log::ERR, MyLogComponents::nodemgmt()) << "For node " << wrapId(getFullName()) << " requestedState written by client was out of range " << wrapValue(v.toUtf8());
        return OpcUa_BadOutOfRange;
    }
}


/* ASYNCHRONOUS !! */
UaStatus DNode::writeNmtPartialBackwardsCompat (
    OpcUa_UInt32& value
)
{
    if (value == CANopen::NmtRequests::RESET)
    {
        OpcUa_Boolean bootupReceived;
        return this->callReset(bootupReceived);
    }
    else
    {
        LOG(Log::ERR, MyLogComponents::nodemgmt()) << "For node " << wrapId(getFullName()) << 
            " received address-space request different than RESET (NMT code #" << wrapValue(std::to_string((unsigned int)value)) << 
            "). This is not supported in the new CANopen server. Remove such calls from your applications. Refer to feature FN4.1 in the writeup for more info.";
        return OpcUa_BadUserAccessDenied;
    }
}


/* delegators for methods */
UaStatus DNode::callReset (
    OpcUa_Boolean& bootupReceived
)
{

    LOG(Log::INF, MyLogComponents::nodemgmt()) << "For node " << wrapId(getFullName()) << " received RESET request. Result will be reported"; 
    /* Piotr: I know it'd be *fancier* to solve this problem using e.g. cond variables but I think it would
       complicate it much further with marginally better outcome, while what I propose here seems alright for the application*/ 

    // Feature clause FN3.1: Node reset

    // store before-reset number of bootup messages
    unsigned int bootupsBeforeReset = getAddressSpaceLink()->getBootupCounter();

    auto t0 = std::chrono::steady_clock::now();

    getParent()->sendMessage(CANopen::makeNodeManagementServiceRequest(id(), CANopen::NmtRequests::RESET ));

    bool replyReceived;
    bool timePassed;

    do
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        replyReceived = getAddressSpaceLink()->getBootupCounter() != bootupsBeforeReset;
        timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() > 5000; // TODO configurable?
    } while (!replyReceived && !timePassed);

    bootupReceived = replyReceived;
    LOG(Log::INF, MyLogComponents::nodemgmt()) << "For node " << wrapId(getFullName()) << " after RESET: expected bootup was received? " << wrapValue(replyReceived?"yes":"no");
    return replyReceived? OpcUa_Good : OpcUa_BadTimeout;
}

// 3333333333333333333333333333333333333333333333333333333333333333333333333
// 3     FULLY CUSTOM CODE STARTS HERE                                     3
// 3     Below you put bodies for custom methods defined for this class.   3
// 3     You can do whatever you want, but please be decent.               3
// 3333333333333333333333333333333333333333333333333333333333333333333333333

void DNode::initialize()
{
    m_nodeStateEngine.setLoggingName(getFullName());
    m_nodeStateEngine.setIsInSpyMode(getParent()->isInSpyMode());
    m_nodeStateEngine.setNodeGuardingReplyTimeout(DRoot::getInstance()->globalsettings()->nodeGuardingReplyTimeout());
    if (m_stateInfoModel == CANopen::StateInfoModel::NODEGUARDING)
        m_nodeStateEngine.setStateInfoPeriod(getParent()->getAddressSpaceLink()->getNodeGuardIntervalMs() / 1000.0); // node guarding interval to be in seconds
    else
        m_nodeStateEngine.setStateInfoPeriod(DRoot::getInstance()->globalsettings()->heartBeatWindowSeconds());

    for (DTpdo* tpdo : tpdos())
        tpdo->initialize();

    for (DTpdoMultiplex* tpdoMultiplex : tpdomultiplexs())
        tpdoMultiplex->initialize();

    for (DRpdo* rpdo : rpdos())
        rpdo->initialize();

    if (m_sdoEnginePtr)
        m_sdoEnginePtr->setInSpyMode(getParent()->isInSpyMode()); 

    for (Device::DSdoSameIndexGroup* sdogroup : sdosameindexgroups())
    {
        sdogroup->propagateIndex();
        for (Device::DSdoVariable* variable : sdogroup->sdovariables())
        {
            registerSdoByShortName(sdogroup->name()+"."+variable->name(), variable);
            variable->initialize(getParent(), this);
        }
    }

    for (Device::DSdoVariable* variable : sdovariables())
    {
        if (variable->index() == Configuration::SdoVariable::index_default_value())
            throw std::runtime_error("Stage2 Configuration error: Independent SDO variable needs exact SDO index. (at [" + variable->getFullName() + "]"); // TODO Config error
        registerSdoByShortName(variable->name(), variable);
        variable->initialize(getParent(), this);
    }
    
    for (DOnlineConfigValidator* validator : onlineconfigvalidators())
        validator->initialize();


    LOG(Log::TRC, "SdoValidator") << wrapId(getFullName()) << " SDO short-list established after initialize() is: ";
    for (auto& nameObjectPair : m_sdosByShortName)
        LOG(Log::TRC, "SdoValidator") << wrapValue(nameObjectPair.first);
    
    // register COBids for intrinsics --- this should go to a separate function
    getParent()->cobidCoordinator().registerCobid(0x700 + id(), getFullName(), "StateInformation",
        std::bind(&CANopen::NodeStateEngine::onNodeManagementReplyReceived, &m_nodeStateEngine, std::placeholders::_1));
    getParent()->cobidCoordinator().registerCobid(0x80 + id(), getFullName(), "EmergencyObject",
        std::bind(&DNode::onEmergencyReceived, this, std::placeholders::_1));

    if (m_sdoEnginePtr)
    {
        m_sdoEnginePtr->setNodeFullName(getFullName());
        m_sdoEnginePtr->initialize();
    }

}

void DNode::onBootupReceived ()
{
    // Feature clause: FN2.1: Count bootups
    getAddressSpaceLink()->setBootupCounter( getAddressSpaceLink()->getBootupCounter()+1, OpcUa_Good );
    LOG(Log::INF, MyLogComponents::nodemgmt()) << "For node " << wrapId(getFullName()) << " bootup msg received. Current bootup counter is " 
        << wrapValue(std::to_string(getAddressSpaceLink()->getBootupCounter()));

    // TODO FSM notification that reception of bootup is equivalent of the PREOP state
}

void DNode::onEmergencyReceived (const CanMessage& msg)
{
    emergencyparsers()[0]->onEmergencyReceived(msg);
}


void DNode::tick()
{
    m_nodeStateEngine.tick();
    for (DTpdo* tpdo : tpdos())
        tpdo->tick();
}


void DNode::publishState (uint8_t rawState, CANopen::NodeState state)
{
    // Feature clause FN1.1.2: Presenting node state
    getAddressSpaceLink()->setState(rawState, OpcUa_Good);
    getAddressSpaceLink()->setStateNoToggle(rawState & 0x7f, OpcUa_Good);
    getAddressSpaceLink()->setStateAsText(CANopen::stateEnumToText(state).c_str(), OpcUa_Good);
}

void DNode::notifySync ()
{
    for (DTpdo* tpdo : tpdos())
        tpdo->notifySync();
    for (DTpdoMultiplex* multiplex : tpdomultiplexs())
        multiplex->notifySync();
}

void DNode::registerSdoByShortName (const std::string& shortName, DSdoVariable* sdoVariable)
{
    m_sdosByShortName[shortName] = sdoVariable;
}

DSdoVariable* DNode::getSdoByShortName (const std::string& shortName)
{
    return m_sdosByShortName[shortName];
}

void DNode::propagateNullToTpdos ()
{
    for (DTpdo* tpdo : tpdos())
        tpdo->propagateNull();
    for (DTpdoMultiplex* multiplex : tpdomultiplexs())
        multiplex->propagateNull();
}

void DNode::requestState (CANopen::NodeState state)
{
    //! Going via address-space to keep coherence.
    getAddressSpaceLink()->setRequestedState(CANopen::stateEnumToText(state).c_str(), OpcUa_Good);  
}

}