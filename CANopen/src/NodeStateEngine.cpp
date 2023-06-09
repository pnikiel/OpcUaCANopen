#include <LogIt.h>

#include <NodeStateEngine.hpp>
#include <FrameFactory.hpp>

#include <Logging.hpp>
#include <PiotrsUtils.h>

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
        NodeState initialRequestedState,
        const std::string& nodeAddressForDebug,
        MessageSendFunction messageSendFunction):
    m_nodeId(nodeId),
    m_stateInfoModel(stateInfoModel),
    m_currentStateInfoPeriod(10), // Expected to be overwritten 
    m_nodeAddressForDebug(nodeAddressForDebug),
    m_nodeGuardingOperationsState(IDLE),
    m_heartBeatWindowStartTimePoint(std::chrono::steady_clock::now()), // some sane init even if HB not used.

    m_requestedStateEnum(initialRequestedState),
    m_previousState(CANopen::NodeState::UNKNOWN),
    m_currentState(NodeState::UNKNOWN),
    m_nodeGuardingReplyTimeoutMs(1000),
    m_messageSendFunction(messageSendFunction),
    m_lastToggleBit(DUNNO),
    m_inSpyMode(false),
    m_stefansNgGraceCounter(0)
{
    // if (m_nodeGuardingReplyTimeout > stateInfoPeriodSeconds)
    //     throw std::runtime_error("NodeGuarding Reply Timeout *must be* smaller that NodeGuarding interval");
    this->addNodeStateNotification([this](uint8_t rawState, NodeState state){this->evaluateStateChange(state);});

    addNodeStateChangeNotification([this](NodeState previous, NodeState current)
    {
        if (previous == NodeState::DISCONNECTED) // we're quitting the DISCONNECTED
            m_lastToggleBit = DUNNO;
    });
}

void NodeStateEngine::tick()
{
    // TODO: must be synchronized with other things that might affect the state.
    LOG(Log::TRC, MyLogComponents::nodemgmt()) << wrapId(m_nodeAddressForDebug) << " (tick)";

    std::lock_guard<std::mutex> lock (m_accessLock);

    if (m_stateInfoModel == NODEGUARDING)
        tickNodeGuarding();
    else if (m_stateInfoModel == HEARTBEAT)
    {
        tickHeartBeat();
    }
    else
        throw std::logic_error("StateInfoModel logic error");

}

//! Assuming we're synchronized already from the parent function.
void NodeStateEngine::tickNodeGuarding()
{
    // Feature FN1.1
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    unsigned int millisecondsSinceLastNgRequest = std::chrono::duration_cast<std::chrono::milliseconds> (now - m_lastNodeGuardingTimePoint).count();
    // Nodes non-replying to NG requests, notice the timeout.

    checkNodeGuardingTimeout(millisecondsSinceLastNgRequest);

    if (!m_inSpyMode && millisecondsSinceLastNgRequest >= m_currentStateInfoPeriod * 1000)
    {
        // Feature clause FN1.1: Node monitoring by node-guarding
        LOG(Log::TRC, MyLogComponents::nodemgmt()) << wrapId(m_nodeAddressForDebug) << " Sending NG request";
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
        
        m_stefansNgGraceCounter++;

        LOG(Log::DBG, MyLogComponents::nodemgmt()) << wrapId(m_nodeAddressForDebug) << " Timeout for NodeGuarding reply. " << 
            wrapValue(std::to_string(millisecondsSinceLastNgRequest)) << "ms elapsed since last NG request. (Stefan's grace counter is " << wrapValue(std::to_string(m_stefansNgGraceCounter)) << ")";
        
        // Feature clause FN1.1.3: Grace for node guarding replies
        if (m_stefansNgGraceCounter > 3)
        {
            m_currentState = NodeState::DISCONNECTED;
            notifyState(1, NodeState::DISCONNECTED);
        }
    }
}

