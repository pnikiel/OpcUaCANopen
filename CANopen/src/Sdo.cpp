#include <condition_variable>
#include <chrono>
#include <iomanip>
#include <algorithm>

#include <LogIt.h>

#include <Sdo.h>
#include <Logging.hpp>
#include <Utils.h>
#include <PiotrsUtils.h>
#include <CobidCoordinator.hpp>
#include <FrameFactory.hpp>

using namespace Logging;
using namespace Common;

namespace CANopen
{

SdoEngine::SdoEngine (
    uint8_t nodeId,
    MessageSendFunction messageSendFunction,
    CobidCoordinator& cobidCoordinator):

m_nodeFullName("(not configured)"),
m_nodeId(nodeId),
m_isInSpyMode (true), /* safe init */
m_sendFunction(messageSendFunction),
m_replyCame(false),
m_replyExpected(false),
m_cobidCoordinator(cobidCoordinator)
{

}

void SdoEngine::initialize()
{
    m_cobidCoordinator.registerCobid(0x600 + m_nodeId, m_nodeFullName, "SDO requests",
        std::bind(&SdoEngine::sdoRequestNotifier, this, std::placeholders::_1));  
    m_cobidCoordinator.registerCobid(0x580 + m_nodeId, m_nodeFullName, "SDO replies",
        std::bind(&SdoEngine::replyCame, this, std::placeholders::_1));
}

CanMessage SdoEngine::invokeTransactionAndThrowOnNoReply(
    const CanMessage& request,
    const std::string& where,
    const std::string& what,
    uint16_t index,
    uint8_t subIndex,
    unsigned int timeoutMs)
{
    std::unique_lock<std::mutex> lock (m_condVarChangeLock);

    m_replyCame = false;
    m_replyExpected = true;

    m_sendFunction(request);

    // Feature clause FS0.1: Features common to any mode of SDO usage: timeout detection
    auto wait_status = m_condVarForReply.wait_for(lock, std::chrono::milliseconds(timeoutMs));
    if (wait_status == std::cv_status::timeout)
    {
        LOG(Log::ERR, "Sdo") << wrapId(where) << " <-- " << what <<
            " index=0x" << wrapValue(Utils::toHexString(index)) <<
            " subIndex=" << wrapValue(Utils::toString((unsigned int)subIndex)) << ERROR << " no reply to SDO request! " << ERROR_ << "(timeout was "  << timeoutMs << "ms)";
        throw std::runtime_error(what + " timeout (detailed error logged)");
    }
    return m_lastSdoReply;
}

// bool SdoEngine::readExpedited (
//     const std::string& where,
//     uint16_t index, 
//     uint8_t subIndex, 
//     std::vector<unsigned char>& output, unsigned int timeoutMs) // TODO: subIndex is uint8 !
// {
//     LOG(Log::TRC, "Sdo") <<wrapId(where) << " --> SDO read index=0x" << wrapValue(Utils::toHexString(index)) << 
//         " subIndex=" << wrapValue(std::to_string(subIndex));

//     CanMessage initiateDomainUpload = CANopen::makeInitiateDomainUpload(m_nodeId, index, subIndex);
//     CanMessage reply = this->invokeTransactionAndThrowOnNoReply(
//         initiateDomainUpload, where, "expedited SDO read", index, subIndex, timeoutMs);
//     throwIfAbortDomainTransfer(reply, where);  
//     throwIfQuestionableSize(reply);
//     throwIfSdoObjectMismatch(initiateDomainUpload, reply, where);

//     if ((reply.c_data[0] & 0xf0) != 0x40)
//     {
//         LOG(Log::ERR, "Sdo") <<wrapId(where) << " <-- SDO read index=0x" << wrapValue(Utils::toHexString(index)) << 
//             " subIndex=" << wrapValue(std::to_string(subIndex)) << " " << 
//             ERROR << "SDO reply indicates invalid reply, expected 0x4X got [" << std::hex << (unsigned int)reply.c_data[0] << "]" << ERROR_;
//         return false;        
//     }

//     bool deviceWantsExpeditedTransfer = reply.c_data[0] & 0x02;
//     if (!deviceWantsExpeditedTransfer)
//     {
//         LOG(Log::ERR, "Sdo") << wrapId(where) << " <-- SDO read index=0x" << wrapValue(Utils::toHexString(index)) << 
//             " subIndex=" << wrapValue(std::to_string(subIndex)) << " " << 
//             ERROR << "CANopen device wanted non-expedited transfer" << ERROR_ << " (" << timeoutMs << "ms)";
//         return false;
//     }

//     // how do I know how many bytes are returned?
//     bool dataSetSizeIndicated = reply.c_data[0] & 0x01;
//     if (!dataSetSizeIndicated)
//     {
//         LOG(Log::ERR, "Sdo") << wrapId(where) << " <-- Data size set was not indicated - can't parse the SDO reply!";
//         return false;
//     }
//     unsigned char sizeOfDataSet = 4 - ((reply.c_data[0] & 0x0c) >> 2);

//     output.assign(sizeOfDataSet, 0);

//     std::copy(
//         reply.c_data + 4,
//         reply.c_data + 4 + sizeOfDataSet,
//         output.begin());

//     LOG(Log::TRC, "Sdo") << 
//         wrapId(where) << " <-- SDO read index=0x" << wrapValue(Utils::toHexString(index)) << 
//         " subIndex=" << wrapValue(std::to_string(subIndex)) << " data(hex)=[" << wrapValue(bytesToHexString(output)) << "] SUCCESS";

//     return true;
// }

// TODO: protection for too long interaction.
// bool SdoEngine::readSegmented (
//     const std::string& where, 
//     uint16_t index, 
//     uint8_t subIndex, 
//     std::vector<unsigned char>& output, 
//     unsigned int timeoutMsPerPair,
//     unsigned int maxSegments)
// {
//     LOG(Log::TRC, "Sdo") << wrapId(where) << 
//         " --> SDO segmented read index=" << wrapValue(Utils::toHexString(index)) << " subIndex=" << wrapValue(std::to_string(subIndex));

//     output.clear();
//     output.reserve(2048); /* Knowledge from the field that biggest transfers are like 1500B long */

//     CanMessage initiateDomainUpload = CANopen::makeInitiateDomainUpload(m_nodeId, index, subIndex);
//     CanMessage reply = this->invokeTransactionAndThrowOnNoReply(initiateDomainUpload, where, "segmented SDO read", index, subIndex, timeoutMsPerPair);
//     throwIfAbortDomainTransfer(reply, where);
//     throwIfQuestionableSize(reply); 
//     throwIfSdoObjectMismatch(initiateDomainUpload, reply, where);   

//     bool deviceWantsExpeditedTransfer = reply.c_data[0] & 0x02;
//     if (deviceWantsExpeditedTransfer)
//     {
//         LOG(Log::ERR, "Sdo") << wrapId(where) << " <-- Segmented SDO read index=0x" << wrapValue(Utils::toHexString(index)) << 
//             " subIndex=" << wrapValue(std::to_string(subIndex)) << " " << 
//             ERROR << "CANopen device wanted expedited transfer instead of segmented. Looks like configuration issue." << ERROR_;
//         return false;
//     }

//     bool nextSegmentToggle = false;
//     bool lastSegment;
//     size_t segmentsReceived(0);
//     do
//     {
//         CanMessage uploadDomainSegment = makeUploadDomainSegment(m_nodeId, index, subIndex, nextSegmentToggle);
//         CanMessage reply = this->invokeTransactionAndThrowOnNoReply(uploadDomainSegment, where, "segmented SDO read", index, subIndex, timeoutMsPerPair);
//         throwIfAbortDomainTransfer(reply, where);
//         throwIfQuestionableSize(reply); 
//         bool receivedToggleBit = reply.c_data[0] & 0x10;
//         /* Refer to Henk's CANopen writeup for understanding what is n */
//         uint8_t n = (reply.c_data[0] >> 1) & 0x07;
//         uint8_t dataInThisSegment = 7-n;

//         LOG(Log::TRC, "Sdo") << wrapId(where) << " Segmented SDO read index=0x" << wrapValue(Utils::toHexString(index)) << 
//             " subIndex=" << wrapValue(std::to_string(subIndex)) << " received segment #" << wrapValue(Utils::toString(segmentsReceived)) << 
//             " toggleBit " << receivedToggleBit << " size in this segment:" << wrapValue(Utils::toString((unsigned int)dataInThisSegment));

//         /* check toggle bit? */
//         if (receivedToggleBit != nextSegmentToggle)
//         {
//             LOG(Log::ERR, "Sdo") << wrapId(where) << " Segmented SDO read index=0x" << wrapValue(Utils::toHexString(index)) << 
//                 " subIndex=" << wrapValue(std::to_string(subIndex)) << " " << 
//                 ERROR << "Toggle bit mismatch." << ERROR_;
//             throw std::runtime_error("Toggle bit mismatch (detailed error was logged)");
//         }
//         /* insert useful data */
//         output.insert(std::end(output), &reply.c_data[1], &reply.c_data[1+dataInThisSegment]);

//         /* next segment ? */
//         nextSegmentToggle = !nextSegmentToggle;
//         segmentsReceived++;
//         if (segmentsReceived >= maxSegments)
//         {
//             LOG(Log::ERR, "Sdo") << "Segmented SDO read " << ERROR << "Too many segments" << ERROR_ << ", hit the configured limit of " << wrapValue(std::to_string(maxSegments));
//             return false;
//         }
//         /* is it the last segment? */
//         lastSegment = reply.c_data[0] & 0x01;
//     }
//     while (!lastSegment); // TODO should we have some protection in case this is really taking too long?

//     LOG(Log::TRC, "Sdo") << wrapId(where) << 
//         " <-- SDO segmented read index=" << wrapValue(Utils::toHexString(index)) << " subIndex=" << wrapValue(std::to_string(subIndex)) <<
//         " OK received " << wrapValue(Utils::toString(output.size())) << " octets " << " data=[" << wrapValue(bytesToHexString(output)) << "] ";

//     return true;
// }

void SdoEngine::extractExpeditedReadData(
    const CanMessage& reply,
    const std::string& where,
    uint16_t index,
    uint8_t subIndex,
    std::vector<unsigned char>& output
)
{
    bool dataSetSizeIndicated = reply.c_data[0] & 0x01;
    if (!dataSetSizeIndicated)
    {
        LOG(Log::ERR, "Sdo") << wrapId(where) << " <-- Data size set was not indicated - can't parse the SDO reply!";
        throw std::runtime_error("Data set size was not indicated (detailed error was logged)");
    }
    unsigned char sizeOfDataSet = 4 - ((reply.c_data[0] & 0x0c) >> 2);

    output.assign(sizeOfDataSet, 0);

    std::copy(
        reply.c_data + 4,
        reply.c_data + 4 + sizeOfDataSet,
        output.begin());
}

bool SdoEngine::readSegments(
    const std::string& where,
    uint16_t index,
    uint8_t subIndex,
    std::vector<unsigned char>& output,
    unsigned int timeoutMsPerPair, /* Each segment req-rep pair when using segmented SDO */
    unsigned int maxSegments) /* Max segments when segmented SDO */
{
    output.clear();
    output.reserve(2048);
    bool nextSegmentToggle = false;
    bool lastSegment;
    size_t segmentsReceived(0);
    do
    {
        CanMessage uploadDomainSegment = makeUploadDomainSegment(m_nodeId, index, subIndex, nextSegmentToggle);
        CanMessage reply = this->invokeTransactionAndThrowOnNoReply(uploadDomainSegment, where, "segmented SDO read", index, subIndex, timeoutMsPerPair);
        throwIfAbortDomainTransfer(reply, where);
        throwIfQuestionableSize(reply); 
        bool receivedToggleBit = reply.c_data[0] & 0x10;
        /* Refer to Henk's CANopen writeup for understanding what is n */
        uint8_t n = (reply.c_data[0] >> 1) & 0x07;
        uint8_t dataInThisSegment = 7-n;

        LOG(Log::TRC, "Sdo") << wrapId(where) << " Segmented SDO read index=0x" << wrapValue(Utils::toHexString(index)) << 
            " subIndex=" << wrapValue(std::to_string(subIndex)) << " received segment #" << wrapValue(Utils::toString(segmentsReceived)) << 
            " toggleBit " << receivedToggleBit << " size in this segment:" << wrapValue(Utils::toString((unsigned int)dataInThisSegment));

        /* check toggle bit? */
        if (receivedToggleBit != nextSegmentToggle)
        {
            LOG(Log::ERR, "Sdo") << wrapId(where) << " Segmented SDO read index=0x" << wrapValue(Utils::toHexString(index)) << 
                " subIndex=" << wrapValue(std::to_string(subIndex)) << " " << 
                ERROR << "Toggle bit mismatch." << ERROR_;
            throw std::runtime_error("Toggle bit mismatch (detailed error was logged)");
        }
        /* insert useful data */
        output.insert(std::end(output), &reply.c_data[1], &reply.c_data[1+dataInThisSegment]);

        /* next segment ? */
        nextSegmentToggle = !nextSegmentToggle;
        segmentsReceived++;
        if (segmentsReceived >= maxSegments)
        {
            LOG(Log::ERR, "Sdo") << "Segmented SDO read " << ERROR << "Too many segments" << ERROR_ << ", hit the configured limit of " << wrapValue(std::to_string(maxSegments));
            return false;
        }
        /* is it the last segment? */
        lastSegment = reply.c_data[0] & 0x01;
    }
    while (!lastSegment);

    LOG(Log::TRC, "Sdo") << wrapId(where) << 
        " <-- SDO segmented read index=" << wrapValue(Utils::toHexString(index)) << " subIndex=" << wrapValue(std::to_string(subIndex)) <<
        " OK received " << wrapValue(Utils::toString(output.size())) << " octets " << " data=[" << wrapValue(bytesToHexString(output)) << "] ";

    return true;    
}

bool SdoEngine::readExpeditedOrSegmented(
    const std::string& where,
    uint16_t index,
    uint8_t subIndex,
    std::vector<unsigned char>& output,
    unsigned int timeoutMsExpedited, /* Init req */
    unsigned int timeoutMsPerPair, /* Each segment req-rep pair when using segmented SDO */
    unsigned int maxSegments) /* Max segments when segmented SDO */
{
    LOG(Log::TRC, "Sdo") << wrapId(where) << 
        " --> SDO segm?/exp? read index=" << wrapValue(Utils::toHexString(index)) << " subIndex=" << wrapValue(std::to_string(subIndex)); 
    CanMessage initiateDomainUpload = CANopen::makeInitiateDomainUpload(m_nodeId, index, subIndex);
    CanMessage reply = this->invokeTransactionAndThrowOnNoReply(initiateDomainUpload, where, "segm?exp? SDO read", index, subIndex, timeoutMsExpedited);
    throwIfAbortDomainTransfer(reply, where);
    throwIfQuestionableSize(reply); 
    throwIfSdoObjectMismatch(initiateDomainUpload, reply, where); 
    bool deviceWantsExpeditedTransfer = reply.c_data[0] & 0x02;
    if (deviceWantsExpeditedTransfer)
    {
        extractExpeditedReadData(reply, where, index, subIndex, output);
        LOG(Log::TRC, "Sdo") << 
            wrapId(where) << " <-- SDO read index=0x" << wrapValue(Utils::toHexString(index)) << 
            " subIndex=" << wrapValue(std::to_string(subIndex)) << " exp.SDO, data(hex)=[" << wrapValue(bytesToHexString(output)) << "] SUCCESS";

        return true;    
    }
    /* So, we have segmented SDO for sure */
    return readSegments(where, index, subIndex, output, timeoutMsPerPair, maxSegments);
}

bool SdoEngine::writeExpedited (
    const std::string& where,
    uint16_t index, 
    uint8_t subIndex, 
    const std::vector<unsigned char>& data, unsigned int timeoutMs)
{
    LOG(Log::TRC, "Sdo") << wrapId(where) << 
        " --> SDO write index=" << wrapValue(Utils::toHexString(index)) << " subIndex=" << wrapValue(std::to_string(subIndex)) << 
        " data=[" << wrapValue(bytesToHexString(data)) << "] ";


    CanMessage initiateDomainDownload = CANopen::makeInitiateDomainDownload(m_nodeId, index, subIndex, data, where);
    CanMessage reply = this->invokeTransactionAndThrowOnNoReply(
        initiateDomainDownload, where, "expedited SDO write", index, subIndex, timeoutMs);
    throwIfAbortDomainTransfer(reply, where);
    throwIfQuestionableSize(reply);
    throwIfSdoObjectMismatch(initiateDomainDownload, reply, where);

    if (reply.c_data[0] != 0x60)
    {
        LOG(Log::ERR, "Sdo") << wrapId(where) << 
            " <-- SDO write index=" << std::hex << index << " subIndex=" << wrapValue(std::to_string(subIndex)) << std::dec << " \033[41;37m"
 << " SDO reply header is invalid, expected 0x60 got ["  << std::hex << (unsigned int)reply.c_data[0] << "]" << SPOOKY_;
        return false;
    } 

    LOG(Log::TRC, "Sdo") << wrapId(where) << " <-- SDO write index=" << std::hex << index << std::dec << " subIndex=" << wrapValue(std::to_string(subIndex)) <<  " OK";
    return true;

}

bool SdoEngine::writeSegmented (const std::string& where, uint16_t index, uint8_t subIndex, const std::vector<unsigned char>& data, unsigned int timeoutMsPerPair, unsigned int maxSegments)
{
    LOG(Log::TRC, "Sdo") << wrapId(where) << 
        " --> Segmented SDO write index=" << wrapValue(Utils::toHexString(index)) << " subIndex=" << wrapValue(std::to_string(subIndex)) << 
        " data=[" << wrapValue(bytesToHexString(data)) << "] ";
    if (data.size() < 1)
        throw_runtime_error_with_origin(where + " Empty data was given");

    if (!this->writeSegmentedInitialize(where, index, subIndex, data, timeoutMsPerPair))
    {
        LOG(Log::ERR, "Sdo") << wrapId(where) << 
            " <-- Segmented SDO write index=" << wrapValue(Utils::toHexString(index)) << " subIndex=" << wrapValue(std::to_string(subIndex)) << 
            " initialize of segment transfered failed";
        return false;
    }
    bool success = this->writeSegmentedStream(where, index, subIndex, data, timeoutMsPerPair, maxSegments);
    LOG(Log::INF, "Sdo") << wrapId(where) << 
            " <-- Segmented SDO write index=" << wrapValue(Utils::toHexString(index)) << " subIndex=" << wrapValue(std::to_string(subIndex)) << 
            " segments transfer success: " << success;
    return success;
}

bool SdoEngine::writeSegmentedInitialize (const std::string& where, uint16_t index, uint8_t subIndex, const std::vector<unsigned char>& data, unsigned int timeoutMsPerPair)
{
    CanMessage initiateDomainDownload = CANopen::makeInitiateDomainDownloadSegmented(m_nodeId, index, subIndex, data.size());
    
    CanMessage reply = this->invokeTransactionAndThrowOnNoReply(
        initiateDomainDownload, where, "segmented SDO write", index, subIndex, timeoutMsPerPair);
    throwIfAbortDomainTransfer(reply, where);
    throwIfQuestionableSize(reply);
    throwIfSdoObjectMismatch(initiateDomainDownload, reply, where);
    return true;
}

bool SdoEngine::writeSegmentedStream (const std::string& where, uint16_t index, uint8_t subIndex, const std::vector<unsigned char>& data, unsigned int timeoutMs, unsigned int maxSegments)
{
    size_t octetsTransferred = 0;
    unsigned int segmentNumber = 0;
    bool nextSegmentToggle = false;
    while (octetsTransferred < data.size())
    {
        size_t octetsLeft = data.size() - octetsTransferred;
        size_t octetsInThisSegment = std::min(octetsLeft, 7UL);
        bool lastSegment = octetsLeft <= 7;
        CanMessage downloadDomainSegment = CANopen::makeDownloadDomainSegment(
            m_nodeId, index, subIndex, 
            octetsInThisSegment, lastSegment, nextSegmentToggle, &data[octetsTransferred]);
        
        LOG(Log::TRC, "Sdo") << wrapId(where) << " segmented SDO: will send next segment #" <<
            wrapValue(std::to_string(segmentNumber)) << " w/ " << octetsInThisSegment << " octets, togglebit [" << nextSegmentToggle << "]";
        
        CanMessage reply = this->invokeTransactionAndThrowOnNoReply(
            downloadDomainSegment, where, "segmented SDO write", index, subIndex, timeoutMs);
        throwIfAbortDomainTransfer(reply, where);
        throwIfQuestionableSize(reply);
        if (reply.c_data[0] != (0x20 | (nextSegmentToggle? 0x10: 0x00)))
        {
            LOG(Log::ERR, "Sdo") << wrapId(where) << ERROR << "Wrong reply received" << ERROR_ << ": " << wrapValue(Utils::toHexString(reply.c_data[0])) << ", might be a toggle bit issue?";
            return false;
        }
        
        // prep for next segment, if any.
        octetsTransferred += octetsInThisSegment;
        nextSegmentToggle = !nextSegmentToggle;
        segmentNumber++;
        if (segmentNumber >= maxSegments)
        {
            LOG(Log::ERR, "Sdo") << "Segmented SDO write " << ERROR << "Too many segments" << ERROR_ << ", hit the configured limit of " << wrapValue(std::to_string(maxSegments));
            return false;
        }
    }
    return true;
}

void SdoEngine::replyCame (const CanMessage& msg)
{
    if (m_isInSpyMode)
    {
        LOG(Log::TRC, MyLogComponents::spy()) << wrapId(m_nodeFullName) << " seeing SDO reply (not parsing - bus in spy mode)";
        return;
    }
    LOG(Log::TRC, "Sdo") << wrapId(m_nodeFullName) << " Received SDO reply";
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
        LOG(Log::ERR, "Sdo") << wrapId(m_nodeFullName) << " Received unexpected SDO reply " << wrapValue(Common::CanMessageToString(msg));
    }

}

