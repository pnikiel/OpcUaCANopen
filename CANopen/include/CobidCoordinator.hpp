// @author Piotr Nikiel <piotr@nikiel.info>
// @date 25-Nov-2022

#pragma once

#include <map>
#include <functional>
#include <CanMessage.h>

namespace CANopen
{

typedef uint16_t Cobid;
typedef std::function<void(const CanMessage& msg)> CobidReceiver; 

struct CobidEntry
{
    Cobid cobid;
    std::string node;
    std::string function;
    CobidReceiver receiver;
    // TODO Info if we're transmitting on it.
};

class CobidCoordinator
{
public:
    CobidCoordinator(const std::string& busName);

    void addEntry(const CobidEntry& entry);
    void registerCobid (
        Cobid cobid,
        const std::string& node,
        const std::string& function,
        CobidReceiver receiver = CobidReceiver() );

    void printDiagnostics();
    void dispatch(const CanMessage& msg);

    void setSpookyOnUnknownCobid(bool isOn) { m_spookyOnUnknownCobid=isOn; }
private:
    const std::string m_loggingBusName;
    std::map<Cobid, CobidEntry> m_cobids;
    bool m_spookyOnUnknownCobid;
};

}