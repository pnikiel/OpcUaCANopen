#include <map>
#include <sstream>
#include <iomanip>

#include <EmergencyErrors.hpp>

// Feature clause: FE1.2: Emergency errors in human readable way (the whole file!)

namespace CANopen
{

const static std::map<uint8_t, std::string> ErrorCodeMsbCANopen = 
{
    /* Check Henk's CANopen for the source */
    {0x00, "Error Reset or No Error"},
    {0x10, "Generic Error"},

    {0x20, "Current (generic)"},
    {0x21, "Current, device input side"},
    {0x22, "Current, inside the device"},
    {0x23, "Current, device output side"},

    {0x30, "Voltage (generic)"},
    {0x31, "Voltage (mains)"},
    {0x32, "Voltage (inside the device)"},
    {0x33, "Voltage (output)"},


    {0x40, "Temperature (generic)"},
    {0x41, "Temperature (ambient)"},
    {0x42, "Temperature (device)"},

    {0x50, "Device hardware"},

    {0x60, "Device software (generic)"},
    {0x61, "Device software (internal)"},
    {0x62, "Device software (user)"},
    {0x63, "Device software (data set)"},

    {0x70, "Additional modules"},

    {0x80, "Monitoring (generic)"},
    /* 0x81, 0x82 handled as specific subcodes algorithmically below */
    {0x90, "External error"},
    {0xF0, "Additional functions"},
    {0xFF, "Device specific"}

};

bool tryDecodeCANopenEmergency (const CanMessage& msg, std::string& output)
{
    uint8_t errorCodeMsb = msg.c_data[1];
    uint8_t errorCodeLsb = msg.c_data[0];

    try
    {
        output = ErrorCodeMsbCANopen.at(errorCodeMsb);
        return true;
    }
    catch(const std::out_of_range& e)
    {
        if (errorCodeMsb == 0x81)
        {
            switch (errorCodeLsb)
            {
                case 0x10:
                    output = "CAN overrun";
                    return true;
                case 0x20:
                    output = "Error Passive";
                    return true;
                case 0x30:
                    output = "Life Guard Error or Heartbeat Error";
                    return true;
                case 0x40:
                    output = "Recovered from Bus-Off";
                    return true;
                default:
                    return false; // unknown.
            }
        }
        else if (errorCodeMsb == 0x82)
        {
            switch (errorCodeLsb)
            {
                case 0x10:
                    output = "Protocol error: PDO not processed due to length error";
                    return true;
                case 0x20:
                    output = "Protocol error: Length exceeded";
                    return true;
                default:
                    return false; // unknown
            }
        }
        else
            return false;
    }
    
}

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
        else if (tryDecodeCANopenEmergency(msg, output))
            return output;
        else
            return "Unknown code for human-readable resolution";
    }
    else if (emergencyMappingModel == Enumerator::Node::EmergencyMappingModel::CANopen)
    {
        if (tryDecodeCANopenEmergency(msg, output))
            return output;
        else
            return "Unknown code for human-readable resolution in the chosen model";    
    }
    else
        return "The chosen EmergencyMappingModel [" + Enumerator::Node::emergencyMappingModelToString(emergencyMappingModel) + "] is not able to parse the error";
}

}