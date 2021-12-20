#include <functional>
#include <mutex>
#include <condition_variable>

#include <CanMessage.h>


namespace CANopen
{

typedef std::function<void (const CanMessage& msg)> MessageSendFunction;

class SdoEngine
{
    public:
        SdoEngine (MessageSendFunction messageSendFunction, unsigned char nodeId);

        bool readExpedited (uint16_t index, uint16_t subIndex, std::vector<unsigned char>& output);
        void writeExpedited (uint16_t index, uint16_t subIndex);

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