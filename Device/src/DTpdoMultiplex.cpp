
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

#include <DTpdoMultiplex.h>
#include <ASTpdoMultiplex.h>
#include <DRoot.h>
#include <DWarnings.h>
#include <DNode.h>
#include <DBus.h>
#include <PdoCobidMapper.h>

#include <DMultiplexedChannel.h>

#include <Logging.hpp>
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
DTpdoMultiplex::DTpdoMultiplex (
    const Configuration::TpdoMultiplex& config,
    Parent_DTpdoMultiplex* parent
):
    Base_DTpdoMultiplex( config, parent),
    m_name(config.name()),
    m_transportMechanism(Enumerator::TpdoMultiplex::transportMechanismFromString(config.transportMechanism()))

    /* fill up constructor initialization list here */
{
    /* fill up constructor body here */
    if (m_transportMechanism == Enumerator::TpdoMultiplex::sync && parent->getParent()->syncLockOut())
    {
        throw_config_error_with_origin("on-SYNC M-TPDO can not be used on buses with SYNC locked out (at TPDO " + m_name + " Node " + parent->getFullName() + ")");
    }
}

/* sample dtr */
DTpdoMultiplex::~DTpdoMultiplex ()
{
}

/* delegates for cachevariables */



/* delegators for methods */

// 3333333333333333333333333333333333333333333333333333333333333333333333333
// 3     FULLY CUSTOM CODE STARTS HERE                                     3
// 3     Below you put bodies for custom methods defined for this class.   3
// 3     You can do whatever you want, but please be decent.               3
// 3333333333333333333333333333333333333333333333333333333333333333333333333

void DTpdoMultiplex::initialize()
{
    for (DMultiplexedChannel* channel : multiplexedchannels())
    {
        channel->setParentMultiplex(this);
        channel->initialize();
    }
    getParent()->getParent()->cobidCoordinator().registerCobid(
        PdoCobidMapper::tpdoSelectorToBaseCobid(selector()) + getParent()->id(),
        getParent()->getFullName(),
        m_name + " (TPDO mux, " + std::to_string(multiplexedchannels().size()) + " channels)",
        std::bind(&DTpdoMultiplex::onReplyReceived, this, std::placeholders::_1));
}

void DTpdoMultiplex::onReplyReceived(const CanMessage& msg)
{
    // Feature clause FP3.1: MPDO: multiplexing of TPDOs

    // FP2.1.1 is missing here. TODO?

    LOG(Log::TRC) << "received TPDO reply: " << msg.toString(); // TODO move to PDO Log component
    // getting the channel number ...
    // Note: this is the standard channel demultiplexer

    if (msg.c_dlc < 1)
    {
        if (DRoot::getInstance()->warningss()[0]->tpdoTooShort())
        {
            SPOOKY(getFullName()) << "received TPDO is too short " << wrapValue(std::to_string(msg.c_dlc)) << SPOOKY_ << "[WtpdoTooShort]";
        }
        return;
    }

    unsigned int channelNumber = msg.c_data[0]; // this is the MPDO algorithm.
    // do we have such a channel??
    DMultiplexedChannel* channel = this->getMultiplexedChannelById(channelNumber);
    if (!channel)
    {
        if (DRoot::getInstance()->warningss()[0]->unknownMultiplexedChannel())
        {
            SPOOKY(getFullName()) << "Channel number " << wrapValue(std::to_string(channelNumber))
                << " is not in the configuration " << SPOOKY_ << 
                ", but we seem to be getting data from it. Fix the configuration of your ELMBs or fix your configuration [WunknownMultiplexedChannel]" << SPOOKY_;
        }
        return;
    }
    
    channel->onReplyReceived(msg);

}

void DTpdoMultiplex::notifySync ()
{
    //! FP2.1.1: Warning of missing data between SYNCs
    //! Aggregation: Before, the per-channel version, was really generating too much of info.
    size_t mismatchCtr = 0;
    for (DMultiplexedChannel* channel : multiplexedchannels())
    {
        bool mismatch = channel->notifySyncAndCheckMismatch();
        if (mismatch)
            mismatchCtr++;
    }
    if (mismatchCtr > 0)
    {
        if (DRoot::getInstance()->warningss()[0]->tpdoSyncMismatch())
        {
            SPOOKY(getFullName()) << "expected 1 M-TPDO per channel in the previous SYNC cycle but the expectation failed for "
                << mismatchCtr << " channels out of " << multiplexedchannels().size() << " in this multiplex" << SPOOKY_;
        }
    }
}

void DTpdoMultiplex::propagateNull ()
{
    for (DMultiplexedChannel* channel : multiplexedchannels())
        channel->notifyBusError();
}

}