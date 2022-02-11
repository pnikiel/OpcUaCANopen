#include <map>

#include <EmergencyErrors.hpp>

namespace CANopen
{

static std::map<uint8_t, std::string> ErrorCodeMsbCANopen = 
{
    {0x00, "Error Reset or No Error"},
    {0x10, "Generic Error"}
};

std::string decodeEmergencyHumanFriendly(const CanMessage& msg)
{

    return "?";
}

}