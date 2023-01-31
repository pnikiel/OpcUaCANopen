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
        SdoEngine (
            uint8_t nodeId,
            MessageSendFunction messageSendFunction,
            CobidCoordinator& cobidCoordinator
            );

        void initialize();

        bool readExpedited (const std::string& where, uint16_t index, uint8_t subIndex, std::vector<unsigned char>& output, unsigned int timeoutMs=1000);
        bool readSegmented (const std::string& where, uint16_t index, uint8_t subIndex, std::vector<unsigned char>& output, unsigned int timeoutMsPerPair=1000);

        bool writeExpedited (const std::string& where, uint16_t index, uint8_t subIndex, const std::vector<unsigned char>& data, unsigned int timeoutMs=1000);
        bool writeSegmented (const std::string& where, uint16_t index, uint8_t subIndex, const std::vector<unsigned char>& data, unsigned int timeoutMsPerPair=1000);

        void replyCame (const CanMessage& msg);
        void setInSpyMode (bool spyMode) { m_isInSpyMode=spyMode; }
        void setNodeFullName (const std::string& name) { m_nodeFullName=name; }

    private:
        std::string m_nodeFullName;
        const uint8_t m_nodeId;
        Device::DNode* m_node;
        bool m_isInSpyMode;
        MessageSendFunction m_sendFunction;
        std::mutex m_condVarChangeLock;
        std::condition_variable m_condVarForReply;
        bool m_replyCame;
        bool m_replyExpected;
        CanMessage m_lastSdoReply;
        CobidCoordinator& m_cobidCoordinator;

        //! This is just a helper for warning + spooky messages.
        void sdoRequestNotifier (const CanMessage& msg);

        void handleAbortDomainTransfer (const std::string& where, const CanMessage& msg);

        std::string explainAbortCode (uint8_t errorClass, uint8_t errorCode);

        bool writeSegmentedInitialize (const std::string& where, uint16_t index, uint8_t subIndex, const std::vector<unsigned char>& data, unsigned int timeoutMs);
        bool writeSegmentedStream (const std::string& where, uint16_t index, uint8_t subIndex, const std::vector<unsigned char>& data, unsigned int timeoutMs);

        CanMessage invokeTransactionAndThrowOnNoReply(
            const CanMessage& request,
            const std::string& where,
            const std::string& what,
            uint16_t index,
            uint8_t subIndex,
            unsigned int timeoutMs);

        void throwIfAbortDomainTransfer(const CanMessage& reply, const std::string& where);
        void throwIfQuestionableSize(const CanMessage& reply);
        void throwIfSdoObjectMismatch(const CanMessage& request, const CanMessage& reply, const std::string& where);

};

}