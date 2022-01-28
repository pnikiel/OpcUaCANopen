
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
#include <DEmergencyParser.h>

#include <FrameFactory.hpp>

#include <Logging.hpp>

using namespace Logging;

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
        m_requestedStateEnum(CANopen::textToStateEnum(config.requestedState())),
        m_previousState(CANopen::NodeState::UNKNOWN),
        m_nodeGuardingOperationsState(CANopen::NodeGuardingOperationsState::IDLE),
        m_sdoEngine(std::bind(&DBus::sendMessage, getParent(), std::placeholders::_1), config.id()),
        m_nodeStateEngine(CANopen::stateInfoModelFromText(config.stateInfoSource()), 10, getFullName() ) // TODO: fix 10  

    /* fill up constructor initialization list here */
{
    /* fill up constructor body here */
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
        m_requestedStateEnum = CANopen::textToStateEnum(v.toUtf8());
        return OpcUa_Good;
    }
    catch(const std::exception& e)
    {
        LOG(Log::ERR) << "For node " << wrapId(getFullName()) << " written state is out of range, it was: " << wrapValue(v.toUtf8());
        return OpcUa_BadOutOfRange;
    }
}


/* delegators for methods */
UaStatus DNode::callReset (
    OpcUa_Boolean& bootupReceived
)
{
    /* Piotr: I know it'd be *fancier* to solve this problem using e.g. cond variables but I think it would
       complicate it much further with marginally better outcome, while what I propose here seems alright for the application*/ 

    // Feature FN3.1

    // store before-reset number of bootup messages
    unsigned int bootupsBeforeReset = getAddressSpaceLink()->getBootupCounter();

    auto t0 = std::chrono::steady_clock::now();

    getParent()->sendMessage(CANopen::makeNodeManagementServiceRequest(id(), 129 /* RESET */ ));

    bool replyReceived;
    bool timePassed;

    do
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        replyReceived = getAddressSpaceLink()->getBootupCounter() != bootupsBeforeReset;
        timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() > 5000; // TODO configurable?
    } while (!replyReceived && !timePassed);

    bootupReceived = replyReceived;

    return replyReceived? OpcUa_Good : OpcUa_BadTimeout;
}

// 3333333333333333333333333333333333333333333333333333333333333333333333333
// 3     FULLY CUSTOM CODE STARTS HERE                                     3
// 3     Below you put bodies for custom methods defined for this class.   3
// 3     You can do whatever you want, but please be decent.               3
// 3333333333333333333333333333333333333333333333333333333333333333333333333

void DNode::onMessageReceived (const CanMessage& msg)
{
    /* Doc: check table 4 in the Henk's document */
    // TODO impl
    unsigned int functionCode = msg.c_id >> 7;
    switch (functionCode)
    {
        case 0x1:
            onEmergencyReceived(msg); break;

        // TPDOs
        case 0x3: // TPDO1
        case 0x5: // TPDO2
        case 0x7: // TPDO3
        case 0x9: // TPDO4
            onTpdoReceived(msg); break; 

        case 0xb: m_sdoEngine.replyCame(msg); break;
        // various NMts
        case 0xe: onNodeManagementReplyReceived(msg); break;
        
        default:
            SPOOKY(getFullName()) << "Received unintelligible message, fnc code " << wrapValue(std::to_string(functionCode)) << " " << msg.toString() << SPOOKY_;
    }
}

void DNode::onBootupReceived (const CanMessage& msg)
{
    // TODO missing impl

    // TODO logging

    // Feature FN2.1: counting bootups
    getAddressSpaceLink()->setBootupCounter( getAddressSpaceLink()->getBootupCounter()+1, OpcUa_Good );

    // TODO FSM notification that reception of bootup is equivalent of the PREOP state
}

void DNode::onEmergencyReceived (const CanMessage& msg)
{
    emergencyparsers()[0]->onEmergencyReceived(msg);
}