// Feature clause: FS0.3: Abort domain transfer support
void SdoEngine::handleAbortDomainTransfer (const std::string& where, const CanMessage& msg)
{
    /* AbortDomainTransfer message */
    /* We validated that the reply applies to the correct index/subindex above*/
    uint32_t reasonCode = Common::bytesAsTypeNativeAlignSafeCast<uint32_t>(msg.c_data, msg.c_dlc, /*offset*/4);
    LOG(Log::ERR, "Sdo") << wrapId(where) << " AbortDomainTransfer! Reason code is "
        << ERROR << explainAbortCode(reasonCode>>24, (reasonCode>>16)&0xff) << ERROR_ 
        << " (full abort code was " << wrapValue(Utils::toHexString(reasonCode)) << ")";
}

void SdoEngine::throwIfAbortDomainTransfer(const CanMessage& reply, const std::string& where)
{
    if (reply.c_data[0] == 0x80)
    {
        handleAbortDomainTransfer(where, reply);
        throw std::runtime_error("abort domain transfer (detailed error was logged)");
    }
}

void SdoEngine::throwIfQuestionableSize(const CanMessage& reply)
{
    if (reply.c_dlc != 8)
        throw std::runtime_error("SDO reply size is "+Utils::toString((unsigned int)reply.c_dlc)+" expected is 8");
}

void SdoEngine::throwIfSdoObjectMismatch(const CanMessage& request, const CanMessage& reply, const std::string& where)
{
    if (std::mismatch(
        request.c_data + 1,
        request.c_data + 4,
        reply.c_data + 1).first != request.c_data + 4)
    {
        LOG(Log::ERR, "Sdo") << wrapId(where) << 
            " <-- " << ERROR << "SDO request-reply object ID mismatch" << ERROR_; // TODO details?
        throw std::runtime_error("SDO request-reply object ID mismatch (detailed error was logged)");
    }
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
    if (m_isInSpyMode)
    {
        LOG(Log::TRC, MyLogComponents::spy()) << wrapId(m_nodeFullName) << " seeing SDO request (not parsing - bus in spy mode)";
    }
    else
        SPOOKY(m_nodeFullName) << " SDO request is seen " << SPOOKY_ << " ... hmm ... another host on the bus. It's BAD!";
}

}
