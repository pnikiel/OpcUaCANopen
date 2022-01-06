#include <CanMessage.h>
#include <uavariant.h>

class ValueMapper
{
public:
    static UaVariant extractFromBytesIntoVariant (
        const uint8_t* bytes, 
        size_t size,
        const std::string& dataType, 
        unsigned int offset, 
        const std::string& booleanFromBit);

    static UaVariant extractFromFrameIntoVariant (
        const CanMessage& msg, 
        const std::string& dataType, 
        unsigned int offset, 
        const std::string& booleanFromBit);

};