void DNode::onNodeManagementReplyReceived (const CanMessage& msg)
{
    if (msg.c_dlc != 1)
    {
        LOG(Log::WRN, "Spooky") << "CANopen protocol violation, received NMT reply and the data size is " << msg.c_dlc << " (should be 1)"; // TODO which bus which elmb message content 
        return;
    }
    if (msg.c_data[0] == 0)
        this->onBootupReceived(msg);
    else
    {
        if (m_nodeGuardingOperationsState != CANopen::NodeGuardingOperationsState::AWAITING_REPLY)
        {
            SPOOKY(getFullName()) << "unsolicited(!!) NG reply is coming. Discarding." << SPOOKY_;
            return;
        }
        else
            m_nodeGuardingOperationsState = CANopen::NodeGuardingOperationsState::IDLE; // we're awaiting reply, so now we got it.
        // TODO are we actually expecting to get the NG?

        // Feature FN1.1.2
        // TODO implement actual NG reply logic

        // TODO toggle bit checking
        getAddressSpaceLink()->setState(msg.c_data[0], OpcUa_Good);
        uint8_t stateNoToggle = msg.c_data[0] & 0x7f;
        CANopen::NodeState currentState = CANopen::noToggleNgReplyToStateEnum(stateNoToggle);

        // TODO stateNoToggle -> currentState must be improved!!

        getAddressSpaceLink()->setStateNoToggle(stateNoToggle, OpcUa_Good);
        getAddressSpaceLink()->setStateAsText(CANopen::stateEnumToText(currentState).c_str(), OpcUa_Good);

        // identify node change by comparing to the previous state
        // TODO: the NMT state machine would be running from here ;-)
        if (currentState != m_previousState)
        {
            if (m_previousState != CANopen::NodeState::UNKNOWN)
            {
                LOG(Log::INF, "NodeMgmt") << "For node " << wrapId(getFullName()) << " state change was seen, last known state was "
                    << wrapValue(CANopen::stateEnumToText(m_previousState)) << " current is " << wrapValue(CANopen::stateEnumToText(currentState)); // TODO what state into what state?
                // TODO here we should have a list of state changes receivers.
                for (CANopen::NodeStateChangeCallBack callBack : m_nodeStateChangeCallBacks)
                    callBack(m_previousState, currentState);
            }
            // TODO what shall we do?
            // For going to operational, we should send RTRs
            m_previousState = currentState;
        }

        // TODO: disconnected support

        // shall we do something with state management here?
        if (stateNoToggle != m_requestedStateEnum)
        {
            LOG(Log::INF, "NodeMgmt") << "For node " << wrapId(getFullName()) << " state mismatch is seen; current state is " 
                << wrapValue(CANopen::stateEnumToText(currentState)) << " requested is " << wrapValue(CANopen::stateEnumToText(m_requestedStateEnum));
            // send therefore a message for correcting

            uint8_t nmtCommand;

            switch(m_requestedStateEnum)
            {
                case 5: // to be started
                    nmtCommand = 1; break; // start
                case 4: // to be stopped
                    nmtCommand = 2; break; // stop remote node
                case 127: // go to preoperational
                    nmtCommand = 128; break;
                default: throw std::logic_error("requested state as int is unsupported");
            }

            getParent()->sendMessage(CANopen::makeNodeManagementServiceRequest(id(), nmtCommand));
            LOG(Log::INF, "NodeMgmt") << "For node " << wrapId(getFullName()) << " sent NMT service request to change state, CS was " << wrapValue(std::to_string((int)nmtCommand));

        }
    }
}

void DNode::onTpdoReceived (const CanMessage& msg)
{
    unsigned int functionCode = msg.c_id >> 7;
    unsigned int pdoSelector = (functionCode-1) / 2;

    DTpdo* tpdo = getTpdoBySelector(pdoSelector);
    DTpdoMultiplex* multiplex = getTpdoMultiplexBySelector(pdoSelector);
    if (!multiplex && !tpdo)
    {
        SPOOKY(getFullName()) << "received TPDO" << wrapValue(std::to_string(pdoSelector)) << " however no such TPDOs are configured (neither Tpdo nor TpdoMultiplex). Fix your configuration. This is a *GRAVE* configuration problem." << SPOOKY_;
        return;
    }
    if (tpdo)
        tpdo->onReplyReceived(msg);
    if (multiplex) 
        multiplex->onReplyReceived(msg);
    

}

void DNode::tick()
{
    m_nodeStateEngine.tick();

    // Feature FN1.1
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    unsigned int millisecondsSinceLastNgRequest = std::chrono::duration_cast<std::chrono::milliseconds> (now - m_lastNodeGuardingTimePoint).count();
    // Nodes non-replying to NG requests, notice the timeout.

    // TODO: natural condition that the node guarding interval should be bigger than 1000
    if (m_nodeGuardingOperationsState == CANopen::NodeGuardingOperationsState::AWAITING_REPLY && millisecondsSinceLastNgRequest >= 1000)
    {
        // this is an official report that we did not get the reply to the NG
        m_nodeGuardingOperationsState = CANopen::NodeGuardingOperationsState::IDLE;
        // TODO move the current state to unknown
        
        SPOOKY(getFullName()) << "Timeout for NodeGuarding reply." << SPOOKY_; 
    }

    if (millisecondsSinceLastNgRequest >= getParent()->getAddressSpaceLink()->getNodeGuardIntervalMs())
    {
        getParent()->sendMessage(CANopen::makeNodeGuardingRequest(id()));
        m_nodeGuardingOperationsState = CANopen::NodeGuardingOperationsState::AWAITING_REPLY;
        m_lastNodeGuardingTimePoint = now;
    }
}

void DNode::addNodeStateChangeCallBack(CANopen::NodeStateChangeCallBack callBack)
{
    m_nodeStateChangeCallBacks.push_back(callBack);
}

}