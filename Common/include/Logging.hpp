#pragma once

#include <string>
#include <Utils.h>

namespace Logging
{

std::string wrapValue (const std::string& value);
std::string wrapId (const std::string& id);
}

#define SPOOKY(ID) LOG(Log::WRN, "Spooky") << Logging::wrapId((ID)) << " \033[41;37m"
#define SPOOKY_ Quasar::TermColors::StyleReset()

