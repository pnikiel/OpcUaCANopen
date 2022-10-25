#include <MyLogComponents.h>

namespace MyLogComponents
{

Log::LogComponentHandle s_spooky = Log::INVALID_HANDLE;
Log::LogComponentHandle s_spy = Log::INVALID_HANDLE;
Log::LogComponentHandle s_nodemgmt = Log::INVALID_HANDLE;

Log::LogComponentHandle spooky() { return s_spooky; } 
Log::LogComponentHandle spy() { return s_spy; } 
Log::LogComponentHandle nodemgmt() { return s_nodemgmt; } 

}