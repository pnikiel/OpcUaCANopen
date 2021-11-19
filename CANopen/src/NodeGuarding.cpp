#include <stdexcept>

#include <LogIt.h>

#include <NodeGuarding.hpp>

namespace CANopen

{

// TODO propose a map for it?
std::string stateToText (uint8_t state)
{
    switch (state)
    {
        case 0: return "BOOTUP";
        case 4: return "STOPPED";
        case 5: return "OPERATIONAL";
        case 127: return "PREOPERATIONAL";
        default:
            LOG(Log::WRN, "Spooky") << "Parsing invalid state: " << (int)state;
            return "INVALID_STATE(" + std::to_string(state) + ")";

    }
}

uint8_t textToState (const std::string& text)
{
    if (text == "STOPPED")
        return 4;
    else if (text == "OPERATIONAL")
        return 5;
    else if (text == "PREOPERATIONAL")
        return 127;
    else
        throw std::out_of_range("Illegal value");
}

}