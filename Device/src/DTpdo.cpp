
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
#include <DGlobalSettings.h>

#include <DExtractedValue.h>

#include <PdoCobidMapper.h>

#include <Logging.hpp>
#include <FrameFactory.hpp>
#include <PiotrsUtils.h>

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
    m_name(config.name()),
    m_baseCobid(0),
    m_firstIteration(true),
    m_transportMechanism(Enumerator::Tpdo::transportMechanismFromString(config.transportMechanism())),
    m_receivedCtrSinceLastSync (0),
    m_numRtrsInvokedAddressSpace (0)
    

    /* fill up constructor initialization list here */
{
    /* fill up constructor body here */
    // for the mode of on-sync supports rtr, we should send an RTR each time there is a state change towards preop
    // Feature clause FP6.2: RTR and initialization of on-change data
    if (m_transportMechanism == Enumerator::Tpdo::asyncSupportsRtr || m_transportMechanism == Enumerator::Tpdo::asyncPeriodicRtrWithRequests)
    {
        getParent()->addNodeStateChangeCallBack(
            [this](CANopen::NodeState previous, CANopen::NodeState current)
            {
                if (current == CANopen::NodeState::OPERATIONAL)
                {
                    if (getParent()->getParent()->isInSpyMode())
                    {
                        LOG(Log::DBG, MyLogComponents::nodemgmt()) << wrapId(getFullName()) << " would send RTR for state change into OPERATIONAL, but we're in the spy mode.";
                    }
                    else
                    {
                        // Note: Feature clause FP6.2
                        LOG(Log::DBG, MyLogComponents::nodemgmt()) << wrapId(getFullName()) << " sending RTR for state change into OPERATIONAL";
                        this->sendRtr();
                    }
                }
            });
    }
    if (m_transportMechanism == Enumerator::Tpdo::sync && parent->getParent()->syncLockOut())
    {
        throw_config_error_with_origin("on-SYNC TPDO can not be used on buses with SYNC locked out (at TPDO " + m_name + " Node " + parent->getFullName() + ")");
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

void DTpdo::initialize ()
{
    m_baseCobid = PdoCobidMapper::tpdoSelectorToBaseCobid(selector());
    getParent()->getParent()->cobidCoordinator().registerCobid(
        m_baseCobid + getParent()->id(),
        getParent()->getFullName(),
        m_name + " (TPDO non-mux)",
        std::bind(&DTpdo::onReplyReceived, this, std::placeholders::_1));
}

void DTpdo::onReplyReceived(const CanMessage& msg)
{
    if (msg.c_rtr)
    {
        if (getParent()->getParent()->isInSpyMode())
        {
            // Feature clause FC4.1: Spy mode
            LOG(Log::TRC, MyLogComponents::spy()) << wrapId(getFullName()) << " seeing TPDO with RTR from another master.";
        }
        else
        {
            SPOOKY(getFullName()) << "Seeing unexpected TPDO RTR" << SPOOKY_ << 
                    " This is only expected for the server in the spy mode. Someone is messing badly with your CAN bus!";
        }
        return;
    }
    std::lock_guard<std::mutex> lock (m_accessLock); // operating on shared data, e.g. m_receivedCtrSinceLastSync

    LOG(Log::TRC, MyLogComponents::nm_tpdo()) << wrapId(getFullName()) << 
        " received: 0x[" << wrapValue(Common::bytesToHexString( std::vector<uint8_t>{msg.c_data, msg.c_data + msg.c_dlc} )) << "]";

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
    getParent()->getParent()->sendMessage(
        CANopen::makeTpdoRtr(getParent()->id(), m_baseCobid));
}

    
void DTpdo::notifySync ()
{
    std::lock_guard<std::mutex> lock (m_accessLock);

    if (m_transportMechanism == Enumerator::Tpdo::TransportMechanism::sync)
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

    
void DTpdo::propagateNull ()
{
    for (DExtractedValue* extractedValue : extractedvalues())
        extractedValue->propagateNull();
}

void DTpdo::tick()
{
    if (m_transportMechanism == Enumerator::Tpdo::asyncPeriodicRtrWithRequests)
    {
        unsigned int msPassed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_lastPeriodicRtr).count();
        if (msPassed >= 1000*DRoot::getInstance()->globalsettings()->asyncPeriodicRtrPeriodSeconds())
        {
            // FP6.3: Periodic RTR
            this->sendRtr(); // of course, after the delay.
            m_lastPeriodicRtr = std::chrono::steady_clock::now();
        }
    }
}

}