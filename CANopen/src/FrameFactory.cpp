#include <stdexcept>

#include <FrameFactory.hpp>
#include <Utils.h>

namespace CANopen
{

CanMessage makeNodeGuardingRequest (uint8_t targetId)
{
    CanMessage ngRequestMessage;
    ngRequestMessage.c_id = 0x700 + targetId;
    ngRequestMessage.c_rtr = true;
    return ngRequestMessage;
}

CanMessage makeNodeManagementServiceRequest (uint8_t targetId, uint8_t commandSpecifier)
{
    CanMessage nmtServiceRequest;
    nmtServiceRequest.c_id = 0;
    nmtServiceRequest.c_data[0] = commandSpecifier;
    nmtServiceRequest.c_data[1] = targetId;
    nmtServiceRequest.c_dlc = 2;
    return nmtServiceRequest;
}

CanMessage makeSyncRequest ()
{
    CanMessage syncRequest;
    syncRequest.c_id = 0x80;
    syncRequest.c_dlc = 0;
    return syncRequest;
}

CanMessage makeTpdoRtr (uint8_t targetId, uint16_t cobid)
{
    CanMessage tpdoRtr;
    tpdoRtr.c_id = cobid + targetId;
    tpdoRtr.c_rtr = true;
    tpdoRtr.c_dlc = 0;
    return tpdoRtr;    
}

CanMessage makeInitiateDomainDownloadSegmented(uint8_t targetId, uint16_t index, uint8_t subIndex, uint32_t indicatedSize)
{
    CanMessage initiateDomainDownload;
    initiateDomainDownload.c_id = 0x600 + targetId;
    initiateDomainDownload.c_data[0] = 0x21; /* Normal (segmented) transfer, size is indicated in the data bytes */
    initiateDomainDownload.c_data[1] = index;
    initiateDomainDownload.c_data[2] = index >> 8;
    initiateDomainDownload.c_data[3] = subIndex;
    initiateDomainDownload.c_data[4] = indicatedSize & 0xff;
    initiateDomainDownload.c_data[5] = (indicatedSize >> 8) & 0xff;
    initiateDomainDownload.c_data[6] = (indicatedSize >> 16) & 0xff;
    initiateDomainDownload.c_data[7] = (indicatedSize >> 24) & 0xff;
    initiateDomainDownload.c_dlc = 8;
    return initiateDomainDownload;
}

CanMessage makeDownloadDomainSegment(uint8_t targetId, uint16_t index, uint8_t subIndex, uint8_t dataSize, bool isLastSegment, bool toggleBit, const uint8_t* data)
{
    if (dataSize < 1)
        throw std::logic_error("Can't have an empty segment");
    if (dataSize > 7) // TODO: is that correct?
        throw std::logic_error("Can't have more than 7 octets for one segment");

    CanMessage downloadDomainSegment;
    downloadDomainSegment.c_id = 0x600 + targetId;
    downloadDomainSegment.c_data[0] = 0x00 | (toggleBit? 0x10 : 0x00) | ((7-dataSize) << 1) | (isLastSegment? 0x01 : 0x00);
    std::copy(
        data,
        data + dataSize,
        &downloadDomainSegment.c_data[1]
    );
    downloadDomainSegment.c_dlc = 8;
    return downloadDomainSegment;

}

CanMessage makeInitiateDomainUpload(uint8_t targetId, uint16_t index, uint8_t subIndex)
{
    CanMessage initiateDomainUpload;
    initiateDomainUpload.c_id = 0x600 + targetId;
    initiateDomainUpload.c_data[0] = 0x40;
    initiateDomainUpload.c_data[1] = index;
    initiateDomainUpload.c_data[2] = index >> 8;
    initiateDomainUpload.c_data[3] = subIndex;
    initiateDomainUpload.c_dlc = 8;
    return initiateDomainUpload;
}

CanMessage makeInitiateDomainDownload(uint8_t targetId, uint16_t index, uint8_t subIndex, const std::vector<unsigned char>& data, const std::string& where)
{
    if (data.size() < 1)
        throw_runtime_error_with_origin(where + " Empty data was given");
    if (data.size() > 4)
        throw_runtime_error_with_origin(where + "Too much data [" + std::to_string(data.size()) + "] for expedited SDO");
    CanMessage initiateDomainDownload;
    initiateDomainDownload.c_id = 0x600 + targetId;
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

    return initiateDomainDownload;
}

CanMessage makeUploadDomainSegment(uint8_t targetId, uint16_t index, uint8_t subIndex, bool toggleBit)
{
    CanMessage uploadDomainSegment;
    uploadDomainSegment.c_id = 0x600 + targetId;
    uploadDomainSegment.c_data[0] = 0x60 | (toggleBit? 0x10 : 0x00);
    uploadDomainSegment.c_dlc = 8;
    return uploadDomainSegment;
}

}