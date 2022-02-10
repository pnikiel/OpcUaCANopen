#pragma once

#include <CanMessage.h>

namespace CANopen
{

std::string decodeEmergencyHumanFriendly(const CanMessage& msg);

}