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

    void addEntry(const CobidEntry& entry);
    void registerCobid (
        Cobid cobid,
        const std::string& node,
        const std::string& function,
        CobidReceiver receiver = CobidReceiver() );

    void printDiagnostics();
    void dispatch(const CanMessage& msg);
private:
    std::map<Cobid, CobidEntry> m_cobids;
};

}