
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

#include <DSdoVariable.h>
#include <ASSdoVariable.h>

#include <DSdoSameIndexGroup.h>
#include <DNode.h>
#include <DRoot.h>
#include <DGlobalSettings.h>
#include <DBus.h>

#include <PiotrsUtils.h>
#include <ValueMapper.h>
#include <Logging.hpp>



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
DSdoVariable::DSdoVariable (
    const Configuration::SdoVariable& config,
    Parent_DSdoVariable* parent
):
    Base_DSdoVariable( config, parent),
    m_name (config.name()),
    m_expeditedReadTimeoutInheritsGlobal(config.expeditedSdoReadTimeoutSeconds() == "inheritGlobal"),
    m_expeditedReadTimeoutSecondsFromConfig(1), // sane initialization in case of fuckup however this value won't ever be used.
    m_expeditedWriteTimeoutInheritsGlobal(config.expeditedSdoWriteTimeoutSeconds() == "inheritGlobal"),
    m_expeditedWriteTimeoutSecondsFromConfig(1) // sane initialization in case of fuckup however this value won't ever be used.

    /* fill up constructor initialization list here */
{
    /* fill up constructor body here */
    m_subIndex = config.subIndex(); // perhaps we should optimize it ;-)
    if (config.index() != "fromGroup")
    {
        m_index = std::stoi(config.index(), nullptr, /*base*/ 16);
    }
    if (!m_expeditedReadTimeoutInheritsGlobal)
    {
        m_expeditedReadTimeoutSecondsFromConfig = std::stoi(config.expeditedSdoReadTimeoutSeconds(), nullptr);
    }
    if (!m_expeditedWriteTimeoutInheritsGlobal)
    {
        m_expeditedWriteTimeoutSecondsFromConfig = std::stoi(config.expeditedSdoWriteTimeoutSeconds(), nullptr);
    }
}

/* sample dtr */
DSdoVariable::~DSdoVariable ()
{
}

/* delegates for cachevariables */


