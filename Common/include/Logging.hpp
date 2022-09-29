#pragma once

#include <string>
#include <Utils.h>
#include <MyLogComponents.h>

namespace Logging
{

std::string wrapValue (const std::string& value);
std::string wrapId (const std::string& id);
}

// old spooky in red: #define SPOOKY(ID) LOG(Log::WRN, "Spooky") << Logging::wrapId((ID)) << " \033[41;37m"
#define SPOOKY(ID) LOG(Log::WRN, MyLogComponents::spooky()) << Logging::wrapId((ID)) << " \033[43;30m"
#define SPOOKY_ Quasar::TermColors::StyleReset()

#define ERROR " \033[41;37m"
#define ERROR_ Quasar::TermColors::StyleReset()