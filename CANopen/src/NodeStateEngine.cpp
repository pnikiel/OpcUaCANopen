#include <LogIt.h>

#include <NodeStateEngine.hpp>

#include <Logging.hpp>

using namespace Logging;

namespace CANopen
{

StateInfoModel stateInfoModelFromText(const std::string &text)
{
    if (text == "NodeGuard")
        return NODEGUARDING;
    else if (text == "HeartBeat")
        return HEARTBEAT;
    else
        throw std::logic_error("enum invalid [" + text + "]");
}

NodeStateEngine::NodeStateEngine(StateInfoModel stateInfoModel, float stateInfoPeriodSeconds, const std::string& nodeAddressForDebug):
    m_nodeAddressForDebug(nodeAddressForDebug)
{
    // TODO write it.
}

void NodeStateEngine::tick()
{
    // TODO: must be synchronized with other things that might affect the state.
    LOG(Log::TRC, "NodeMgmt") << wrapId(m_nodeAddressForDebug) << " (tick)";
}

}