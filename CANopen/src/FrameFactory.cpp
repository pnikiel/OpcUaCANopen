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

CanMessage makeNodeManagementServiceRequest (uint8_t targetId, uint8_t commandSpecifier)
{
    CanMessage nmtServiceRequest;
    nmtServiceRequest.c_id = 0;
    nmtServiceRequest.c_data[0] = commandSpecifier;
    nmtServiceRequest.c_data[1] = targetId;
    nmtServiceRequest.c_dlc = 2;
    return nmtServiceRequest;
}

}