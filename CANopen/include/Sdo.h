#pragma once

#include <functional>
#include <mutex>
#include <condition_variable>

#include <Definitions.hpp>

namespace CANopen
{

class SdoEngine
{
    public:
        SdoEngine (MessageSendFunction messageSendFunction, unsigned char nodeId);

        bool readExpedited (const std::string& where, uint16_t index, uint16_t subIndex, std::vector<unsigned char>& output, unsigned int timeoutMs=1000);
        bool writeExpedited (const std::string& where, uint16_t index, uint16_t subIndex, const std::vector<unsigned char>& data, unsigned int timeoutMs=1000);

        void replyCame (const CanMessage& msg);

    private:
        MessageSendFunction m_sendFunction;
        const unsigned char m_nodeId;
        std::mutex m_condVarChangeLock;
        std::condition_variable m_condVarForReply;
        bool m_replyCame;
        CanMessage m_lastSdoReply;
};

}