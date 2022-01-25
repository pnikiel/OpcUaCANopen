#pragma once

// author @pnikiel
// according to Henk's doc, the frame formats.

#include <cstdint>
#include <CanMessage.h>

namespace CANopen
{
    CanMessage makeNodeGuardingRequest (uint8_t targetId);
    CanMessage makeNodeManagementServiceRequest (uint8_t targetId, uint8_t commandSpecifier);
    CanMessage makeSyncRequest ();
    CanMessage makeTpdoRtr (uint8_t targetId, uint16_t cobid);
}