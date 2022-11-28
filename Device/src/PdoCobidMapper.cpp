#include <stdexcept>

#include <PdoCobidMapper.h>

uint16_t PdoCobidMapper::tpdoSelectorToBaseCobid(const std::string& tpdoSelector)
{
    if (tpdoSelector == "1")
        return 0x180;
    else if (tpdoSelector == "3")
        return 0x380;
    else
        throw std::runtime_error("cant map the tpdoSelector"); // what is the selector ?
}