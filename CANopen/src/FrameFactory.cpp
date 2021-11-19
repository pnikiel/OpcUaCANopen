#include <FrameFactory.hpp>

namespace CANopen
{

CanMessage makeNodeGuardingRequest (uint8_t targetId)
{
    CanMessage ngRequestMessage;
    ngRequestMessage.c_id = 0x700 + targetId;
    ngRequestMessage.c_rtr = true;
    return ngRequestMessage;
}

}