void NodeStateEngine::tickHeartBeat()
{
    unsigned int millisecondsInTheWindow = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::steady_clock::now() - m_heartBeatWindowStartTimePoint).count();
    if (!m_inSpyMode && millisecondsInTheWindow >= m_currentStateInfoPeriod * 1000)
    {
        // Feature FN1.2: Node monitoring by heart-beating
        // Timeout handling.
        m_stefansNgGraceCounter++;
        LOG(Log::TRC, MyLogComponents::nodemgmt()) << wrapId(m_nodeAddressForDebug) << " HB timeout, " <<
            wrapValue(std::to_string(millisecondsInTheWindow)) << "ms in the present window, " <<
            "(Stefan's grace counter is " << wrapValue(std::to_string(m_stefansNgGraceCounter)) << ")";
        // Feature clause FN1.2.1: Grace for heart-beating
        if (m_stefansNgGraceCounter > 3)
        {
            m_currentState = NodeState::DISCONNECTED;
            notifyState(1, NodeState::DISCONNECTED);
        }
        m_heartBeatWindowStartTimePoint = std::chrono::steady_clock::now(); // m_heartBeatWindowStartTimePoint we should rename it to heart beat action or something like this as this will confuse people
    }
}

//! This gets called when state information was obtained.
//! The notified state might or might not be different from "current state"
void NodeStateEngine::notifyState(uint8_t rawState, CANopen::NodeState state)
{
    for (NodeStateNotification& notification : m_nodeStateNotifications)
        notification(rawState, state);
}

// TODO: this also handles NG request in the spy mode
void NodeStateEngine::onNodeManagementReplyReceived (const CanMessage& msg)
{
    // are we in the spy mode ?
    LOG(Log::TRC, MyLogComponents::nodemgmt()) << wrapId(m_nodeAddressForDebug) << " NodeManagement reply received (NG, HB or bootup)";

    std::lock_guard<std::mutex> lock (m_accessLock);

    // TODO: Heartbeat vs NodeGuarding

    if (msg.c_rtr)
    {
        if (m_inSpyMode)
        {   // this is legal.
            m_nodeGuardingOperationsState = CANopen::NodeGuardingOperationsState::AWAITING_REPLY;
            m_lastNodeGuardingTimePoint = std::chrono::steady_clock::now();
        }
        else
        {
            SPOOKY(m_nodeAddressForDebug) << " Received NG *REQUEST*, that is legal only with spymode on. Someone's messing with your bus! " << SPOOKY_;
        }
        return;
    }

    if (msg.c_dlc < 1)
    {
        SPOOKY(m_nodeAddressForDebug) << " CANopen protocol violation, received NMT reply and the data size is " << wrapValue(std::to_string(msg.c_dlc)) << 
            " (should be 1+)" << SPOOKY_;
        return;
    }
    if (msg.c_data[0] == 0)
        this->onBootupReceived(msg);
    else
    {
        /* Does this state at all make any sense? Filter out bullshit replies or noise. */
        uint8_t stateToggled = msg.c_data[0];
        uint8_t stateNoToggle = stateToggled & 0x7f;
        decltype(m_currentState) receivedState;
        try
        {
            receivedState = CANopen::noToggleNgReplyToStateEnum(stateNoToggle);
        }
        catch (const std::out_of_range& e)
        {
            LOG(Log::ERR, MyLogComponents::nodemgmt()) << "For node " << wrapId(m_nodeAddressForDebug) << " received sick/invalid state [" << wrapValue(std::to_string(stateNoToggle)) << "], ignoring.";
            return;
        };
        
        /* We know that the received state makes sense */
        if (m_stateInfoModel == CANopen::StateInfoModel::NODEGUARDING)
        {
            // Feature FN1.1: Node monitoring by node-guarding
            if (m_nodeGuardingOperationsState != CANopen::NodeGuardingOperationsState::AWAITING_REPLY)
            {
                SPOOKY(m_nodeAddressForDebug) << "unsolicited(!!) NG reply is coming. Discarding." << SPOOKY_ << " (the frame was: " << wrapValue(Common::CanMessageToString(msg)) << ")";
                return;
            }
            else
                m_nodeGuardingOperationsState = CANopen::NodeGuardingOperationsState::IDLE; // we're awaiting reply, so now we got it.
        }
        else if (m_stateInfoModel == CANopen::StateInfoModel::HEARTBEAT)
        {
            // Feature FN1.2: Node monitoring by heart-beating
            m_heartBeatWindowStartTimePoint = std::chrono::steady_clock::now();
        }
        else
            throw std::logic_error("State info model seems unsupported");

        m_stefansNgGraceCounter=0;

        // Feature FN1.1.2
        // TODO implement actual NG reply logic

        checkToggleBit(stateToggled);

        m_currentState = receivedState;
        notifyState(msg.c_data[0],   m_currentState);
        
        actOnStateMismatch(receivedState);
    }


}

