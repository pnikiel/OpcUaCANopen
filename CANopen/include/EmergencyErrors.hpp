#pragma once

#include <CanMessage.h>

#include <Enumerator.hpp>

namespace CANopen
{

std::string decodeEmergencyHumanFriendly(const CanMessage& msg, Enumerator::Node::EmergencyMappingModel emergencyMappingModel);

}