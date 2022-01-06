#include <ValueMapper.h>

template<typename T>
//! "primitive cast" is because we don't do any byte-order manipulation here
static T bytesAsTypePrimitiveCast(const uint8_t* bytes, size_t size, size_t offset)
{
    if (offset + sizeof (T) > size)
        throw std::runtime_error("message too short [" + std::to_string(size) + "] to perform value extraction");
    return *(T*)&bytes[offset];
}

UaVariant ValueMapper::extractFromBytesIntoVariant (
    const uint8_t* bytes, 
    size_t size,
    const std::string& dataType, 
    unsigned int offset, 
    const std::string& booleanFromBit)
{
    UaVariant output;
    if (dataType == "Int32")
        output.setInt32(bytesAsTypePrimitiveCast<int32_t>(bytes, size, offset));
    else if (dataType == "UInt32")
        output.setUInt32(bytesAsTypePrimitiveCast<uint32_t>(bytes, size, offset));
    else if (dataType == "Int16")
        output.setInt16(bytesAsTypePrimitiveCast<int16_t>(bytes, size, offset));
    else if (dataType == "UInt16")
        output.setInt16(bytesAsTypePrimitiveCast<uint16_t>(bytes, size, offset));
    else if (dataType == "Byte")
    {
        if (offset > size)
            throw std::runtime_error("message too short [" + std::to_string(size) + "] to perform value extraction");
        output.setByte(bytes[offset]);
    }
    else if (dataType == "Boolean")
    {
        if (offset > size)
            throw std::runtime_error("message too short [" + std::to_string(size) + "] to perform value extraction");
        unsigned char byte = bytes[offset];
        if (booleanFromBit == "-") // whole word
            output.setBoolean(byte != 0);
        else
        {
            unsigned int bitIndex = std::stoi(booleanFromBit); // TODO: check if we cover all cases!
            output.setBoolean(byte & 1<<bitIndex);
        }
    }
    else
        throw std::logic_error("Unknown extraction dataType [" + dataType + "]");
    return output;
}

UaVariant ValueMapper::extractFromFrameIntoVariant (const CanMessage& msg, const std::string& dataType, unsigned int offset, const std::string& booleanFromBit)
{
    return extractFromBytesIntoVariant(msg.c_data, msg.c_dlc, dataType, offset, booleanFromBit);
}