#include <CanMessage.h>
#include <uavariant.h>

#include <Enumerator.hpp>

class ValueMapper
{
public:
    static UaVariant extractFromBytesIntoVariant (
        const uint8_t* bytes, 
        size_t size,
        const std::string& dataType, 
        unsigned int offset, 
        const std::string& booleanFromBit);

    static UaVariant extractFromBytesIntoVariant (
        const uint8_t* bytes, 
        size_t size,
        Enumerator::ExtractedValue::DataType dataType, 
        unsigned int offset, 
        const std::string& booleanFromBit);

    static UaVariant extractFromFrameIntoVariant (
        const CanMessage& msg, 
        const std::string& dataType, 
        unsigned int offset, 
        const std::string& booleanFromBit);

    static std::vector<uint8_t> packVariantToBytes ( // offset and all the rest probably not relevant in this direction
        const UaVariant& data,
        const std::string& dataType
    );

};