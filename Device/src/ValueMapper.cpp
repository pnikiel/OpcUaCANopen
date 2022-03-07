#include <ValueMapper.h>

template<typename T>
//! "primitive cast" is because we don't do any byte-order manipulation here
static T bytesAsTypePrimitiveCast(const uint8_t* bytes, size_t size, size_t offset)
{
    if (offset + sizeof (T) > size)
        throw std::runtime_error("message too short [" + std::to_string(size) + "] to perform value extraction");
    return *(T*)&bytes[offset];
}

template<typename T>
std::vector<uint8_t> typeAsBytesPrimitiveCast(T input)
{
    std::vector<uint8_t> bytes;
    bytes.assign(sizeof input, 0);
    std::copy(
        (uint8_t*)&input,
        (uint8_t*)&input + sizeof input,
        bytes.begin());
    return bytes;
}

UaVariant ValueMapper::extractFromBytesIntoVariant (
    const uint8_t* bytes, 
    size_t size,
    const std::string& dataType, 
    unsigned int offset, 
    const std::string& booleanFromBit)
{
    // TODO Certain data types are missing here, e.g. float.
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

std::vector<uint8_t> ValueMapper::packVariantToBytes ( // offset and all the rest probably not relevant in this direction
    const UaVariant& data,
    const std::string& dataType
)
{
    std::vector<uint8_t> bytes;
    if (dataType == "Int32")
    {
        int32_t out;
        if (data.toInt32(out) != OpcUa_Good)
            throw std::runtime_error("Failed transforming variant to bytes");
        return typeAsBytesPrimitiveCast(out);
    }
    else if (dataType == "UInt32")
    {
        uint32_t out;
        if (data.toUInt32(out) != OpcUa_Good)
            throw std::runtime_error("Failed transforming variant to bytes");
        return typeAsBytesPrimitiveCast(out);
    }
    else if (dataType == "UInt16")
    {
        uint16_t out;
        if (data.toUInt16(out) != OpcUa_Good)
            throw std::runtime_error("Failed transforming variant to bytes");
        return typeAsBytesPrimitiveCast(out);
    }
    else if (dataType == "Byte")
    {
        uint8_t out;
        if (data.toByte(out) != OpcUa_Good)
            throw std::runtime_error("Failed transforming variant to bytes");
        return typeAsBytesPrimitiveCast(out);
    }
    else if (dataType == "Boolean")
    {
        uint8_t out; // OPC-UA boolean is uint8_t
        if (data.toBoolean(out) != OpcUa_Good)
            throw std::runtime_error("Failed transforming variant to bytes");
        bytes.assign(1, (uint8_t)out);
        // TODO: encoding to bits?

    }
    else
        throw std::runtime_error("Datatype is not recognized");
    return bytes;
}