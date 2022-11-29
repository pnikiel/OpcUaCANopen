#include <stdexcept>

#include <boost/regex.hpp>

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
        // is it the generalized TPDO ?
        boost::smatch matchResults;
        bool matched = boost::regex_match( tpdoSelector, matchResults, baseCobidSpec );
        if (matched)
        {
            uint16_t baseCobid = std::stol(matchResults[1], nullptr, /*hex*/16);
            return baseCobid;
        }
        else /* no other option- God knows that that is */
            throw std::runtime_error("unsupported specification of the TPDO selector ["+tpdoSelector+"]");
    }
        
}