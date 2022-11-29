#include <LogIt.h>
#include <Utils.h>
#include <CobidMessageRouter.hpp>
#include <Logging.hpp>
#include <fort.hpp>

using namespace Logging;

namespace CANopen
{

// TODO universal cobid entry operator?

void CobidCoordinator::addEntry(const CobidEntry& entry)
{
    if (m_cobids.count(entry.cobid) > 0)
    {
        LOG(Log::ERR) << "Registering hint: the pre-existing slot is " <<
            m_cobids[entry.cobid].node << "," << m_cobids[entry.cobid].function << "   new is "
            << entry.node << "," << entry.function;
        throw_runtime_error_with_origin("The cobid [0x" + Utils::toHexString(entry.cobid) + "] cant be registered -- the slot is already taken"); 
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
    std::cout << table.to_string() << std::endl;
}

void CobidCoordinator::dispatch(const CanMessage& msg)
{
    try
    {
        CobidEntry& entry = m_cobids.at(msg.c_id);
        if (entry.receiver)
            entry.receiver(msg);
        else
            LOG(Log::INF) << "Leaking a Cobid receiver for cobid " << Utils::toHexString(msg.c_id);
    }
    catch (const std::out_of_range& ex)
    {
        SPOOKY("TODO BUS NAME") << "Unknown cobid" << SPOOKY_ << " [0x" << Utils::toHexString(msg.c_id) << "]"; // TODO
        return;
    }
}

}