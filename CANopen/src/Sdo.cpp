#include <condition_variable>
#include <chrono>

#include <LogIt.h>

#include <Sdo.h>

namespace CANopen
{

SdoEngine::SdoEngine (MessageSendFunction messageSendFunction, unsigned char nodeId):
m_sendFunction(messageSendFunction),
m_nodeId(nodeId)
{

}

bool SdoEngine::readExpedited (uint16_t index, uint16_t subIndex, std::vector<unsigned char>& output) // TODO: subIndex is uint8 !
{
    


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
    initiateDomainUpload.c_dlc = 4; // TODO this is to be checked!!
    m_sendFunction(initiateDomainUpload);

    auto wait_status = m_condVarForReply.wait_for(lock, std::chrono::milliseconds(100));
    if (wait_status == std::cv_status::timeout)
    {
        LOG(Log::ERR) << "SDO reply hasn't come";
        return false;
    }

    LOG(Log::INF) << "Notification from after condvar wait"; 

    // parse the SDO reply message
    // TODO: is the size of the message correct to take further assumptions? it should be 8 bytes precisely,

    // TODO: is the header correct ?

    bool deviceWantsExpeditedTransfer = m_lastSdoReply.c_data[0] & 0x02;
    if (!deviceWantsExpeditedTransfer)
    {
        LOG(Log::ERR) << "Device wants expedited transfer only!";
        return false;
    }



    // TODO: is the index and subIndex correct wrt was supposed to be done

    // how do I know how many bytes are returned?
    bool dataSetSizeIndicated = m_lastSdoReply.c_data[0] & 0x01;
    if (!dataSetSizeIndicated)
    {
        LOG(Log::ERR) << "Data size set was not indicated!";
        return false;
    }
    unsigned char sizeOfDataSet = 4 - ((m_lastSdoReply.c_data[0] & 0x0c) >> 2);

    output.assign(sizeOfDataSet, 0);

    std::copy(
        m_lastSdoReply.c_data + 4,
        m_lastSdoReply.c_data + 4 + sizeOfDataSet,
        output.begin());

    return true;

}

void SdoEngine::replyCame (const CanMessage& msg)
{
    // TODO should have state here that say whether a reply was actually expected?
    LOG(Log::INF) << "Received SDO reply";
    std::lock_guard<std::mutex> lock (m_condVarChangeLock);
    m_lastSdoReply = msg;
    m_replyCame = true;
    m_condVarForReply.notify_one();
}

}