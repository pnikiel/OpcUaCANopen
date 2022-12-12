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

bool SdoEngine::writeExpedited (
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
        return false;
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
        return false;
    }

    if (m_lastSdoReply.c_data[0] == 0x80)
    {
        handleAbortDomainTransfer(m_lastSdoReply);
        return false;
    }

    if (m_lastSdoReply.c_data[0] != 0x60)
    {
        LOG(Log::ERR, "Sdo") << wrapId(m_node->getFullName()) << 
            " - SDO write index=" << std::hex << index << " subIndex=" << wrapValue(std::to_string(subIndex)) << std::dec << " \033[41;37m"
 << " SDO reply header is invalid, expected 0x60 got ["  << std::hex << (unsigned int)m_lastSdoReply.c_data[0] << "]" << SPOOKY_;
        return false;
    } 

    LOG(Log::TRC, "Sdo") << wrapId(m_node->getFullName()) << " - SDO write index=" << std::hex << index << std::dec << " subIndex=" << wrapValue(std::to_string(subIndex)) <<  " OK";
    return true;

    //throw std::runtime_error("not-implemented");
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

void SdoEngine::handleAbortDomainTransfer (const CanMessage& msg)
{
    /* AbortDomainTransfer message */
    /* We validated that the reply applies to the correct index/subindex above*/
    // TODO necessary to guarantee size of 8!
    uint32_t reasonCode = *reinterpret_cast<uint32_t*>(&m_lastSdoReply.c_data + 4);
    LOG(Log::ERR, "Sdo") << wrapId(m_node->getFullName()) << "AbortDomainTransfer! Reason code is " << Utils::toHexString(reasonCode);
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
