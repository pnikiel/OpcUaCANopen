#include <iomanip>

#include <PiotrsUtils.h>

#include <Utils.h>

namespace Common
{

std::string CanMessageToString(const CanMessage &msg)
{
	std::stringstream result;
	result << "[id=0x" << std::hex << msg.c_id << " ";
	if (msg.c_rtr)
		result << "RTR ";
	result << "dlc=" << int(msg.c_dlc) << " data=[";

	for (int i=0; i < msg.c_dlc; i++)
		result << std::hex << std::setw(2) << std::setfill('0') <<  ((unsigned int)msg.c_data[i]) << " ";
	result << "]]";

	return result.str();
}


std::string bytesToHexString (const std::vector<uint8_t>& bytes)
{
    std::stringstream dataAsStr;
    for ( uint8_t byte : bytes)
        dataAsStr << std::setfill('0') << std::setw(2) << std::hex << (unsigned int)byte << ",";
    return dataAsStr.str(); 
} 

}