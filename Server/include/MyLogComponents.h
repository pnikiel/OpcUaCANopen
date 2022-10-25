#pragma once

#include <LogIt.h>

namespace MyLogComponents
{

extern Log::LogComponentHandle s_spooky;
extern Log::LogComponentHandle s_spy;
extern Log::LogComponentHandle s_nodemgmt;

Log::LogComponentHandle spooky();
Log::LogComponentHandle spy();
Log::LogComponentHandle nodemgmt();



}