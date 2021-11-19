#include <string>

namespace CANopen
{

/** This FSM is Piotr's approach to handle the NG. Not related to anything particular in the CANopen standard */
enum NodeGuardingStates
{
    IDLE = 0,
    AWAITING_REPLY
};

//! Textual repr of the state.
std::string stateToText (uint8_t state);

// TODO at a map for consistency? state <-> text representation
uint8_t textToState (const std::string& text);

}