#pragma once

#include <functional>

#include <CanMessage.h>

namespace CANopen
{

typedef std::function<void (const CanMessage& msg)> MessageSendFunction;

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

enum NmtRequests
{
    START = 1,
    STOP = 2,
    GOTO_PREOP = 128,
    RESET = 129
};

}