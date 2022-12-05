#pragma once

#include <string>
#include <stdexcept>

#include <CanMessage.h>


namespace Common
{

std::string CanMessageToString(const CanMessage &msg);

class ConfigurationError: public std::runtime_error
{
    public:
    ConfigurationError (const std::string& what): std::runtime_error(what) {}
};

#define throw_config_error_with_origin(MSG) throw Common::ConfigurationError(std::string("At ")+__FILE__+":"+Utils::toString(__LINE__)+" "+MSG)

}