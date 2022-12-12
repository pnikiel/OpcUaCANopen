#pragma once

#include <string>
#include <stdexcept>

#include <CanMessage.h>
#include <Utils.h>

namespace Common
{

std::string CanMessageToString(const CanMessage &msg);


 //TODO these to be isolated away in some exception coding, etc.
class ConfigurationError: public std::runtime_error
{
    public:
    ConfigurationError (const std::string& what): std::runtime_error(what) {}
};

#define throw_config_error_with_origin(MSG) throw Common::ConfigurationError(std::string("At ")+__FILE__+":"+Utils::toString(__LINE__)+" "+MSG)

// Note @piotr: this obviously is LSB/MSB specific
template<typename T>
static T bytesAsTypeNativeAlignSafeCast(const uint8_t* bufferStart, size_t bufferSize, size_t offset)
{  
    if (offset + sizeof (T) > bufferSize)
        throw std::runtime_error("input too short [" + std::to_string(bufferSize) + "] to perform aligned cast");
    T output;
    std::copy(
        bufferStart + offset,
        bufferStart + offset + sizeof (T),
        (uint8_t*)&output
    );
    return output;
}

}