/* ASYNCHRONOUS !! */
UaStatus DSdoVariable::readValue (
    UaVariant& value,
    UaDateTime& sourceTime
)
{
    // Feature clause FS0.1: Features common to any mode of SDO usage: R/W control
    /* Is it actually allowed to read this SDO? */
    if (access().find("R") == std::string::npos)
    {
        LOG(Log::ERR) << wrapId(getFullName()) << ": SDO read was denied because it is denied in the configuration.";
        return OpcUa_BadUserAccessDenied;
    }

    /* Not in spy mode? */
    if (m_bus->isInSpyMode())
    {
        LOG(Log::ERR) << wrapId(getFullName()) << ": SDO read was denied because the bus is currently in the spy mode.";
        return OpcUa_BadOutOfService;
    }

    // Feature clause FS0.1: Features common to any mode of SDO usage: synchronization
    // Synchronization: use quasar's made mutex:
    std::lock_guard<boost::mutex> lock (m_node->getLock());
    std::vector<unsigned char> readData;
    if (dataType() == "ByteString")
    {
        bool status = m_node->sdoEngine()->readSegmented(
            getFullName(),
            m_index, 
            m_subIndex, 
            readData, 
            /*timeout TODO*/ 1000);    
        char shit[70];
        for (size_t i=0; i < sizeof(shit); i++)
            shit[i] = i;
        UaByteString bs (sizeof(shit), (OpcUa_Byte*)shit);
        
        value.setByteString(bs, false);
        return OpcUa_Good;
    }

    unsigned int timeoutMs = m_expeditedReadTimeoutInheritsGlobal ? 
        1000.0 * Device::DRoot::getInstance()->globalsettings()->expeditedSdoReadTimeoutSeconds() :
        1000.0 * m_expeditedReadTimeoutSecondsFromConfig;
    bool status = m_node->sdoEngine()->readExpedited(
        getFullName(),
        m_index, 
        m_subIndex, 
        readData, 
        timeoutMs);
    if (!status)    
        return OpcUa_BadOutOfService; // maybe we should return uastatus right away
    sourceTime = UaDateTime::now();
    LOG(Log::TRC) << wrapId(getFullName()) << "SDO transaction finished, performing value mapping now";
    try
    {
        value = ValueMapper::extractFromBytesIntoVariant(&readData[0], readData.size(), dataType(), 0, booleanFromBit());
    }
    catch(const std::exception& e)
    {
        LOG(Log::ERR) << wrapId(getFullName()) << e.what() << "(note: dataType was " << dataType() << ")";
        return OpcUa_Bad;
    }
    return OpcUa_Good;
}
/* ASYNCHRONOUS !! */
UaStatus DSdoVariable::writeValue (
    UaVariant& value
)
{
    // Feature clause FS0.1: Features common to any mode of SDO usage: R/W control
    /* Is it actually allowed to write this SDO? */
    if (access().find("W") == std::string::npos)
    {
        LOG(Log::ERR, "Sdo") << wrapId(getFullName()) << ": SDO write was denied because it is denied in the configuration.";
        return OpcUa_BadUserAccessDenied;
    }

    /* Not in spy mode? */
    if (m_bus->isInSpyMode())
    {
        LOG(Log::ERR, "Sdo") << wrapId(getFullName()) << ": SDO read was denied because the bus is currently in the spy mode.";
        return OpcUa_BadOutOfService;
    }

    // Feature clause FS0.1: Features common to any mode of SDO usage: synchronization
    // Synchronization: use quasar's made mutex:
    std::lock_guard<boost::mutex> lock (m_node->getLock());

    // Expedited SDO or Segmented SDO?
    if (dataType() != "ByteString") /* Expedited SDO */
    {
        /* Anything not declared as ByteString goes as Expedited SDO */
        std::vector<uint8_t> bytes = ValueMapper::packVariantToBytes(value, dataType());

        unsigned int timeoutMs = m_expeditedWriteTimeoutInheritsGlobal ? 
            1000.0 * Device::DRoot::getInstance()->globalsettings()->expeditedSdoWriteTimeoutSeconds() :
            1000.0 * m_expeditedWriteTimeoutSecondsFromConfig;

        bool status = m_node->sdoEngine()->writeExpedited(
            getFullName(),
            m_index, 
            m_subIndex, 
            bytes, 
            timeoutMs);    
        return status ? OpcUa_Good : OpcUa_Bad;
    }
    else
    {
        if (value.dataType() != UaNodeId(OpcUaType_ByteString))
        {
            LOG(Log::ERR, "Sdo") << wrapId(getFullName()) << "Segmented SDO is supported only for ByteString input data. Given encoding type was [" << wrapValue(value.dataType().toString().toUtf8()) << "]";
            return OpcUa_BadDataEncodingInvalid;
        }
        UaByteString bs;
        if (value.toByteString(bs) != OpcUa_Good)
        {
            LOG(Log::ERR, "Sdo") << wrapId(getFullName()) << "ByteString decoding error";
            return OpcUa_BadDataEncodingInvalid;
        }
        m_node->sdoEngine()->writeSegmented(
            getFullName(),
            m_index,
            m_subIndex,
            std::vector<unsigned char>(bs.data(), bs.data() + bs.length()),
            1000); // TODO TIMEOUT CONDITION
        
    }

}

/* delegators for methods */

// 3333333333333333333333333333333333333333333333333333333333333333333333333
// 3     FULLY CUSTOM CODE STARTS HERE                                     3
// 3     Below you put bodies for custom methods defined for this class.   3
// 3     You can do whatever you want, but please be decent.               3
// 3333333333333333333333333333333333333333333333333333333333333333333333333

void DSdoVariable::initialize (DBus* bus, DNode* node)
{
    m_bus = bus;
    m_node = node;
    if (!node->sdoEngine())
        throw_config_error_with_origin("SDO declared with a node that explicitly has no SDO support");
}

void DSdoVariable::setIndex (uint16_t _index)
{
    if (index() != "fromGroup")
        throw std::runtime_error("SdoVariable " + getFullName() + " index is not fromGroup, thus you have configuration issue.");
    m_index = _index;
}

}