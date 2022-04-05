
/*  © Copyright CERN, 2015. All rights not expressly granted are reserved.

    The stub of this file was generated by quasar (https://github.com/quasar-team/quasar/)

    Quasar is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public Licence as published by
    the Free Software Foundation, either version 3 of the Licence.
    Quasar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public Licence for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Quasar.  If not, see <http://www.gnu.org/licenses/>.


 */


#include <Configuration.hxx> // TODO; should go away, is already in Base class for ages

#include <DRoot.h>
#include <DWarnings.h>
#include <DTpdo.h>
#include <ASTpdo.h>
#include <DNode.h>
#include <DBus.h>

#include <DExtractedValue.h>

#include <Logging.hpp>
#include <FrameFactory.hpp>

using namespace Logging;

namespace Device
{
// 1111111111111111111111111111111111111111111111111111111111111111111111111
// 1     GENERATED CODE STARTS HERE AND FINISHES AT SECTION 2              1
// 1     Users don't modify this code!!!!                                  1
// 1     If you modify this code you may start a fire or a flood somewhere,1
// 1     and some human being may possible cease to exist. You don't want  1
// 1     to be charged with that!                                          1
// 1111111111111111111111111111111111111111111111111111111111111111111111111






// 2222222222222222222222222222222222222222222222222222222222222222222222222
// 2     SEMI CUSTOM CODE STARTS HERE AND FINISHES AT SECTION 3            2
// 2     (code for which only stubs were generated automatically)          2
// 2     You should add the implementation but dont alter the headers      2
// 2     (apart from constructor, in which you should complete initializati2
// 2     on list)                                                          2
// 2222222222222222222222222222222222222222222222222222222222222222222222222

/* sample ctr */
DTpdo::DTpdo (
    const Configuration::Tpdo& config,
    Parent_DTpdo* parent
):
    Base_DTpdo( config, parent),
    m_onSync( config.transportMechanism() == "sync" ),
    m_firstIteration(true),
    m_receivedCtrSinceLastSync (0),
    m_numRtrsInvokedAddressSpace (0)

    /* fill up constructor initialization list here */
{
    /* fill up constructor body here */
    // for the mode of on-sync supports rtr, we should send an RTR each time there is a state change towards preop
    // TODO Mark the feature clause
    if (config.transportMechanism() == "asyncSupportsRtr")
    {
        getParent()->addNodeStateChangeCallBack(
            [this](CANopen::NodeState previous, CANopen::NodeState current)
            {
                if (current == CANopen::NodeState::OPERATIONAL)
                {
                    if (getParent()->getParent()->isInSpyMode())
                    {
                        LOG(Log::DBG, "NodeMgmt") << wrapId(getFullName()) << " would send RTR for state change into OPERATIONAL, but we're in the spy mode.";
                    }
                    else
                    {
                        // Note: Feature clause FP6.2
                        LOG(Log::DBG, "NodeMgmt") << wrapId(getFullName()) << " sending RTR for state change into OPERATIONAL";
                        this->sendRtr();
                    }
                }
            });
    }
}

/* sample dtr */
DTpdo::~DTpdo ()
{
}

/* delegates for cachevariables */


/* ASYNCHRONOUS !! */
UaStatus DTpdo::readInvokeRtr (
    OpcUa_UInt32& value,
    UaDateTime& sourceTime
)
{
    sourceTime = UaDateTime::now();
    value = m_numRtrsInvokedAddressSpace;
    return OpcUa_Good;
}
/* ASYNCHRONOUS !! */
UaStatus DTpdo::writeInvokeRtr (
    OpcUa_UInt32& value
)
{
    // Note: "value" is not relevant; we use this function to invoked action(s) from WinCC OA.
    // Feature clause FP6.2: RTR and initialization of on-change data (invoking on request)
    this->sendRtr(); // TODO If in spy mode, mention it, also catch potential exceptions.
    m_numRtrsInvokedAddressSpace++; // safe increase is assured by quasar mutex on this quasar variable (see Design)
    return OpcUa_Good;
}

/* delegators for methods */

// 3333333333333333333333333333333333333333333333333333333333333333333333333
// 3     FULLY CUSTOM CODE STARTS HERE                                     3
// 3     Below you put bodies for custom methods defined for this class.   3
// 3     You can do whatever you want, but please be decent.               3
// 3333333333333333333333333333333333333333333333333333333333333333333333333

void DTpdo::onReplyReceived(const CanMessage& msg)
{
    std::lock_guard<std::mutex> lock (m_accessLock); // operating on shared data, e.g. m_receivedCtrSinceLastSync

    LOG(Log::TRC) << "received TPDO reply: " << msg.toString(); // TODO: Wrong log component
    m_receivedCtrSinceLastSync++;

    // Feature clause FP2.1: TPDO (Transmit PDOs)
    // Feature clause FP6.1: Non-SYNC traffic
    for (DExtractedValue* extractedValue : extractedvalues())
    {
        extractedValue->onReplyReceived(msg);
    }
}

void DTpdo::sendRtr()
{
    // only when in non spy mode!!!! --> also complete the documentation
    // TODO: send this fucking RTR.
    getParent()->getParent()->sendMessage(
        CANopen::makeTpdoRtr(getParent()->id(), 0x180/*TODO*/));
}

    
void DTpdo::notifySync ()
{
    std::lock_guard<std::mutex> lock (m_accessLock);

    if (m_onSync)
    {
        // Feature clause FP2.1.1: Warning of missing data between SYNCs
        if (!m_firstIteration &&
            m_receivedCtrSinceLastSync != 1 && // 1 per sync is a valid number for non-MPDO traffic.
            getParent()->nodeStateEngine().currentState() == CANopen::NodeState::OPERATIONAL)
        {
            if (DRoot::getInstance()->warningss()[0]->tpdoSyncMismatch())
            {
                SPOOKY(getFullName()) << "expected 1 TPDO in the previous SYNC cycle but got " << wrapValue(std::to_string(m_receivedCtrSinceLastSync)) << 
                    ". Likely a grave configuration issue. " << SPOOKY_ << "[WtpdoSyncMismatch]";
            }
        }
        m_receivedCtrSinceLastSync = 0;
        m_firstIteration = false;
    }
}

    
void DTpdo::notifyBusError ()
{
    for (DExtractedValue* extractedValue : extractedvalues())
        extractedValue->notifyBusError();
}

}