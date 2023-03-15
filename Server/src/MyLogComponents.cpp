#include <MyLogComponents.h>

namespace MyLogComponents
{

Log::LogComponentHandle s_spooky = Log::INVALID_HANDLE;
Log::LogComponentHandle s_spy = Log::INVALID_HANDLE;
Log::LogComponentHandle s_nodemgmt = Log::INVALID_HANDLE;
Log::LogComponentHandle s_nm_tpdo = Log::INVALID_HANDLE; /* non-mux TPDO */
Log::LogComponentHandle s_m_tpdo = Log::INVALID_HANDLE; /* mux TPDO */

Log::LogComponentHandle spooky() { return s_spooky; } 
Log::LogComponentHandle spy() { return s_spy; } 
Log::LogComponentHandle nodemgmt() { return s_nodemgmt; }
Log::LogComponentHandle nm_tpdo() { return s_nm_tpdo; } 
Log::LogComponentHandle m_tpdo() { return s_m_tpdo; }

}