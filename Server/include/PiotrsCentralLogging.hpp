#include "mqtt/client.h"

#include <LogSinkInterface.h>
#include <WaitFreeStorage.h>


class PiotrsCentralLogging: public LogSinkInterface
{
public:
    PiotrsCentralLogging();
    virtual ~PiotrsCentralLogging();

    virtual bool initialize();
    virtual void logMessage(const std::string& msg);

    void dispatchThread();

private:

    mqtt::client* m_mqttClient;
    
    struct OneLogItem
    {
        std::string message;
    };

    WaitFreeStorage<OneLogItem> m_logStorage;

    std::thread m_dispatcher;
    bool m_dataLost;

    struct Configuration
    {
        std::string mqttBrokerHostName;
        std::string appInstanceName;
    };

    Configuration loadConfiguration (); //! throws if configuration can't be established

    Configuration m_configuration;
    
};

