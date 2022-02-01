#pragma once

// @author pnikiel
// @created 27-Jan-2022

#include <string>
#include <stdexcept>

#include <Definitions.hpp>
#include <NodeGuarding.hpp>

namespace CANopen
{

    //! Represents the configuration of given CANopen node on how we obtain its state.
    enum StateInfoModel
    {
        NODEGUARDING,
        HEARTBEAT
    };

    StateInfoModel stateInfoModelFromText(const std::string &text);

    typedef std::function<void (uint8_t rawState, NodeState state)> NodeStateNotification;

    class NodeStateEngine
    {
    public:
        //! stateInfoPeriod is NG polling for NodeGuarding and the worst HB interval for HeartBeat
        NodeStateEngine(
            uint8_t nodeId,
            StateInfoModel stateInfoModel,
            float stateInfoPeriodSeconds,
            NodeState initialRequestedState,
            const std::string& nodeAddressForDebug,
            unsigned int nodeGuardingReplyTimeout,
            MessageSendFunction messageSendFunction);

        //! Expected to be called by the stack somehow regularly (every second).
        void tick();

        //! This is to control the state setpoint, i.e. the desired state of this node.
        void setRequestedState(NodeState requestedState);

        void setLoggingName(const std::string& name) { m_nodeAddressForDebug = name; }

        void setStateInfoPeriod (float seconds);

        void addNodeStateNotification (NodeStateNotification notification) { m_nodeStateNotifications.push_back(notification); }

        void onNodeManagementReplyReceived (const CanMessage& msg);
        void onBootupReceived (const CanMessage& msg);
        void evaluateStateChange (NodeState newState);

    private:
        const uint8_t m_nodeId;
        const StateInfoModel m_stateInfoModel;
        float m_currentStateInfoPeriod;

        std::string m_nodeAddressForDebug;

        //! Triggers to the FSM should be synchronized.
        std::mutex m_accessLock;

        std::chrono::steady_clock::time_point m_lastNodeGuardingTimePoint;
        CANopen::NodeGuardingOperationsState m_nodeGuardingOperationsState;
        CANopen::NodeState m_previousState;
        CANopen::NodeState m_requestedStateEnum;

        unsigned int m_nodeGuardingReplyTimeoutMs;

        MessageSendFunction m_messageSendFunction;

        void tickNodeGuarding();
        void checkNodeGuardingTimeout(unsigned int millisecondsSinceLastNgRequest);

        //! This gets called when state information was obtained.
        void notifyState(uint8_t rawState, CANopen::NodeState state);

        std::vector<NodeStateNotification> m_nodeStateNotifications;
        std::vector<NodeStateChangeCallBack>  m_nodeStateChangeCallBacks;

    };

}