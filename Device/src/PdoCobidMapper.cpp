#include <stdexcept>

#include <boost/regex.hpp>

#include <PiotrsUtils.h>
#include <PdoCobidMapper.h>

static boost::regex baseCobidSpec ("^base_cobid:0x([0-9A-F]{1,4})$");

uint16_t PdoCobidMapper::tpdoSelectorToBaseCobid(const std::string& tpdoSelector)
{
   
    if (tpdoSelector == "1")
        return 0x180;
    else if (tpdoSelector == "2")
        return 0x280;
    else if (tpdoSelector == "3")
        return 0x380;
    else if (tpdoSelector == "4")
        return 0x480;
    else
    {
        // Feature clause FP1.1.1: Support for non-standard Cobid mappings
        boost::smatch matchResults;
        bool matched = boost::regex_match( tpdoSelector, matchResults, baseCobidSpec );
        if (matched)
        {
            uint16_t baseCobid = std::stol(matchResults[1], nullptr, /*hex*/16);
            return baseCobid;
        }
        else /* no other option- God knows that that is */
            throw_config_error_with_origin("unsupported specification of the TPDO selector ["+tpdoSelector+"]");
    }
        
}

uint16_t PdoCobidMapper::rpdoSelectorToBaseCobid(const std::string& rpdoSelector)
{
    if (rpdoSelector == "1")
        return 0x200;
    else if (rpdoSelector == "2")
        return 0x300;
    else if (rpdoSelector == "3")
        return 0x400;
    else if (rpdoSelector == "4")
        return 0x500;
    else
    {
        // TODO: merge that with the function above
         // Feature clause FP1.1.1: Support for non-standard Cobid mappings
        boost::smatch matchResults;
        bool matched = boost::regex_match( rpdoSelector, matchResults, baseCobidSpec );
        if (matched)
        {
            uint16_t baseCobid = std::stol(matchResults[1], nullptr, /*hex*/16);
            return baseCobid;
        }
        else /* no other option- God knows that that is */
            throw std::runtime_error("unsupported specification of the RPDO selector ["+rpdoSelector+"]");
    }    
}