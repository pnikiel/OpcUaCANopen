#include <condition_variable>
#include <chrono>
#include <iomanip>

#include <LogIt.h>

#include <Sdo.h>
#include <Logging.hpp>
#include <Utils.h>
#include <PiotrsUtils.h>
#include <DNode.h>
#include <DBus.h>
#include <CobidCoordinator.hpp>

using namespace Logging;

namespace CANopen
{

static std::string bytesToHexString (const std::vector<uint8_t>& bytes)
{
    std::stringstream dataAsStr;
    for ( uint8_t byte : bytes)
        dataAsStr << std::setfill('0') << std::setw(2) << std::hex << (unsigned int)byte << ",";
    return dataAsStr.str(); 
} 

SdoEngine::SdoEngine (
    Device::DNode* myNode,
    MessageSendFunction messageSendFunction,
    CobidCoordinator& cobidCoordinator):

m_node(myNode),
m_sendFunction(messageSendFunction),
m_replyCame(false),
m_replyExpected(false)
{
    cobidCoordinator.registerCobid(0x600 + m_node->id(), m_node->getFullName(), "SDO requests",
        std::bind(&SdoEngine::sdoRequestNotifier, this, std::placeholders::_1));  
    cobidCoordinator.registerCobid(0x580 + m_node->id(), m_node->getFullName(), "SDO replies",
        std::bind(&SdoEngine::replyCame, this, std::placeholders::_1));
}

bool SdoEngine::readExpedited (
    uint16_t index, 
    uint8_t subIndex, 
    std::vector<unsigned char>& output, unsigned int timeoutMs) // TODO: subIndex is uint8 !
{
    LOG(Log::TRC, "Sdo") <<wrapId(m_node->getFullName()) << " --> SDO read index=0x" << wrapValue(Utils::toHexString(index)) << 
        " subIndex=" << wrapValue(std::to_string(subIndex));
    // TODO: we need to synchronize access to SDOs, preferably at the level of the node. (quasar synchronization)??

    std::unique_lock<std::mutex> lock (m_condVarChangeLock);

    m_replyCame = false;
    // TODO also something indicating that the answer is now expected.

    /* translates basically into Initiate Domain Upload */
    CanMessage initiateDomainUpload;
    initiateDomainUpload.c_id = 0x600 + m_node->id();
    initiateDomainUpload.c_data[0] = 0x40;
    initiateDomainUpload.c_data[1] = index;
    initiateDomainUpload.c_data[2] = index >> 8;
    initiateDomainUpload.c_data[3] = subIndex;
    initiateDomainUpload.c_dlc = 8; // TODO this is to be checked!!  Was checked and 4 did not work. Trying ;-)
    m_sendFunction(initiateDomainUpload);

    m_replyExpected = true;

    auto wait_status = m_condVarForReply.wait_for(lock, std::chrono::milliseconds(timeoutMs));
    m_replyExpected = false;
    if (wait_status == std::cv_status::timeout)
    {
        LOG(Log::ERR, "Sdo") <<wrapId(m_node->getFullName()) << " <-- SDO read index=0x" << wrapValue(Utils::toHexString(index)) << 
            " subIndex=" << wrapValue(std::to_string(subIndex)) << " " << 
            ERROR << "SDO reply has not come in expected time (" << timeoutMs << "ms)" << ERROR_;
        return false;
    }

    if (m_lastSdoReply.c_data[0] == 0x80)
    { /* Abort domain transfer*/
        handleAbortDomainTransfer(m_lastSdoReply);
        return false;
    }    

    if ((m_lastSdoReply.c_data[0] & 0xf0) != 0x40)
    {
        LOG(Log::ERR, "Sdo") <<wrapId(m_node->getFullName()) << " <-- SDO read index=0x" << wrapValue(Utils::toHexString(index)) << 
            " subIndex=" << wrapValue(std::to_string(subIndex)) << " " << 
            ERROR << "SDO reply indicates invalid reply, expected 0x4X got [" << std::hex << (unsigned int)m_lastSdoReply.c_data[0] << "]" << ERROR_;
        return false;        
    }

    // parse the SDO reply message
    // TODO: is the size of the message correct to take further assumptions? it should be 8 bytes precisely,

    // TODO: is the header correct ?

    bool deviceWantsExpeditedTransfer = m_lastSdoReply.c_data[0] & 0x02;
    if (!deviceWantsExpeditedTransfer)
    {
        LOG(Log::ERR, "Sdo") << wrapId(m_node->getFullName()) << " <-- SDO read index=0x" << wrapValue(Utils::toHexString(index)) << 
            " subIndex=" << wrapValue(std::to_string(subIndex)) << " " << 
            Quasar::TermColors::ForeRed << "CANopen device wanted non-expedited transfer (" << timeoutMs << "ms)" << 
            Quasar::TermColors::StyleReset;
        return false;
    }

    // TODO: is the index and subIndex correct wrt was supposed to be done

    // how do I know how many bytes are returned?
    bool dataSetSizeIndicated = m_lastSdoReply.c_data[0] & 0x01;
    if (!dataSetSizeIndicated)
    {
        LOG(Log::ERR, "Sdo") << "Data size set was not indicated!";
        return false;
    }
    unsigned char sizeOfDataSet = 4 - ((m_lastSdoReply.c_data[0] & 0x0c) >> 2);

    output.assign(sizeOfDataSet, 0);

    std::copy(
        m_lastSdoReply.c_data + 4,
        m_lastSdoReply.c_data + 4 + sizeOfDataSet,
        output.begin());

    LOG(Log::TRC, "Sdo") << 
        wrapId(m_node->getFullName()) << " <-- SDO read index=0x" << wrapValue(Utils::toHexString(index)) << 
        " subIndex=" << wrapValue(std::to_string(subIndex)) << " data(hex)=[" << wrapValue(bytesToHexString(output)) << "] SUCCESS";

    return true;

}

SdoEngine::SdoResult SdoEngine::writeExpedited (
    uint16_t index, 
    uint8_t subIndex, 
    const std::vector<unsigned char>& data, unsigned int timeoutMs)
{

    LOG(Log::TRC, "Sdo") << wrapId(m_node->getFullName()) << 
        " + SDO write index=" << std::hex << index << std::dec << " subIndex=" << wrapValue(std::to_string(subIndex)) << 
        " data=[" << wrapValue(bytesToHexString(data)) << "] ";
    if (data.size() < 1)
        throw std::runtime_error("Empty data was given");
    if (data.size() > 4)
        throw std::runtime_error("Too much data [" + std::to_string(data.size()) + "] for expedited SDO");

    std::unique_lock<std::mutex> lock (m_condVarChangeLock);

    m_replyCame = false;

    CanMessage initiateDomainDownload;
    initiateDomainDownload.c_id = 0x600 + m_node->id();
    unsigned char n = 4 - data.size();
    initiateDomainDownload.c_data[0] = 0x23 | ((n & 0x03) << 2); // E=1 S=1 N is dependent on data size
    initiateDomainDownload.c_data[1] = index;
    initiateDomainDownload.c_data[2] = index >> 8;
    initiateDomainDownload.c_data[3] = subIndex;
    initiateDomainDownload.c_dlc = 8;

    std::copy(
        data.begin(),
        data.end(),
        initiateDomainDownload.c_data + 4);

    m_sendFunction(initiateDomainDownload);

    m_replyExpected = true;
    // Feature clause FS0.1: Features common to any mode of SDO usage: timeout detection
    auto wait_status = m_condVarForReply.wait_for(lock, std::chrono::milliseconds(timeoutMs));
    if (wait_status == std::cv_status::timeout)
    {
        LOG(Log::ERR, "Sdo") << wrapId(m_node->getFullName()) << " - SDO write index=" << std::hex << index << " subIndex=" << subIndex << std::dec << " no reply to SDO request! (timeout was "  << timeoutMs << "ms)";
        return SdoEngine::Timeout;
    }

    // TODO need to check the size!

    // m_lastSdoReply is where we got the reply
    if (std::mismatch(
        initiateDomainDownload.c_data + 1,
        initiateDomainDownload.c_data + 4,
        m_lastSdoReply.c_data + 1).first != initiateDomainDownload.c_data + 4)
    {
        LOG(Log::ERR, "Sdo") << wrapId(m_node->getFullName()) << 
            " - SDO write index=" << std::hex << index << " subIndex=" << wrapValue(std::to_string(subIndex)) << std::dec << " \033[41;37m"
 << " SDO reply was for another object(!)" << SPOOKY_;
        return SdoEngine::OtherFailure;
    }

    if (m_lastSdoReply.c_data[0] == 0x80)
    {
        handleAbortDomainTransfer(m_lastSdoReply);
        return SdoEngine::AbortedDomainTransfer;
    }

    if (m_lastSdoReply.c_data[0] != 0x60)
    {
        LOG(Log::ERR, "Sdo") << wrapId(m_node->getFullName()) << 
            " - SDO write index=" << std::hex << index << " subIndex=" << wrapValue(std::to_string(subIndex)) << std::dec << " \033[41;37m"
 << " SDO reply header is invalid, expected 0x60 got ["  << std::hex << (unsigned int)m_lastSdoReply.c_data[0] << "]" << SPOOKY_;
        return SdoEngine::OtherFailure;
    } 

    LOG(Log::TRC, "Sdo") << wrapId(m_node->getFullName()) << " - SDO write index=" << std::hex << index << std::dec << " subIndex=" << wrapValue(std::to_string(subIndex)) <<  " OK";
    return SdoEngine::Success;

}


void SdoEngine::replyCame (const CanMessage& msg) // TODO: add where field
{
    if (m_node->getParent()->isInSpyMode())
    {
        LOG(Log::TRC, MyLogComponents::spy()) << wrapId(m_node->getFullName()) << " seeing SDO reply (not parsing - bus in spy mode)";
        return;
    }
    LOG(Log::TRC, "Sdo") << "Received SDO reply";
    std::lock_guard<std::mutex> lock (m_condVarChangeLock);
    if (m_replyExpected)
    {
        m_lastSdoReply = msg;
        m_replyCame = true;
        m_replyExpected = false;
        m_condVarForReply.notify_one();
    }
    else
    {
        LOG(Log::ERR, "Sdo") << "Received unexpected SDO reply " << wrapValue(Common::CanMessageToString(msg));
    }

}

// Feature clause: FS0.3: Abort domain transfer support
void SdoEngine::handleAbortDomainTransfer (const CanMessage& msg)
{
    /* AbortDomainTransfer message */
    /* We validated that the reply applies to the correct index/subindex above*/
    uint32_t reasonCode = Common::bytesAsTypeNativeAlignSafeCast<uint32_t>(msg.c_data, msg.c_dlc, /*offset*/4);
    LOG(Log::ERR, "Sdo") << wrapId(m_node->getFullName()) << " AbortDomainTransfer! Reason code is "
        << ERROR << explainAbortCode(reasonCode>>24, (reasonCode>>16)&0xff) << ERROR_ 
        << " (full abort code was " << wrapValue(Utils::toHexString(reasonCode)) << ")";
}

std::string SdoEngine::explainAbortCode (uint8_t errorClass, uint8_t errorCode)
{
    /* ELMBio has incomplete support to these fields so not everything can be analyzed... */
    switch (errorClass)
    {
        case 0x05:
            switch (errorCode)
            {
                case 0x03: return "(Toggle not alternated) or (CRC error) or (Out of mem)";
                case 0x04: return "(Protocol Timed out) or (Client/Server command invalid/unknown) or (Invalid block size) or (Invalid seq number)";
                default: return "SDO Protocol issue (generic)";
            }
            break;
        case 0x06:
            switch (errorCode)
            {
                case 0x01: return "Unsupported access (Read-Write definition mismatch?)";
                case 0x02: return "Object does not exist";
                case 0x03: return "Inconsistent parameter";
                case 0x04: return "Illegal parameter";
                case 0x06: return "Hardware error (generic or data type does not match the device)";
                case 0x07: return "Type conflict";
                case 0x09: return "Attribute issues: (sub-index not existing) or (value not in allowed range range) or (peripheral in use)";
                default: return "Access issue (generic)";
            }
            break;
        case 0x08: 
            return "General error (data can not be transferred/stored)";
        default: return "Error class unknown (can't decode)";
    }
}

void SdoEngine::sdoRequestNotifier (const CanMessage& msg)
{
    if (m_node->getParent()->isInSpyMode())
    {
        LOG(Log::TRC, MyLogComponents::spy()) << wrapId(m_node->getFullName()) << " seeing SDO request (not parsing - bus in spy mode)";
    }
    else
        SPOOKY(m_node->getFullName()) << " SDO request is seen " << SPOOKY_ << " ... hmm ... another host on the bus. It's BAD!";
}

}
