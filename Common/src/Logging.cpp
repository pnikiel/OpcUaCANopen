#include <Logging.hpp>

namespace Logging
{

std::string wrapValue (const std::string& value)
{ 
    return "\033[1m" + value + Quasar::TermColors::StyleReset();
}

std::string wrapId (const std::string& id)
{ 
    return Quasar::TermColors::ForeBlue() + id + Quasar::TermColors::StyleReset();
}

}