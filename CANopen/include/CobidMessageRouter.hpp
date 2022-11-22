#include <map>
#include <functional>

namespace CANopen
{

typedef uint16_t Cobid;
typedef std::function<void()> CobidReceiver; 

struct CobidEntry
{
    Cobid cobid;
    CobidReceiver receiver;
    // TODO Info if we're transmitting on it.
};

class CobidCoordinator
{
public:

    void addEntry(const CobidEntry& entry);
    void printDiagnostics();
private:
    std::map<Cobid, CobidEntry> m_cobids;
};

}