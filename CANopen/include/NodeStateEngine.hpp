#pragma once

// @author pnikiel
// @created 27-Jan-2022

#include <Definitions.hpp>

namespace CANopen
{

//! Represents the configuration of given CANopen node on how we obtain its state.
enum StateInfoModel
{
    NODEGUARDING,
    HEARTBEAT
};

class NodeStateEngine
{
    NodeStateEngine(StateInfoModel stateInfoModel, float stateInfoPeriodSeconds);

public:
    //! Expected to be called by the stack somehow regularly (every second).
    void tick();

    //! This is to 
    void setRequestedState (NodeState requestedState);

};

}