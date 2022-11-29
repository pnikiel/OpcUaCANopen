#pragma once

#include <string>
#include <stdint.h>

class PdoCobidMapper
{
public:
    
    static uint16_t tpdoSelectorToBaseCobid(const std::string& tpdoSelector);
    static uint16_t rpdoSelectorToBaseCobid(const std::string& rpdoSelector);

};