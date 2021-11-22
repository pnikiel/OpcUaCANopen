#pragma once

namespace CANopen
{

enum NodeState
{
    INITIALISING = 0,
    DISCONNECTED = 1,
    CONNECTING = 2,
    PREPARING = 3,
    STOPPED = 4,
    OPERATIONAL = 5,
    PREOPERATIONAL = 127,
    // the one below is a pseudo-state only for this piece of software
    UNKNOWN
};



}