#include <map>
#include <sstream>
#include <iomanip>

#include <EmergencyErrors.hpp>

namespace CANopen
{

static std::map<uint8_t, std::string> ErrorCodeMsbCANopen = 
{
    {0x00, "Error Reset or No Error"},
    {0x10, "Generic Error"}
};

bool tryDecodeElmbIoEmergency (const CanMessage& msg, std::string& output)
{
    std::stringstream ss;
    // TODO: guarantee minimum length of the msg ;-)
    // This info comes from Henk's ELMBio doc, see the section about emergency errors.
    uint16_t errorCode = msg.c_data[0] | (msg.c_data[1] << 8);
    if (errorCode == 0x8100)
    {
        ss << "CAN communication (error ctr [" << 
            (unsigned int)msg.c_data[5] << "] bus-off ctr [" << 
            (unsigned int)msg.c_data[6] << "])";
        output = ss.str();
        return true;
    }
    else if (errorCode == 0x8110)
    {
        output = "CAN buffer overrun (CAN message buffer in RAM full: at least 1 message was lost)";
        return true;
    }
    else if (errorCode == 0x8130)
    {
        output = "Life Guarding time-out (CAN-controller has been reinitialized)";
        return true;
    }
    else if (errorCode == 0x8210)
    {
        output = "RPDO: too few bytes (minimum is " + std::to_string((unsigned int)msg.c_data[3]) + ")";
        return true;
    }
    else if (errorCode == 0x5000)
    {
        if (msg.c_data[3] == 0x01)
        {
            output = "ADC: conversion timeout (channel #" + std::to_string((unsigned int)msg.c_data[4]) + ")";
            return true;
        }
        else if (msg.c_data[3] == 0x02)
        {
            output = "ADC: reset failed (error id  " + std::to_string((unsigned int)msg.c_data[5]) + ")";
            return true;
        }
        else if (msg.c_data[3] == 0x03)
        {
            output = "ADC: offset calibration failed";
            return true;
        }
        else if (msg.c_data[3] == 0x04)
        {
            output = "ADC: gain calibration failed";
            return true;
        }
        else if (msg.c_data[3] == 0x10)
        {
            output = "ADC problem(s) during initialisation";
            return true;
        }
        else if (msg.c_data[3] == 0x11)
        {
            output = "ADC calibration constants: not available";
            return true;
        }
        else if (msg.c_data[3] == 0x20)
        {
            output = "Slave processor not responding (ELMB103 only)";
            return true;
        }
        else if (msg.c_data[3] == 0x30)
        {
            output = "CRC error (Byte4 is " + std::to_string((unsigned int)msg.c_data[4]) + ")";
            return true;
        }
        else if (msg.c_data[3] == 0x41)
        {
            ss << "EEPROM: write error (Param block idx [" << 
                (unsigned int)msg.c_data[4] << "] Byte5 [" << 
                (unsigned int)msg.c_data[5] << "])";
            output = ss.str();
            return true;
        }
        else if (msg.c_data[3] == 0x42)
        {
            ss << "EEPROM: read error (Param block idx [" << 
                (unsigned int)msg.c_data[4] << "] Error_id [" << 
                (unsigned int)msg.c_data[5] << "])";
            output = ss.str();
            return true;
        }
        else if (msg.c_data[3] == 0x43)
        {
            ss << "EEPROM: ADC-limits write error (Param block idx [" << 
                (unsigned int)msg.c_data[4] << "] size [" << 
                (unsigned int)msg.c_data[5] << "])";
            output = ss.str();
            return true;
        }
        else if (msg.c_data[3] == 0xF0)
        {
            ss << "Irregular reset (MCUCSR [" << std::hex << std::setw(2) << std::setfill('0') << msg.c_data[4] << "])"; 
            output = ss.str();
            return true;
        }
        else if (msg.c_data[3] == 0xF1)
        {
            output = "Bootloader: not present";
            return true;
        }
        else if (msg.c_data[3] == 0xFE)
        {
            output = "Bootloader is now in control (This is *NORMAL* for an ELMB which boots up)";
            return true;
        }
        // else if (msg.c_data[3] == 0xFE)    ERROR CODE 0x6000 perhaps -- checking with Henk TODO
        // {
        //     output = "Bootloader is now in control";
        //     return true;
        // }
        return false;
    }
    return false;
}

std::string decodeEmergencyHumanFriendly(const CanMessage& msg, Enumerator::Node::EmergencyMappingModel emergencyMappingModel)
{
    std::string output;
    if (emergencyMappingModel == Enumerator::Node::EmergencyMappingModel::ELMBio) // ELMBio has precedence as it overlays the CANopen model
    {
        if (tryDecodeElmbIoEmergency(msg, output))
            return output;
        else
            0; // TODO: decode CANopen

    }
    return "?";
}

}