#pragma once

// @author pnikiel
// @created 27-Jan-2022

#include <string>
#include <stdexcept>

#include <Definitions.hpp>

namespace CANopen
{

    //! Represents the configuration of given CANopen node on how we obtain its state.
    enum StateInfoModel
    {
        NODEGUARDING,
        HEARTBEAT
    };

    StateInfoModel stateInfoModelFromText(const std::string &text);

    class NodeStateEngine
    {
    public:
        NodeStateEngine(StateInfoModel stateInfoModel, float stateInfoPeriodSeconds, const std::string& nodeAddressForDebug);

        //! Expected to be called by the stack somehow regularly (every second).
        void tick();

        //! This is to control the state setpoint, i.e. the desired state of this node.
        void setRequestedState(NodeState requestedState);

    private:
        const std::string m_nodeAddressForDebug;
    };

}