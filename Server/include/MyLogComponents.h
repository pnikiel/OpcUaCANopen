#pragma once

#include <LogIt.h>

namespace MyLogComponents
{

extern Log::LogComponentHandle s_spooky;
extern Log::LogComponentHandle s_spy;
extern Log::LogComponentHandle s_nodemgmt;
extern Log::LogComponentHandle s_nm_tpdo;

Log::LogComponentHandle spooky();
Log::LogComponentHandle spy();
Log::LogComponentHandle nodemgmt();
Log::LogComponentHandle nm_tpdo();


}