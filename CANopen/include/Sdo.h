#pragma once

#include <functional>
#include <mutex>
#include <condition_variable>

#include <Definitions.hpp>

namespace Device { class DNode; }

namespace CANopen
{

class CobidCoordinator;

class SdoEngine
{
    public:
        enum SdoResult { Success, Timeout, AbortedDomainTransfer, OtherFailure };

        SdoEngine (
            Device::DNode* myNode,
            MessageSendFunction messageSendFunction,
            CobidCoordinator& cobidCoordinator
            );

        bool readExpedited (uint16_t index, uint8_t subIndex, std::vector<unsigned char>& output, unsigned int timeoutMs=1000);
        SdoResult writeExpedited (uint16_t index, uint8_t subIndex, const std::vector<unsigned char>& data, unsigned int timeoutMs=1000);

        void replyCame (const CanMessage& msg);

    private:
        Device::DNode* m_node;
        MessageSendFunction m_sendFunction;
        std::mutex m_condVarChangeLock;
        std::condition_variable m_condVarForReply;
        bool m_replyCame;
        bool m_replyExpected;
        CanMessage m_lastSdoReply;

        //! This is just a helper for warning + spooky messages.
        void sdoRequestNotifier (const CanMessage& msg);

        void handleAbortDomainTransfer (const CanMessage& msg);

        std::string explainAbortCode (uint8_t errorClass, uint8_t errorCode);
};

}