#pragma once

#include <string>
#include <functional>

#include <Definitions.hpp>

namespace CANopen
{

/** This FSM is Piotr's approach to handle the NG. Not related to anything particular in the CANopen standard */
enum NodeGuardingOperationsState
{
    IDLE = 0,
    AWAITING_REPLY
};

//! Textual repr of the state.
std::string stateEnumToText (NodeState state);

// TODO at a map for consistency? state <-> text representation
//! throws!
NodeState textToStateEnum (const std::string& text);

NodeState noToggleNgReplyToStateEnum (uint8_t noToggleReply);

typedef std::function<void(NodeState previous, NodeState current)> NodeStateChangeCallBack;

}