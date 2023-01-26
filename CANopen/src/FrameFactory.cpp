#include <FrameFactory.hpp>

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

}