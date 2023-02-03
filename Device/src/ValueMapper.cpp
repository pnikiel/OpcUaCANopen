#include <ValueMapper.h>

using namespace Enumerator::ExtractedValue;

template<typename T>
//! "primitive cast" is because we don't do any byte-order manipulation here
static T bytesAsTypePrimitiveCast(const uint8_t* bytes, size_t size, size_t offset)
{
    if (offset + sizeof (T) > size)
        throw std::runtime_error("message too short [" + std::to_string(size) + "] to perform value extraction");
    return *(T*)&bytes[offset]; // TODO maybe improve it to prevent from alignment issues ?
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
    Enumerator::ExtractedValue::DataType dataTypeAsEnum (dataTypeFromString(dataType));
    return extractFromBytesIntoVariant(bytes, size, dataTypeAsEnum, offset, booleanFromBit);
}

UaVariant ValueMapper::extractFromBytesIntoVariant (
    const uint8_t* bytes, 
    size_t size,
    Enumerator::ExtractedValue::DataType dataType, 
    unsigned int offset, 
    const std::string& booleanFromBit)
{
    UaVariant output;
    switch(dataType)
    {
        case Boolean:
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
            break;
        case Byte:
            {
                if (offset > size)
                    throw std::runtime_error("message too short [" + std::to_string(size) + "] to perform value extraction");
                output.setByte(bytes[offset]);
            }
            break;
        case Int16:
            output.setInt16(bytesAsTypePrimitiveCast<int16_t>(bytes, size, offset));
            break;
        case UInt16:
            output.setUInt16(bytesAsTypePrimitiveCast<uint16_t>(bytes, size, offset));
            break;
        case Int24:
            {
                if (offset + 3 > size)
                    throw std::runtime_error("message too short [" + std::to_string(size) + "] to perform value extraction");
                OpcUa_Int32 x (0);
                x = bytes[offset] | (bytes[offset+1] << 8) | (bytes[offset+2] << 16); // LSB first
                // below: twos-complement decoding. Shitty software engineers think they can remove this line with no consequences. Greetings from Piotr.
                x = (x << 8) >> 8; 
                output.setInt32(x);
            };
            break;
        case UInt24:
            {
                if (offset + 3 > size)
                    throw std::runtime_error("message too short [" + std::to_string(size) + "] to perform value extraction");
                OpcUa_UInt32 x(0);
                x = bytes[offset] | (bytes[offset+1] << 8) | (bytes[offset+2] << 16); // LSB first
                output.setUInt32(x);
            }
            break;
        case Int32:
            output.setInt32(bytesAsTypePrimitiveCast<int32_t>(bytes, size, offset));
            break;
        case UInt32:
            output.setUInt32(bytesAsTypePrimitiveCast<uint32_t>(bytes, size, offset));
            break;
        case Float:
            output.setFloat(bytesAsTypePrimitiveCast<float>(bytes, size, offset));
            break;
        default:
            throw std::logic_error("Unknown extraction dataType [" + dataTypeToString(dataType) + "]");
    }
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