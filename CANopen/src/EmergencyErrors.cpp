#include <map>

#include <EmergencyErrors.hpp>

namespace CANopen
{

static std::map<uint8_t, std::string> ErrorCodeMsbCANopen = 
{

};

std::string decodeEmergencyHumanFriendly(const CanMessage& msg)
{

    return "?";
}

}