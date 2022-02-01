#include <LogIt.h>

#include <NodeStateEngine.hpp>
#include <FrameFactory.hpp>

#include <Logging.hpp>


using namespace Logging;

namespace CANopen
{

StateInfoModel stateInfoModelFromText(const std::string &text)
{
    if (text == "NodeGuard")
        return NODEGUARDING;
    else if (text == "HeartBeat")
        return HEARTBEAT;
    else
        throw std::logic_error("enum invalid [" + text + "]");
}

NodeStateEngine::NodeStateEngine(
        uint8_t nodeId,
        StateInfoModel stateInfoModel, 
        float stateInfoPeriodSeconds, 
        NodeState initialRequestedState,
        const std::string& nodeAddressForDebug,
        unsigned int nodeGuardingReplyTimeout,
        MessageSendFunction messageSendFunction):
    m_nodeId(nodeId),
    m_requestedStateEnum(initialRequestedState),
    m_nodeAddressForDebug(nodeAddressForDebug),
    m_stateInfoModel(stateInfoModel),
    m_currentStateInfoPeriod(stateInfoPeriodSeconds),
    m_nodeGuardingOperationsState(IDLE),
    m_previousState(CANopen::NodeState::UNKNOWN),
    m_nodeGuardingReplyTimeoutMs(nodeGuardingReplyTimeout),
    m_messageSendFunction(messageSendFunction)
{
    if (nodeGuardingReplyTimeout > stateInfoPeriodSeconds)
        throw std::runtime_error("NodeGuarding Reply Timeout *must be* smaller that NodeGuarding interval");
    this->addNodeStateNotification([this](uint8_t rawState, NodeState state){this->evaluateStateChange(state);});
}

void NodeStateEngine::tick()
{
    // TODO: must be synchronized with other things that might affect the state.
    LOG(Log::TRC, "NodeMgmt") << wrapId(m_nodeAddressForDebug) << " (tick)";

    std::lock_guard<std::mutex> lock (m_accessLock);

    if (m_stateInfoModel == NODEGUARDING)
        tickNodeGuarding();
    else
        throw std::runtime_error("not-implemented-yet");

}

//! Assuming we're synchronized already from the parent function.
void NodeStateEngine::tickNodeGuarding()
{
    // Feature FN1.1
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    unsigned int millisecondsSinceLastNgRequest = std::chrono::duration_cast<std::chrono::milliseconds> (now - m_lastNodeGuardingTimePoint).count();
    // Nodes non-replying to NG requests, notice the timeout.

    checkNodeGuardingTimeout(millisecondsSinceLastNgRequest);

    if (millisecondsSinceLastNgRequest >= m_currentStateInfoPeriod * 1000)
    {
        LOG(Log::TRC, "NodeMgmt") << wrapId(m_nodeAddressForDebug) << " Sending NG request";
        m_messageSendFunction(CANopen::makeNodeGuardingRequest(m_nodeId));
        m_nodeGuardingOperationsState = CANopen::NodeGuardingOperationsState::AWAITING_REPLY;
        m_lastNodeGuardingTimePoint = now;
    }
}

void NodeStateEngine::checkNodeGuardingTimeout(unsigned int millisecondsSinceLastNgRequest)
{
    if (m_nodeGuardingOperationsState == CANopen::NodeGuardingOperationsState::AWAITING_REPLY &&
        millisecondsSinceLastNgRequest >= m_nodeGuardingReplyTimeoutMs)
    {
        m_nodeGuardingOperationsState = CANopen::NodeGuardingOperationsState::IDLE;
        
        LOG(Log::TRC, "NodeMgmt") << wrapId(m_nodeAddressForDebug) << " Timeout for NodeGuarding reply. " << 
            wrapValue(std::to_string(millisecondsSinceLastNgRequest)) << "ms elapsed since last NG request.";
        
        notifyState(1, NodeState::DISCONNECTED);
    }
}

//! This gets called when state information was obtained.
//! The notified state might or might not be different from "current state"
void NodeStateEngine::notifyState(uint8_t rawState, CANopen::NodeState state)
{
    for (NodeStateNotification& notification : m_nodeStateNotifications)
        notification(rawState, state);
}

void NodeStateEngine::onNodeManagementReplyReceived (const CanMessage& msg)
{
    LOG(Log::TRC, "NodeMgmt") << wrapId(m_nodeAddressForDebug) << " NodeManagement reply received (NG, HB or bootup)";

    std::lock_guard<std::mutex> lock (m_accessLock);

    // TODO: Heartbeat vs NodeGuarding

    if (msg.c_dlc != 1)
    {
        SPOOKY(m_nodeAddressForDebug) << " CANopen protocol violation, received NMT reply and the data size is " << wrapValue(std::to_string(msg.c_dlc)) << 
            " (should be 1)" << SPOOKY_;
        return;
    }
    if (msg.c_data[0] == 0)
        this->onBootupReceived(msg);
    else
    {
        if (m_nodeGuardingOperationsState != CANopen::NodeGuardingOperationsState::AWAITING_REPLY)
        {
            SPOOKY(m_nodeAddressForDebug) << "unsolicited(!!) NG reply is coming. Discarding." << SPOOKY_;
            return;
        }
        else
            m_nodeGuardingOperationsState = CANopen::NodeGuardingOperationsState::IDLE; // we're awaiting reply, so now we got it.

        // Feature FN1.1.2
        // TODO implement actual NG reply logic

        // TODO toggle bit checking
        
        uint8_t stateNoToggle = msg.c_data[0] & 0x7f;
        CANopen::NodeState currentState = CANopen::noToggleNgReplyToStateEnum(stateNoToggle);
        notifyState(msg.c_data[0], currentState);
        
        if (stateNoToggle != m_requestedStateEnum)  // maybe compare actual states rather than uint8_t with an enum // TODO IMPORTANY
        {
            LOG(Log::INF, "NodeMgmt") << "For node " << wrapId(m_nodeAddressForDebug) << " state mismatch is seen; current state is " 
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

            m_messageSendFunction(CANopen::makeNodeManagementServiceRequest(m_nodeId, nmtCommand));
            LOG(Log::INF, "NodeMgmt") << "For node " << wrapId(m_nodeAddressForDebug) << " sent NMT service request to change state, CS was " << 
                wrapValue(std::to_string((int)nmtCommand));

        }
    }


}

void NodeStateEngine::onBootupReceived (const CanMessage& msg)
{
    if (m_bootUpNotification)
        m_bootUpNotification();
}

void NodeStateEngine::evaluateStateChange (NodeState newState)
{
    if (newState != m_previousState)
    {
        if (m_previousState != CANopen::NodeState::UNKNOWN)
        {
            LOG(Log::INF, "NodeMgmt") << "For node " << wrapId(m_nodeAddressForDebug) << " state change was seen, last known state was "
                << wrapValue(CANopen::stateEnumToText(m_previousState)) << " current is " << wrapValue(CANopen::stateEnumToText(newState));
            for (CANopen::NodeStateChangeCallBack callBack : m_nodeStateChangeCallBacks)
                callBack(m_previousState, newState);
        }
        m_previousState = newState;
    }
}

}