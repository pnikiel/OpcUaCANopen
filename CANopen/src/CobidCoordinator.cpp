#include <LogIt.h>
#include <Utils.h>
#include <CobidCoordinator.hpp>
#include <Logging.hpp>
#include <fort.hpp>

#include <PiotrsUtils.h>

using namespace Logging;

namespace CANopen
{

CobidCoordinator::CobidCoordinator (const std::string& busName) :
    m_loggingBusName(busName)
{
}

// TODO universal cobid entry operator?

void CobidCoordinator::addEntry(const CobidEntry& entry)
{
    if (m_cobids.count(entry.cobid) > 0)
    {
        LOG(Log::ERR) << "Registering hint: the pre-existing slot is " <<
            m_cobids[entry.cobid].node << "," << m_cobids[entry.cobid].function << "   new is "
            << entry.node << "," << entry.function;
        throw_config_error_with_origin("The cobid [0x" + Utils::toHexString(entry.cobid) + "] cant be registered -- the slot is already taken"); 
    }
    m_cobids[entry.cobid] = entry;
}

//! This should throw for cobid issues.
void CobidCoordinator::registerCobid (
    Cobid cobid,
    const std::string& node,
    const std::string& function,
    CobidReceiver receiver)
{
    CobidEntry entry;
    entry.cobid = cobid;
    entry.node = node;
    entry.function = function;
    entry.receiver = receiver;
    this->addEntry(entry);
}

void CobidCoordinator::printDiagnostics()
{
    fort::char_table table;
    table.set_border_style(FT_BOLD_STYLE);

    table << fort::header << "cobid [hex]" << "node" << "function" << fort::endr;

    for (auto& x : m_cobids)
    {
        table << Utils::toHexString(x.first) << x.second.node << x.second.function << fort::endr;
    }
    LOG(Log::INF) << "Cobid table for bus [" << m_loggingBusName << "]" << std::endl << table.to_string() << std::endl;
}

std::string wtfIsThisCobid(Cobid cobid)
{
    unsigned int functionCode = cobid >> 7;
    switch(functionCode)
    {
        case 0x0: return "NMT control(another master on the bus?)";
        case 0x1: return "Emergency(node) or SYNC(another master on the bus?)";
        case 0x2: return "TIMESTAMP(not supported)";
        case 0x3: return "TPDO1";
        case 0x4: return "RPDO2";
        case 0x5: return "TPDO2";
        case 0x6: return "RPDO3";
        case 0x7: return "TPDO3";
        case 0x8: return "RPDO4";
        case 0x9: return "TPDO4";
        case 0xa: return "SDO-repl";
        case 0xb: return "SDO-req";
        case 0xe: return "NG/HB repl";
        default: return "-unknown-function-";
    }
}

std::string describeCobid (Cobid cobid)
{
    unsigned int node = cobid & 0x7f;
    return "it might be a [" + wtfIsThisCobid(cobid) + "] from node [" + std::to_string(node) + "]";
}

void CobidCoordinator::dispatch(const CanMessage& msg)
{
    try
    {
        CobidEntry& entry = m_cobids.at(msg.c_id);
        if (entry.receiver)
            entry.receiver(msg);
        else
            LOG(Log::INF) << "Leaking a Cobid receiver for cobid " << Utils::toHexString(msg.c_id); // TODO spy mode fixes
    }
    catch (const std::out_of_range& ex)
    {
        SPOOKY(m_loggingBusName) << "Unknown cobid" << SPOOKY_ << " [0x" << Utils::toHexString(msg.c_id) << "], hint: " << describeCobid(msg.c_id) << "?";
        return;
    }
}

}