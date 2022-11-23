#include <LogIt.h>
#include <CobidMessageRouter.hpp>

#include <fort.hpp>

namespace CANopen
{

void CobidCoordinator::addEntry(const CobidEntry& entry)
{
    if (m_cobids.count(entry.cobid) > 0)
    {
        LOG(Log::ERR) << "The cobid " << entry.cobid << " cant be registered -- the slot is already taken";
    }
    m_cobids[entry.cobid] = entry;
}

void CobidCoordinator::printDiagnostics()
{
    fort::char_table table;
    table.set_border_style(FT_BOLD_STYLE);

    table << fort::header << "cobid" << "node" << fort::endr;

    for (auto& x : m_cobids)
    {
        table << x.first << fort::endr;
    }
    std::cout << table.to_string() << std::endl;
}

}