#include <stdexcept>

#include <LogIt.h>

#include <NodeGuarding.hpp>

namespace CANopen

{

// TODO propose a map for it?
std::string stateEnumToText (NodeState state)
{
    switch (state)
    {
        case NodeState::CONNECTING: return "CONNECTING";
        case NodeState::DISCONNECTED: return "DISCONNECTED";
        case NodeState::INITIALISING: return "INITIALISING";
        case NodeState::OPERATIONAL: return "OPERATIONAL";
        case NodeState::PREOPERATIONAL: return "PREOPERATIONAL";
        case NodeState::PREPARING: return "PREPARING";
        case NodeState::STOPPED: return "STOPPED";
        case NodeState::UNKNOWN: return "UNKNOWN";
        default:
            LOG(Log::WRN, "Spooky") << "Parsing invalid state: " << (int)state;
            return "INVALID_STATE(" + std::to_string(state) + ")";

    }
}

NodeState textToStateEnum (const std::string& text)
{
    if (text == "STOPPED")
        return NodeState::STOPPED;
    else if (text == "OPERATIONAL")
        return NodeState::OPERATIONAL;
    else if (text == "PREOPERATIONAL")
        return NodeState::PREOPERATIONAL;
    else
        throw std::out_of_range("Illegal value");
}

NodeState noToggleNgReplyToStateEnum (uint8_t noToggleReply)
{
    // Note from Piotr: these states mostly conform to the NodeState enum states, 
    // but because they come from external world -- "CAN cable" -- I do not want
    // any "casting" without checking what's actually there.

    switch (noToggleReply) // State defs come from Henk's paper
    {
        case 0: return NodeState::INITIALISING;
        case 1: return NodeState::DISCONNECTED;
        case 2: return NodeState::CONNECTING;
        case 3: return NodeState::PREPARING;
        case 4: return NodeState::STOPPED;
        case 5: return NodeState::OPERATIONAL;
        case 127: return NodeState::PREOPERATIONAL;
        default: throw std::out_of_range("NodeGuarding reply state of " + std::to_string(noToggleReply) + " is considered invalid.");
    }
}

}