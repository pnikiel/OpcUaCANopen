#pragma once

#include <string>

#include <CanMessage.h>

namespace Common
{

std::string CanMessageToString(const CanMessage &msg);

}