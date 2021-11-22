#include <string>
#include <Utils.h>

namespace Logging
{

std::string wrapValue (const std::string& value) { return "\033[1m" + value + Quasar::TermColors::StyleReset(); }
std::string wrapId (const std::string& id) { return Quasar::TermColors::ForeBlue() + id + Quasar::TermColors::StyleReset(); }

#define SPOOKY(ID) LOG(Log::WRN, "Spooky") << wrapId((ID)) << Quasar::TermColors::ForeYellow()

}