#include <condition_variable>
#include <chrono>
#include <iomanip>

#include <LogIt.h>

#include <Sdo.h>
#include <Logging.hpp>
#include <Utils.h>

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

SdoEngine::SdoEngine (MessageSendFunction messageSendFunction, unsigned char nodeId):
m_sendFunction(messageSendFunction),
m_nodeId(nodeId)
{

}

bool SdoEngine::readExpedited (
    const std::string& where,
    uint16_t index, 
    uint16_t subIndex, 
    std::vector<unsigned char>& output, unsigned int timeoutMs) // TODO: subIndex is uint8 !
{
    LOG(Log::TRC, "Sdo") <<wrapId(where) << " --> SDO read index=0x" << wrapValue(Utils::toHexString(index)) << 
        " subIndex=" << wrapValue(std::to_string(subIndex));
    // TODO: we need to synchronize access to SDOs, preferably at the level of the node. (quasar synchronization)??

    std::unique_lock<std::mutex> lock (m_condVarChangeLock);

    m_replyCame = false;
    // TODO also something indicating that the answer is now expected.

    /* translates basically into Initiate Domain Upload */
    CanMessage initiateDomainUpload;
    initiateDomainUpload.c_id = 0x600 + m_nodeId;
    initiateDomainUpload.c_data[0] = 0x40;
    initiateDomainUpload.c_data[1] = index;
    initiateDomainUpload.c_data[2] = index >> 8;
    initiateDomainUpload.c_data[3] = subIndex;
    initiateDomainUpload.c_dlc = 8; // TODO this is to be checked!!  Was checked and 4 did not work. Trying ;-)
    m_sendFunction(initiateDomainUpload);

    auto wait_status = m_condVarForReply.wait_for(lock, std::chrono::milliseconds(timeoutMs));
    if (wait_status == std::cv_status::timeout)
    {
        LOG(Log::ERR, "Sdo") <<wrapId(where) << " <-- SDO read index=0x" << wrapValue(Utils::toHexString(index)) << 
            " subIndex=" << wrapValue(std::to_string(subIndex)) << " " << 
            Quasar::TermColors::ForeRed << "SDO reply has not come in expected time (" << timeoutMs << "ms)" << 
            Quasar::TermColors::StyleReset;
        return false;
    }


    // parse the SDO reply message
    // TODO: is the size of the message correct to take further assumptions? it should be 8 bytes precisely,

    // TODO: is the header correct ?

    bool deviceWantsExpeditedTransfer = m_lastSdoReply.c_data[0] & 0x02;
    if (!deviceWantsExpeditedTransfer)
    {
        LOG(Log::ERR, "Sdo") << wrapId(where) << " <-- SDO read index=0x" << wrapValue(Utils::toHexString(index)) << 
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
        wrapId(where) << " <-- SDO read index=0x" << wrapValue(Utils::toHexString(index)) << 
        " subIndex=" << wrapValue(std::to_string(subIndex)) << " data(hex)=[" << wrapValue(bytesToHexString(output)) << "] SUCCESS";

    return true;

}

bool SdoEngine::writeExpedited (const std::string& where, uint16_t index, uint16_t subIndex, const std::vector<unsigned char>& data, unsigned int timeoutMs)
{

    LOG(Log::TRC, "Sdo") << wrapId(where) << 
        " + SDO write index=" << std::hex << index << std::dec << " subIndex=" << wrapValue(std::to_string(subIndex)) << 
        " data=[" << wrapValue(bytesToHexString(data)) << "] ";
    if (data.size() < 1)
        throw std::runtime_error("Empty data was given");
    if (data.size() > 4)
        throw std::runtime_error("Too much data [" + std::to_string(data.size()) + "] for expedited SDO");

    std::unique_lock<std::mutex> lock (m_condVarChangeLock);

    m_replyCame = false;

    CanMessage initiateDomainDownload;
    initiateDomainDownload.c_id = 0x600 + m_nodeId;
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

    auto wait_status = m_condVarForReply.wait_for(lock, std::chrono::milliseconds(timeoutMs));
    if (wait_status == std::cv_status::timeout)
    {
        LOG(Log::ERR, "Sdo") << wrapId(where) << " - SDO write index=" << std::hex << index << " subIndex=" << subIndex << std::dec << " no reply to SDO req!";
        return false;
    }

    // m_lastSdoReply is where we got the reply
    if (std::mismatch(
        initiateDomainDownload.c_data + 1,
        initiateDomainDownload.c_data + 4,
        m_lastSdoReply.c_data + 1).first != initiateDomainDownload.c_data + 4)
    {
        LOG(Log::ERR, "Sdo") << wrapId(where) << 
            " - SDO write index=" << std::hex << index << " subIndex=" << wrapValue(std::to_string(subIndex)) << std::dec << " \033[41;37m"
 << " SDO reply was for another object(!)" << SPOOKY_;
        return false;
    }

    // TODO we're not checking the return!
    LOG(Log::TRC, "Sdo") << wrapId(where) << " - SDO write index=" << std::hex << index << std::dec << " subIndex=" << wrapValue(std::to_string(subIndex)) <<  " OK";
    return true;

    //throw std::runtime_error("not-implemented");
}


void SdoEngine::replyCame (const CanMessage& msg)
{
    // TODO should have state here that say whether a reply was actually expected?
    LOG(Log::TRC) << "Received SDO reply";
    std::lock_guard<std::mutex> lock (m_condVarChangeLock);
    m_lastSdoReply = msg;
    m_replyCame = true;
    m_condVarForReply.notify_one();
}

}