void NodeStateEngine::checkToggleBit(uint8_t stateToggled)
{
    // Feature clause FN1.1.1: Toggle bit support
    ToggleBit currentToggleBit = stateToggled & 0x80 ? ON : OFF;
    if (m_currentState != NodeState::DISCONNECTED) // DISCONNECTED is the only state where there is no toggling, so we don't check.
    {
        if (m_lastToggleBit != DUNNO)
        {
            if (m_lastToggleBit == currentToggleBit) // no toggle detected!
            {          
                if (m_toggleViolationNotification)
                    m_toggleViolationNotification(stateToggled);
            }
        }
    }
    m_lastToggleBit = currentToggleBit;
}

void NodeStateEngine::actOnStateMismatch(CANopen::NodeState receivedState)
{
    if (!m_inSpyMode && receivedState != m_requestedStateEnum)  // maybe compare actual states rather than uint8_t with an enum // TODO IMPORTANY
    {
        // Feature clause FN1.3: Maintaining state
        LOG(Log::INF, MyLogComponents::nodemgmt()) << "For node " << wrapId(m_nodeAddressForDebug) << " state mismatch is seen; current state is " 
            << wrapValue(CANopen::stateEnumToText(m_currentState)) << " requested is " << wrapValue(CANopen::stateEnumToText(m_requestedStateEnum));
        // send therefore a message for correcting

        uint8_t nmtCommand;

        switch(m_requestedStateEnum)
        {
            case CANopen::NodeState::OPERATIONAL: // to be started
                nmtCommand = NmtRequests::START; break;
            case CANopen::NodeState::STOPPED: // to be stopped
                nmtCommand = NmtRequests::STOP; break; // stop remote node
            case CANopen::NodeState::PREOPERATIONAL: // go to preoperational
                nmtCommand = NmtRequests::GOTO_PREOP; break;
            default: throw std::logic_error("requested state as int is unsupported");
        }

        m_messageSendFunction(CANopen::makeNodeManagementServiceRequest(m_nodeId, nmtCommand));
        LOG(Log::INF, MyLogComponents::nodemgmt()) << "For node " << wrapId(m_nodeAddressForDebug) << " sent NMT service request to change state, CS was " << 
            wrapValue(std::to_string((int)nmtCommand));

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
            LOG(Log::INF, MyLogComponents::nodemgmt()) << "For node " << wrapId(m_nodeAddressForDebug) << " state change was seen, last known state was "
                << wrapValue(CANopen::stateEnumToText(m_previousState)) << " current is " << wrapValue(CANopen::stateEnumToText(newState));
        }
        for (CANopen::NodeStateChangeCallBack callBack : m_nodeStateChangeCallBacks)
            callBack(m_previousState, newState);
        m_previousState = newState;
    }
}

void NodeStateEngine::setRequestedState(NodeState requestedState)
{
    m_requestedStateEnum = requestedState;
}

void NodeStateEngine::setStateInfoPeriod (float seconds)
{
    m_currentStateInfoPeriod = seconds;
}

}